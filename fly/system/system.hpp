#pragma once

#include <functional>
#include <string>

namespace fly {

/**
 * Static class to provide interface to system calls.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 2, 2016
 */
class System
{
public:
    using SignalHandler = std::function<void(int)>;

    /**
     * Print the backtrace to stderr.
     */
    static void PrintBacktrace() noexcept;

    /**
     * @return The local time formatted as a string.
     */
    static std::string LocalTime() noexcept;

    /**
     * @return The last system error code.
     */
    static int GetErrorCode() noexcept;

    /**
     * @return The last system error code as a string.
     */
    static std::string GetErrorString() noexcept;

    /**
     * Convert a system error code to a string.
     *
     * @param int The system error code to convert.
     *
     * @return The given system error code as a string.
     */
    static std::string GetErrorString(int) noexcept;

    /**
     * Set a signal handler for all terminal signals.
     *
     * @param SignalHandler The signal handler function to set.
     */
    static void SetSignalHandler(SignalHandler) noexcept;
};

} // namespace fly
