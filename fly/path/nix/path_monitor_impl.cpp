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
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<PathConfig> &config) noexcept :
    PathMonitor(task_runner, config),
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
bool PathMonitorImpl::is_valid() const noexcept
{
    return m_monitor_descriptor != -1;
}

//==================================================================================================
void PathMonitorImpl::poll(const std::chrono::milliseconds &timeout) noexcept
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
PathMonitorImpl::create_path_info(const std::filesystem::path &path) const noexcept
{
    std::unique_ptr<PathMonitor::PathInfo> info;

    if (is_valid())
    {
        info = std::make_unique<PathInfoImpl>(m_monitor_descriptor, path);
    }

    return info;
}

//==================================================================================================
bool PathMonitorImpl::read_events() const noexcept
{
    static constexpr const std::size_t s_event_size = sizeof(inotify_event);

    // Some systems cannot read integer variables if they are not properly
    // aligned. On other systems, incorrect alignment may decrease performance.
    // Hence, the buffer used for reading from the inotify file descriptor
    // should have the same alignment as inotify_event.
    char buff[8 << 10] __attribute__((aligned(__alignof__(inotify_event))));

    ssize_t size = ::read(m_monitor_descriptor, buff, sizeof(buff));

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

        for (char *ptr = buff; ptr < buff + size; ptr += s_event_size + event->len)
        {
            event = reinterpret_cast<inotify_event *>(ptr);

            if (event->len > 0)
            {
                handle_event(event);
            }
        }
    }

    return size > 0;
}

//==================================================================================================
void PathMonitorImpl::handle_event(const inotify_event *event) const noexcept
{
    auto path_it = std::find_if(
        m_path_info.begin(),
        m_path_info.end(),
        [&event](const PathInfoMap::value_type &value) -> bool {
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

                LOGI("Handling event %d for %s", path_event, path);
                callback(path, path_event);
            }
        }
    }
}

//==================================================================================================
PathMonitor::PathEvent PathMonitorImpl::convert_to_event(std::uint32_t mask) const noexcept
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
        LOGS("Could not add watcher for %s", path);
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
bool PathMonitorImpl::PathInfoImpl::is_valid() const noexcept
{
    return m_watch_descriptor != -1;
}

} // namespace fly
