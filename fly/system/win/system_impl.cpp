#include "fly/system/win/system_impl.h"

#include <atomic>
#include <chrono>
#include <csignal>

#include <Windows.h>

#include "fly/logger/logger.h"

namespace fly {

namespace
{
    static std::atomic<ExitCode> g_aExitCode(Normal);
    static std::atomic_bool g_aKeepRunning(true);

    //==========================================================================
    void handleSignal(int sig)
    {
        LOGC_NO_LOCK("Received signal %d", sig);
        LOGI(-1, "Received signal %d", sig);

        bool fatalSignal = false;
        bool cleanExit = false;

        switch (sig)
        {
        case SIGINT:
        case SIGTERM:
            LOGC_NO_LOCK("Non-fatal exit signal caught");
            cleanExit = true;
            break;

        case SIGILL:
        case SIGFPE:
        case SIGABRT:
        case SIGSEGV:
            LOGC_NO_LOCK("Fatal exit signal caught");
            fatalSignal = true;
            cleanExit = true;
            break;

        default:
            break;
        }

        if (cleanExit)
        {
            ExitCode exitCode = Normal;

            if (fatalSignal)
            {
                SystemImpl::PrintBacktrace();
                exitCode = FatalSignal;
            }

            SystemImpl::CleanExit(exitCode);
        }
    }
}

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
void SystemImpl::SetupSignalHandler()
{
    ::signal(SIGINT, handleSignal);
    ::signal(SIGTERM, handleSignal);
    ::signal(SIGILL, handleSignal);
    ::signal(SIGFPE, handleSignal);
    ::signal(SIGABRT, handleSignal);
    ::signal(SIGSEGV, handleSignal);
}

//==============================================================================
void SystemImpl::CleanExit(ExitCode exitCode)
{
    g_aExitCode.store(exitCode);
    g_aKeepRunning.store(false);
}

//==============================================================================
bool SystemImpl::KeepRunning()
{
    return g_aKeepRunning.load();
}

//==============================================================================
ExitCode SystemImpl::GetExitCode()
{
    return g_aExitCode.load();
}

}
