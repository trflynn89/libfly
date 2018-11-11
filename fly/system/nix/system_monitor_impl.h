#pragma once

#include <cstdint>
#include <ctime>

#include "fly/system/system_monitor.h"

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

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
    /**
     * Constructor. Set the system CPU count.
     */
    SystemMonitorImpl(
        const std::shared_ptr<SequencedTaskRunner> &,
        const std::shared_ptr<SystemConfig> &
    );

protected:
    void UpdateSystemCpuCount() override;
    void UpdateSystemCpuUsage() override;
    void UpdateProcessCpuUsage() override;

    void UpdateSystemMemoryUsage() override;
    void UpdateProcessMemoryUsage() override;

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
