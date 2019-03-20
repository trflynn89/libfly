#include "fly/path/path_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
PathConfig::PathConfig() noexcept : m_defaultPollInterval(I64(1000))
{
}

//==============================================================================
std::chrono::milliseconds PathConfig::PollInterval() const noexcept
{
    return std::chrono::milliseconds(GetValue<std::chrono::milliseconds::rep>(
        "poll_interval", m_defaultPollInterval));
}

} // namespace fly
