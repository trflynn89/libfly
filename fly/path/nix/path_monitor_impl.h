#pragma once

#include "fly/path/path_monitor.h"

#include <sys/inotify.h>

#include <chrono>
#include <filesystem>
#include <memory>

namespace fly {

class PathConfig;
class SequencedTaskRunner;

/**
 * Linux implementation of the PathMonitor interface. Uses the inotify API to
 * detect path changes.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 19, 2017
 */
class PathMonitorImpl : public PathMonitor
{
public:
    /**
     * Constructor. Create the path monitor's inotify handle.
     */
    PathMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<PathConfig> &) noexcept;

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
    bool IsValid() const noexcept override;

    /**
     * Check if the path monitor's inotify handle has any events to be read,
     * and handle any that are readable.
     *
     * @param milliseconds Max time allow for an event to be readable.
     */
    void Poll(const std::chrono::milliseconds &) noexcept override;

    std::shared_ptr<PathMonitor::PathInfo>
    CreatePathInfo(const std::filesystem::path &) const noexcept override;

private:
    /**
     * Linux implementation of the PathInfo interface. Stores the path monitor's
     * inotify handle for adding/removing the monitored path, as well as a
     * handle to the added inotify watch.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        PathInfoImpl(int, const std::filesystem::path &) noexcept;
        ~PathInfoImpl() override;

        /**
         * @return bool True if initialization was sucessful.
         */
        bool IsValid() const noexcept override;

        int m_monitorDescriptor;
        int m_watchDescriptor;
    };

    /**
     * Read the inotify path monitor handle for any events.
     *
     * @return bool True if any events were read.
     */
    bool readEvents() const noexcept;

    /**
     * Handle a single inotify event. Find a monitored path that corresponds
     * to the event and trigger its callback. If no path was found, drop the
     * event.
     */
    void handleEvent(const struct inotify_event *) const noexcept;

    /**
     * Convert an inotify event mask to a PathEvent.
     *
     * @param int The inotify event mask.
     *
     * @return PathEvent A PathEvent that matches the event mask.
     */
    PathMonitor::PathEvent convertToEvent(int) const noexcept;

    int m_monitorDescriptor;
};

} // namespace fly
