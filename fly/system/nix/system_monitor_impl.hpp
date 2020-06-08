#pragma once

#include "fly/system/system_monitor.hpp"

#include <cstdint>
#include <ctime>

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

/**
 * Linux implementation of the SystemMonitor interface. Uses the /proc file system to gather system
 * resource information.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 15, 2017
 */
class SystemMonitorImpl : public SystemMonitor
{
public:
    /**
     * Constructor. Set the system CPU count.
     */
    SystemMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<SystemConfig> &config) noexcept;

protected:
    void update_system_cpu_count() noexcept override;
    void update_system_cpu_usage() noexcept override;
    void update_process_cpu_usage() noexcept override;

    void update_system_memory_usage() noexcept override;
    void update_process_memory_usage() noexcept override;

private:
    std::uint64_t m_prev_system_user_time;
    std::uint64_t m_prev_system_nice_time;
    std::uint64_t m_prev_system_system_time;
    std::uint64_t m_prev_system_idle_time;

    clock_t m_prev_process_system_time;
    clock_t m_prev_process_user_time;
    clock_t m_prev_time;
};

} // namespace fly
