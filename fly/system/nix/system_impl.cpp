#include "fly/system/nix/system_impl.hpp"

#include <execinfo.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstring>

namespace fly {

//==================================================================================================
void SystemImpl::print_backtrace() noexcept
{
    void *trace[10];
    int trace_size = ::backtrace(trace, 10);
    ::backtrace_symbols_fd(trace, trace_size, STDERR_FILENO);
}

//==================================================================================================
std::string SystemImpl::local_time(const char *fmt) noexcept
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm time_val;
    std::string result;

    if (::localtime_r(&now, &time_val) != nullptr)
    {
        char time_str[32];

        if (::strftime(time_str, sizeof(time_str), fmt, &time_val) != 0)
        {
            result = std::string(time_str);
        }
    }

    return result;
}

//==================================================================================================
int SystemImpl::get_error_code() noexcept
{
    return errno;
}

//==================================================================================================
std::vector<int> SystemImpl::get_signals() noexcept
{
    return {SIGINT, SIGTERM, SIGSYS, SIGBUS, SIGILL, SIGFPE, SIGABRT, SIGSEGV};
}

} // namespace fly
