#include "system.h"

#if defined(FLY_WINDOWS)
    #include "win_system.h"
#elif defined(FLY_LINUX)
    #include "nix_system.h"
#endif

namespace fly {

//==============================================================================
bool System::MakeDirectory(const std::string &path)
{
    return SystemImpl::MakeDirectory(path);
}

//==============================================================================
char System::GetSeparator()
{
    return SystemImpl::GetSeparator();
}

//==============================================================================
std::string System::GetTempDirectory()
{
    std::string ret = SystemImpl::GetTempDirectory();

    if (ret.back() == System::GetSeparator())
    {
        ret = ret.substr(0, ret.size() - 1);
    }

    return ret;
}

//==============================================================================
void System::PrintBacktrace()
{
    SystemImpl::PrintBacktrace();
}

//==============================================================================
std::string System::LocalTime()
{
    return SystemImpl::LocalTime("%m-%d-%Y %H:%M:%S");
}

//==============================================================================
std::string System::GetLastError(int *code)
{
    return SystemImpl::GetLastError(code);
}

//==============================================================================
void System::SetupSignalHandler()
{
    return SystemImpl::SetupSignalHandler();
}

//==============================================================================
void System::CleanExit(ExitCode exitCode)
{
    return SystemImpl::CleanExit(exitCode);
}

//==============================================================================
bool System::KeepRunning()
{
    return SystemImpl::KeepRunning();
}

//==============================================================================
ExitCode System::GetExitCode()
{
    return SystemImpl::GetExitCode();
}

}
