#include "fly/system/system_monitor.hpp"

#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#include <chrono>
#include <thread>

namespace fly {

//==============================================================================
SystemMonitor::SystemMonitor(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<SystemConfig> &spConfig) noexcept :
    m_systemCpuCount(0),
    m_systemCpuUsage(0.0),
    m_processCpuUsage(0.0),
    m_totalSystemMemory(0),
    m_systemMemoryUsage(0),
    m_processMemoryUsage(0),
    m_spTaskRunner(spTaskRunner),
    m_spConfig(spConfig)
{
}

//==============================================================================
bool SystemMonitor::Start() noexcept
{
    if (isValid())
    {
        std::shared_ptr<SystemMonitor> spSystemMonitor = shared_from_this();

        m_spTask = std::make_shared<SystemMonitorTask>(spSystemMonitor);
        m_spTaskRunner->PostTask(m_spTask);

        return true;
    }

    return false;
}

//==============================================================================
std::uint32_t SystemMonitor::GetSystemCpuCount() const noexcept
{
    return m_systemCpuCount.load();
}

//==============================================================================
double SystemMonitor::GetSystemCpuUsage() const noexcept
{
    return m_systemCpuUsage.load();
}

//==============================================================================
double SystemMonitor::GetProcessCpuUsage() const noexcept
{
    return m_processCpuUsage.load();
}

//==============================================================================
std::uint64_t SystemMonitor::GetTotalSystemMemory() const noexcept
{
    return m_totalSystemMemory.load();
}

//==============================================================================
std::uint64_t SystemMonitor::GetSystemMemoryUsage() const noexcept
{
    return m_systemMemoryUsage.load();
}

//==============================================================================
std::uint64_t SystemMonitor::GetProcessMemoryUsage() const noexcept
{
    return m_processMemoryUsage.load();
}

//==============================================================================
bool SystemMonitor::isValid() const noexcept
{
    return GetSystemCpuCount() > 0;
}

//==============================================================================
void SystemMonitor::poll() noexcept
{
    UpdateSystemCpuCount();
    UpdateSystemCpuUsage();
    UpdateProcessCpuUsage();

    UpdateSystemMemoryUsage();
    UpdateProcessMemoryUsage();
}

//==============================================================================
SystemMonitorTask::SystemMonitorTask(
    std::weak_ptr<SystemMonitor> wpSystemMonitor) noexcept :
    Task(),
    m_wpSystemMonitor(wpSystemMonitor)
{
}

//==============================================================================
void SystemMonitorTask::Run() noexcept
{
    std::shared_ptr<SystemMonitor> spSystemMonitor = m_wpSystemMonitor.lock();

    if (spSystemMonitor && spSystemMonitor->isValid())
    {
        spSystemMonitor->poll();

        if (spSystemMonitor->isValid())
        {
            spSystemMonitor->m_spTaskRunner->PostTaskWithDelay(
                spSystemMonitor->m_spTask,
                spSystemMonitor->m_spConfig->PollInterval());
        }
    }
}

} // namespace fly
