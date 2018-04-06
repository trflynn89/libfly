#include "fly/config/config_manager.h"

#include <chrono>
#include <functional>
#include <memory>

#include "fly/parser/ini_parser.h"
#include "fly/path/path_monitor.h"

namespace fly {

namespace
{
    // TODO make configurable
    static const std::chrono::milliseconds s_delay(5000);
}

//==============================================================================
ConfigManager::ConfigManager(
    ConfigFileType fileType,
    const std::string &path,
    const std::string &file
) :
    Runner("ConfigManager", 1),
    m_path(path),
    m_file(file),
    m_aFileChanged(true)
{
    switch (fileType)
    {
    case CONFIG_TYPE_INI:
        m_spParser = std::make_shared<IniParser>(path, file);
        break;

    default:
        LOGE(-1, "Unrecognized configuration type: %d", fileType);
        break;
    }
}

//==============================================================================
ConfigManager::~ConfigManager()
{
    Stop();
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
bool ConfigManager::StartRunner()
{
    if (m_spParser)
    {
        ConfigManagerPtr spThis = SharedFromThis<ConfigManager>();

        if (spThis)
        {
            m_spMonitor = std::make_shared<PathMonitorImpl>(spThis);
        }

        if (m_spMonitor && m_spMonitor->Start())
        {
            auto callback = [&aFileChanged = spThis->m_aFileChanged](...)
            {
                aFileChanged.store(true);
            };

            return m_spMonitor->AddFile(m_path, m_file, callback);
        }
    }

    return false;
}

//==============================================================================
void ConfigManager::StopRunner()
{
    if (m_spMonitor)
    {
        m_spMonitor->Stop();
    }
}

//==============================================================================
bool ConfigManager::DoWork()
{
    static bool expected = true;

    if (m_aFileChanged.compare_exchange_strong(expected, false))
    {
        try
        {
            m_spParser->Parse();

            std::lock_guard<std::mutex> lock(m_configsMutex);

            for (auto it = m_configs.begin(); it != m_configs.end(); )
            {
                ConfigPtr spConfig = it->second.lock();

                if (spConfig)
                {
                    Parser::ValueList values = m_spParser->GetValues(it->first);
                    spConfig->Update(values);

                    ++it;
                }
                else
                {
                    it = m_configs.erase(it);
                }
            }
        }
        catch (const ParserException &)
        {
            LOGW(-1, "Could not parse file, ignoring update");
        }
    }

    std::this_thread::sleep_for(s_delay);
    return true;
}

}
