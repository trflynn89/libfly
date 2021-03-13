#include "fly/system/system.hpp"

#include <Windows.h>

#include <chrono>
#include <cstdio>

namespace fly::system {

//==================================================================================================
void print_backtrace()
{
    void *trace[10];
    const USHORT trace_size = ::CaptureStackBackTrace(0, 10, trace, nullptr);

    for (USHORT i = 0; i < trace_size; ++i)
    {
        fprintf(stderr, "[%3u] %p\n", i, trace[i]);
    }
}

//==================================================================================================
std::string local_time()
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm time_val;
    std::string result;

    if (::localtime_s(&time_val, &now) == 0)
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
    return ::GetLastError();
}

} // namespace fly::system
