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

    const char *s_proc_stat_file = "/proc/stat";
    const char *s_self_status_file = "/proc/self/status";

} // namespace

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl(
    const std::shared_ptr<SequencedTaskRunner> &task_runner,
    const std::shared_ptr<SystemConfig> &config) noexcept :
    SystemMonitor(task_runner, config),
    m_prev_system_user_time(0),
    m_prev_system_nice_time(0),
    m_prev_system_system_time(0),
    m_prev_system_idle_time(0),
    m_prev_process_system_time(0),
    m_prev_process_user_time(0),
    m_prev_time(0)
{
}

//==============================================================================
void SystemMonitorImpl::update_system_cpu_count() noexcept
{
    std::ifstream stream(s_proc_stat_file, std::ios::in);
    std::string contents, line;

    std::uint32_t cpu_count = 0;

    while (stream.good() && std::getline(stream, line))
    {
        contents += line + "\\n";

        if (String::starts_with(line, "cpu"))
        {
            if ((line.size() > 3) && (line[3] != ' '))
            {
                ++cpu_count;
            }
        }
    }

    if (cpu_count == 0)
    {
        LOGS("Could not poll system CPU count (%s)", contents);
    }
    else
    {
        m_system_cpu_count.store(cpu_count);
    }
}

//==============================================================================
void SystemMonitorImpl::update_system_cpu_usage() noexcept
{
    std::ifstream stream(s_proc_stat_file, std::ios::in);
    std::string line;

    std::uint64_t user = 0, nice = 0, system = 0, idle = 0;
    int scanned = 0;

    if (stream.good() && std::getline(stream, line))
    {
        scanned = std::sscanf(
            line.c_str(),
            "cpu %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64,
            &user,
            &nice,
            &system,
            &idle);
    }

    if (scanned != 4)
    {
        LOGS("Could not poll system CPU (%s)", line);
        return;
    }

    if ((user >= m_prev_system_user_time) &&
        (nice >= m_prev_system_nice_time) &&
        (system >= m_prev_system_system_time) &&
        (idle >= m_prev_system_idle_time))
    {
        std::uint64_t active = (user - m_prev_system_user_time) +
            (nice - m_prev_system_nice_time) +
            (system - m_prev_system_system_time);

        std::uint64_t total = active + (idle - m_prev_system_idle_time);

        m_system_cpu_usage.store(100.0 * active / total);
    }

    m_prev_system_user_time = user;
    m_prev_system_nice_time = nice;
    m_prev_system_system_time = system;
    m_prev_system_idle_time = idle;
}

//==============================================================================
void SystemMonitorImpl::update_process_cpu_usage() noexcept
{
    struct tms sample;
    clock_t now = ::times(&sample);

    if (now == static_cast<clock_t>(-1))
    {
        LOGS("Could not poll process CPU");
        return;
    }

    if ((now > m_prev_time) &&
        (sample.tms_stime >= m_prev_process_system_time) &&
        (sample.tms_utime >= m_prev_process_user_time))
    {
        std::int64_t cpu = (sample.tms_stime - m_prev_process_system_time) +
            (sample.tms_utime - m_prev_process_user_time);

        clock_t time = now - m_prev_time;

        m_process_cpu_usage.store(
            100.0 * cpu / time / m_system_cpu_count.load());
    }

    m_prev_process_system_time = sample.tms_stime;
    m_prev_process_user_time = sample.tms_utime;
    m_prev_time = now;
}

//==============================================================================
void SystemMonitorImpl::update_system_memory_usage() noexcept
{
    struct sysinfo info;

    if (::sysinfo(&info) == 0)
    {
        auto total_memory =
            static_cast<std::uint64_t>(info.totalram) * info.mem_unit;
        auto free_memory =
            static_cast<std::uint64_t>(info.freeram) * info.mem_unit;

        m_total_system_memory.store(total_memory);
        m_system_memory_usage.store(total_memory - free_memory);
    }
    else
    {
        LOGS("Could not poll system memory");
    }
}

//==============================================================================
void SystemMonitorImpl::update_process_memory_usage() noexcept
{
    std::ifstream stream(s_self_status_file, std::ios::in);
    std::string contents, line;

    std::uint64_t process_memory_usage = 0;
    int count = 0;

    while (stream.good() && std::getline(stream, line) && (count != 1))
    {
        count = std::sscanf(
            line.c_str(),
            "VmRSS: %" SCNu64 " kB",
            &process_memory_usage);
        contents += line + "\\n";
    }

    if (process_memory_usage == 0)
    {
        LOGS("Could not poll process memory (%s)", contents);
    }
    else
    {
        // Value stored in status file is in KB
        m_process_memory_usage.store(process_memory_usage << 10);
    }
}

} // namespace fly
