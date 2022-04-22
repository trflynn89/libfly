#pragma once

#include "fly/path/path_monitor.hpp"

#include <sys/inotify.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly::path {

class PathConfig;

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
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<PathConfig> config) noexcept;

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
     * @param timeout Max time allowed to wait for an event to be readable.
     */
    void poll(std::chrono::milliseconds timeout) override;

    std::unique_ptr<PathMonitor::PathInfo>
    create_path_info(std::filesystem::path const &path) const override;

private:
    /**
     * Linux implementation of the PathInfo interface. Stores the path monitor's inotify handle for
     * adding/removing the monitored path, as well as a handle to the added inotify watch.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        PathInfoImpl(int monitor_descriptor, std::filesystem::path const &path) noexcept;
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
    bool read_events();

    /**
     * Handle a single inotify event. Find a monitored path that corresponds to the event and
     * trigger its callback. If no path was found, drop the event.
     *
     * @param event The inotify event to handle.
     */
    void handle_event(inotify_event const *event) const;

    /**
     * Convert an inotify event mask to a PathEvent.
     *
     * @param mask The inotify event mask.
     *
     * @return A PathEvent that matches the event mask.
     */
    PathEvent convert_to_event(std::uint32_t mask) const;

    // Notes from INOTIFY(7) man page:
    //
    //     Some systems cannot read integer variables if they are not properly aligned. On other
    //     systems, incorrect alignment may decrease performance. Hence, the buffer used for reading
    //     from the inotify file descriptor should have the same alignment as inotify_event.
    alignas(alignof(inotify_event)) std::array<std::uint8_t, 4 << 10> m_event_data;

    int m_monitor_descriptor;
};

} // namespace fly::path
