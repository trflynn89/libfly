#pragma once

#include "fly/system/system_monitor.hpp"

#include <cstdint>
#include <ctime>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly {

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
     * Constructor.
     */
    SystemMonitorImpl(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<SystemConfig> config) noexcept;

protected:
    void update_system_cpu_count() override;
    void update_system_cpu_usage() override;
    void update_process_cpu_usage() override;

    void update_system_memory_usage() override;
    void update_process_memory_usage() override;

private:
    std::uint64_t m_prev_system_user_time {0};
    std::uint64_t m_prev_system_nice_time {0};
    std::uint64_t m_prev_system_system_time {0};
    std::uint64_t m_prev_system_idle_time {0};

    clock_t m_prev_process_system_time {0};
    clock_t m_prev_process_user_time {0};
    clock_t m_prev_time {0};
};

} // namespace fly
