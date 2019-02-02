#include "fly/system/system_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
SystemConfig::SystemConfig() : m_defaultPollInterval(I64(1000))
{
}

//==============================================================================
std::string SystemConfig::GetName()
{
    return "system";
}

//==============================================================================
std::chrono::milliseconds SystemConfig::PollInterval() const
{
    return std::chrono::milliseconds(GetValue<std::chrono::milliseconds::rep>(
        "poll_interval", m_defaultPollInterval));
}

} // namespace fly
