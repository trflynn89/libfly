#pragma once

#include "fly/system/system_monitor.hpp"

#include <cstdint>
#include <ctime>

namespace fly {

class SequencedTaskRunner;
class SystemConfig;

/**
 * Linux implementation of the SystemMonitor interface. Uses the /proc file
 * system to gather system resource information.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
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
        const std::shared_ptr<SystemConfig> &) noexcept;

protected:
    void UpdateSystemCpuCount() noexcept override;
    void UpdateSystemCpuUsage() noexcept override;
    void UpdateProcessCpuUsage() noexcept override;

    void UpdateSystemMemoryUsage() noexcept override;
    void UpdateProcessMemoryUsage() noexcept override;

private:
    std::uint64_t m_prevSystemUserTime;
    std::uint64_t m_prevSystemNiceTime;
    std::uint64_t m_prevSystemSystemTime;
    std::uint64_t m_prevSystemIdleTime;

    clock_t m_prevProcessSystemTime;
    clock_t m_prevProcessUserTime;
    clock_t m_prevTime;
};

} // namespace fly
