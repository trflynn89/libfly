#pragma once

#include "fly/config/config.hpp"
#include "fly/logger/logger.hpp"
#include "fly/task/task.hpp"
#include "fly/types/json/json.hpp"

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
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 21, 2016
 */
class ConfigManager : public std::enable_shared_from_this<ConfigManager>
{
    friend class ConfigUpdateTask;

public:
    /**
     * Map of configuration group names to configuration objects.
     */
    using ConfigMap = std::map<const char *, std::weak_ptr<Config>>;

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
     * @param task_runner Task runner for posting config-related tasks onto.
     * @param file_type File format of the configuration file.
     * @param path Path to the configuration file.
     */
    ConfigManager(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        ConfigFileType file_type,
        const std::filesystem::path &path) noexcept;

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
    std::shared_ptr<T> create_config() noexcept;

    /**
     * Get the number of configuration objects currently created. Erases any
     * expired configuration objects.
     *
     * @return The number of configurations.
     */
    ConfigMap::size_type get_size() noexcept;

    /**
     * Start the configuration manager and underlying objects.
     *
     * @return True if the monitor could be started.
     */
    bool start() noexcept;

private:
    /**
     * Parse the configuration file and store the parsed values in memory.
     */
    void update_config() noexcept;

    std::shared_ptr<PathMonitor> m_monitor;
    std::unique_ptr<Parser> m_parser;
    Json m_values;

    const std::filesystem::path m_path;

    std::shared_ptr<SequencedTaskRunner> m_task_runner;
    std::shared_ptr<Task> m_task;

    mutable std::mutex m_configs_mutex;
    ConfigMap m_configs;
};

/**
 * Task to be executed when the configuration file changes.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class ConfigUpdateTask : public Task
{
public:
    explicit ConfigUpdateTask(std::weak_ptr<ConfigManager>) noexcept;

protected:
    /**
     * Call back into the config manager to re-parse the configuration file.
     */
    void Run() noexcept override;

private:
    std::weak_ptr<ConfigManager> m_weak_config_manager;
};

//==============================================================================
template <typename T>
std::shared_ptr<T> ConfigManager::create_config() noexcept
{
    static_assert(std::is_base_of_v<Config, T>);

    std::shared_ptr<T> config;

    std::lock_guard<std::mutex> lock(m_configs_mutex);
    ConfigMap::const_iterator it = m_configs.find(T::identifier);

    if (it == m_configs.end())
    {
        config = std::make_shared<T>();
        m_configs[T::identifier] = config;
    }
    else
    {
        std::shared_ptr<Config> base_config = it->second.lock();

        if (base_config)
        {
            config = std::dynamic_pointer_cast<T>(base_config);
        }
        else
        {
            config = std::make_shared<T>();
            m_configs[T::identifier] = config;
        }
    }

    if (config)
    {
        config->update(m_values[T::identifier]);
    }
    else
    {
        LOGW("Could not create configuration for type %s", T::identifier);
    }

    return config;
}

} // namespace fly
