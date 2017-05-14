#pragma once

#include <chrono>
#include <string>

#include <sys/inotify.h>

#include "fly/fly.h"
#include "fly/path/path_monitor.h"

namespace fly {

DEFINE_CLASS_PTRS(PathMonitorImpl);

/**
 * Linux implementation of the PathMonitor interface. Uses the inotify API to
 * detect path changes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version January 19, 2017
 */
class PathMonitorImpl : public PathMonitor
{
public:
    PathMonitorImpl();
    virtual ~PathMonitorImpl();

    /**
     * Check if the path monitor's inotify handle was successfully created.
     *
     * @return bool True if the handle is valid.
     */
    virtual bool IsValid() const;

protected:
    virtual PathMonitor::PathInfoPtr CreatePathInfo(const std::string &) const;
    virtual void Poll(const std::chrono::milliseconds &);
    virtual void Close();

private:
    DEFINE_STRUCT_PTRS(PathInfoImpl);

    /**
     * Linux implementation of the PathInfo interface. Stores the path monitor's
     * inotify handle for adding/removing the monitored path, as well as a
     * handle to the added inotify watch.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        PathInfoImpl(int, const std::string &);
        virtual ~PathInfoImpl();

        /**
         * @return bool True if initialization was sucessful.
         */
        virtual bool IsValid() const;

        int m_monitorDescriptor;
        int m_watchDescriptor;
    };

    /**
     * Read the inotify path monitor handle for any events.
     *
     * @return bool True if any events were read.
     */
    bool readEvents() const;

    /**
     * Handle a single inotify event. Find a monitored path that corresponds
     * to the event and trigger its callback. If no path was found, drop the
     * event.
     */
    void handleEvent(const struct inotify_event *) const;

    /**
     * Convert an inotify event mask to a PathEvent.
     *
     * @param int The inotify event mask.
     *
     * @return PathEvent A PathEvent that matches the event mask.
     */
    PathMonitor::PathEvent convertToEvent(int) const;

    int m_monitorDescriptor;
};

}
