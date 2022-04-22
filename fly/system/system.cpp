#include "fly/system/system.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"

#include <array>
#include <csignal>
#include <system_error>

namespace fly::system {

namespace {

    SignalHandler s_signal_handler = nullptr;

    void handle_signal(int signal)
    {
        std::invoke(s_signal_handler, signal);
    }

} // namespace

//==================================================================================================
std::string get_error_string()
{
    return get_error_string(get_error_code());
}

//==================================================================================================
std::string get_error_string(int code)
{
    return fly::String::format("({}) {}", code, std::system_category().message(code));
}

//==================================================================================================
void set_signal_handler(SignalHandler handler)
{
#if defined(FLY_WINDOWS)
    static constexpr std::array<int, 6> const
        s_signals {SIGINT, SIGTERM, SIGILL, SIGFPE, SIGABRT, SIGSEGV};
#else
    static constexpr std::array<int, 8> const
        s_signals {SIGINT, SIGTERM, SIGILL, SIGFPE, SIGABRT, SIGSEGV, SIGSYS, SIGBUS};
#endif

    auto handler_or_default = handler ? handle_signal : SIG_DFL;
    s_signal_handler = std::move(handler);

    for (int signal : s_signals)
    {
        std::signal(signal, handler_or_default);
    }
}

} // namespace fly::system
