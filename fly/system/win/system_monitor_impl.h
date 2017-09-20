#pragma once

#include <cstdint>

#include <Windows.h>
#include <Pdh.h>

#include "fly/fly.h"
#include "fly/system/system_monitor.h"

namespace fly {

DEFINE_CLASS_PTRS(SystemMonitorImpl);

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
    SystemMonitorImpl();
    virtual ~SystemMonitorImpl();

protected:
    /**
     * Check if the system monitor's CPU query was successfully created, and if
     * the CPU scaling factor was successfully set.
     *
     * @return bool True if the CPU query and scaling factor are valid.
     */
    virtual bool IsValid() const;

    virtual void UpdateSystemCpuUsage();
    virtual void UpdateProcessCpuUsage();

    virtual void UpdateSystemMemoryUsage();
    virtual void UpdateProcessMemoryUsage();

    virtual void Close();

private:
    /**
     * @return DWORD Number of CPUs on the system.
     */
    DWORD getCpuCount() const;

    HANDLE m_process;

    PDH_HQUERY m_cpuQuery;
    PDH_HCOUNTER m_cpuCounter;

    uint64_t m_currCpuTicks;
    uint64_t m_prevCpuTicks;

    uint64_t m_currTime;
    uint64_t m_prevTime;

    double m_scale;
};

}
