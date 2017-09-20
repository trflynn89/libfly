#include "fly/system/win/system_monitor_impl.h"

#include <cstring>

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
    m_process(GetCurrentProcess()),
    m_cpuQuery(NULL),
    m_cpuCounter(NULL),
    m_currCpuTicks(0),
    m_prevCpuTicks(0),
    m_currTime(0),
    m_prevTime(0),
    m_scale(0.0)
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

    DWORD cpuCount = getCpuCount();

    if (cpuCount > 0)
    {
        m_scale = (100.0 / cpuCount);
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
    return ((m_cpuQuery != NULL) && (m_scale > 0.0));
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuUsage()
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
        m_systemCpuUsage.store(value.doubleValue);
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessCpuUsage()
{
    FILETIME ftime, fsys, fuser;
    ULARGE_INTEGER now;

    GetSystemTimeAsFileTime(&ftime);
    memcpy(&now, &ftime, sizeof(FILETIME));
    m_currTime = now.QuadPart;

    if (GetProcessTimes(m_process, &ftime, &ftime, &fsys, &fuser))
    {
        ULARGE_INTEGER stime, utime;

        memcpy(&stime, &fsys, sizeof(FILETIME));
        memcpy(&utime, &fuser, sizeof(FILETIME));

        m_currCpuTicks = stime.QuadPart + utime.QuadPart;
    }
    else
    {
        LOGS(-1, "Could not poll process CPU");
    }

    if ((m_currCpuTicks > m_prevCpuTicks) && (m_currTime > m_prevTime))
    {
        uint64_t ticks = m_currCpuTicks - m_prevCpuTicks;
        uint64_t time = m_currTime - m_prevTime;

        m_processCpuUsage.store(m_scale * ticks / time);
    }

    m_prevCpuTicks = m_currCpuTicks;
    m_prevTime = m_currTime;
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemMemoryUsage()
{
    MEMORYSTATUSEX info;
    info.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&info))
    {
        m_totalSystemMemory.store(info.ullTotalPhys);
        m_systemMemoryUsage.store(info.ullTotalPhys - info.ullAvailPhys);
    }
    else
    {
        LOGS(-1, "Could not poll system memory");
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessMemoryUsage()
{
    PROCESS_MEMORY_COUNTERS pmc;

    if (GetProcessMemoryInfo(m_process, &pmc, sizeof(pmc)))
    {
        m_processMemoryUsage.store(pmc.WorkingSetSize);
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

//==============================================================================
DWORD SystemMonitorImpl::getCpuCount() const
{
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    return sysInfo.dwNumberOfProcessors;
}

}
