#pragma once

#include <string>

#include <fly/fly.h>
#include <fly/exit_codes.h>

namespace fly {

/**
 * Linux declaration of the SystemImpl interface.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 2, 2016
 */
class SystemImpl
{
public:
    FLY_API static bool MakeDirectory(const std::string &);
    FLY_API static char GetSeparator();
    FLY_API static std::string GetTempDirectory();
    FLY_API static void PrintBacktrace();
    FLY_API static std::string LocalTime(const std::string &);
    FLY_API static std::string GetLastError(int *);
    FLY_API static void SetupSignalHandler();
    FLY_API static void CleanExit(ExitCode);
    FLY_API static bool KeepRunning();
    FLY_API static ExitCode GetExitCode();
};

}
