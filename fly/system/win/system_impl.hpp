#pragma once

#include <array>
#include <csignal>
#include <string>

namespace fly {

/**
 * Windows declaration of the SystemImpl interface.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 2, 2016
 */
class SystemImpl
{
public:
    static void print_backtrace();
    static std::string local_time(const char *fmt);
    static int get_error_code();

    static constexpr std::array<int, 6> fatal_signals()
    {
        return {SIGINT, SIGTERM, SIGILL, SIGFPE, SIGABRT, SIGSEGV};
    }
};

} // namespace fly
