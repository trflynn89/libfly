#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

#include "fly/fly.h"
#include "fly/config/config.h"
#include "fly/logger/logger.h"
#include "fly/parser/parser.h"
#include "fly/task/task.h"
#include "fly/types/json.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);
FLY_CLASS_PTRS(ConfigUpdateTask);

FLY_CLASS_PTRS(PathMonitor);
FLY_CLASS_PTRS(TaskRunner);

/**
 * Class to create and manage a set of configurations.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class ConfigManager : public std::enable_shared_from_this<ConfigManager>
{
    friend class ConfigUpdateTask;

    /**
     * Map of configuration group names to configuration objects.
     */
    typedef std::map<std::string, ConfigWPtr> ConfigMap;

public:
    /**
     * Enumerated list of supported configuration file formats.
     */
    enum class ConfigFileType
    {
        Ini,
        Json,
    };

    /**
     * Constructor.
     *
     * @param ConfigFileType File format of the configuration file.
     * @param string Directory containing the configuration file.
     * @param string Name of the configuration file.
     */
    ConfigManager(
        const TaskRunnerPtr &,
        ConfigFileType,
        const std::string &,
        const std::string &
    );

    /**
     * Destructor. Stop the configuration manager and underlying objects.
     */
    ~ConfigManager();

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

    /**
     * Start the configuration manager and underlying objects.
     *
     * @return True if the monitor could be started.
     */
    bool Start();

private:
    /**
     * Parse the configuration file and store the parsed values in memory.
     */
    void updateConfig();

    PathMonitorPtr m_spMonitor;
    ParserPtr m_spParser;
    Json m_values;

    const std::string m_path;
    const std::string m_file;

    TaskRunnerPtr m_spTaskRunner;
    TaskPtr m_spTask;

    mutable std::mutex m_configsMutex;
    ConfigMap m_configs;
};

/**
 * Task to be executed when the configuration file changes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class ConfigUpdateTask : public Task
{
public:
    ConfigUpdateTask(const ConfigManagerWPtr &);

protected:
    /**
     * Call back into the config manager to re-parse the configuration file.
     */
    void Run() override;

private:
    ConfigManagerWPtr m_wpConfigManager;
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
            spConfig = std::dynamic_pointer_cast<T>(spBaseConfig);
        }
        else
        {
            spConfig = std::make_shared<T>();
            m_configs[name] = spConfig;
        }
    }

    if (spConfig)
    {
        spConfig->Update(m_values[name]);
    }
    else
    {
        LOGW(-1, "Could not create configuration for type %s", name);
    }

    return spConfig;
}

}
