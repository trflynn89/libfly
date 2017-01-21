#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

#include <fly/fly.h>
#include <fly/config/config.h>
#include <fly/file/file_monitor_impl.h>
#include <fly/file/parser.h>
#include <fly/task/runner.h>

namespace fly {

DEFINE_CLASS_PTRS(ConfigManager);

/**
 * Class to create and manage a set of configurations.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class ConfigManager : public Runner
{
    /**
     * Map of configuration group names to configuration objects.
     */
    typedef std::map<std::string, ConfigWPtr> ConfigMap;

public:
    /**
     * Enumerated list of supported configuration file formats.
     */
    enum ConfigFileType
    {
        CONFIG_TYPE_INI
    };

    /**
     * Constructor.
     *
     * @param ConfigFileType File format of the configuration file.
     * @param string Directory containing the configuration file.
     * @param string Name of the configuration file.
     */
    ConfigManager(
        ConfigFileType,
        const std::string &,
        const std::string &
    );

    /**
     * Destructor. Stop the configuration manager.
     */
    virtual ~ConfigManager();

    /**
     * Create a configuration object, or if one with the given type's name
     * exists, fetch it.
     *
     * @tparam T Config type (must derive from or be fly::Config).
     *
     * @return A reference to the created/found configuration.
     */
    template <typename T>
    std::shared_ptr<T> CreateConfig();

    /**
     * Get the number of configuration objects currently created. Erases any
     * expired configuration objects.
     *
     * @return The number of configurations.
     */
    ConfigMap::size_type GetSize();

protected:
    /**
     * Start the configuration manager and underlying objects.
     *
     * @return True if the monitor could be started.
     */
    virtual bool StartRunner();

    /**
     * Stop the configuration manager and underlying objects.
     */
    virtual void StopRunner();

    /**
     * @return False - no workers are used, thus this should not be called.
     */
    bool DoWork();

private:
    FileMonitorPtr m_spMonitor;
    ParserPtr m_spParser;

    const std::string m_path;
    const std::string m_file;

    std::atomic_bool m_aFileChanged;

    mutable std::mutex m_configsMutex;
    ConfigMap m_configs;
};

//==============================================================================
template <typename T>
std::shared_ptr<T> ConfigManager::CreateConfig()
{
    static_assert(std::is_base_of<Config, T>::value,
        "Given type is not a configuration type");

    std::shared_ptr<T> spConfig;
    std::string name(T::GetName());

    std::lock_guard<std::mutex> lock(m_configsMutex);
    ConfigMap::const_iterator it = m_configs.find(name);

    if (it == m_configs.end())
    {
        spConfig = std::make_shared<T>();
        m_configs[name] = spConfig;
    }
    else
    {
        ConfigPtr spBaseConfig = it->second.lock();

        if (spBaseConfig)
        {
            spConfig = std::static_pointer_cast<T>(spBaseConfig);
        }
        else
        {
            spConfig = std::make_shared<T>();
            m_configs[name] = spConfig;
        }
    }

    spConfig->Update(m_spParser->GetValues(name));
    return spConfig;
}

}
