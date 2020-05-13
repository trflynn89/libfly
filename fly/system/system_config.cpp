#include "fly/system/system_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==============================================================================
SystemConfig::SystemConfig() noexcept : m_default_poll_interval(1000_i64)
{
}
//==============================================================================
std::chrono::milliseconds SystemConfig::poll_interval() const noexcept
{
    return std::chrono::milliseconds(get_value<std::chrono::milliseconds::rep>(
        "poll_interval",
        m_default_poll_interval));
}

} // namespace fly
