#pragma once

#include <functional>
#include <string>

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
     * @return The last system error code.
     */
    static int GetErrorCode();

    /**
     * @return The last system error code as a string.
     */
    static std::string GetErrorString();

    /**
     * Convert a system error code to a string.
     *
     * @param int The system error code to convert.
     *
     * @return The given system error code as a string.
     */
    static std::string GetErrorString(int);

    /**
     * Set a signal handler for all terminal signals.
     *
     * @param SignalHandler The signal handler function to set.
     */
    static void SetSignalHandler(SignalHandler);
};

} // namespace fly
