#include "fly/system/win/system_monitor_impl.h"

#include <cmath>

#include <Psapi.h>

#include "fly/logger/logger.h"

namespace fly {

namespace
{
    static const LPCSTR s_cpuPath = "\\Processor(0)\\% Processor Time";
}

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl() :
    SystemMonitor(),
    m_cpuQuery(NULL),
    m_cpuCounter(NULL)
{
    PDH_STATUS status = ERROR_SUCCESS;

    if ((status = PdhOpenQuery(NULL, NULL, &m_cpuQuery)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not open CPU query (0x%x)", status);
    }
    else if ((status = PdhAddCounter(m_cpuQuery, s_cpuPath, NULL, &m_cpuCounter)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not add CPU counter (0x%x)", status);
        Close();
    }
    else if ((status = PdhCollectQueryData(m_cpuQuery)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not poll CPU counter (0x%x)", status);
        Close();
    }
}

//==============================================================================
SystemMonitorImpl::~SystemMonitorImpl()
{
    Close();
}

//==============================================================================
bool SystemMonitorImpl::IsValid() const
{
    return (m_cpuQuery != NULL);
}

//==============================================================================
void SystemMonitorImpl::UpdateCpuUsage()
{
    PDH_STATUS status = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE value;

    if ((status = PdhCollectQueryData(m_cpuQuery)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not poll CPU counter (0x%x)", status);
        Close();
    }
    else if ((status = PdhGetFormattedCounterValue(m_cpuCounter, PDH_FMT_DOUBLE, NULL, &value)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not format CPU counter (0x%x)", status);
        Close();
    }
    else
    {
        m_cpuUsage.store(static_cast<uint64_t>(std::round(100.0f * value.doubleValue)));
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateMemoryUsage()
{
    MEMORYSTATUSEX info;
    info.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&info))
    {
        m_totalMemory.store(info.ullTotalPhys);
        m_freeMemory.store(info.ullAvailPhys);
    }
    else
    {
        LOGS(-1, "Could not poll system memory");
    }

    PROCESS_MEMORY_COUNTERS pmc;

    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc)))
    {
        m_processMemory.store(pmc.PagefileUsage);
    }
    else
    {
        LOGS(-1, "Could not poll process memory");
    }
}

//==============================================================================
void SystemMonitorImpl::Close()
{
    if (m_cpuQuery != NULL)
    {
        PdhCloseQuery(m_cpuQuery);
        m_cpuQuery = NULL;
    }
}

}
