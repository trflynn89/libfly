#pragma once

#include <string>
#include <vector>

namespace fly {

/**
 * Linux declaration of the SystemImpl interface.
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
    static std::vector<int> get_signals();
};

} // namespace fly
