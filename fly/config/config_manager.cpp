#include "fly/config/config_manager.hpp"

#include "fly/parser/exceptions.hpp"
#include "fly/parser/ini_parser.hpp"
#include "fly/parser/json_parser.hpp"
#include "fly/path/path_config.hpp"
#include "fly/path/path_monitor.hpp"
#include "fly/task/task_runner.hpp"

#include <filesystem>
#include <functional>

namespace fly {

//==============================================================================
ConfigManager::ConfigManager(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    ConfigFileType file_type,
    const std::filesystem::path &path) noexcept :
    m_path(path),
    m_task_runner(task_runner)
{
    switch (file_type)
    {
        case ConfigFileType::Ini:
            m_parser = std::make_unique<IniParser>();
            break;

        case ConfigFileType::Json:
            m_parser = std::make_unique<JsonParser>();
            break;

        default:
            LOGE(
                "Unrecognized configuration type: %d",
                static_cast<int>(file_type));
            break;
    }
}

//==============================================================================
ConfigManager::~ConfigManager()
{
    if (m_monitor)
    {
        m_monitor->RemoveFile(m_path);
    }
}

//==============================================================================
ConfigManager::ConfigMap::size_type ConfigManager::get_size() noexcept
{
    std::lock_guard<std::mutex> lock(m_configs_mutex);

    for (auto it = m_configs.begin(); it != m_configs.end();)
    {
        std::shared_ptr<Config> config = it->second.lock();

        if (config)
        {
            ++it;
        }
        else
        {
            it = m_configs.erase(it);
        }
    }

    return m_configs.size();
}

//==============================================================================
bool ConfigManager::start() noexcept
{
    if (m_parser)
    {
        m_monitor = std::make_shared<PathMonitorImpl>(
            m_task_runner,
            create_config<PathConfig>());

        if (m_monitor->Start())
        {
            std::weak_ptr<ConfigManager> weak_config_manager =
                shared_from_this();

            m_task = std::make_shared<ConfigUpdateTask>(weak_config_manager);
            std::weak_ptr<Task> weak_task = m_task;

            // Formatter badly handles hanging indent in lambda parameters
            // clang-format off
            auto callback = [weak_config_manager, weak_task](
                const std::filesystem::path &,
                PathMonitor::PathEvent)
            {
                auto config_manager = weak_config_manager.lock();
                auto task = weak_task.lock();

                if (config_manager && task)
                {
                    config_manager->m_task_runner->PostTask(task);
                }
            };
            // clang-format on

            return m_monitor->AddFile(m_path, callback);
        }
    }

    return false;
}

//==============================================================================
void ConfigManager::update_config() noexcept
{
    std::lock_guard<std::mutex> lock(m_configs_mutex);

    try
    {
        m_values = m_parser->parse_file(m_path);
    }
    catch (const ParserException &)
    {
        LOGW("Could not parse file, ignoring update");
        m_values = nullptr;
    }

    if (m_values.is_object() || m_values.is_null())
    {
        for (auto it = m_configs.begin(); it != m_configs.end();)
        {
            std::shared_ptr<Config> config = it->second.lock();

            if (config)
            {
                config->update(m_values[it->first]);
                ++it;
            }
            else
            {
                it = m_configs.erase(it);
            }
        }
    }
    else
    {
        LOGW("Parsed non key-value pairs file, ignoring update");
        m_values = nullptr;
    }
}

//==============================================================================
ConfigUpdateTask::ConfigUpdateTask(
    std::weak_ptr<ConfigManager> weak_config_manager) noexcept :
    Task(),
    m_weak_config_manager(weak_config_manager)
{
}

//==============================================================================
void ConfigUpdateTask::Run() noexcept
{
    std::shared_ptr<ConfigManager> config_manager =
        m_weak_config_manager.lock();

    if (config_manager)
    {
        config_manager->update_config();
    }
}

} // namespace fly
