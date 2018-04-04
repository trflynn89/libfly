#pragma once

#include <cstdint>
#include <ctime>

#include "fly/fly.h"
#include "fly/system/system_monitor.h"

namespace fly {

FLY_CLASS_PTRS(ConfigManager);
FLY_CLASS_PTRS(SystemMonitorImpl);

/**
 * Linux implementation of the SystemMonitor interface. Uses the /proc file
 * system to gather system resource information.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version September 15, 2017
 */
class SystemMonitorImpl : public SystemMonitor
{
public:
    SystemMonitorImpl();
    SystemMonitorImpl(ConfigManagerPtr &);
    virtual ~SystemMonitorImpl();

protected:
    /**
     * Set the system CPU count.
     */
    virtual void StartMonitor();

    /**
     * Does nothing.
     */
    virtual void StopMonitor();

    /**
     * Check if the system CPU count was successfully set.
     *
     * @return bool True if the CPU count is valid.
     */
    virtual bool IsValid() const;

    virtual void UpdateSystemCpuCount();
    virtual void UpdateSystemCpuUsage();
    virtual void UpdateProcessCpuUsage();

    virtual void UpdateSystemMemoryUsage();
    virtual void UpdateProcessMemoryUsage();

private:
    uint64_t m_prevSystemUserTime;
    uint64_t m_prevSystemNiceTime;
    uint64_t m_prevSystemSystemTime;
    uint64_t m_prevSystemIdleTime;

    clock_t m_prevProcessSystemTime;
    clock_t m_prevProcessUserTime;
    clock_t m_prevTime;
};

}
