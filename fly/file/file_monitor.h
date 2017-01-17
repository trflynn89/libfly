#pragma once

#include <chrono>
#include <functional>
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
 * @version July 21, 2016
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
     */
    virtual bool AddFile(const std::string &, const std::string &, FileEventCallback) = 0;

    /**
     * Stop monitoring a file.
     *
     * @param string Directory containing the file to stop monitoring.
     * @param string Name of the file to stop monitoring.
     */
    virtual bool RemoveFile(const std::string &, const std::string &) = 0;

protected:
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
     * Poll for any file changes.
     *
     * @param milliseconds Max time to poll for file changes.
     */
    virtual void Poll(const std::chrono::milliseconds &) = 0;

    /**
     * Close any open file handles.
     */
    virtual void Close() = 0;
};

}
