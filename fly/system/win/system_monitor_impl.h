#pragma once

#include <Windows.h>
#include <Pdh.h>

#include "fly/fly.h"
#include "fly/system/system_monitor.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);
FLY_CLASS_PTRS(SystemMonitorImpl);

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
    SystemMonitorImpl(ConfigManagerPtr &);
    virtual ~SystemMonitorImpl();

protected:
    /**
     * Open the system monitor's CPU query and set the system CPU count.
     */
    virtual void StartMonitor();

    /**
     * Close the system monitor's CPU query.
     */
    virtual void StopMonitor();

    /**
     * Check if the system monitor's CPU query was successfully created, and if
     * the system CPU count was successfully set.
     *
     * @return bool True if the CPU query and count are valid.
     */
    virtual bool IsValid() const;

    virtual void UpdateSystemCpuCount();
    virtual void UpdateSystemCpuUsage();
    virtual void UpdateProcessCpuUsage();

    virtual void UpdateSystemMemoryUsage();
    virtual void UpdateProcessMemoryUsage();

private:
    HANDLE m_process;

    PDH_HQUERY m_cpuQuery;
    PDH_HCOUNTER m_cpuCounter;

    ULONGLONG m_prevProcessSystemTime;
    ULONGLONG m_prevProcessUserTime;
    ULONGLONG m_prevTime;
};

}
