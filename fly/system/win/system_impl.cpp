#include "fly/system/win/system_impl.h"

#include <atomic>
#include <chrono>
#include <csignal>

#include <Windows.h>

#include "fly/logger/logger.h"

namespace fly {

//==============================================================================
void SystemImpl::PrintBacktrace()
{
    void *trace[10];
    const USHORT traceSize = ::CaptureStackBackTrace(0, 10, trace, NULL);

    for (USHORT i = 0; i < traceSize; ++i)
    {
        LOGC_NO_LOCK("[%3u] %x", i, trace[i]);
    }
}

//==============================================================================
std::string SystemImpl::LocalTime(const std::string &fmt)
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm timeVal;

    if (::localtime_s(&timeVal, &now) == 0)
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
    int error = ::WSAGetLastError();
    LPTSTR str = NULL;
    std::string ret;

    ::FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&str, 0, NULL
    );

    if (str == NULL)
    {
        ret = std::to_string(error);
    }
    else
    {
        ret = "(" + std::to_string(error) + ") " + str;
        ::LocalFree(str);
    }

    if (pCode != NULL)
    {
        *pCode = error;
    }

    return ret;
}

//==============================================================================
std::vector<int> SystemImpl::GetSignals()
{
    return std::vector<int>
    {
        SIGINT, SIGTERM, SIGILL, SIGFPE, SIGABRT, SIGSEGV
    };
}

}
