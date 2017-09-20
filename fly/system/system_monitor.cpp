#include "fly/system/system_monitor.h"

#include <chrono>

namespace
{
    // TODO make configurable
    static const std::chrono::seconds s_delay(1);
}

namespace fly {

//==============================================================================
SystemMonitor::SystemMonitor() :
    Runner("SystemMonitor", 1),
    m_systemCpuCount(0),
    m_systemCpuUsage(0.0),
    m_processCpuUsage(0.0),
    m_totalSystemMemory(0),
    m_systemMemoryUsage(0),
    m_processMemoryUsage(0)
{
}

//==============================================================================
SystemMonitor::~SystemMonitor()
{
    Stop();
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
bool SystemMonitor::StartRunner()
{
    return IsValid();
}

//==============================================================================
void SystemMonitor::StopRunner()
{
    Close();
}

//==============================================================================
bool SystemMonitor::DoWork()
{
    if (IsValid())
    {
        UpdateSystemCpuCount();

        UpdateSystemCpuUsage();
        UpdateProcessCpuUsage();

        UpdateSystemMemoryUsage();
        UpdateProcessMemoryUsage();

        std::this_thread::sleep_for(s_delay);
    }

    return IsValid();
}

}
