#include "fly/path/path_config.hpp"

namespace fly::path {

//==================================================================================================
std::chrono::milliseconds PathConfig::poll_interval() const
{
    return std::chrono::milliseconds(
        get_value<std::chrono::milliseconds::rep>("poll_interval", m_default_poll_interval));
}

} // namespace fly::path
