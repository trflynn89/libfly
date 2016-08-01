#include "nix_system.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fly/logging/logger.h>
#include <fly/string/string.h>

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

        LoggerPtr spLogger = Logger::GetInstance();

        bool fatalSignal = false;
        bool cleanExit = false;

        switch (sig)
        {
        case SIGINT:
        case SIGTERM:
            LOGC_NO_LOCK("Non-fatal exit signal caught");
            cleanExit = true;
            break;

        case SIGSYS:
        case SIGBUS:
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
bool SystemImpl::MakeDirectory(const std::string &path)
{
    static const mode_t mode = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    struct stat st;

    if (::stat(path.c_str(), &st) == 0)
    {
        if (!S_ISDIR(st.st_mode))
        {
            errno = ENOTDIR;
            return false;
        }

        return true;
    }

    size_t pos = path.rfind('/');

    if (pos != std::string::npos)
    {
        if (!MakeDirectory(path.substr(0, pos)))
        {
            return false;
        }
    }

    return ((::mkdir(path.c_str(), mode) == 0) || (errno == EEXIST));
}

//==============================================================================
char SystemImpl::GetSeparator()
{
    return '/';
}

//==============================================================================
std::string SystemImpl::GetTempDirectory()
{
    static const std::string envs[] = { "TMPDIR", "TMP", "TEMP", "TEMPDIR", "" };

    for (int i = 0; !envs[i].empty(); ++i)
    {
        char *dir = ::getenv(envs[i].c_str());

        if (dir != NULL)
        {
            return std::string(dir);
        }
    }

    return std::string("/tmp");
}

//==============================================================================
void SystemImpl::PrintBacktrace()
{
    void *trace[10];
    int traceSize = backtrace(trace, 10);
    backtrace_symbols_fd(trace, traceSize, STDERR_FILENO);
}

//==============================================================================
std::string SystemImpl::LocalTime(const std::string &fmt)
{
    auto sys = std::chrono::system_clock::now();
    time_t now = std::chrono::system_clock::to_time_t(sys);

    struct tm timeVal;

    if (localtime_r(&now, &timeVal) != NULL)
    {
        char timeStr[32];

        if (strftime(timeStr, sizeof(timeStr), fmt.c_str(), &timeVal) != 0)
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

    return "(" + std::to_string(error) + ") " + strerror(error);
}

//==============================================================================
void SystemImpl::SetupSignalHandler()
{
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    signal(SIGSYS, handleSignal);
    signal(SIGBUS, handleSignal);
    signal(SIGILL, handleSignal);
    signal(SIGFPE, handleSignal);
    signal(SIGABRT, handleSignal);
    signal(SIGSEGV, handleSignal);
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
