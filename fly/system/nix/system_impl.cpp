#include "fly/system/system.hpp"

#include <execinfo.h>
#include <unistd.h>

#include <cerrno>
#include <chrono>
#include <cstring>

namespace fly::system {

//==================================================================================================
void print_backtrace()
{
    void *trace[10];
    int trace_size = ::backtrace(trace, 10);

    if (trace_size > 0)
    {
        ::backtrace_symbols_fd(trace, trace_size, STDERR_FILENO);
    }
}

//==================================================================================================
std::string local_time()
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm time_val;
    std::string result;

    if (::localtime_r(&now, &time_val) != nullptr)
    {
        char time_str[32];

        if (::strftime(time_str, sizeof(time_str), "%m-%d-%Y %H:%M:%S", &time_val) != 0)
        {
            result = std::string(time_str);
        }
    }

    return result;
}

//==================================================================================================
int get_error_code()
{
    return errno;
}

} // namespace fly::system
