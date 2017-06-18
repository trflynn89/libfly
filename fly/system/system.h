#pragma once

#include <functional>
#include <string>

#include "fly/fly.h"

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
    typedef std::function<void(int)> SignalHandler;

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
     * Set a signal handler for all terminal signals.
     *
     * @param SignalHandler The signal handler function to set.
     */
    static void SetSignalHandler(SignalHandler);
};

}
