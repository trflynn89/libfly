#include "fly/system/win/system_impl.h"

#include <Windows.h>

#include <chrono>
#include <csignal>
#include <cstdio>

namespace fly {

//==============================================================================
void SystemImpl::PrintBacktrace() noexcept
{
    void *trace[10];
    const USHORT traceSize = ::CaptureStackBackTrace(0, 10, trace, nullptr);

    for (USHORT i = 0; i < traceSize; ++i)
    {
        fprintf(stderr, "[%3u] %p\n", i, trace[i]);
    }
}

//==============================================================================
std::string SystemImpl::LocalTime(const std::string &fmt) noexcept
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm timeVal;
    std::string ret;

    if (::localtime_s(&timeVal, &now) == 0)
    {
        char timeStr[32];

        if (::strftime(timeStr, sizeof(timeStr), fmt.c_str(), &timeVal) != 0)
        {
            ret = std::string(timeStr);
        }
    }

    return ret;
}

//==============================================================================
int SystemImpl::GetErrorCode() noexcept
{
    return ::GetLastError();
}

//==============================================================================
std::vector<int> SystemImpl::GetSignals() noexcept
{
    return {SIGINT, SIGTERM, SIGILL, SIGFPE, SIGABRT, SIGSEGV};
}

} // namespace fly
