#pragma once

#include "fly/fly.hpp"
#include "fly/task/task.hpp"

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace fly {

class PathConfig;
class PathMonitorTask;
class SequencedTaskRunner;

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
    friend class PathMonitorTask;

public:
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
     * Callback definition for function to be triggered on a path change.
     */
    using PathEventCallback = std::function<void(const std::filesystem::path &, PathEvent)>;

    /**
     * Constructor.
     *
     * @param task_runner Task runner for posting path-related tasks onto.
     * @param config Reference to path configuration.
     */
    PathMonitor(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<PathConfig> &config) noexcept;

    /**
     * Destructor. Remove all paths from the path monitor.
     */
    virtual ~PathMonitor();

    /**
     * Initialize the path monitor task.
     *
     * @return True if the path monitor is in a valid state.
     */
    bool start();

    /**
     * Monitor for changes to all files under a directory. Callbacks registered with AddFile take
     * precendence over callbacks registered with AddPath.
     *
     * @param path Path to the directory to start monitoring.
     * @param callback Callback to trigger when a file changes.
     *
     * @return True if the directory could be added.
     */
    bool add_path(const std::filesystem::path &path, PathEventCallback callback);

    /**
     * Stop monitoring for changes to all files under a directory.
     *
     * @param path Path to the directory to stop monitoring.
     *
     * @return True if the directory was removed.
     */
    bool remove_path(const std::filesystem::path &path);

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
    bool add_file(const std::filesystem::path &file, PathEventCallback callback);

    /**
     * Stop monitoring for changes to a single file. If there are no more files monitored in the
     * file's directory, and there is no callback registered for that directory, the directory
     * itself is removed from the monitor.
     *
     * @param file Path to the file to stop monitoring.
     *
     * @return True if the file was removed.
     */
    bool remove_file(const std::filesystem::path &file);

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
     * Create an instance of the OS dependent PathInfo struct.
     *
     * @param path The path to be monitored.
     *
     * @return Up-casted pointer to the PathInfo struct.
     */
    virtual std::unique_ptr<PathInfo> create_path_info(const std::filesystem::path &path) const = 0;

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
    virtual void poll(const std::chrono::milliseconds &timeout) = 0;

    mutable std::mutex m_mutex;
    PathInfoMap m_path_info;

private:
    /**
     * Search for a path to be monitored in the PathInfo map. If the map does not contain the path,
     * create an entry.
     *
     * @param path The path to be monitored.
     *
     * @return Shared pointer to the PathInfo struct.
     */
    PathInfo *get_or_create_path_info(const std::filesystem::path &path);

    /**
     * Stream the name of a PathEvent instance.
     */
    friend std::ostream &operator<<(std::ostream &stream, PathEvent event);

    std::shared_ptr<SequencedTaskRunner> m_task_runner;
    std::shared_ptr<Task> m_task;

    std::shared_ptr<PathConfig> m_config;
};

/**
 * Task to be executed to check for changes to the monitored paths.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class PathMonitorTask : public Task
{
public:
    explicit PathMonitorTask(std::weak_ptr<PathMonitor> weak_path_monitor) noexcept;

protected:
    /**
     * Call back into the path monitor to check for any changes to the monitored paths. If the path
     * monitor implementation is still valid, the task re-arms itself.
     */
    void run() override;

private:
    std::weak_ptr<PathMonitor> m_weak_path_monitor;
};

} // namespace fly

#include FLY_OS_IMPL_PATH(path, path_monitor)
