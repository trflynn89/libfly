#include "fly/system/win/system_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"

#include <Psapi.h>

#include <cstring>

namespace fly::system {

namespace {

    const LPCWSTR s_cpu_path = L"\\Processor(_Total)\\% Processor Time";

} // namespace

//==================================================================================================
SystemMonitorImpl::SystemMonitorImpl(
    std::shared_ptr<fly::task::SequencedTaskRunner> task_runner,
    std::shared_ptr<SystemConfig> config) noexcept :
    SystemMonitor(std::move(task_runner), std::move(config)),
    m_process(::GetCurrentProcess()),
    m_cpu_query(nullptr),
    m_cpu_counter(nullptr)
{
    PDH_STATUS status = ::PdhOpenQuery(nullptr, 0, &m_cpu_query);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not open CPU query ({:#04x})", status);
        return;
    }

    status = ::PdhAddCounter(m_cpu_query, s_cpu_path, 0, &m_cpu_counter);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not add CPU counter ({:#04x})", status);
        return;
    }

    status = ::PdhCollectQueryData(m_cpu_query);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not poll CPU counter ({:#04x})", status);
        return;
    }
}

//==================================================================================================
SystemMonitorImpl::~SystemMonitorImpl()
{
    if (m_cpu_query != nullptr)
    {
        ::PdhCloseQuery(m_cpu_query);
        m_cpu_query = nullptr;
    }
}

//==================================================================================================
void SystemMonitorImpl::update_system_cpu_count()
{
    SYSTEM_INFO info;
    ::GetSystemInfo(&info);

    if (info.dwNumberOfProcessors == 0)
    {
        LOGS("Could not poll system CPU count");
    }
    else
    {
        m_system_cpu_count.store(info.dwNumberOfProcessors);
    }
}

//==================================================================================================
void SystemMonitorImpl::update_system_cpu_usage()
{
    PDH_FMT_COUNTERVALUE value;

    PDH_STATUS status = ::PdhCollectQueryData(m_cpu_query);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not poll CPU counter ({:#04x})", status);
        return;
    }

    status = ::PdhGetFormattedCounterValue(m_cpu_counter, PDH_FMT_DOUBLE, nullptr, &value);
    if (status != ERROR_SUCCESS)
    {
        LOGS("Could not format CPU counter ({:#04x})", status);
        return;
    }

    m_system_cpu_usage.store(value.doubleValue);
}

//==================================================================================================
void SystemMonitorImpl::update_process_cpu_usage()
{
    ULARGE_INTEGER now, system, user;
    FILETIME fnow, fsystem, fuser;

    ::GetSystemTimeAsFileTime(&fnow);
    ::memcpy(&now, &fnow, sizeof(FILETIME));

    if (::GetProcessTimes(m_process, &fnow, &fnow, &fsystem, &fuser))
    {
        ::memcpy(&system, &fsystem, sizeof(FILETIME));
        ::memcpy(&user, &fuser, sizeof(FILETIME));

        const ULONGLONG cpu = (system.QuadPart - m_prev_process_system_time) +
            (user.QuadPart - m_prev_process_user_time);
        const ULONGLONG time = now.QuadPart - m_prev_time;

        m_process_cpu_usage.store(100.0 * cpu / time / m_system_cpu_count.load());

        m_prev_process_system_time = system.QuadPart;
        m_prev_process_user_time = user.QuadPart;
        m_prev_time = now.QuadPart;
    }
    else
    {
        LOGS("Could not poll process CPU");
    }
}

//==================================================================================================
void SystemMonitorImpl::update_system_memory_usage()
{
    MEMORYSTATUSEX info;
    info.dwLength = sizeof(MEMORYSTATUSEX);

    if (::GlobalMemoryStatusEx(&info))
    {
        m_total_system_memory.store(info.ullTotalPhys);
        m_system_memory_usage.store(info.ullTotalPhys - info.ullAvailPhys);
    }
    else
    {
        LOGS("Could not poll system memory");
    }
}

//==================================================================================================
void SystemMonitorImpl::update_process_memory_usage()
{
    PROCESS_MEMORY_COUNTERS pmc;

    if (::GetProcessMemoryInfo(m_process, &pmc, sizeof(pmc)))
    {
        m_process_memory_usage.store(pmc.WorkingSetSize);
    }
    else
    {
        LOGS("Could not poll process memory");
    }
}

} // namespace fly::system
