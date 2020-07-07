#include "fly/system/system.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"

#include <csignal>
#include <system_error>
#include <vector>

#include FLY_OS_IMPL_PATH(system, system)

namespace fly {

//==================================================================================================
void System::print_backtrace()
{
    SystemImpl::print_backtrace();
}

//==================================================================================================
std::string System::local_time()
{
    return SystemImpl::local_time("%m-%d-%Y %H:%M:%S");
}

//==================================================================================================
int System::get_error_code()
{
    return SystemImpl::get_error_code();
}

//==================================================================================================
std::string System::get_error_string()
{
    return get_error_string(get_error_code());
}

//==================================================================================================
std::string System::get_error_string(int code)
{
    return fly::String::format("(%d) %s", code, std::system_category().message(code));
}

//==================================================================================================
void System::set_signal_handler(SignalHandler handler)
{
    static std::vector<int> s_signals = SystemImpl::get_signals();

    auto pp_handler = handler.target<void (*)(int)>();
    auto p_handler = (pp_handler == nullptr) ? SIG_DFL : *pp_handler;

    for (auto it = s_signals.begin(); it != s_signals.end(); ++it)
    {
        std::signal(*it, p_handler);
    }
}

} // namespace fly
