#include "fly/path/mac/path_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/string/string.hpp"

#import <Foundation/Foundation.h>

#include <algorithm>

namespace fly {

namespace {

    const FSEventStreamCreateFlags s_stream_flags = kFSEventStreamCreateFlagFileEvents |
        kFSEventStreamCreateFlagUseCFTypes | kFSEventStreamCreateFlagUseExtendedData;

    const CFAbsoluteTime s_latency = 0.25;

    /**
     * Retreive the inode ID of the provided directory, returning 0 if an error occurs.
     */
    ino_t get_inode_id(const std::filesystem::path &path)
    {
        struct stat info;

        if (::stat(path.c_str(), &info) == 0)
        {
            return info.st_ino;
        }

        LOGS("Could not determine inode ID for {}", path);
        return 0;
    }

} // namespace

//==================================================================================================
PathMonitorImpl::PathMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<PathConfig> &config) noexcept :
    PathMonitor(task_runner, config),
    m_context(std::make_unique<FSEventStreamContext>()),
    m_dispatch_queue(dispatch_queue_create(
        fly::String::format("fly.PathMonitor.{:p}", this).c_str(),
        DISPATCH_QUEUE_SERIAL)),
    m_stream(nullptr)
{
    if (m_dispatch_queue == nullptr)
    {
        LOGS("Could not initialize monitor");
    }
    else
    {
        m_context->version = 0;
        m_context->info = this;
        m_context->retain = nullptr;
        m_context->release = nullptr;
        m_context->copyDescription = nullptr;
    }
}

//==================================================================================================
PathMonitorImpl::~PathMonitorImpl()
{
    close_event_stream();

    if (m_dispatch_queue != nullptr)
    {
        dispatch_release(m_dispatch_queue);
    }
}

//==================================================================================================
bool PathMonitorImpl::is_valid() const
{
    return m_dispatch_queue != nullptr;
}

//==================================================================================================
void PathMonitorImpl::poll(const std::chrono::milliseconds &timeout)
{
    std::chrono::milliseconds time_remaining = timeout;
    EventInfo event;

    while (time_remaining != std::chrono::milliseconds::zero())
    {
        const auto start = std::chrono::steady_clock::now();

        if (m_event_queue.pop(event, time_remaining))
        {
            handle_event(std::move(event));
        }

        const auto end = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        if (duration >= time_remaining)
        {
            time_remaining = std::chrono::milliseconds::zero();
        }
        else
        {
            time_remaining -= duration;
        }
    }
}

//==================================================================================================
std::unique_ptr<PathMonitor::PathInfo>
PathMonitorImpl::create_path_info(const std::filesystem::path &path) const
{
    std::unique_ptr<PathMonitor::PathInfo> info;

    if (is_valid())
    {
        info = std::make_unique<PathInfoImpl>(this, path);
    }

    return info;
}

//==================================================================================================
void PathMonitorImpl::refresh_monitored_paths()
{
    close_event_stream();

    dispatch_sync(m_dispatch_queue, ^{
        CFArrayRef pathsToWatch = CFArrayCreate(
            nullptr,
            reinterpret_cast<const void **>(m_paths.data()),
            static_cast<CFIndex>(m_paths.size()),
            &kCFTypeArrayCallBacks);

        m_stream = FSEventStreamCreate(
            kCFAllocatorDefault,
            &PathMonitorImpl::event_callback,
            m_context.get(),
            pathsToWatch,
            kFSEventStreamEventIdSinceNow,
            s_latency,
            s_stream_flags);

        if (m_stream == nullptr)
        {
            LOGS("Could not create FSEvents stream");
        }
        else
        {
            FSEventStreamSetDispatchQueue(m_stream, m_dispatch_queue);
            FSEventStreamStart(m_stream);
        }
    });
}

//==================================================================================================
void PathMonitorImpl::close_event_stream()
{
    if (m_stream != nullptr)
    {
        dispatch_sync(m_dispatch_queue, ^{
            FSEventStreamStop(m_stream);
            FSEventStreamInvalidate(m_stream);
            FSEventStreamRelease(m_stream);

            m_stream = nullptr;
        });
    }
}

//==================================================================================================
void PathMonitorImpl::event_callback(
    ConstFSEventStreamRef,
    void *user_data,
    std::size_t event_size,
    void *event_paths,
    const FSEventStreamEventFlags event_flags[],
    const FSEventStreamEventId[])
{
    auto *path_monitor = reinterpret_cast<PathMonitorImpl *>(user_data);
    auto paths = reinterpret_cast<CFArrayRef>(event_paths);

    char file_path[PATH_MAX];

    for (std::size_t i = 0; i < event_size; ++i)
    {
        auto index = static_cast<CFIndex>(i);
        auto path_dictionary = static_cast<CFDictionaryRef>(CFArrayGetValueAtIndex(paths, index));

        auto event_path_as_cfstring = static_cast<CFStringRef>(
            CFDictionaryGetValue(path_dictionary, kFSEventStreamEventExtendedDataPathKey));

        if (CFStringGetFileSystemRepresentation(event_path_as_cfstring, file_path, PATH_MAX))
        {
            const std::filesystem::path event_path(file_path);

            if (!std::filesystem::is_directory(event_path))
            {
                for (const auto &path_event : convert_to_events(event_flags[i]))
                {
                    EventInfo event {event_path, path_event};
                    path_monitor->m_event_queue.push(std::move(event));
                }
            }
        }
        else
        {
            LOGS("Could not convert CFStringRef to path");
            continue;
        }
    }
}

//==================================================================================================
std::vector<PathMonitor::PathEvent>
PathMonitorImpl::convert_to_events(const FSEventStreamEventFlags &flags)
{
    std::vector<PathMonitor::PathEvent> path_events;

    if ((flags & kFSEventStreamEventFlagItemCreated) != 0)
    {
        path_events.push_back(PathMonitor::PathEvent::Created);
    }
    if ((flags & kFSEventStreamEventFlagItemModified) != 0)
    {
        path_events.push_back(PathMonitor::PathEvent::Changed);
    }
    if ((flags & kFSEventStreamEventFlagItemRemoved) != 0)
    {
        path_events.push_back(PathMonitor::PathEvent::Deleted);
    }

    return path_events;
}

//==================================================================================================
void PathMonitorImpl::handle_event(EventInfo &&event) const
{
    const ino_t inode_id = get_inode_id(event.path.parent_path());
    std::lock_guard<std::mutex> lock(m_mutex);

    auto path_it = std::find_if(
        m_path_info.begin(),
        m_path_info.end(),
        [&inode_id](const PathInfoMap::value_type &value) -> bool
        {
            const auto *info = static_cast<PathInfoImpl *>(value.second.get());
            return info->m_inode_id == inode_id;
        });

    if (path_it != m_path_info.end())
    {
        const auto *info = static_cast<PathInfoImpl *>(path_it->second.get());

        auto file_it = info->m_file_handlers.find(event.path.filename());
        PathEventCallback callback = nullptr;

        if (file_it == info->m_file_handlers.end())
        {
            callback = info->m_path_handler;
        }
        else
        {
            callback = file_it->second;
        }

        if (callback != nullptr)
        {
            auto path = std::filesystem::path(path_it->first) / event.path.filename();

            LOGI("Handling event {} for {}", event.event, path);
            callback(path, event.event);
        }
    }
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::PathInfoImpl(
    const PathMonitorImpl *path_monitor,
    const std::filesystem::path &path) noexcept :
    PathMonitorImpl::PathInfo(),
    m_path_monitor(const_cast<PathMonitorImpl *>(path_monitor)),
    m_path(CFStringCreateWithCString(nullptr, path.c_str(), kCFStringEncodingUTF8)),
    m_inode_id(get_inode_id(path))
{
    if (is_valid())
    {
        m_path_monitor->m_paths.push_back(m_path);
        m_path_monitor->refresh_monitored_paths();
    }
    else
    {
        LOGS("Could not add watcher for {}", path);
    }
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (is_valid())
    {
        auto it = std::find(m_path_monitor->m_paths.begin(), m_path_monitor->m_paths.end(), m_path);

        if (it != m_path_monitor->m_paths.end())
        {
            m_path_monitor->m_paths.erase(it);
            m_path_monitor->refresh_monitored_paths();
        }
    }
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::is_valid() const
{
    return (m_path != nullptr) && (m_inode_id != 0);
}

} // namespace fly
