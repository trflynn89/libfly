#include "fly/path/path_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==============================================================================
PathConfig::PathConfig() noexcept : m_defaultPollInterval(1000_i64)
{
}

//==============================================================================
std::chrono::milliseconds PathConfig::PollInterval() const noexcept
{
    return std::chrono::milliseconds(get_value<std::chrono::milliseconds::rep>(
        "poll_interval",
        m_defaultPollInterval));
}

} // namespace fly
