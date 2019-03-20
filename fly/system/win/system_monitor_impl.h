#pragma once

#include "fly/system/system_monitor.h"

#include <Pdh.h>
#include <Windows.h>

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

/**
 * Windows implementation of the SystemMonitor interface. Uses the Windows, PDH,
 * and PSAPI libraries to gather system resource information.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 17, 2017
 */
class SystemMonitorImpl : public SystemMonitor
{
public:
    /**
     * Constructor. Open the system monitor's CPU query and set the system CPU
     * count.
     */
    SystemMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SystemConfig> &) noexcept;

    /**
     * Destructor. Close the system monitor's CPU query.
     */
    ~SystemMonitorImpl() override;

protected:
    void UpdateSystemCpuCount() noexcept override;
    void UpdateSystemCpuUsage() noexcept override;
    void UpdateProcessCpuUsage() noexcept override;

    void UpdateSystemMemoryUsage() noexcept override;
    void UpdateProcessMemoryUsage() noexcept override;

private:
    HANDLE m_process;

    PDH_HQUERY m_cpuQuery;
    PDH_HCOUNTER m_cpuCounter;

    ULONGLONG m_prevProcessSystemTime;
    ULONGLONG m_prevProcessUserTime;
    ULONGLONG m_prevTime;
};

} // namespace fly
