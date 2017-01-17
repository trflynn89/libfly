#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <poll.h>
#include <string>
#include <sys/inotify.h>
#include <vector>

#include <fly/file/file_monitor.h>

namespace fly {

/**
 * Linux implementation of the FileMonitor interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class FileMonitorImpl : public FileMonitor
{
    /**
     * Information pertaining to a monitored path.
     */
    struct PathMonitor
    {
        PathMonitor() : m_watchDescriptor(-1)
        {
        }

        std::map<std::string, FileEventCallback> m_handlers;
        int m_watchDescriptor;
    };

    /**
     * Map of monitored path names to their monitor information.
     */
    typedef std::map<std::string, PathMonitor> PathMap;

public:
    FileMonitorImpl();
    virtual ~FileMonitorImpl();

    virtual bool IsValid() const;

    virtual bool AddFile(const std::string &, const std::string &, FileEventCallback);
    virtual bool RemoveFile(const std::string &, const std::string &);

protected:
    virtual void Poll(const std::chrono::milliseconds &);
    virtual void Close();

private:
    /**
     * Read the inotify file monitor handle for any events.
     *
     * @return bool True if any events were read.
     */
    bool readEvents();

    /**
     * Handle a single inotify event. Find a monitored file that corresponds
     * to the event and trigger its callback. If no file was found, drop the
     * event.
     */
    void handleEvent(const struct inotify_event *);

    /**
     * Convert an inotify event mask to a FileEvent.
     *
     * @param int The inotify event mask.
     *
     * @return FileEvent A FileEvent that matches the event mask.
     */
    FileMonitor::FileEvent convertToEvent(int);

    mutable std::mutex m_mutex;
    int m_monitorDescriptor;
    PathMap m_monitoredPaths;
};

}
