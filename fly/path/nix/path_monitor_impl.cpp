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

    const int s_initFlags = IN_NONBLOCK;

    const int s_changeFlags =
        IN_CREATE | IN_DELETE | IN_MOVED_TO | IN_MOVED_FROM | IN_MODIFY;

} // namespace

//==============================================================================
PathMonitorImpl::PathMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<PathConfig> &spConfig) noexcept :
    PathMonitor(spTaskRunner, spConfig),
    m_monitorDescriptor(::inotify_init1(s_initFlags))
{
    if (m_monitorDescriptor == -1)
    {
        LOGS("Could not initialize monitor");
    }
}

//==============================================================================
PathMonitorImpl::~PathMonitorImpl()
{
    if (m_monitorDescriptor != -1)
    {
        ::close(m_monitorDescriptor);
        m_monitorDescriptor = -1;
    }
}

//==============================================================================
bool PathMonitorImpl::IsValid() const noexcept
{
    return m_monitorDescriptor != -1;
}

//==============================================================================
void PathMonitorImpl::Poll(const std::chrono::milliseconds &timeout) noexcept
{
    pollfd pollFd;

    pollFd.fd = m_monitorDescriptor;
    pollFd.events = POLLIN;

    int numEvents = ::poll(&pollFd, 1, timeout.count());

    if (numEvents == -1)
    {
        LOGS("Could not create poller");
    }
    else if ((numEvents > 0) && (pollFd.revents & POLLIN))
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        while (readEvents())
        {
        }
    }
}

//==============================================================================
std::shared_ptr<PathMonitor::PathInfo> PathMonitorImpl::CreatePathInfo(
    const std::filesystem::path &path) const noexcept
{
    std::shared_ptr<PathMonitor::PathInfo> spInfo;

    if (IsValid())
    {
        spInfo = std::make_shared<PathInfoImpl>(m_monitorDescriptor, path);
    }

    return spInfo;
}

//==============================================================================
bool PathMonitorImpl::readEvents() const noexcept
{
    static const std::size_t eventSize = sizeof(inotify_event);

    // Some systems cannot read integer variables if they are not properly
    // aligned. On other systems, incorrect alignment may decrease performance.
    // Hence, the buffer used for reading from the inotify file descriptor
    // should have the same alignment as inotify_event.
    char buff[8 << 10] __attribute__((aligned(__alignof__(inotify_event))));

    ssize_t len = ::read(m_monitorDescriptor, buff, sizeof(buff));

    if (len <= 0)
    {
        if ((len == -1) && (System::GetErrorCode() != EAGAIN))
        {
            LOGS("Could not read polled event");
        }
    }
    else
    {
        const inotify_event *pEvent;

        for (char *ptr = buff; ptr < buff + len; ptr += eventSize + pEvent->len)
        {
            pEvent = reinterpret_cast<inotify_event *>(ptr);

            if (pEvent->len > 0)
            {
                handleEvent(pEvent);
            }
        }
    }

    return len > 0;
}

//==============================================================================
void PathMonitorImpl::handleEvent(const inotify_event *pEvent) const noexcept
{
    auto it = std::find_if(
        m_pathInfo.begin(),
        m_pathInfo.end(),
        [&pEvent](const PathInfoMap::value_type &value) -> bool {
            auto spInfo(std::static_pointer_cast<PathInfoImpl>(value.second));
            return spInfo->m_watchDescriptor == pEvent->wd;
        });

    if (it != m_pathInfo.end())
    {
        auto spInfo(std::static_pointer_cast<PathInfoImpl>(it->second));
        PathMonitor::PathEvent event = convertToEvent(pEvent->mask);

        if (event != PathMonitor::PathEvent::None)
        {
            const std::filesystem::path file(pEvent->name);
            auto callback = spInfo->m_fileHandlers[file];

            if (callback == nullptr)
            {
                callback = spInfo->m_pathHandler;
            }

            if (callback != nullptr)
            {
                auto path = std::filesystem::path(it->first) / file;

                LOGI("Handling event %d for %s", event, path);
                callback(path, event);
            }
        }
    }
}

//==============================================================================
PathMonitor::PathEvent
PathMonitorImpl::convertToEvent(std::uint32_t mask) const noexcept
{
    PathMonitor::PathEvent event = PathMonitor::PathEvent::None;

    if ((mask & IN_CREATE) || (mask & IN_MOVED_TO))
    {
        event = PathMonitor::PathEvent::Created;
    }
    else if ((mask & IN_DELETE) || (mask & IN_MOVED_FROM))
    {
        event = PathMonitor::PathEvent::Deleted;
    }
    else if (mask & IN_MODIFY)
    {
        event = PathMonitor::PathEvent::Changed;
    }

    return event;
}

//==============================================================================
PathMonitorImpl::PathInfoImpl::PathInfoImpl(
    int monitorDescriptor,
    const std::filesystem::path &path) noexcept :
    PathMonitorImpl::PathInfo(),
    m_monitorDescriptor(monitorDescriptor),
    m_watchDescriptor(-1)
{
    m_watchDescriptor = ::inotify_add_watch(
        m_monitorDescriptor,
        path.string().c_str(),
        s_changeFlags);

    if (m_watchDescriptor == -1)
    {
        LOGS("Could not add watcher for %s", path);
    }
}

//==============================================================================
PathMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (m_watchDescriptor != -1)
    {
        ::inotify_rm_watch(m_monitorDescriptor, m_watchDescriptor);
        m_watchDescriptor = -1;
    }
}

//==============================================================================
bool PathMonitorImpl::PathInfoImpl::IsValid() const noexcept
{
    return m_watchDescriptor != -1;
}

} // namespace fly
