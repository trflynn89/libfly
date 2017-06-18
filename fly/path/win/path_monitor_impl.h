#pragma once

#include <chrono>
#include <string>

#include <Windows.h>

#include "fly/fly.h"
#include "fly/path/path_monitor.h"

namespace fly {

DEFINE_CLASS_PTRS(ConfigManager);
DEFINE_CLASS_PTRS(PathMonitorImpl);

/**
 * Windows implementation of the PathMonitor interface. Uses an IOCP with the
 * ReadDirectoryChangesW API to detect path changes.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version January 19, 2017
 */
class PathMonitorImpl : public PathMonitor
{
public:
    PathMonitorImpl();
    PathMonitorImpl(ConfigManagerPtr &);
    virtual ~PathMonitorImpl();

    /**
     * Check if the path monitor's IOCP was successfully created.
     *
     * @return bool True if the IOCP is valid.
     */
    virtual bool IsValid() const;

protected:
    virtual PathMonitor::PathInfoPtr CreatePathInfo(const std::string &) const;
    virtual void Poll(const std::chrono::milliseconds &);
    virtual void Close();

private:
    DEFINE_STRUCT_PTRS(PathInfoImpl);

    /**
     * Windows implementation of the PathInfo interface. Stores a handle to the
     * monitored path, as well as an array to store changes found by the
     * ReadDirectoryChangesW API for the monitored path.
     */
    struct PathInfoImpl : public PathMonitor::PathInfo
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
     * @param PathInfoImplPtr The path's entry in the PathInfo map.
     * @param string Name of the path.
     */
    void handleEvents(const PathInfoImplPtr &, const std::string &) const;

    /**
     * Convert a FILE_NOTIFY_INFORMATION event to a PathEvent.
     *
     * @param int The FILE_NOTIFY_INFORMATION event.
     *
     * @return PathEvent A PathEvent that matches the given event.
     */
    PathMonitor::PathEvent convertToEvent(DWORD) const;

    HANDLE m_iocp;
};

}
