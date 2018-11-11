#pragma once

#include <Windows.h>
#include <Pdh.h>

#include "fly/system/system_monitor.h"

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
        const std::shared_ptr<SystemConfig> &
    );

    /**
     * Destructor. Close the system monitor's CPU query.
     */
    ~SystemMonitorImpl() override;

protected:
    void UpdateSystemCpuCount() override;
    void UpdateSystemCpuUsage() override;
    void UpdateProcessCpuUsage() override;

    void UpdateSystemMemoryUsage() override;
    void UpdateProcessMemoryUsage() override;

private:
    HANDLE m_process;

    PDH_HQUERY m_cpuQuery;
    PDH_HCOUNTER m_cpuCounter;

    ULONGLONG m_prevProcessSystemTime;
    ULONGLONG m_prevProcessUserTime;
    ULONGLONG m_prevTime;
};

}
