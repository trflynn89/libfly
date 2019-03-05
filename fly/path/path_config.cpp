#include "fly/path/path_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
PathConfig::PathConfig() : m_defaultPollInterval(I64(1000))
{
}

//==============================================================================
std::string PathConfig::GetName()
{
    return "path";
}

//==============================================================================
std::chrono::milliseconds PathConfig::PollInterval() const
{
    return std::chrono::milliseconds(GetValue<std::chrono::milliseconds::rep>(
        "poll_interval", m_defaultPollInterval));
}

} // namespace fly
