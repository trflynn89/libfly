#pragma once

#include "fly/types/string/formatters.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly::path {

class PathConfig;
class PathMonitorTask;

/**
 * Enumerated list of path events.
 */
enum class PathEvent : std::uint8_t
{
    None,
    Created,
    Deleted,
    Changed
};

/**
 * Virtual interface to monitor a local path. Provides monitoring of either all files or
 * user-specified files under a path for addition, deletion, or change. This interface is platform
 * independent - OS dependent implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 14, 2017
 */
class PathMonitor : public std::enable_shared_from_this<PathMonitor>
{
public:
    /**
     * Callback definition for function to be triggered on a path change.
     */
    using PathEventCallback = std::function<void(std::filesystem::path, PathEvent)>;

    /**
     * Create and start a path monitor.
     *
     * @param task_runner Task runner for posting path-related tasks onto.
     * @param config Reference to path configuration.
     *
     * @return The created path monitor.
     */
    static std::shared_ptr<PathMonitor> create(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<PathConfig> config);

    /**
     * Destructor. Remove all paths from the path monitor.
     */
    virtual ~PathMonitor();

    /**
     * Monitor for changes to all files under a directory. Callbacks registered with AddFile take
     * precendence over callbacks registered with AddPath.
     *
     * @param path Path to the directory to start monitoring.
     * @param callback Callback to trigger when a file changes.
     *
     * @return True if the directory could be added.
     */
    bool add_path(std::filesystem::path const &path, PathEventCallback callback);

    /**
     * Stop monitoring for changes to all files under a directory.
     *
     * @param path Path to the directory to stop monitoring.
     *
     * @return True if the directory was removed.
     */
    bool remove_path(std::filesystem::path const &path);

    /**
     * Stop monitoring all paths.
     */
    void remove_all_paths();

    /**
     * Monitor for changes to a single file. Callbacks registered with AddFile take precendence over
     * callbacks registered with AddPath.
     *
     * @param file Path to the file to start monitoring.
     * @param callback Callback to trigger when the file changes.
     *
     * @return True if the file could be added.
     */
    bool add_file(std::filesystem::path const &file, PathEventCallback callback);

    /**
     * Stop monitoring for changes to a single file. If there are no more files monitored in the
     * file's directory, and there is no callback registered for that directory, the directory
     * itself is removed from the monitor.
     *
     * @param file Path to the file to stop monitoring.
     *
     * @return True if the file was removed.
     */
    bool remove_file(std::filesystem::path const &file);

protected:
    /**
     * Struct to store information about a monitored path. OS dependent implementations of
     * PathMonitor should also have a concrete defintion of this struct.
     */
    struct PathInfo
    {
        /**
         * Destructor.
         */
        virtual ~PathInfo() = default;

        /**
         * Check if the monitored path is in a good state.
         *
         * @return True if the monitored path is healthy.
         */
        virtual bool is_valid() const = 0;

        PathEventCallback m_path_handler;
        std::map<std::filesystem::path, PathEventCallback> m_file_handlers;
    };

    /**
     * Map of monitored paths to their path information.
     */
    using PathInfoMap = std::map<std::filesystem::path, std::unique_ptr<PathInfo>>;

    /**
     * Constructor.
     *
     * @param task_runner Task runner for posting path-related tasks onto.
     * @param config Reference to path configuration.
     */
    PathMonitor(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<PathConfig> config) noexcept;

    /**
     * Create an instance of the OS dependent PathInfo struct.
     *
     * @param path The path to be monitored.
     *
     * @return Up-casted pointer to the PathInfo struct.
     */
    virtual std::unique_ptr<PathInfo> create_path_info(std::filesystem::path const &path) const = 0;

    /**
     * Check if the path monitor implementation is valid.
     *
     * @return True if the implementation is valid.
     */
    virtual bool is_valid() const = 0;

    /**
     * Check the path monitor implementation for any changes to the monitored paths.
     *
     * @param timeout Max time allow for an event to be occur.
     */
    virtual void poll(std::chrono::milliseconds timeout) = 0;

    mutable std::mutex m_mutex;
    PathInfoMap m_path_info;

private:
    /**
     * Queue a task to poll monitored paths.
     *
     * @return True if the path monitor is in a valid state.
     */
    bool start();

    /**
     * Search for a path to be monitored in the PathInfo map. If the map does not contain the path,
     * create an entry.
     *
     * @param path The path to be monitored.
     *
     * @return Shared pointer to the PathInfo struct.
     */
    PathInfo *get_or_create_path_info(std::filesystem::path const &path);

    /**
     * Queue a task to poll monitored paths. When the task is completed, it re-arms itself (if the
     * path monitor is still in a valid state).
     *
     * @return True if task was able to be queued.
     */
    bool poll_paths_later();

    std::shared_ptr<fly::task::SequencedTaskRunner> m_task_runner;
    std::shared_ptr<PathConfig> m_config;
};

} // namespace fly::path

//==================================================================================================
template <>
struct fly::string::Formatter<fly::path::PathEvent>
{
    /**
     * Format a path event.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param event The path event to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(fly::path::PathEvent event, FormatContext &context)
    {
        auto append = [&context](std::string_view value) {
            for (auto ch : value)
            {
                context.out()++ = ch;
            }
        };

        switch (event)
        {
            case fly::path::PathEvent::None:
                append("None");
                break;

            case fly::path::PathEvent::Created:
                append("Created");
                break;

            case fly::path::PathEvent::Deleted:
                append("Deleted");
                break;

            case fly::path::PathEvent::Changed:
                append("Changed");
                break;
        }
    }
};

//==================================================================================================
template <>
struct fly::string::Formatter<std::filesystem::path> : public fly::string::Formatter<std::string>
{
    /**
     * Format a filesystem path.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param path The filesystem path to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(std::filesystem::path const &path, FormatContext &context)
    {
        fly::string::Formatter<std::string>::format(path.string(), context);
    }
};
