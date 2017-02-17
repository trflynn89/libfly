#include "fly/file/nix/file_monitor_impl.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <poll.h>
#include <unistd.h>

#include "fly/logger/logger.h"
#include "fly/system/system.h"

namespace fly {

namespace
{
    static const int s_initFlags = (
        IN_NONBLOCK
    );

    static const int s_changeFlags = (
        IN_CREATE |
        IN_DELETE |
        IN_MOVED_TO |
        IN_MOVED_FROM |
        IN_MODIFY
    );
}

//==============================================================================
FileMonitorImpl::FileMonitorImpl() :
    FileMonitor(),
    m_monitorDescriptor(::inotify_init1(s_initFlags))
{
    if (m_monitorDescriptor == -1)
    {
        LOGW(-1, "Could not initialize monitor: %s", fly::System::GetLastError());
    }
}

//==============================================================================
FileMonitorImpl::~FileMonitorImpl()
{
    Close();
}

//==============================================================================
bool FileMonitorImpl::IsValid() const
{
    return (m_monitorDescriptor != -1);
}

//==============================================================================
FileMonitor::PathInfoPtr FileMonitorImpl::CreatePathInfo(const std::string &path) const
{
    FileMonitor::PathInfoPtr spInfo;

    if (IsValid())
    {
        spInfo = std::make_shared<PathInfoImpl>(m_monitorDescriptor, path);
    }

    return spInfo;
}

//==============================================================================
void FileMonitorImpl::Poll(const std::chrono::milliseconds &timeout)
{
    struct pollfd pollFd;

    pollFd.fd = m_monitorDescriptor;
    pollFd.events = POLLIN;

    int numEvents = ::poll(&pollFd, 1, timeout.count());

    if (numEvents == -1)
    {
        LOGW(-1, "Could not create poller: %s", fly::System::GetLastError());
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
void FileMonitorImpl::Close()
{
    if (m_monitorDescriptor != -1)
    {
        ::close(m_monitorDescriptor);
        m_monitorDescriptor = -1;
    }
}

//==============================================================================
bool FileMonitorImpl::readEvents() const
{
    static const size_t eventSize = sizeof(struct inotify_event);

    // Some systems cannot read integer variables if they are not properly
    // aligned. On other systems, incorrect alignment may decrease performance.
    // Hence, the buffer used for reading from the inotify file descriptor
    // should have the same alignment as struct inotify_event.
    char buff[8 << 10] __attribute__ ((aligned(__alignof__(struct inotify_event))));

    ssize_t len = ::read(m_monitorDescriptor, buff, sizeof(buff));

    if (len <= 0)
    {
        if (len == -1)
        {
            int error = 0;
            std::string errorStr = fly::System::GetLastError(&error);

            if (error != EAGAIN)
            {
                LOGW(-1, "Could not read polled event: %s", errorStr);
            }
        }
    }
    else
    {
        const struct inotify_event *pEvent;

        for (char *ptr = buff; ptr < buff + len; ptr += eventSize + pEvent->len)
        {
            pEvent = (struct inotify_event *)ptr;

            if (pEvent->len > 0)
            {
                handleEvent(pEvent);
            }
        }
    }

    return (len > 0);
}

//==============================================================================
void FileMonitorImpl::handleEvent(const struct inotify_event *pEvent) const
{
    auto it = std::find_if(m_pathInfo.begin(), m_pathInfo.end(),
        [&pEvent](const PathInfoMap::value_type &value) -> bool
        {
            PathInfoImplPtr spInfo(DownCast<PathInfoImpl>(value.second));
            return (spInfo->m_watchDescriptor == pEvent->wd);
        }
    );

    if (it != m_pathInfo.end())
    {
        PathInfoImplPtr spInfo(DownCast<PathInfoImpl>(it->second));

        FileMonitor::FileEventCallback &callback = spInfo->m_handlers[pEvent->name];
        FileMonitor::FileEvent event = convertToEvent(pEvent->mask);

        if ((callback != nullptr) && (event != FileMonitor::FILE_NO_CHANGE))
        {
            LOGI(-1, "Handling event %d for \"%s\" in \"%s\"",
                event, pEvent->name, it->first);

            callback(it->first, pEvent->name, event);
        }
    }
}

//==============================================================================
FileMonitor::FileEvent FileMonitorImpl::convertToEvent(int mask) const
{
    FileMonitor::FileEvent event = FileMonitor::FILE_NO_CHANGE;

    if ((mask & IN_CREATE) || (mask & IN_MOVED_TO))
    {
        event = FileMonitor::FILE_CREATED;
    }
    else if ((mask & IN_DELETE) || (mask & IN_MOVED_FROM))
    {
        event = FileMonitor::FILE_DELETED;
    }
    else if (mask & IN_MODIFY)
    {
        event = FileMonitor::FILE_CHANGED;
    }

    return event;
}

//==============================================================================
FileMonitorImpl::PathInfoImpl::PathInfoImpl(
    int monitorDescriptor,
    const std::string &path
) :
    FileMonitorImpl::PathInfo(),
    m_monitorDescriptor(monitorDescriptor),
    m_watchDescriptor(-1)
{
    m_watchDescriptor = ::inotify_add_watch(
        m_monitorDescriptor, path.c_str(), s_changeFlags
    );

    if (m_watchDescriptor == -1)
    {
        LOGW(-1, "Could not add watcher for \"%s\": %s",
            path, fly::System::GetLastError()
        );
    }
}

//==============================================================================
FileMonitorImpl::PathInfoImpl::~PathInfoImpl()
{
    if (m_watchDescriptor != -1)
    {
        ::inotify_rm_watch(m_monitorDescriptor, m_watchDescriptor);
        m_watchDescriptor = -1;
    }
}

//==============================================================================
bool FileMonitorImpl::PathInfoImpl::IsValid() const
{
    return (m_watchDescriptor != -1);
}

}
