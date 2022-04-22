#pragma once

#include "fly/concepts/concepts.hpp"
#include "fly/config/config.hpp"
#include "fly/logger/logger.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/string/formatters.hpp"

#include <cstdint>
#include <filesystem>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <type_traits>

namespace fly::parser {
class Parser;
} // namespace fly::parser

namespace fly::path {
class PathMonitor;
} // namespace fly::path

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly::config {

/**
 * Enumerated list of supported configuration file formats.
 */
enum class ConfigFileType : std::uint8_t
{
    Ini,
    Json,
};

/**
 * Class to create and manage a set of configurations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 21, 2016
 */
class ConfigManager : public std::enable_shared_from_this<ConfigManager>
{
public:
    /**
     * Map of configuration group names to configuration objects.
     */
    using ConfigMap = std::map<std::string, std::weak_ptr<Config>>;

    /**
     * Create and start a configuration manager.
     *
     * @param task_runner Task runner for posting config-related tasks onto.
     * @param file_type File format of the configuration file.
     * @param path Path to the configuration file.
     *
     * @return The created configuration manager.
     */
    static std::shared_ptr<ConfigManager> create(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        ConfigFileType file_type,
        std::filesystem::path path);

    /**
     * Destructor. Stop the configuration manager and underlying objects.
     */
    ~ConfigManager();

    /**
     * Create a configuration object, or if one with the given type's name exists, fetch it.
     *
     * @tparam T Config type (must derive from or be fly::config::Config).
     *
     * @return A reference to the created/found configuration.
     */
    template <fly::DerivedFrom<Config> T>
    std::shared_ptr<T> create_config();

    /**
     * Erase any expired configuration objects.
     *
     * @return The remaining number of configurations.
     */
    ConfigMap::size_type prune();

private:
    /**
     * Constructor.
     *
     * @param task_runner Task runner for posting config-related tasks onto.
     * @param file_type File format of the configuration file.
     * @param path Path to the configuration file.
     */
    ConfigManager(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        ConfigFileType file_type,
        std::filesystem::path path) noexcept;

    /**
     * Start the configuration manager and underlying objects.
     *
     * @return True if the manager could be started.
     */
    bool start();

    /**
     * Parse the configuration file and store the parsed values in memory.
     */
    void update_config();

    std::shared_ptr<fly::task::SequencedTaskRunner> m_task_runner;

    std::shared_ptr<fly::path::PathMonitor> m_monitor;
    std::unique_ptr<fly::parser::Parser> m_parser;
    fly::Json m_values;

    std::filesystem::path const m_path;

    mutable std::mutex m_configs_mutex;
    ConfigMap m_configs;
};

//==================================================================================================
template <fly::DerivedFrom<Config> T>
std::shared_ptr<T> ConfigManager::create_config()
{
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
        LOGW("Could not create configuration for type {}", T::identifier);
    }

    return config;
}

} // namespace fly::config

//==================================================================================================
template <>
struct fly::Formatter<fly::config::ConfigFileType> : public fly::Formatter<std::uint8_t>
{
    /**
     * Format a configuration file type.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param file_type The configuration file type to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(fly::config::ConfigFileType file_type, FormatContext &context)
    {
        fly::Formatter<std::uint8_t>::format(static_cast<std::uint8_t>(file_type), context);
    }
};
