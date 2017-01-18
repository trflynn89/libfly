#pragma once

#include <chrono>
#include <map>
#include <mutex>
#include <string>

#include <Windows.h>

#include <fly/fly.h>
#include <fly/file/file_monitor.h>

namespace fly {

/**
 * Windows implementation of the FileMonitor interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 21, 2016
 */
class FileMonitorImpl : public FileMonitor
{
    /**
     * Information pertaining to a monitored path.
     */
    struct PathMonitor
    {
        PathMonitor() : m_handle(INVALID_HANDLE_VALUE), m_pInfo(NULL)
        {
            ::memset(&m_overlapped, 0, sizeof(m_overlapped));
        }

        virtual ~PathMonitor()
        {
            if (m_pInfo != NULL)
            {
                delete[] m_pInfo;
                m_pInfo = NULL;
            }

            if (m_handle != INVALID_HANDLE_VALUE)
            {
                CancelIo(m_handle);
                CloseHandle(m_handle);
                m_handle = INVALID_HANDLE_VALUE;
            }
        }

        std::map<std::string, FileEventCallback> m_handlers;

        HANDLE m_handle;
        OVERLAPPED m_overlapped;
        PFILE_NOTIFY_INFORMATION m_pInfo;
    };

    /**
     * Map of monitored path names to their monitor information.
     */
    typedef std::map<std::string, PathMonitor> PathMap;

public:
    FileMonitorImpl();
    virtual ~FileMonitorImpl();

    virtual bool IsValid() const;

    virtual bool AddFile(const std::string &, const std::string &, FileEventCallback);
    virtual bool RemoveFile(const std::string &, const std::string &);

protected:
    virtual void Poll(const std::chrono::milliseconds &);
    virtual void Close();

private:
    /**
     * Handle a FILE_NOTIFY_INFORMATION event for a path.
     *
     * @param PathMap::value_type The path's entry in the PathMap.
     */
    void handleEvents(PathMap::value_type &);

    /**
     * Convert a FILE_NOTIFY_INFORMATION event to a FileEvent.
     *
     * @param int The FILE_NOTIFY_INFORMATION event.
     *
     * @return FileEvent A FileEvent that matches the given.
     */
    FileMonitor::FileEvent convertToEvent(DWORD);

    mutable std::mutex m_mutex;
    PathMap m_monitoredPaths;
    HANDLE m_iocp;
};

}
