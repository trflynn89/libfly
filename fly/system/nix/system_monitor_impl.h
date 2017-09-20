#pragma once

#include <chrono>
#include <cstdint>

#include "fly/fly.h"
#include "fly/system/system_monitor.h"

namespace fly {

DEFINE_CLASS_PTRS(SystemMonitorImpl);

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
    virtual ~SystemMonitorImpl();

protected:
    /**
     * Check if the system monitor's CPU scaling factor was successfully set.
     *
     * @return bool True if the CPU scaling factor is valid.
     */
    virtual bool IsValid() const;

    virtual void UpdateCpuUsage();
    virtual void UpdateMemoryUsage();
    virtual void Close();

private:
    /**
     * @return Number of CPUs on the system.
     */
    int getCpuCount() const;

    uint64_t m_currCpuTicks;
    uint64_t m_prevCpuTicks;

    std::chrono::seconds m_currTime;
    std::chrono::seconds m_prevTime;

    double m_scale;
};

}
