#pragma once

#include "fly/path/path_monitor.hpp"

#include <sys/inotify.h>

#include <chrono>
#include <filesystem>
#include <memory>

namespace fly {

class PathConfig;
class SequencedTaskRunner;

/**
 * Linux implementation of the PathMonitor interface. Uses the inotify API to detect path changes.
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
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<PathConfig> &config) noexcept;

    /**
     * Destructor. Close the path monitor's inotify handle.
     */
    ~PathMonitorImpl() override;

protected:
    /**
     * Check if the path monitor's inotify handle was successfully created.
     *
     * @return True if the inotify handle is valid.
     */
    bool is_valid() const override;

    /**
     * Check if the path monitor's inotify handle has any events to be read, and handle any that are
     * readable.
     *
     * @param timeout Max time allow for an event to be readable.
     */
    void poll(const std::chrono::milliseconds &timeout) override;

    std::unique_ptr<PathMonitor::PathInfo>
    create_path_info(const std::filesystem::path &path) const override;

private:
    /**
     * Linux implementation of the PathInfo interface. Stores the path monitor's inotify handle for
     * adding/removing the monitored path, as well as a handle to the added inotify watch.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        PathInfoImpl(int monitor_descriptor, const std::filesystem::path &path) noexcept;
        ~PathInfoImpl() override;

        /**
         * @return True if initialization was sucessful.
         */
        bool is_valid() const override;

        int m_monitor_descriptor;
        int m_watch_descriptor;
    };

    /**
     * Read the inotify path monitor handle for any events.
     *
     * @return True if any events were read.
     */
    bool read_events() const;

    /**
     * Handle a single inotify event. Find a monitored path that corresponds
     * to the event and trigger its callback. If no path was found, drop the
     * event.
     */
    void handle_event(const inotify_event *event) const;

    /**
     * Convert an inotify event mask to a PathEvent.
     *
     * @param mask The inotify event mask.
     *
     * @return A PathEvent that matches the event mask.
     */
    PathMonitor::PathEvent convert_to_event(std::uint32_t mask) const;

    int m_monitor_descriptor;
};

} // namespace fly
