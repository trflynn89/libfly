#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <fly/fly.h>
#include <fly/concurrency/concurrent_queue.h>
#include <fly/task/runner.h>

namespace fly {

DEFINE_CLASS_PTRS(FileMonitor);

/**
 * Virtual interface to monitor a local file. This interface is platform
 * independent - OS dependent implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version January 19, 2017
 */
class FileMonitor : public Runner
{
public:
    /**
     * Enumerated list of file events.
     */
    enum FileEvent
    {
        FILE_NO_CHANGE,
        FILE_CREATED,
        FILE_DELETED,
        FILE_CHANGED
    };

    /**
     * Callback definition for function to be triggered on a file change.
     */
    typedef std::function<void(const std::string &, const std::string &, FileEvent)> FileEventCallback;

    /**
     * Constructor.
     */
    FileMonitor();

    /**
     * Destructor. Stop the file monitor thread if necessary.
     */
    virtual ~FileMonitor();

    /**
     * Check if the monitor implementation is in a good state.
     *
     * @return True if the monitor is healthy.
     */
    virtual bool IsValid() const = 0;

    /**
     * Add a file to be monitored.
     *
     * @param string Directory containing the file to start monitoring.
     * @param string Name of the file to start monitoring.
     * @param FileEventCallback Callback to trigger when the file changes.
     *
     * @return bool True if the file could be added.
     */
    bool AddFile(const std::string &, const std::string &, FileEventCallback);

    /**
     * Stop monitoring a file.
     *
     * @param string Directory containing the file to stop monitoring.
     * @param string Name of the file to stop monitoring.
     *
     * @return bool True if the file was removed.
     */
    bool RemoveFile(const std::string &, const std::string &);

    /**
     * Stop monitoring all files under the given path.
     *
     * @param string Directory containing the files to stop monitoring.
     *
     * @return bool True if the path was removed.
     */
    bool RemovePath(const std::string &);

    /**
     * Stop monitoring all files.
     */
    void RemoveAllPaths();

protected:
    DEFINE_STRUCT_PTRS(PathInfo);

    /**
     * Struct to store information about a monitored path. OS dependent
     * implementations of FileMonitor should also have a concrete defintion
     * of this struct.
     */
    struct PathInfo
    {
        /**
         * Check if the monitored path is in a good state.
         *
         * @return bool True if the monitored path is healthy.
         */
        virtual bool IsValid() const = 0;

        std::map<std::string, FileEventCallback> m_handlers;
    };

    /**
     * Map of monitored path names to their path information.
     */
    typedef std::map<std::string, PathInfoPtr> PathInfoMap;

    /**
     * @return True if the monitor is in a good state.
     */
    virtual bool StartRunner();

    /**
     * Clear the monitor callback.
     */
    virtual void StopRunner();

    /**
     * Poll the monitored file for changes.
     *
     * @return True if the monitor is in a good state.
     */
    virtual bool DoWork();

    /**
     * Create an instance of the OS dependent PathInfo struct.
     *
     * @param string Directory containing the file to be monitored.
     *
     * @return PathInfoPtr Up-casted shared pointer to the PathInfo struct.
     */
    virtual PathInfoPtr CreatePathInfo(const std::string &) const = 0;

    /**
     * Poll for any file changes.
     *
     * @param milliseconds Max time to poll for file changes.
     */
    virtual void Poll(const std::chrono::milliseconds &) = 0;

    /**
     * Close any open file handles.
     */
    virtual void Close() = 0;

    mutable std::mutex m_mutex;
    PathInfoMap m_pathInfo;
};

}
