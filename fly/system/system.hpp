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
    static void print_backtrace();

    /**
     * @return The local time formatted as a string.
     */
    static std::string local_time();

    /**
     * @return The last system error code.
     */
    static int get_error_code();

    /**
     * @return The last system error code as a string.
     */
    static std::string get_error_string();

    /**
     * Convert a system error code to a string.
     *
     * @param code The system error code to convert.
     *
     * @return The given system error code as a string.
     */
    static std::string get_error_string(int code);

    /**
     * Set a signal handler for all terminal signals.
     *
     * @param handler The signal handler function to set.
     */
    static void set_signal_handler(SignalHandler handler);
};

} // namespace fly
