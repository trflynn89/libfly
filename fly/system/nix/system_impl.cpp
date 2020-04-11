#include "fly/system/nix/system_impl.hpp"

#include <execinfo.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstring>

namespace fly {

//==============================================================================
void SystemImpl::PrintBacktrace() noexcept
{
    void *trace[10];
    int traceSize = ::backtrace(trace, 10);
    ::backtrace_symbols_fd(trace, traceSize, STDERR_FILENO);
}

//==============================================================================
std::string SystemImpl::LocalTime(const std::string &fmt) noexcept
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm timeVal;
    std::string ret;

    if (::localtime_r(&now, &timeVal) != nullptr)
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
    return errno;
}

//==============================================================================
std::vector<int> SystemImpl::GetSignals() noexcept
{
    return {SIGINT, SIGTERM, SIGSYS, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGSEGV};
}

} // namespace fly
