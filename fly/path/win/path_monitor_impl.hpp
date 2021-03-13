#pragma once

#include "fly/path/path_monitor.hpp"

#include <Windows.h>

#include <array>
#include <chrono>
#include <filesystem>
#include <memory>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly {

class PathConfig;

/**
 * Windows implementation of the PathMonitor interface. Uses the ReadDirectoryChangesW API to detect
 * path changes.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 19, 2017
 */
class PathMonitorImpl : public PathMonitor
{
public:
    /**
     * Constructor.
     */
    PathMonitorImpl(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<PathConfig> config) noexcept;

protected:
    /**
     * @return True.
     */
    bool is_valid() const override;

    /**
     * Check the overlapped result of all monitored paths and handle any that have been posted.
     *
     * @param timeout Max time allowed to wait for a completion to be posted.
     */
    void poll(std::chrono::milliseconds timeout) override;

    std::unique_ptr<PathMonitor::PathInfo>
    create_path_info(const std::filesystem::path &path) const override;

private:
    /**
     * Windows implementation of the PathInfo interface. Stores a handle to the monitored path, as
     * well as an array to store changes found by the ReadDirectoryChangesW API for the monitored
     * path.
     */
    struct PathInfoImpl : public PathMonitor::PathInfo
    {
        explicit PathInfoImpl(const std::filesystem::path &path) noexcept;
        ~PathInfoImpl() override;

        /**
         * @return True if initialization was sucessful.
         */
        bool is_valid() const override;

        /**
         * Call the ReadDirectoryChangesW API for this path. Should be called after initialization
         * and each time an overlapped completion occurs for this path.
         *
         * @param path Name of the monitored path.
         */
        bool refresh(const std::filesystem::path &path);

        bool m_valid {false};
        HANDLE m_handle {INVALID_HANDLE_VALUE};
        OVERLAPPED m_overlapped {};
        std::array<FILE_NOTIFY_INFORMATION, 128> m_file_info;
    };

    /**
     * Handle a FILE_NOTIFY_INFORMATION event for a path.
     *
     * @param info The path's entry in the PathInfo map.
     * @param path Name of the path.
     */
    void handle_events(const PathInfoImpl *info, const std::filesystem::path &path) const;

    /**
     * Convert a FILE_NOTIFY_INFORMATION event to a PathEvent.
     *
     * @param action The FILE_NOTIFY_INFORMATION event.
     *
     * @return A PathEvent that matches the given event.
     */
    PathMonitor::PathEvent convert_to_event(DWORD action) const;
};

} // namespace fly
