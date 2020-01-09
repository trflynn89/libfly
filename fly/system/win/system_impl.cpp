#include "fly/system/win/system_impl.h"

#include "fly/logger/logger.h"
#include "fly/types/string/string.h"

#include <Windows.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>

namespace fly {

namespace {

    const DWORD s_formatFlags = FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS;

    const DWORD s_langId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT);

} // namespace

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
    return ::WSAGetLastError();
}

//==============================================================================
std::string SystemImpl::GetErrorString(int code) noexcept
{
    LPTSTR str = nullptr;
    std::string ret;

    ::FormatMessage(
        s_formatFlags,
        nullptr,
        code,
        s_langId,
        (LPTSTR)&str,
        0,
        nullptr);

    if (str == nullptr)
    {
        ret = std::to_string(code);
    }
    else
    {
        ret = "(" + std::to_string(code) + ") " + str;
        String::Trim(ret);

        ::LocalFree(str);
    }

    return ret;
}

//==============================================================================
std::vector<int> SystemImpl::GetSignals() noexcept
{
    return {SIGINT, SIGTERM, SIGILL, SIGFPE, SIGABRT, SIGSEGV};
}

} // namespace fly
