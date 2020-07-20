#include "fly/system/nix/system_impl.hpp"

#include <execinfo.h>
#include <unistd.h>

#include <chrono>
#include <cstring>

namespace fly {

//==================================================================================================
void SystemImpl::print_backtrace()
{
    void *trace[10];
    int trace_size = ::backtrace(trace, 10);

    if (trace_size > 0)
    {
        ::backtrace_symbols_fd(trace, trace_size, STDERR_FILENO);
    }
}

//==================================================================================================
std::string SystemImpl::local_time(const char *fmt)
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
int SystemImpl::get_error_code()
{
    return errno;
}

} // namespace fly
