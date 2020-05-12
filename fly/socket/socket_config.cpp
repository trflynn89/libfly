#include "fly/socket/socket_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==============================================================================
SocketConfig::SocketConfig() noexcept :
    m_default_io_wait_time(10000_i64),
    m_default_end_of_message(0x04),
    m_default_packet_size(4096)
{
}

//==============================================================================
std::chrono::microseconds SocketConfig::io_wait_time() const noexcept
{
    return std::chrono::microseconds(get_value<std::chrono::microseconds::rep>(
        "io_wait_time",
        m_default_io_wait_time));
}

//==============================================================================
char SocketConfig::end_of_message() const noexcept
{
    return get_value<char>("end_of_message", m_default_end_of_message);
}

//==============================================================================
std::size_t SocketConfig::packet_size() const noexcept
{
    return get_value<std::size_t>("packet_size", m_default_packet_size);
}

} // namespace fly
