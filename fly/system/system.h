#pragma once

#include <sstream>
#include <string>

#include <fly/fly.h>
#include <fly/exit_codes.h>

namespace fly {

/**
 * Static class to provide interface to system calls.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 2, 2016
 */
class System
{
public:
    /**
     * Create a directory and the path to that directory, if needed.
     *
     * @param std::string Path to the directory to create.
     *
     * @return True if the directory could be created (or already exists).
     */
    FLY_API static bool MakeDirectory(const std::string &);

    /**
     * @return The system's path separator.
     */
    FLY_API static char GetSeparator();

    /**
     * @return The system's temporary directory path.
     */
    FLY_API static std::string GetTempDirectory();

    /**
     * Print the backtrace to stderr.
     */
    FLY_API static void PrintBacktrace();

    /**
     * @return The local time formatted as a string.
     */
    FLY_API static std::string LocalTime();

    /**
     * Get the last system error as a string, and optionally store the numeric
     * error code.
     *
     * @param int* Pointer to store the numeric error code, or NULL.
     *
     * @return The last system error as a string.
     */
    FLY_API static std::string GetLastError(int *code = NULL);

    /**
     * Setup handlers for fatal and non-fatal exit codes, to allow the system
     * to cleanly exit;
     */
    FLY_API static void SetupSignalHandler();

    /**
     * Signal the main thread to exit with the given exit code.
     *
     * @param ExitCode Code to exit with.
     */
    FLY_API static void CleanExit(ExitCode);

    /**
     * @return Whether the system is in a state in which it should keep running.
     */
    FLY_API static bool KeepRunning();

    /**
     * @return The code the system should exit with.
     */
    FLY_API static ExitCode GetExitCode();
};

}
