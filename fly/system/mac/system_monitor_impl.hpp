#pragma once

#include "fly/system/system_monitor.hpp"

#include <chrono>
#include <cstdint>

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

/**
 * macOS implementation of the SystemMonitor interface. Uses the Mach kernel API to gather system
 * resource information.
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

private:
    std::uint64_t m_prev_system_user_time {0};
    std::uint64_t m_prev_system_system_time {0};
    std::uint64_t m_prev_system_idle_time {0};
    std::uint64_t m_prev_system_nice_time {0};

    std::uint64_t m_prev_process_user_time {0};
    std::uint64_t m_prev_process_system_time {0};
    std::chrono::high_resolution_clock::time_point m_prev_time;
};

} // namespace fly
