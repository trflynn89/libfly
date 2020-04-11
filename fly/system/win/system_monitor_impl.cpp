#include "fly/system/win/system_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#include <Psapi.h>

#include <cstring>

namespace fly {

namespace {

    const LPCSTR s_cpuPath = "\\Processor(_Total)\\% Processor Time";

} // namespace

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<SystemConfig> &spConfig) noexcept :
    SystemMonitor(spTaskRunner, spConfig),
    m_process(::GetCurrentProcess()),
    m_cpuQuery(nullptr),
    m_cpuCounter(nullptr),
    m_prevProcessSystemTime(0),
    m_prevProcessUserTime(0),
    m_prevTime(0)
{
    PDH_STATUS status = ::PdhOpenQuery(nullptr, 0, &m_cpuQuery);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not open CPU query (%x)", status);
        return;
    }

    status = ::PdhAddCounter(m_cpuQuery, s_cpuPath, 0, &m_cpuCounter);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not add CPU counter (%x)", status);
        return;
    }

    status = ::PdhCollectQueryData(m_cpuQuery);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not poll CPU counter (%x)", status);
        return;
    }

    UpdateSystemCpuCount();
}

//==============================================================================
SystemMonitorImpl::~SystemMonitorImpl()
{
    if (m_cpuQuery != nullptr)
    {
        ::PdhCloseQuery(m_cpuQuery);
        m_cpuQuery = nullptr;
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuCount() noexcept
{
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);

    if (info.dwNumberOfProcessors == 0)
    {
        LOGS("Could not poll system CPU count");
    }
    else
    {
        m_systemCpuCount.store(info.dwNumberOfProcessors);
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuUsage() noexcept
{
    PDH_FMT_COUNTERVALUE value;

    PDH_STATUS status = ::PdhCollectQueryData(m_cpuQuery);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not poll CPU counter (%x)", status);
        return;
    }

    status = ::PdhGetFormattedCounterValue(
        m_cpuCounter,
        PDH_FMT_DOUBLE,
        nullptr,
        &value);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not format CPU counter (%x)", status);
        return;
    }

    m_systemCpuUsage.store(value.doubleValue);
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessCpuUsage() noexcept
{
    ULARGE_INTEGER now, system, user;
    FILETIME fnow, fsystem, fuser;

    ::GetSystemTimeAsFileTime(&fnow);
    ::memcpy(&now, &fnow, sizeof(FILETIME));

    if (::GetProcessTimes(m_process, &fnow, &fnow, &fsystem, &fuser))
    {
        ::memcpy(&system, &fsystem, sizeof(FILETIME));
        ::memcpy(&user, &fuser, sizeof(FILETIME));

        ULONGLONG cpu = (system.QuadPart - m_prevProcessSystemTime) +
            (user.QuadPart - m_prevProcessUserTime);

        ULONGLONG time = now.QuadPart - m_prevTime;

        m_processCpuUsage.store(100.0 * cpu / time / m_systemCpuCount.load());

        m_prevProcessSystemTime = system.QuadPart;
        m_prevProcessUserTime = user.QuadPart;
        m_prevTime = now.QuadPart;
    }
    else
    {
        LOGS("Could not poll process CPU");
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemMemoryUsage() noexcept
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
        LOGS("Could not poll system memory");
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessMemoryUsage() noexcept
{
    PROCESS_MEMORY_COUNTERS pmc;

    if (::GetProcessMemoryInfo(m_process, &pmc, sizeof(pmc)))
    {
        m_processMemoryUsage.store(pmc.WorkingSetSize);
    }
    else
    {
        LOGS("Could not poll process memory");
    }
}

} // namespace fly
