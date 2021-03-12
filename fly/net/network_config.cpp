#include "fly/net/network_config.hpp"

namespace fly::net {

//==================================================================================================
std::chrono::microseconds NetworkConfig::socket_io_wait_time() const
{
    return std::chrono::microseconds(get_value<std::chrono::microseconds::rep>(
        "socket_io_wait_time",
        m_default_socket_io_wait_time));
}

//==================================================================================================
std::size_t NetworkConfig::packet_size() const
{
    return get_value<std::size_t>("packet_size", m_default_packet_size);
}

} // namespace fly::net
