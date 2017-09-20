#include "fly/system/nix/system_monitor_impl.h"

#include <cinttypes>
#include <cstring>
#include <fstream>
#include <string>

#include <sys/sysinfo.h>
#include <sys/times.h>

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/string/string.h"

namespace fly {

namespace
{
    static const char *s_procStatFile = "/proc/stat";
    static const char *s_procStatFormat =
        "cpu %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64;

    static const char *s_selfStatusFile = "/proc/self/status";
    static const char *s_selfStatusFormat = "VmRSS: %" SCNu64 " kB";
}

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl() :
    SystemMonitor(),
    m_prevSystemUserTime(0),
    m_prevSystemNiceTime(0),
    m_prevSystemSystemTime(0),
    m_prevSystemIdleTime(0),
    m_prevProcessSystemTime(0),
    m_prevProcessUserTime(0),
    m_prevTime(0)
{
}

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl(ConfigManagerPtr &spConfigManager) :
    SystemMonitor(spConfigManager),
    m_prevSystemUserTime(0),
    m_prevSystemNiceTime(0),
    m_prevSystemSystemTime(0),
    m_prevSystemIdleTime(0),
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
    UpdateSystemCpuCount();
}

//==============================================================================
void SystemMonitorImpl::StopMonitor()
{
}

//==============================================================================
bool SystemMonitorImpl::IsValid() const
{
    return (m_systemCpuCount.load() > 0);
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuCount()
{
    std::ifstream stream(s_procStatFile, std::ios::in);
    std::string line;

    uint32_t cpuCount = 0;

    while (stream.good() && std::getline(stream, line))
    {
        if (String::StartsWith(line, "cpu"))
        {
            if ((line.size() > 3) && (line[3] != ' '))
            {
                ++cpuCount;
            }
        }
    }

    if (cpuCount == 0)
    {
        LOGS(-1, "Could not poll system CPU count");
    }
    else
    {
        m_systemCpuCount.store(cpuCount);
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuUsage()
{
    std::ifstream stream(s_procStatFile, std::ios::in);
    std::string line;

    uint64_t user = 0, nice = 0, system = 0, idle = 0;

    if (stream.good() && std::getline(stream, line))
    {
        ::sscanf(line.c_str(), s_procStatFormat, &user, &nice, &system, &idle);
    }

    if ((user == 0) || (nice == 0) || (system == 0) || (idle == 0))
    {
        LOGS(-1, "Could not poll system CPU");
    }
    else if ((user >= m_prevSystemUserTime) && (nice >= m_prevSystemNiceTime) &&
        (system >= m_prevSystemSystemTime) && (idle >= m_prevSystemIdleTime))
    {
        uint64_t active =
            (user - m_prevSystemUserTime) +
            (nice - m_prevSystemNiceTime) +
            (system - m_prevSystemSystemTime);

        uint64_t total = active + (idle - m_prevSystemIdleTime);

        m_systemCpuUsage.store(100.0 * active / total);
    }

    m_prevSystemUserTime = user;
    m_prevSystemNiceTime = nice;
    m_prevSystemSystemTime = system;
    m_prevSystemIdleTime = idle;
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessCpuUsage()
{
    struct tms sample;
    clock_t now = ::times(&sample);

    if (now == static_cast<clock_t>(-1))
    {
        LOGS(-1, "Could not poll process CPU");
    }
    else if ((now > m_prevTime) &&
        (sample.tms_stime >= m_prevProcessSystemTime) &&
        (sample.tms_utime >= m_prevProcessUserTime))
    {
        uint64_t cpu =
            (sample.tms_stime - m_prevProcessSystemTime) +
            (sample.tms_utime - m_prevProcessUserTime);

        clock_t time = now - m_prevTime;

        m_processCpuUsage.store(100.0 * cpu / time / m_systemCpuCount.load());
    }

    m_prevProcessSystemTime = sample.tms_stime;
    m_prevProcessUserTime = sample.tms_utime;
    m_prevTime = now;
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemMemoryUsage()
{
    struct sysinfo info;

    if (::sysinfo(&info) == 0)
    {
        uint64_t totalMemory = static_cast<uint64_t>(info.totalram) * info.mem_unit;
        uint64_t freeMemory = static_cast<uint64_t>(info.freeram) * info.mem_unit;

        m_totalSystemMemory.store(totalMemory);
        m_systemMemoryUsage.store(totalMemory - freeMemory);
    }
    else
    {
        LOGS(-1, "Could not poll system memory");
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessMemoryUsage()
{
    std::ifstream stream(s_selfStatusFile, std::ios::in);
    std::string line;

    uint64_t processMemoryUsage = 0;

    while (stream.good() && std::getline(stream, line))
    {
        if (::sscanf(line.c_str(), s_selfStatusFormat, &processMemoryUsage) == 1)
        {
            break;
        }
    }

    if (processMemoryUsage == 0)
    {
        LOGS(-1, "Could not poll process memory");
    }
    else
    {
        // Value stored in status file is in KB
        m_processMemoryUsage.store(processMemoryUsage << 10);
    }
}

}
