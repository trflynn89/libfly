#pragma once

#include <functional>
#include <string>

namespace fly::system {

/**
 * Static class to provide interface to system calls.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 2, 2016
 */
using SignalHandler = std::function<void(int)>;

/**
 * Print the backtrace to stderr.
 */
void print_backtrace();

/**
 * @return The local time formatted as a string.
 */
std::string local_time();

/**
 * @return The last system error code.
 */
int get_error_code();

/**
 * @return The last system error code as a string.
 */
std::string get_error_string();

/**
 * Convert a system error code to a string.
 *
 * @param code The system error code to convert.
 *
 * @return The given system error code as a string.
 */
std::string get_error_string(int code);

/**
 * Set a signal handler for all terminal signals.
 *
 * @param handler The signal handler function to set.
 */
void set_signal_handler(SignalHandler handler);

} // namespace fly::system
