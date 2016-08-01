#pragma once

#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include <fly/fly.h>
#include <fly/concurrency/concurrent_stack.h>
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
    typedef std::function<void(FileEvent)> FileEventCallback;

    /**
     * Constructor.
     *
     * @param FileEventCallback Callback to trigger when the file changes.
     * @param string Directory containing the file to monitor.
     * @param string Name of the file to monitor.
     */
    FileMonitor(FileEventCallback, const std::string &, const std::string &);

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
     * Trigger the registered callback for a file change.
     *
     * @param FileEvent The file change event.
     */
    void HandleEvent(FileEvent);

    const std::string m_path;
    const std::string m_file;

private:
    mutable std::mutex m_callbackMutex;
    FileEventCallback m_handler;

    ConcurrentStack<FileEvent> m_eventStack;

    size_t m_interval;
};

}
