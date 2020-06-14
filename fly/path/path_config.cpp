#include "fly/path/path_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==================================================================================================
PathConfig::PathConfig() noexcept : m_default_poll_interval(1000_i64)
{
}

//==================================================================================================
std::chrono::milliseconds PathConfig::poll_interval() const
{
    return std::chrono::milliseconds(
        get_value<std::chrono::milliseconds::rep>("poll_interval", m_default_poll_interval));
}

} // namespace fly
