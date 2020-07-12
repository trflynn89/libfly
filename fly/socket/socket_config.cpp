#include "fly/socket/socket_config.hpp"

namespace fly {

//==================================================================================================
std::chrono::microseconds SocketConfig::io_wait_time() const
{
    return std::chrono::microseconds(
        get_value<std::chrono::microseconds::rep>("io_wait_time", m_default_io_wait_time));
}

//==================================================================================================
char SocketConfig::end_of_message() const
{
    return get_value<char>("end_of_message", m_default_end_of_message);
}

//==================================================================================================
std::size_t SocketConfig::packet_size() const
{
    return get_value<std::size_t>("packet_size", m_default_packet_size);
}

} // namespace fly
