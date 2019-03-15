#pragma once

#include "fly/fly.h"
#include "fly/task/task.h"

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
 * Virtual interface to monitor a local path. Provides monitoring of either all
 * files or user-specified files under a path for addition, deletion, or change.
 * This interface is platform independent - OS dependent implementations should
 * inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version May 14, 2017
 */
class PathMonitor : public std::enable_shared_from_this<PathMonitor>
{
    friend class PathMonitorTask;

public:
    /**
     * Enumerated list of path events.
     */
    enum class PathEvent
    {
        None,
        Created,
        Deleted,
        Changed
    };

    /**
     * Callback definition for function to be triggered on a path change.
     */
    using PathEventCallback =
        std::function<void(const std::filesystem::path &, PathEvent)>;

    /**
     * Constructor.
     *
     * @param TaskRunner Task runner for posting path-related tasks onto.
     * @param PathConfig Reference to path configuration.
     */
    PathMonitor(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<PathConfig> &);

    /**
     * Destructor. Remove all paths from the path monitor.
     */
    virtual ~PathMonitor();

    /**
     * Initialize the path monitor task.
     *
     * @return bool True if the path monitor is in a valid state.
     */
    bool Start();

    /**
     * Monitor for changes to all files under a directory. Callbacks registered
     * with AddFile take precendence over callbacks registered with AddPath.
     *
     * @param path Path to the directory to start monitoring.
     * @param PathEventCallback Callback to trigger when a file changes.
     *
     * @return bool True if the directory could be added.
     */
    bool AddPath(const std::filesystem::path &, PathEventCallback);

    /**
     * Stop monitoring for changes to all files under a directory.
     *
     * @param path Path to the directory to stop monitoring.
     *
     * @return bool True if the directory was removed.
     */
    bool RemovePath(const std::filesystem::path &);

    /**
     * Stop monitoring all paths.
     */
    void RemoveAllPaths();

    /**
     * Monitor for changes to a single file. Callbacks registered with AddFile
     * take precendence over callbacks registered with AddPath.
     *
     * @param path Path to the file to start monitoring.
     * @param PathEventCallback Callback to trigger when the file changes.
     *
     * @return bool True if the file could be added.
     */
    bool AddFile(const std::filesystem::path &, PathEventCallback);

    /**
     * Stop monitoring for changes to a single file. If there are no more files
     * monitored in the file's directory, and there is no callback registered
     * for that directory, the directory itself is removed from the monitor.
     *
     * @param path Path to the file to stop monitoring.
     *
     * @return bool True if the file was removed.
     */
    bool RemoveFile(const std::filesystem::path &);

protected:
    /**
     * Struct to store information about a monitored path. OS dependent
     * implementations of PathMonitor should also have a concrete defintion
     * of this struct.
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
         * @return bool True if the monitored path is healthy.
         */
        virtual bool IsValid() const = 0;

        PathEventCallback m_pathHandler;
        std::map<std::filesystem::path, PathEventCallback> m_fileHandlers;
    };

    /**
     * Map of monitored paths to their path information.
     */
    using PathInfoMap =
        std::map<std::filesystem::path, std::shared_ptr<PathInfo>>;

    /**
     * Create an instance of the OS dependent PathInfo struct.
     *
     * @param path The path to be monitored.
     *
     * @return PathInfo Up-casted shared pointer to the PathInfo struct.
     */
    virtual std::shared_ptr<PathInfo>
    CreatePathInfo(const std::filesystem::path &) const = 0;

    /**
     * Check if the path monitor implementation is valid.
     *
     * @return bool True if the implementation is valid.
     */
    virtual bool IsValid() const = 0;

    /**
     * Check the path monitor implementation for any changes to the monitored
     * paths.
     *
     * @param milliseconds Max time allow for an event to be occur.
     */
    virtual void Poll(const std::chrono::milliseconds &) = 0;

    mutable std::mutex m_mutex;
    PathInfoMap m_pathInfo;

private:
    /**
     * Search for a path to be monitored in the PathInfo map. If the map does
     * not contain the path, create an entry.
     *
     * @param path The path to be monitored.
     *
     * @return PathInfo Shared pointer to the PathInfo struct.
     */
    std::shared_ptr<PathInfo>
    getOrCreatePathInfo(const std::filesystem::path &);

    /**
     * Stream the name of a Json instance's type.
     */
    friend std::ostream &operator<<(std::ostream &, PathEvent);

    std::shared_ptr<SequencedTaskRunner> m_spTaskRunner;
    std::shared_ptr<Task> m_spTask;

    std::shared_ptr<PathConfig> m_spConfig;
};

/**
 * Task to be executed to check for changes to the monitored paths.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class PathMonitorTask : public Task
{
public:
    PathMonitorTask(const std::weak_ptr<PathMonitor> &);

protected:
    /**
     * Call back into the path monitor to check for any changes to the monitored
     * paths. If the path monitor implementation is still valid, the task
     * re-arms itself.
     */
    void Run() override;

private:
    std::weak_ptr<PathMonitor> m_wpPathMonitor;
};

} // namespace fly

#include FLY_OS_IMPL_PATH(path, path_monitor)
