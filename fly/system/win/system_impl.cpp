#include "fly/system/win/system_impl.hpp"

#include <Windows.h>

#include <chrono>
#include <cstdio>

namespace fly {

//==================================================================================================
void SystemImpl::print_backtrace()
{
    void *trace[10];
    const USHORT trace_size = ::CaptureStackBackTrace(0, 10, trace, nullptr);

    for (USHORT i = 0; i < trace_size; ++i)
    {
        fprintf(stderr, "[%3u] %p\n", i, trace[i]);
    }
}

//==================================================================================================
std::string SystemImpl::local_time(const char *fmt)
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm time_val;
    std::string result;

    if (::localtime_s(&time_val, &now) == 0)
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
    return ::GetLastError();
}

} // namespace fly
