#pragma once

#include <chrono>
#include <map>
#include <string>

#include <Windows.h>

#include "fly/fly.h"
#include "fly/file/file_monitor.h"

namespace fly {

DEFINE_CLASS_PTRS(FileMonitorImpl);

/**
 * Windows implementation of the FileMonitor interface. Uses an IOCP with the
 * ReadDirectoryChangesW API to detect path changes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version January 19, 2017
 */
class FileMonitorImpl : public FileMonitor
{
public:
    FileMonitorImpl();
    virtual ~FileMonitorImpl();

    /**
     * Check if the file monitor's IOCP was successfully created.
     *
     * @return bool True if the IOCP is valid.
     */
    virtual bool IsValid() const;

protected:
    virtual FileMonitor::PathInfoPtr CreatePathInfo(const std::string &) const;
    virtual void Poll(const std::chrono::milliseconds &);
    virtual void Close();

private:
    DEFINE_STRUCT_PTRS(PathInfoImpl);

    /**
     * Windows implementation of the PathInfo interface. Stores a file handle
     * to the monitored path, as well as an array to store changes found by
     * the ReadDirectoryChangesW API for the monitored path.
     */
    struct PathInfoImpl : public FileMonitor::PathInfo
    {
        PathInfoImpl(HANDLE, const std::string &);
        virtual ~PathInfoImpl();

        /**
         * @return bool True if initialization was sucessful.
         */
        virtual bool IsValid() const;

        /**
         * Call the ReadDirectoryChangesW API for this path. Should be called
         * after initialization and each time an IOCP completion occurs for
         * this path.
         *
         * @param string Name of the monitored path.
         */
        bool Refresh(const std::string &);

        bool m_valid;
        HANDLE m_handle;
        OVERLAPPED m_overlapped;
        PFILE_NOTIFY_INFORMATION m_pInfo;
    };

    /**
     * Handle a FILE_NOTIFY_INFORMATION event for a path.
     *
     * @param PathInfoMap::value_type The path's entry in the PathInfoMap.
     * @param string Name of the path.
     */
    void handleEvents(const PathInfoImplPtr &, const std::string &) const;

    /**
     * Convert a FILE_NOTIFY_INFORMATION event to a FileEvent.
     *
     * @param int The FILE_NOTIFY_INFORMATION event.
     *
     * @return FileEvent A FileEvent that matches the given event.
     */
    FileMonitor::FileEvent convertToEvent(DWORD) const;

    HANDLE m_iocp;
};

}
