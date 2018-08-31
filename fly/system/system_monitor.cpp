#include "fly/system/system_monitor.h"

#include <chrono>
#include <thread>

#include "fly/system/system_config.h"
#include "fly/task/task_runner.h"

namespace fly {

//==============================================================================
SystemMonitor::SystemMonitor(
    const TaskRunnerPtr &spTaskRunner,
    const SystemConfigPtr &spConfig
) :
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
bool SystemMonitor::Start()
{
    if (isValid())
    {
        SystemMonitorPtr spSystemMonitor = shared_from_this();

        m_spTask = std::make_shared<SystemMonitorTask>(spSystemMonitor);
        m_spTaskRunner->PostTask(m_spTask);

        return true;
    }

    return false;
}

//==============================================================================
uint32_t SystemMonitor::GetSystemCpuCount() const
{
    return m_systemCpuCount.load();
}

//==============================================================================
double SystemMonitor::GetSystemCpuUsage() const
{
    return m_systemCpuUsage.load();
}

//==============================================================================
double SystemMonitor::GetProcessCpuUsage() const
{
    return m_processCpuUsage.load();
}

//==============================================================================
uint64_t SystemMonitor::GetTotalSystemMemory() const
{
    return m_totalSystemMemory.load();
}

//==============================================================================
uint64_t SystemMonitor::GetSystemMemoryUsage() const
{
    return m_systemMemoryUsage.load();
}

//==============================================================================
uint64_t SystemMonitor::GetProcessMemoryUsage() const
{
    return m_processMemoryUsage.load();
}

//==============================================================================
bool SystemMonitor::isValid() const
{
    return (GetSystemCpuCount() > 0);
}

//==============================================================================
void SystemMonitor::poll()
{
    UpdateSystemCpuCount();
    UpdateSystemCpuUsage();
    UpdateProcessCpuUsage();

    UpdateSystemMemoryUsage();
    UpdateProcessMemoryUsage();
}

//==============================================================================
SystemMonitorTask::SystemMonitorTask(const SystemMonitorWPtr &wpSystemMonitor) :
    Task(),
    m_wpSystemMonitor(wpSystemMonitor)
{
}

//==============================================================================
void SystemMonitorTask::Run()
{
    SystemMonitorPtr spSystemMonitor = m_wpSystemMonitor.lock();

    if (spSystemMonitor && spSystemMonitor->isValid())
    {
        spSystemMonitor->poll();

        if (spSystemMonitor->isValid())
        {
            spSystemMonitor->m_spTaskRunner->PostTaskWithDelay(
                spSystemMonitor->m_spTask,
                spSystemMonitor->m_spConfig->PollInterval()
            );
        }
    }
}

}
