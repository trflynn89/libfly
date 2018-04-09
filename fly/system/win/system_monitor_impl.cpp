#include "fly/system/win/system_monitor_impl.h"

#include <cstring>

#include <Psapi.h>

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"

namespace fly {

namespace
{
    static const LPCSTR s_cpuPath = "\\Processor(0)\\% Processor Time";
}

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl(ConfigManagerPtr &spConfigManager) :
    SystemMonitor(spConfigManager),
    m_process(::GetCurrentProcess()),
    m_cpuQuery(NULL),
    m_cpuCounter(NULL),
    m_prevProcessSystemTime(0),
    m_prevProcessUserTime(0),
    m_prevTime(0)
{
}

//==============================================================================
SystemMonitorImpl::~SystemMonitorImpl()
{
    Stop();
}

//==============================================================================
void SystemMonitorImpl::StartMonitor()
{
    PDH_STATUS status = ERROR_SUCCESS;

    if ((status = ::PdhOpenQuery(NULL, NULL, &m_cpuQuery)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not open CPU query (0x%x)", status);
    }
    else if ((status = ::PdhAddCounter(m_cpuQuery, s_cpuPath, NULL, &m_cpuCounter)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not add CPU counter (0x%x)", status);
        StopMonitor();
    }
    else if ((status = ::PdhCollectQueryData(m_cpuQuery)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not poll CPU counter (0x%x)", status);
        StopMonitor();
    }

    UpdateSystemCpuCount();
}

//==============================================================================
void SystemMonitorImpl::StopMonitor()
{
    if (m_cpuQuery != NULL)
    {
        ::PdhCloseQuery(m_cpuQuery);
        m_cpuQuery = NULL;
    }
}

//==============================================================================
bool SystemMonitorImpl::IsValid() const
{
    return ((m_cpuQuery != NULL) && (m_systemCpuCount.load() > 0));
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuCount()
{
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);

    if (info.dwNumberOfProcessors == 0)
    {
        LOGS(-1, "Could not poll system CPU count");
    }
    else
    {
        m_systemCpuCount.store(info.dwNumberOfProcessors);
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuUsage()
{
    PDH_STATUS status = ERROR_SUCCESS;
    PDH_FMT_COUNTERVALUE value;

    if ((status = ::PdhCollectQueryData(m_cpuQuery)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not poll CPU counter (0x%x)", status);
        StopMonitor();
    }
    else if ((status = ::PdhGetFormattedCounterValue(m_cpuCounter, PDH_FMT_DOUBLE, NULL, &value)) != ERROR_SUCCESS)
    {
        LOGS(-1, "Could not format CPU counter (0x%x)", status);
        StopMonitor();
    }
    else
    {
        m_systemCpuUsage.store(value.doubleValue);
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessCpuUsage()
{
    ULARGE_INTEGER now, system, user;
    FILETIME fnow, fsystem, fuser;

    ::GetSystemTimeAsFileTime(&fnow);
    ::memcpy(&now, &fnow, sizeof(FILETIME));

    if (::GetProcessTimes(m_process, &fnow, &fnow, &fsystem, &fuser))
    {
        ::memcpy(&system, &fsystem, sizeof(FILETIME));
        ::memcpy(&user, &fuser, sizeof(FILETIME));

        ULONGLONG cpu =
            (system.QuadPart - m_prevProcessSystemTime) +
            (user.QuadPart - m_prevProcessUserTime);

        ULONGLONG time = now.QuadPart - m_prevTime;

        m_processCpuUsage.store(100.0 * cpu / time / m_systemCpuCount.load());

        m_prevProcessSystemTime = system.QuadPart;
        m_prevProcessUserTime = user.QuadPart;
        m_prevTime = now.QuadPart;
    }
    else
    {
        LOGS(-1, "Could not poll process CPU");
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemMemoryUsage()
{
    MEMORYSTATUSEX info;
    info.dwLength = sizeof(MEMORYSTATUSEX);

    if (::GlobalMemoryStatusEx(&info))
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

    if (::GetProcessMemoryInfo(m_process, &pmc, sizeof(pmc)))
    {
        m_processMemoryUsage.store(pmc.WorkingSetSize);
    }
    else
    {
        LOGS(-1, "Could not poll process memory");
    }
}

}
