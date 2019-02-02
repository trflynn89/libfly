#pragma once

#include "fly/path/path_monitor.h"

#include <Windows.h>

#include <chrono>
#include <memory>
#include <string>

namespace fly {

class PathConfig;
class SequencedTaskRunner;

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
    /**
     * Constructor. Create the path monitor's IOCP.
     */
    PathMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<PathConfig> &);

    /**
     * Destructor. Close the path monitor's IOCP.
     */
    ~PathMonitorImpl() override;

protected:
    /**
     * Check if the path monitor's IOCP was successfully created.
     *
     * @return bool True if the IOCP is valid.
     */
    bool IsValid() const override;

    /**
     * Check if the path monitor's IOCP has any posted completions, and handle
     * any that have been posted.
     *
     * @param milliseconds Max time allow for a completion to be posted.
     */
    void Poll(const std::chrono::milliseconds &) override;

    std::shared_ptr<PathMonitor::PathInfo>
    CreatePathInfo(const std::string &) const override;

private:
    /**
     * Windows implementation of the PathInfo interface. Stores a handle to the
     * monitored path, as well as an array to store changes found by the
     * ReadDirectoryChangesW API for the monitored path.
     */
    struct PathInfoImpl : public PathMonitor::PathInfo
    {
        PathInfoImpl(HANDLE, const std::string &);
        ~PathInfoImpl() override;

        /**
         * @return bool True if initialization was sucessful.
         */
        bool IsValid() const override;

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
     * @param PathInfoImpl The path's entry in the PathInfo map.
     * @param string Name of the path.
     */
    void handleEvents(
        const std::shared_ptr<PathInfoImpl> &,
        const std::string &) const;

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

} // namespace fly
