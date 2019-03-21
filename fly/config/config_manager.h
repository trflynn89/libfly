#pragma once

#include "fly/config/config.h"
#include "fly/logger/logger.h"
#include "fly/task/task.h"
#include "fly/types/json.h"

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

namespace fly {

class ConfigUpdateTask;
class Parser;
class PathMonitor;
class SequencedTaskRunner;

/**
 * Class to create and manage a set of configurations.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class ConfigManager : public std::enable_shared_from_this<ConfigManager>
{
    friend class ConfigUpdateTask;

public:
    /**
     * Map of configuration group names to configuration objects.
     */
    typedef std::map<const char *, std::weak_ptr<Config>> ConfigMap;

    /**
     * Enumerated list of supported configuration file formats.
     */
    enum class ConfigFileType : std::uint8_t
    {
        Ini,
        Json,
    };

    /**
     * Constructor.
     *
     * @param TaskRunner Task runner for posting config-related tasks onto.
     * @param ConfigFileType File format of the configuration file.
     * @param path Path to the configuration file.
     */
    ConfigManager(
        const std::shared_ptr<SequencedTaskRunner> &,
        ConfigFileType,
        const std::filesystem::path &) noexcept;

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
    std::shared_ptr<T> CreateConfig() noexcept;

    /**
     * Get the number of configuration objects currently created. Erases any
     * expired configuration objects.
     *
     * @return The number of configurations.
     */
    ConfigMap::size_type GetSize() noexcept;

    /**
     * Start the configuration manager and underlying objects.
     *
     * @return True if the monitor could be started.
     */
    bool Start() noexcept;

private:
    /**
     * Parse the configuration file and store the parsed values in memory.
     */
    void updateConfig() noexcept;

    std::shared_ptr<PathMonitor> m_spMonitor;
    std::shared_ptr<Parser> m_spParser;
    Json m_values;

    const std::filesystem::path m_path;

    std::shared_ptr<SequencedTaskRunner> m_spTaskRunner;
    std::shared_ptr<Task> m_spTask;

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
    ConfigUpdateTask(std::weak_ptr<ConfigManager>) noexcept;

protected:
    /**
     * Call back into the config manager to re-parse the configuration file.
     */
    void Run() noexcept override;

private:
    std::weak_ptr<ConfigManager> m_wpConfigManager;
};

//==============================================================================
template <typename T>
std::shared_ptr<T> ConfigManager::CreateConfig() noexcept
{
    static_assert(std::is_base_of_v<Config, T>);

    std::shared_ptr<T> spConfig;

    std::lock_guard<std::mutex> lock(m_configsMutex);
    ConfigMap::const_iterator it = m_configs.find(T::identifier);

    if (it == m_configs.end())
    {
        spConfig = std::make_shared<T>();
        m_configs[T::identifier] = spConfig;
    }
    else
    {
        std::shared_ptr<Config> spBaseConfig = it->second.lock();

        if (spBaseConfig)
        {
            spConfig = std::dynamic_pointer_cast<T>(spBaseConfig);
        }
        else
        {
            spConfig = std::make_shared<T>();
            m_configs[T::identifier] = spConfig;
        }
    }

    if (spConfig)
    {
        spConfig->Update(m_values[T::identifier]);
    }
    else
    {
        LOGW("Could not create configuration for type %s", T::identifier);
    }

    return spConfig;
}

} // namespace fly
