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
    m_cpuUsage(0.0f),
    m_totalMemory(0),
    m_freeMemory(0),
    m_processMemory(0)
{
}

//==============================================================================
SystemMonitor::~SystemMonitor()
{
    Stop();
}

//==============================================================================
float SystemMonitor::GetCpuUsage() const
{
    return (static_cast<float>(m_cpuUsage.load()) / 100.0f);
}

//==============================================================================
uint64_t SystemMonitor::GetTotalMemory() const
{
    return m_totalMemory;
}

//==============================================================================
uint64_t SystemMonitor::GetFreeMemory() const
{
    return m_freeMemory;
}

//==============================================================================
uint64_t SystemMonitor::GetProcessMemory() const
{
    return m_processMemory;
}

//==============================================================================
bool SystemMonitor::StartRunner()
{
    return true;
}

//==============================================================================
void SystemMonitor::StopRunner()
{
}

//==============================================================================
bool SystemMonitor::DoWork()
{
    UpdateCpuUsage();
    UpdateMemoryUsage();

    std::this_thread::sleep_for(s_delay);
    return true;
}

}
