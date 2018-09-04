#pragma once

#include <chrono>
#include <string>

#include <sys/inotify.h>

#include "fly/fly.h"
#include "fly/path/path_monitor.h"

namespace fly {

FLY_CLASS_PTRS(PathMonitorImpl);

FLY_CLASS_PTRS(SequencedTaskRunner);
FLY_CLASS_PTRS(PathConfig);

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
    /**
     * Constructor. Create the path monitor's inotify handle.
     */
    PathMonitorImpl(const SequencedTaskRunnerPtr &, const PathConfigPtr &);

    /**
     * Destructor. Close the path monitor's inotify handle.
     */
    ~PathMonitorImpl() override;

protected:
    /**
     * Check if the path monitor's inotify handle was successfully created.
     *
     * @return bool True if the inotify handle is valid.
     */
    bool IsValid() const override;

    /**
     * Check if the path monitor's inotify handle has any events to be read,
     * and handle any that are readable.
     *
     * @param milliseconds Max time allow for an event to be readable.
     */
    void Poll(const std::chrono::milliseconds &) override;

    PathMonitor::PathInfoPtr CreatePathInfo(const std::string &) const override;

private:
    FLY_STRUCT_PTRS(PathInfoImpl);

    /**
     * Linux implementation of the PathInfo interface. Stores the path monitor's
     * inotify handle for adding/removing the monitored path, as well as a
     * handle to the added inotify watch.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        PathInfoImpl(int, const std::string &);
        ~PathInfoImpl() override;

        /**
         * @return bool True if initialization was sucessful.
         */
        bool IsValid() const override;

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
