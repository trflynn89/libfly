#include "fly/system/system_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==============================================================================
SystemConfig::SystemConfig() noexcept : m_defaultPollInterval(1000_i64)
{
}
//==============================================================================
std::chrono::milliseconds SystemConfig::PollInterval() const noexcept
{
    return std::chrono::milliseconds(get_value<std::chrono::milliseconds::rep>(
        "poll_interval",
        m_defaultPollInterval));
}

} // namespace fly
