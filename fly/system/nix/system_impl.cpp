#include "fly/system/nix/system_impl.h"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstring>

#include <execinfo.h>
#include <unistd.h>

#if defined(FLY_USE_SANITIZER)

/**
 * AddressSanitizer catches SIGSEGV by default. Override the default options to
 * allow a user-specified handler.
 */
extern "C" const char *__asan_default_options()
{
    return "allow_user_segv_handler=1";
}

#endif

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
    std::string ret;

    if (::localtime_r(&now, &timeVal) != NULL)
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
int SystemImpl::GetErrorCode()
{
    return errno;
}

//==============================================================================
std::string SystemImpl::GetErrorString(int code)
{
    return "(" + std::to_string(code) + ") " + ::strerror(code);
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
