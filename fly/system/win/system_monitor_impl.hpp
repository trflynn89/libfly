#pragma once

#include "fly/system/system_monitor.hpp"

#include <Pdh.h>
#include <Windows.h>

namespace fly {

class SequencedTaskRunner;
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
     * Constructor. Open the system monitor's CPU query and set the system CPU count.
     */
    SystemMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &task_runner,
        const std::shared_ptr<SystemConfig> &config) noexcept;

    /**
     * Destructor. Close the system monitor's CPU query.
     */
    ~SystemMonitorImpl() override;

protected:
    void update_system_cpu_count() noexcept override;
    void update_system_cpu_usage() noexcept override;
    void update_process_cpu_usage() noexcept override;

    void update_system_memory_usage() noexcept override;
    void update_process_memory_usage() noexcept override;

private:
    HANDLE m_process;

    PDH_HQUERY m_cpu_query;
    PDH_HCOUNTER m_cpu_counter;

    ULONGLONG m_prev_process_system_time;
    ULONGLONG m_prev_process_user_time;
    ULONGLONG m_prev_time;
};

} // namespace fly
