#include "nix_file_monitor.h"

#include <cstdlib>
#include <cstring>
#include <sys/inotify.h>
#include <unistd.h>

#include <fly/logging/logger.h>
#include <fly/system/system.h>

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
FileMonitorImpl::FileMonitorImpl(
    FileEventCallback handler,
    const std::string &path,
    const std::string &file
) :
    FileMonitor(handler, path, file),
    m_monitorDescriptor(-1),
    m_watchDescriptor(-1)
{
    m_monitorDescriptor = ::inotify_init1(s_initFlags);

    if (m_monitorDescriptor == -1)
    {
        LOGW(-1, "Could not initialize monitor for \"%s\": %s",
            m_path, fly::System::GetLastError()
        );
    }
    else
    {
        m_watchDescriptor = ::inotify_add_watch(
            m_monitorDescriptor, m_path.c_str(), s_changeFlags
        );

        if (m_watchDescriptor == -1)
        {
            LOGW(-1, "Could not add watcher for \"%s\": %s",
                m_path, fly::System::GetLastError()
            );

            close();
        }
    }
}

//==============================================================================
FileMonitorImpl::~FileMonitorImpl()
{
    close();
}

//==============================================================================
bool FileMonitorImpl::IsValid() const
{
    return (m_monitorDescriptor != -1);
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
        LOGW(-1, "Could not create poller for \"%s\": %s",
            m_path, fly::System::GetLastError()
        );

        close();
    }
    else if ((numEvents > 0) && (pollFd.revents & POLLIN))
    {
        while (handleEvents())
        {
        }
    }
}

//==============================================================================
bool FileMonitorImpl::handleEvents()
{
    const struct inotify_event *event;
    size_t eventSize = sizeof(struct inotify_event);

    // Some systems cannot read integer variables if they are not properly
    // aligned. On other systems, incorrect alignment may decrease performance.
    // Hence, the buffer used for reading from the inotify file descriptor
    // should have the same alignment as struct inotify_event.
    char buff[8 << 10] __attribute__ ((aligned(__alignof__(struct inotify_event))));
    char *ptr;

    ssize_t len = ::read(m_monitorDescriptor, buff, sizeof(buff));

    if (len <= 0)
    {
        if (len == -1)
        {
            int error = 0;
            std::string errorStr = fly::System::GetLastError(&error);

            if (error != EAGAIN)
            {
                LOGW(-1, "Could not read polled event for \"%s\": %s", m_path, errorStr);
                close();
            }
        }

        return false;
    }

    for (ptr = buff; ptr < buff + len; ptr += eventSize + event->len)
    {
        event = (const struct inotify_event *)ptr;

        if ((event->len > 0) && (std::string(event->name).compare(m_file) == 0))
        {
            HandleEvent(convertToEvent(event->mask));
        }
    }

    return true;
}

//==============================================================================
FileMonitor::FileEvent FileMonitorImpl::convertToEvent(int mask)
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
void FileMonitorImpl::close()
{
    if (m_monitorDescriptor != -1)
    {
        if (m_watchDescriptor != -1)
        {
            ::inotify_rm_watch(m_monitorDescriptor, m_watchDescriptor);
            m_watchDescriptor = -1;
        }

        ::close(m_monitorDescriptor);
        m_monitorDescriptor = -1;
    }
}

}
