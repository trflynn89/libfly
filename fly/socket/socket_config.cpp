#include "fly/socket/socket_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==============================================================================
SocketConfig::SocketConfig() noexcept :
    m_defaultIoWaitTime(10000_i64),
    m_defaultEndOfMessage(0x04),
    m_defaultPacketSize(4096)
{
}

//==============================================================================
std::chrono::microseconds SocketConfig::IoWaitTime() const noexcept
{
    return std::chrono::microseconds(get_value<std::chrono::microseconds::rep>(
        "io_wait_time",
        m_defaultIoWaitTime));
}

//==============================================================================
char SocketConfig::EndOfMessage() const noexcept
{
    return get_value<char>("end_of_message", m_defaultEndOfMessage);
}

//==============================================================================
std::size_t SocketConfig::PacketSize() const noexcept
{
    return get_value<std::size_t>("packet_size", m_defaultPacketSize);
}

} // namespace fly
