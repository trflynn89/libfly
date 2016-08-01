#include "config_manager.h"

#include <functional>
#include <memory>

#include <fly/file/ini_parser.h>
#include <fly/logging/logger.h>

namespace fly {

//==============================================================================
ConfigManager::ConfigManager(
    ConfigFileType fileType,
    const std::string &path,
    const std::string &file
) :
    Runner("ConfigManager", 0),
    m_path(path),
    m_file(file)
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

        static const auto onChange = &ConfigManager::onConfigChange;
        auto callback = std::bind(onChange, spThis, std::placeholders::_1);

        m_spMonitor = std::make_shared<FileMonitorImpl>(callback, m_path, m_file);

        if (m_spMonitor->Start())
        {
            onConfigChange(FileMonitor::FILE_NO_CHANGE);
        }
    }

    return (m_spMonitor && m_spMonitor->IsValid());
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
    return false;
}

//==============================================================================
void ConfigManager::onConfigChange(FileMonitor::FileEvent)
{
    try
    {
        m_spParser->Parse();
    }
    catch (const ParserException &)
    {
        LOGW(-1, "Could not parse file, ignoring update");
        return;
    }

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

}
