#pragma once

#include "fly/system/system_monitor.hpp"

#include <Pdh.h>
#include <Windows.h>

namespace fly::task {
class SequencedTaskRunner;
} // namespace fly::task

namespace fly {

class SystemConfig;

/**
 * Windows implementation of the SystemMonitor interface. Uses the Windows, PDH, and PSAPI libraries
 * to gather system resource information.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 17, 2017
 */
class SystemMonitorImpl : public SystemMonitor
{
public:
    /**
     * Constructor. Open the system monitor's CPU query.
     */
    SystemMonitorImpl(
        std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
        std::shared_ptr<SystemConfig> config) noexcept;

    /**
     * Destructor. Close the system monitor's CPU query.
     */
    ~SystemMonitorImpl() override;

protected:
    void update_system_cpu_count() override;
    void update_system_cpu_usage() override;
    void update_process_cpu_usage() override;

    void update_system_memory_usage() override;
    void update_process_memory_usage() override;

private:
    HANDLE m_process;

    PDH_HQUERY m_cpu_query;
    PDH_HCOUNTER m_cpu_counter;

    ULONGLONG m_prev_process_system_time {0};
    ULONGLONG m_prev_process_user_time {0};
    ULONGLONG m_prev_time {0};
};

} // namespace fly
