#pragma once

#include "fly/path/path_monitor.hpp"

#include <chrono>
#include <filesystem>
#include <memory>

namespace fly {

class PathConfig;
class SequencedTaskRunner;

/**
 * macOS implementation of the PathMonitor interface. Currently an empty implementation.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 15, 2020
 */
class PathMonitorImpl : public PathMonitor
{
public:
    /**
     * Constructor.
     */
    PathMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<PathConfig> &config) noexcept;

protected:
    /**
     * @return True.
     */
    bool is_valid() const override;

    /**
     * Empty implementation.
     *
     * @param timeout Max time allow for an event to be readable.
     */
    void poll(const std::chrono::milliseconds &timeout) override;

    std::unique_ptr<PathMonitor::PathInfo>
    create_path_info(const std::filesystem::path &path) const override;

private:
    /**
     * macOS implementation of the PathInfo interface.
     */
    struct PathInfoImpl : PathMonitor::PathInfo
    {
        /**
         * @return True.
         */
        bool is_valid() const override;
    };
};

} // namespace fly
