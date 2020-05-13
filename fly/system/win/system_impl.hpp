#pragma once

#include <string>
#include <vector>

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
    static void print_backtrace() noexcept;
    static std::string local_time(const char *fmt) noexcept;
    static int get_error_code() noexcept;
    static std::vector<int> get_signals() noexcept;
};

} // namespace fly
