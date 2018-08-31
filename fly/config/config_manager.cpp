#include "fly/config/config_manager.h"

#include <chrono>
#include <functional>
#include <memory>

#include "fly/parser/exceptions.h"
#include "fly/parser/ini_parser.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path_config.h"
#include "fly/path/path_monitor.h"
#include "fly/task/task_runner.h"

namespace fly {

//==============================================================================
ConfigManager::ConfigManager(
    const TaskRunnerPtr &spTaskRunner,
    ConfigFileType fileType,
    const std::string &path,
    const std::string &file
) :
    m_path(path),
    m_file(file),
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
        LOGE(-1, "Unrecognized configuration type: %d",
            static_cast<int>(fileType));
        break;
    }
}

//==============================================================================
ConfigManager::~ConfigManager()
{
    if (m_spMonitor)
    {
        m_spMonitor->RemoveFile(m_path, m_file);
    }
}

//==============================================================================
ConfigManager::ConfigMap::size_type ConfigManager::GetSize()
{
    std::lock_guard<std::mutex> lock(m_configsMutex);

    for (auto it = m_configs.begin(); it != m_configs.end(); )
    {
        ConfigPtr spConfig = it->second.lock();

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
bool ConfigManager::Start()
{
    if (m_spParser)
    {
        m_spMonitor = std::make_shared<PathMonitorImpl>(
            m_spTaskRunner,
            CreateConfig<PathConfig>()
        );

        if (m_spMonitor->Start())
        {
            ConfigManagerWPtr wpConfigManager = shared_from_this();

            m_spTask = std::make_shared<ConfigUpdateTask>(wpConfigManager);
            TaskWPtr wpTask = m_spTask;

            auto callback = [wpConfigManager, wpTask](...)
            {
                ConfigManagerPtr spConfigManager = wpConfigManager.lock();
                TaskPtr spTask = wpTask.lock();

                if (spConfigManager && spTask)
                {
                    spConfigManager->m_spTaskRunner->PostTask(spTask);
                }
            };

            return m_spMonitor->AddFile(m_path, m_file, callback);
        }
    }

    return false;
}

//==============================================================================
void ConfigManager::updateConfig()
{
    std::lock_guard<std::mutex> lock(m_configsMutex);

    try
    {
        m_values = m_spParser->Parse(m_path, m_file);
    }
    catch (const ParserException &)
    {
        LOGW(-1, "Could not parse file, ignoring update");
        m_values = nullptr;
    }

    if (m_values.IsObject() || m_values.IsNull())
    {
        for (auto it = m_configs.begin(); it != m_configs.end(); )
        {
            ConfigPtr spConfig = it->second.lock();

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
        LOGW(-1, "Parsed non key-value pairs file, ignoring update");
        m_values = nullptr;
    }
}

//==============================================================================
ConfigUpdateTask::ConfigUpdateTask(const ConfigManagerWPtr &wpConfigManager) :
    Task(),
    m_wpConfigManager(wpConfigManager)
{
}

//==============================================================================
void ConfigUpdateTask::Run()
{
    ConfigManagerPtr spConfigManager = m_wpConfigManager.lock();

    if (spConfigManager)
    {
        spConfigManager->updateConfig();
    }
}

}
