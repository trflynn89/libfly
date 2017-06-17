#include "fly/system/nix/system_impl.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>

#include <execinfo.h>
#include <unistd.h>

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
void SystemImpl::PrintBacktrace()
{
    void *trace[10];
    int traceSize = ::backtrace(trace, 10);
    ::backtrace_symbols_fd(trace, traceSize, STDERR_FILENO);
}

//==============================================================================
std::string SystemImpl::LocalTime(const std::string &fmt)
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm timeVal;

    if (::localtime_r(&now, &timeVal) != NULL)
    {
        char timeStr[32];

        if (::strftime(timeStr, sizeof(timeStr), fmt.c_str(), &timeVal) != 0)
        {
            return std::string(timeStr);
        }
    }

    return std::string();
}

//==============================================================================
std::string SystemImpl::GetLastError(int *pCode)
{
    int error = errno;

    if (pCode != NULL)
    {
        *pCode = error;
    }

    return "(" + std::to_string(error) + ") " + ::strerror(error);
}

//==============================================================================
std::vector<int> SystemImpl::GetSignals()
{
    return std::vector<int>
    {
        SIGINT, SIGTERM, SIGSYS, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGSEGV
    };
}

}
