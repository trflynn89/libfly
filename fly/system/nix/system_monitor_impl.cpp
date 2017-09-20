#include "fly/system/nix/system_monitor_impl.h"

#include <cinttypes>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <sys/sysinfo.h>
#include <unistd.h>

#include "fly/logger/logger.h"
#include "fly/string/string.h"

namespace fly {

namespace
{
    static const char *s_selfStatFile = "/proc/self/stat";
    static const char *s_selfStatFormat =
        "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %" SCNu64 " %" SCNu64;

    static const char *s_selfStatusFile = "/proc/self/status";
    static const char *s_selfStatusFormat = "%s %d kB";

    static const char *s_procStatFile = "/proc/stat";
}

//==============================================================================
SystemMonitorImpl::SystemMonitorImpl() :
    SystemMonitor(),
    m_currCpuTicks(0),
    m_prevCpuTicks(0),
    m_currTime(0),
    m_prevTime(0),
    m_scale(0.0)
{
    int cpuCount = getCpuCount();
    int cpuFreq = ::sysconf(_SC_CLK_TCK);

    if ((cpuCount > 0) && (cpuFreq > 0))
    {
        m_scale = (100.0 / (cpuCount * cpuFreq));
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
    return (m_scale > 0.0);
}

//==============================================================================
void SystemMonitorImpl::UpdateCpuUsage()
{
    std::ifstream stream(s_selfStatFile, std::ios::in);
    std::string line;

    m_currTime = std::chrono::duration_cast<decltype(m_currTime)>(
        std::chrono::system_clock::now().time_since_epoch()
    );

    if (stream.good() && std::getline(stream, line))
    {
        uint64_t utime = 0;
        uint64_t stime = 0;

        if (sscanf(line.c_str(), s_selfStatFormat, &utime, &stime) == 2)
        {
            m_currCpuTicks = utime + stime;
        }
    }

    // Calculation for CPU percentage derived from top's source
    if ((m_currCpuTicks > m_prevCpuTicks) && (m_currTime > m_prevTime))
    {
        uint64_t ticks = m_currCpuTicks - m_prevCpuTicks;
        auto time = m_currTime - m_prevTime;

        m_cpuUsage.store(m_scale * ticks / time.count());
    }

    m_prevCpuTicks = m_currCpuTicks;
    m_prevTime = m_currTime;
}

//==============================================================================
void SystemMonitorImpl::UpdateMemoryUsage()
{
    struct sysinfo info;

    if (sysinfo(&info) == 0)
    {
        m_totalMemory.store(static_cast<uint64_t>(info.totalram) * info.mem_unit);
        m_freeMemory.store(static_cast<uint64_t>(info.freeram) * info.mem_unit);
    }
    else
    {
        LOGS(-1, "Could not read system information");
    }

    std::ifstream stream(s_selfStatusFile, std::ios::in);
    std::string line;

    while (stream.good() && std::getline(stream, line))
    {
        char name[64];
        int32_t value = 0;

        if (sscanf(line.c_str(), s_selfStatusFormat, name, &value) == 2)
        {
            if (strncmp("VmRSS:", name, strlen("VmRSS:")) == 0)
            {
                // Value stored in status file is in KB
                m_processMemory.store(static_cast<uint64_t>(value) << 10);
                break;
            }
        }
    }
}

//==============================================================================
void SystemMonitorImpl::Close()
{
}

//==============================================================================
int SystemMonitorImpl::getCpuCount() const
{
    std::ifstream stream(s_procStatFile, std::ios::in);
    std::string line;
    int cpuCount = 0;

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

    return cpuCount;
}

}
