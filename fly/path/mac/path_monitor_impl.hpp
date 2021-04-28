#pragma once

#include "fly/path/path_monitor.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

#include <CoreServices/CoreServices.h>
#include <sys/stat.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly::path {

class PathConfig;

/**
 * macOS implementation of the PathMonitor interface. Uses the Apple File System Events API to
 * detect path changes. See:
 *
 * https://developer.apple.com/documentation/coreservices/file_system_events?language=objc
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 15, 2020
 */
class PathMonitorImpl : public PathMonitor
{
public:
    /**
     * Constructor. Creates the dispatch queue used to handle path events.
     */
    PathMonitorImpl(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<PathConfig> config) noexcept;

    /**
     * Destructor. Destroys the dispatch queue and any open FSEvents stream.
     */
    ~PathMonitorImpl() override;

protected:
    /**
     * Check if the path monitor's dispatch queue was successfully created.
     *
     * @return True if the dispatch queue is valid.
     */
    bool is_valid() const override;

    /**
     * Check if the FSEvents API has detected any path events, and handle any that occurred.
     *
     * @param timeout Max time allowed to wait for an event to occur.
     */
    void poll(std::chrono::milliseconds timeout) override;

    std::unique_ptr<PathMonitor::PathInfo>
    create_path_info(const std::filesystem::path &path) const override;

private:
    /**
     * macOS implementation of the PathInfo interface. Stores the path in a form used by the
     * FSEvents API and its inode ID. Updates the parent PathMonitorImpl upon creation/destruction.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        PathInfoImpl(
            const PathMonitorImpl *path_monitor,
            const std::filesystem::path &path) noexcept;
        ~PathInfoImpl() override;

        /**
         * @return True if the path could be converted to a CFStringRef and its inode ID is valid.
         */
        bool is_valid() const override;

        PathMonitorImpl *m_path_monitor;
        CFStringRef m_path;
        ino_t m_inode_id;
    };

    /**
     * Structure to hold path event data received by the FSEvents API.
     */
    struct EventInfo
    {
        std::filesystem::path path;
        PathEvent event;
    };

    /**
     * The FSEvents API does not allow for modifying the monitored paths after the event stream has
     * been created. Instead, when a path is added or removed from the PathMonitor, it must close
     * the existing stream and create a new one.
     */
    void refresh_monitored_paths();

    /**
     * Close the FSEvents stream.
     */
    void close_event_stream();

    /**
     * The FSEventStreamCallback invoked when path events occur. Queue the event to the owning
     * PathMonitorImpl to be handled on the next poll cycle.
     *
     * @param stream The stream for which events occurred.
     * @param info A pointer to the PathMonitorImpl that should handle the events.
     * @param event_size The number of events being reported in this callback.
     * @param event_paths An array of paths to which events occurred.
     * @param event_flags An array of flag words corresponding to the paths in event_paths.
     * @param event_ids An array of FSEventStreamEventIds corresponding to the paths in event_paths.
     */
    static void event_callback(
        ConstFSEventStreamRef stream,
        void *info,
        std::size_t event_size,
        void *event_paths,
        const FSEventStreamEventFlags event_flags[],
        const FSEventStreamEventId event_ids[]);

    /**
     * Convert an FSEventStreamEventFlags event mask to a PathEvent list.
     *
     * @param flags The FSEventStreamEventFlags mask.
     *
     * @return A PathEvent list that matches the FSEventStreamEventFlags mask.
     */
    static std::vector<PathEvent> convert_to_events(const FSEventStreamEventFlags &flags);

    /**
     * Handle a single path event received in the FSEventStreamCallback. Find a monitored path that
     * corresponds to the event and trigger its callback. If no path was found, drop the event.
     *
     * @param event Information about the path event which occurred.
     */
    void handle_event(EventInfo &&event) const;

    std::unique_ptr<FSEventStreamContext> m_context;

    dispatch_queue_t m_dispatch_queue;
    FSEventStreamRef m_stream;

    fly::ConcurrentQueue<EventInfo> m_event_queue;
    std::vector<CFStringRef> m_paths;
};

} // namespace fly::path
