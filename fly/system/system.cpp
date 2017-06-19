#include "fly/system/system.h"

#include <csignal>
#include <vector>

#if defined(FLY_WINDOWS)
    #include "fly/system/win/system_impl.h"
#elif defined(FLY_LINUX)
    #include "fly/system/nix/system_impl.h"
#endif

namespace fly {

//==============================================================================
void System::PrintBacktrace()
{
    SystemImpl::PrintBacktrace();
}

//==============================================================================
std::string System::LocalTime()
{
    return SystemImpl::LocalTime("%m-%d-%Y %H:%M:%S");
}

//==============================================================================
int System::GetErrorCode()
{
    return SystemImpl::GetErrorCode();
}

//==============================================================================
std::string System::GetErrorString()
{
    return GetErrorString(GetErrorCode());
}

//==============================================================================
std::string System::GetErrorString(int code)
{
    return SystemImpl::GetErrorString(code);
}

//==============================================================================
void System::SetSignalHandler(SignalHandler handler)
{
    static std::vector<int> signals = SystemImpl::GetSignals();

    auto ppHandler = handler.target<void (*)(int)>();
    auto pHandler = (ppHandler == NULL) ? SIG_DFL : *ppHandler;

    for (auto it = signals.begin(); it != signals.end(); ++it)
    {
        std::signal(*it, pHandler);
    }
}

}
