#include "fly/config/config_manager.hpp"

#include "fly/parser/ini_parser.hpp"
#include "fly/parser/json_parser.hpp"
#include "fly/path/path_config.hpp"
#include "fly/path/path_monitor.hpp"
#include "fly/task/task_runner.hpp"

#include <filesystem>
#include <functional>

namespace fly {

//==================================================================================================
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
            LOGE("Unrecognized configuration type: %d", static_cast<int>(file_type));
            break;
    }
}

//==================================================================================================
ConfigManager::~ConfigManager()
{
    if (m_monitor)
    {
        m_monitor->remove_file(m_path);
    }
}

//==================================================================================================
bool ConfigManager::start()
{
    if (m_parser)
    {
        m_monitor = std::make_shared<PathMonitorImpl>(m_task_runner, create_config<PathConfig>());

        if (m_monitor->start())
        {
            std::weak_ptr<ConfigManager> weak_self = shared_from_this();

            auto callback = [weak_self](const std::filesystem::path &, PathMonitor::PathEvent) {
                if (auto self = weak_self.lock(); self)
                {
                    auto task = [](std::shared_ptr<ConfigManager> nested_self) {
                        nested_self->update_config();
                    };

                    self->m_task_runner->post_task(
                        FROM_HERE,
                        std::move(task),
                        std::move(weak_self));
                }
            };

            return m_monitor->add_file(m_path, callback);
        }
    }

    return false;
}

//==================================================================================================
ConfigManager::ConfigMap::size_type ConfigManager::prune()
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

//==================================================================================================
void ConfigManager::update_config()
{
    std::lock_guard<std::mutex> lock(m_configs_mutex);

    if (auto values = m_parser->parse_file(m_path); values)
    {
        m_values = std::move(values.value());
    }
    else
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

} // namespace fly
