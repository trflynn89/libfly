#include "nix_file_monitor.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
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
    std::lock_guard<std::mutex> lock(m_mutex);
    return (m_monitorDescriptor != -1);
}

//==============================================================================
bool FileMonitorImpl::AddFile(
    const std::string &path,
    const std::string &file,
    FileEventCallback callback
)
{
    if (callback == nullptr)
    {
        LOGW(-1, "Ignoring NULL callback for \"%s\"", path);
        return false;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    PathMonitor &monitor = m_monitoredPaths[path];

    if (monitor.m_watchDescriptor == -1)
    {
        monitor.m_watchDescriptor = ::inotify_add_watch(
            m_monitorDescriptor, path.c_str(), s_changeFlags
        );

        if (monitor.m_watchDescriptor == -1)
        {
            LOGW(-1, "Could not add watcher for \"%s\": %s",
                path, fly::System::GetLastError()
            );

            return false;
        }
    }

    LOGD(-1, "Watching for changes to \"%s\" in \"%s\"", file, path);
    monitor.m_handlers[file] = callback;

    return true;
}

//==============================================================================
bool FileMonitorImpl::RemoveFile(const std::string &path, const std::string &file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_monitoredPaths.find(path);

    if (it != m_monitoredPaths.end())
    {
        PathMonitor &monitor = it->second;

        auto it2 = monitor.m_handlers.find(file);

        if (it2 != monitor.m_handlers.end())
        {
            LOGD(-1, "Stopped watching for changes to \"%s\" in \"%s\"", file, path);
            monitor.m_handlers.erase(it2);

            if (monitor.m_handlers.empty())
            {
                ::inotify_rm_watch(m_monitorDescriptor, monitor.m_watchDescriptor);

                LOGI(-1, "Removed watcher for \"%s\"", path);
                m_monitoredPaths.erase(it);
            }

            return true;
        }
    }

    LOGW(-1, "Not watching for changes to \"%s\" in \"%s\"", file, path);
    return false;
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
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_monitorDescriptor != -1)
    {
        for (auto &value : m_monitoredPaths)
        {
            PathMonitor &monitor = value.second;

            if (monitor.m_watchDescriptor != -1)
            {
                ::inotify_rm_watch(m_monitorDescriptor, monitor.m_watchDescriptor);

                LOGI(-1, "Removed watcher for \"%s\"", value.first);
                monitor.m_watchDescriptor = -1;
            }

            monitor.m_handlers.clear();
        }

        ::close(m_monitorDescriptor);
        m_monitorDescriptor = -1;
    }
}

//==============================================================================
bool FileMonitorImpl::readEvents()
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
void FileMonitorImpl::handleEvent(const struct inotify_event *pEvent)
{
    auto it = std::find_if(m_monitoredPaths.begin(), m_monitoredPaths.end(),
        [&pEvent](const PathMap::value_type &value) -> bool
        {
            return (value.second.m_watchDescriptor == pEvent->wd);
        }
    );

    if (it != m_monitoredPaths.end())
    {
        FileEventCallback &callback = it->second.m_handlers[pEvent->name];
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

}
