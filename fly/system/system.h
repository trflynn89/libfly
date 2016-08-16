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
    static bool MakeDirectory(const std::string &);

    /**
     * @return The system's path separator.
     */
    static char GetSeparator();

    /**
     * @return The system's temporary directory path.
     */
    static std::string GetTempDirectory();

    /**
     * Print the backtrace to stderr.
     */
    static void PrintBacktrace();

    /**
     * @return The local time formatted as a string.
     */
    static std::string LocalTime();

    /**
     * Get the last system error as a string, and optionally store the numeric
     * error code.
     *
     * @param int* Pointer to store the numeric error code, or NULL.
     *
     * @return The last system error as a string.
     */
    static std::string GetLastError(int *code = NULL);

    /**
     * Setup handlers for fatal and non-fatal exit codes, to allow the system
     * to cleanly exit;
     */
    static void SetupSignalHandler();

    /**
     * Signal the main thread to exit with the given exit code.
     *
     * @param ExitCode Code to exit with.
     */
    static void CleanExit(ExitCode);

    /**
     * @return Whether the system is in a state in which it should keep running.
     */
    static bool KeepRunning();

    /**
     * @return The code the system should exit with.
     */
    static ExitCode GetExitCode();
};

}
