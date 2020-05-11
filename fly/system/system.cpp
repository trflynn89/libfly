#include "fly/system/system.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"

#include <csignal>
#include <system_error>
#include <vector>

#include FLY_OS_IMPL_PATH(system, system)

namespace fly {

//==============================================================================
void System::PrintBacktrace() noexcept
{
    SystemImpl::PrintBacktrace();
}

//==============================================================================
std::string System::LocalTime() noexcept
{
    return SystemImpl::LocalTime("%m-%d-%Y %H:%M:%S");
}

//==============================================================================
int System::GetErrorCode() noexcept
{
    return SystemImpl::GetErrorCode();
}

//==============================================================================
std::string System::GetErrorString() noexcept
{
    return GetErrorString(GetErrorCode());
}

//==============================================================================
std::string System::GetErrorString(int code) noexcept
{
    return fly::String::format(
        "(%d) %s",
        code,
        std::system_category().message(code));
}

//==============================================================================
void System::SetSignalHandler(SignalHandler handler) noexcept
{
    static std::vector<int> signals = SystemImpl::GetSignals();

    auto ppHandler = handler.target<void (*)(int)>();
    auto pHandler = (ppHandler == nullptr) ? SIG_DFL : *ppHandler;

    for (auto it = signals.begin(); it != signals.end(); ++it)
    {
        std::signal(*it, pHandler);
    }
}

} // namespace fly
