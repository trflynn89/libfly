#include "fly/system/nix/system_monitor_impl.hpp"

#include "fly/logger/logger.hpp"
#include "fly/system/system_config.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/string/string.hpp"

#include <sys/sysinfo.h>
#include <sys/times.h>

#include <cinttypes>
#include <cstring>
#include <fstream>
#include <string>

namespace fly {

namespace {

    const char *s_procStatFile = "/proc/stat";
    const char *s_selfStatusFile = "/proc/self/status";

} // namespace

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &spTaskRunner,
    const std::shared_ptr<SystemConfig> &spConfig) noexcept :
    SystemMonitor(spTaskRunner, spConfig),
    m_prevSystemUserTime(0),
    m_prevSystemNiceTime(0),
    m_prevSystemSystemTime(0),
    m_prevSystemIdleTime(0),
    m_prevProcessSystemTime(0),
    m_prevProcessUserTime(0),
    m_prevTime(0)
{
    UpdateSystemCpuCount();
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuCount() noexcept
{
    std::ifstream stream(s_procStatFile, std::ios::in);
    std::string contents, line;

    std::uint32_t cpuCount = 0;

    while (stream.good() && std::getline(stream, line))
    {
        contents += line + "\\n";

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
        LOGS("Could not poll system CPU count (%s)", contents);
    }
    else
    {
        m_systemCpuCount.store(cpuCount);
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemCpuUsage() noexcept
{
    std::ifstream stream(s_procStatFile, std::ios::in);
    std::string line;

    std::uint64_t user = 0, nice = 0, sys = 0, idle = 0;
    int scanned = 0;

    if (stream.good() && std::getline(stream, line))
    {
        scanned = std::sscanf(
            line.c_str(),
            "cpu %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
            &user,
            &nice,
            &sys,
            &idle);
    }

    if (scanned != 4)
    {
        LOGS("Could not poll system CPU (%s)", line);
        return;
    }

    if ((user >= m_prevSystemUserTime) && (nice >= m_prevSystemNiceTime) &&
        (sys >= m_prevSystemSystemTime) && (idle >= m_prevSystemIdleTime))
    {
        std::uint64_t active = (user - m_prevSystemUserTime) +
            (nice - m_prevSystemNiceTime) + (sys - m_prevSystemSystemTime);

        std::uint64_t total = active + (idle - m_prevSystemIdleTime);

        m_systemCpuUsage.store(100.0 * active / total);
    }

    m_prevSystemUserTime = user;
    m_prevSystemNiceTime = nice;
    m_prevSystemSystemTime = sys;
    m_prevSystemIdleTime = idle;
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessCpuUsage() noexcept
{
    struct tms sample;
    clock_t now = ::times(&sample);

    if (now == static_cast<clock_t>(-1))
    {
        LOGS("Could not poll process CPU");
        return;
    }

    if ((now > m_prevTime) && (sample.tms_stime >= m_prevProcessSystemTime) &&
        (sample.tms_utime >= m_prevProcessUserTime))
    {
        std::int64_t cpu = (sample.tms_stime - m_prevProcessSystemTime) +
            (sample.tms_utime - m_prevProcessUserTime);

        clock_t time = now - m_prevTime;

        m_processCpuUsage.store(100.0 * cpu / time / m_systemCpuCount.load());
    }

    m_prevProcessSystemTime = sample.tms_stime;
    m_prevProcessUserTime = sample.tms_utime;
    m_prevTime = now;
}

//==============================================================================
void SystemMonitorImpl::UpdateSystemMemoryUsage() noexcept
{
    struct sysinfo info;

    if (::sysinfo(&info) == 0)
    {
        auto totalMemory =
            static_cast<std::uint64_t>(info.totalram) * info.mem_unit;
        auto freeMemory =
            static_cast<std::uint64_t>(info.freeram) * info.mem_unit;

        m_totalSystemMemory.store(totalMemory);
        m_systemMemoryUsage.store(totalMemory - freeMemory);
    }
    else
    {
        LOGS("Could not poll system memory");
    }
}

//==============================================================================
void SystemMonitorImpl::UpdateProcessMemoryUsage() noexcept
{
    std::ifstream stream(s_selfStatusFile, std::ios::in);
    std::string contents, line;

    std::uint64_t processMemoryUsage = 0;
    int count = 0;

    while (stream.good() && std::getline(stream, line) && (count != 1))
    {
        count = std::sscanf(
            line.c_str(),
            "VmRSS: %" SCNu64 " kB",
            &processMemoryUsage);
        contents += line + "\\n";
    }

    if (processMemoryUsage == 0)
    {
        LOGS("Could not poll process memory (%s)", contents);
    }
    else
    {
        // Value stored in status file is in KB
        m_processMemoryUsage.store(processMemoryUsage << 10);
    }
}

} // namespace fly
