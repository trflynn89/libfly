#include "fly/config/config_manager.h"
#include "fly/system/system_monitor.h"

namespace fly {

//==============================================================================
SystemMonitor::SystemMonitor() :
    Monitor("SystemMonitor"),
    m_systemCpuCount(0),
    m_systemCpuUsage(0.0),
    m_processCpuUsage(0.0),
    m_totalSystemMemory(0),
    m_systemMemoryUsage(0),
    m_processMemoryUsage(0)
{
}

//==============================================================================
SystemMonitor::SystemMonitor(ConfigManagerPtr &spConfigManager) :
    Monitor("SystemMonitor", spConfigManager),
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
void SystemMonitor::Poll(const std::chrono::milliseconds &delay)
{
    UpdateSystemCpuCount();
    UpdateSystemCpuUsage();
    UpdateProcessCpuUsage();

    UpdateSystemMemoryUsage();
    UpdateProcessMemoryUsage();

    std::this_thread::sleep_for(delay);
}

}
