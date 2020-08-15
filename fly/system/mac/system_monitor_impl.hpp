#pragma once

#include "fly/system/system_monitor.hpp"

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

/**
 * macOS implementation of the SystemMonitor interface. Currently an empty implementation.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 15, 2020
 */
class SystemMonitorImpl : public SystemMonitor
{
public:
    /**
     * Constructor.
     */
    SystemMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<SystemConfig> &config) noexcept;

protected:
    void update_system_cpu_count() override;
    void update_system_cpu_usage() override;
    void update_process_cpu_usage() override;

    void update_system_memory_usage() override;
    void update_process_memory_usage() override;
};

} // namespace fly
