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
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    ConfigFileType fileType,
    const std::filesystem::path &path) noexcept :
    m_path(path),
    m_spTaskRunner(spTaskRunner)
{
    switch (fileType)
    {
        case ConfigFileType::Ini:
            m_spParser = std::make_shared<IniParser>();
            break;

        case ConfigFileType::Json:
            m_spParser = std::make_shared<JsonParser>();
            break;

        default:
            LOGE(
                "Unrecognized configuration type: %d",
                static_cast<int>(fileType));
            break;
    }
}

//==============================================================================
ConfigManager::~ConfigManager()
{
    if (m_spMonitor)
    {
        m_spMonitor->RemoveFile(m_path);
    }
}

//==============================================================================
ConfigManager::ConfigMap::size_type ConfigManager::GetSize() noexcept
{
    std::lock_guard<std::mutex> lock(m_configsMutex);

    for (auto it = m_configs.begin(); it != m_configs.end();)
    {
        std::shared_ptr<Config> spConfig = it->second.lock();

        if (spConfig)
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
bool ConfigManager::Start() noexcept
{
    if (m_spParser)
    {
        m_spMonitor = std::make_shared<PathMonitorImpl>(
            m_spTaskRunner,
            CreateConfig<PathConfig>());

        if (m_spMonitor->Start())
        {
            std::weak_ptr<ConfigManager> wpConfigManager = shared_from_this();

            m_spTask = std::make_shared<ConfigUpdateTask>(wpConfigManager);
            std::weak_ptr<Task> wpTask = m_spTask;

            // Formatter badly handles hanging indent in lambda parameters
            // clang-format off
            auto callback = [wpConfigManager, wpTask](
                const std::filesystem::path &,
                PathMonitor::PathEvent)
            {
                auto spConfigManager = wpConfigManager.lock();
                auto spTask = wpTask.lock();

                if (spConfigManager && spTask)
                {
                    spConfigManager->m_spTaskRunner->PostTask(spTask);
                }
            };
            // clang-format on

            return m_spMonitor->AddFile(m_path, callback);
        }
    }

    return false;
}

//==============================================================================
void ConfigManager::updateConfig() noexcept
{
    std::lock_guard<std::mutex> lock(m_configsMutex);

    try
    {
        m_values = m_spParser->ParseFile(m_path);
    }
    catch (const ParserException &)
    {
        LOGW("Could not parse file, ignoring update");
        m_values = nullptr;
    }

    if (m_values.IsObject() || m_values.IsNull())
    {
        for (auto it = m_configs.begin(); it != m_configs.end();)
        {
            std::shared_ptr<Config> spConfig = it->second.lock();

            if (spConfig)
            {
                spConfig->Update(m_values[it->first]);
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
    std::weak_ptr<ConfigManager> wpConfigManager) noexcept :
    Task(),
    m_wpConfigManager(wpConfigManager)
{
}

//==============================================================================
void ConfigUpdateTask::Run() noexcept
{
    std::shared_ptr<ConfigManager> spConfigManager = m_wpConfigManager.lock();

    if (spConfigManager)
    {
        spConfigManager->updateConfig();
    }
}

} // namespace fly
