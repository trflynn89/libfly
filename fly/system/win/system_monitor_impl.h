#pragma once

#include <Windows.h>
#include <Pdh.h>

#include "fly/fly.h"
#include "fly/system/system_monitor.h"

namespace fly {

DEFINE_CLASS_PTRS(SystemMonitorImpl);

/**
 * Windows implementation of the SystemMonitor interface. Uses the PDH and PSAPI
 * libraries to gather system resource information.
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
     * Check if the system monitor's CPU query was successfully created.
     *
     * @return bool True if the CPU query is valid.
     */
    virtual bool IsValid() const;

    virtual void UpdateCpuUsage();
    virtual void UpdateMemoryUsage();
    virtual void Close();

private:
    PDH_HQUERY m_cpuQuery;
    PDH_HCOUNTER m_cpuCounter;
};

}
