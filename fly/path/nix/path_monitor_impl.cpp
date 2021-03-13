#include "fly/path/nix/path_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task_runner.hpp"

#include <poll.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace fly {

namespace {

    const int s_init_flags = IN_NONBLOCK;

    const int s_change_flags = IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM | IN_MODIFY;

} // namespace

//==================================================================================================
PathMonitorImpl::PathMonitorImpl(
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<PathConfig> config) noexcept :
    PathMonitor(std::move(task_runner), std::move(config)),
    m_monitor_descriptor(::inotify_init1(s_init_flags))
{
    if (m_monitor_descriptor == -1)
    {
        LOGS("Could not initialize monitor");
    }
}

//==================================================================================================
PathMonitorImpl::~PathMonitorImpl()
{
    if (m_monitor_descriptor != -1)
    {
        ::close(m_monitor_descriptor);
        m_monitor_descriptor = -1;
    }
}

//==================================================================================================
bool PathMonitorImpl::is_valid() const
{
    return m_monitor_descriptor != -1;
}

//==================================================================================================
void PathMonitorImpl::poll(std::chrono::milliseconds timeout)
{
    pollfd poll_fd;

    poll_fd.fd = m_monitor_descriptor;
    poll_fd.events = POLLIN;

    int events = ::poll(&poll_fd, 1, timeout.count());

    if (events == -1)
    {
        LOGS("Could not create poller");
    }
    else if ((events > 0) && (poll_fd.revents & POLLIN))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        while (read_events())
        {
        }
    }
}

//==================================================================================================
std::unique_ptr<PathMonitor::PathInfo>
PathMonitorImpl::create_path_info(const std::filesystem::path &path) const
{
    return std::make_unique<PathInfoImpl>(m_monitor_descriptor, path);
}

//==================================================================================================
bool PathMonitorImpl::read_events()
{
    ssize_t size = ::read(m_monitor_descriptor, m_event_data.data(), m_event_data.size());

    if (size <= 0)
    {
        if ((size == -1) && (System::get_error_code() != EAGAIN))
        {
            LOGS("Could not read polled event");
        }
    }
    else
    {
        const inotify_event *event;

        for (std::uint8_t *event_data = m_event_data.data();
             event_data < (m_event_data.data() + size);
             event_data += sizeof(inotify_event) + event->len)
        {
            event = reinterpret_cast<inotify_event *>(event_data);

            if (event->len > 0)
            {
                handle_event(event);
            }
        }
    }

    return size > 0;
}

//==================================================================================================
void PathMonitorImpl::handle_event(const inotify_event *event) const
{
    auto path_it = std::find_if(
        m_path_info.begin(),
        m_path_info.end(),
        [&event](const PathInfoMap::value_type &value) -> bool
        {
            const auto *info = static_cast<PathInfoImpl *>(value.second.get());
            return info->m_watch_descriptor == event->wd;
        });

    if (path_it != m_path_info.end())
    {
        const auto *info = static_cast<PathInfoImpl *>(path_it->second.get());
        PathMonitor::PathEvent path_event = convert_to_event(event->mask);

        if (path_event != PathMonitor::PathEvent::None)
        {
            const std::filesystem::path file(event->name);

            auto file_it = info->m_file_handlers.find(file);
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
                auto path = std::filesystem::path(path_it->first) / file;

                LOGI("Handling event {} for {}", path_event, path);
                std::invoke(std::move(callback), std::move(path), path_event);
            }
        }
    }
}

//==================================================================================================
PathMonitor::PathEvent PathMonitorImpl::convert_to_event(std::uint32_t mask) const
{
    PathMonitor::PathEvent path_event = PathMonitor::PathEvent::None;

    if ((mask & IN_CREATE) || (mask & IN_MOVED_TO))
    {
        path_event = PathMonitor::PathEvent::Created;
    }
    else if ((mask & IN_DELETE) || (mask & IN_MOVED_FROM))
    {
        path_event = PathMonitor::PathEvent::Deleted;
    }
    else if (mask & IN_MODIFY)
    {
        path_event = PathMonitor::PathEvent::Changed;
    }

    return path_event;
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::PathInfoImpl(
    int monitor_descriptor,
    const std::filesystem::path &path) noexcept :
    PathMonitorImpl::PathInfo(),
    m_monitor_descriptor(monitor_descriptor),
    m_watch_descriptor(-1)
{
    m_watch_descriptor =
        ::inotify_add_watch(m_monitor_descriptor, path.string().c_str(), s_change_flags);

    if (m_watch_descriptor == -1)
    {
        LOGS("Could not add watcher for {}", path);
    }
}

//==================================================================================================
PathMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (m_watch_descriptor != -1)
    {
        ::inotify_rm_watch(m_monitor_descriptor, m_watch_descriptor);
        m_watch_descriptor = -1;
    }
}

//==================================================================================================
bool PathMonitorImpl::PathInfoImpl::is_valid() const
{
    return m_watch_descriptor != -1;
}

} // namespace fly
