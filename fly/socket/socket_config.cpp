#include "fly/socket/socket_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
SocketConfig::SocketConfig() noexcept :
    m_defaultIoWaitTime(I64(10000)),
    m_defaultEndOfMessage(0x04),
    m_defaultPacketSize(4096)
{
}

//==============================================================================
std::chrono::microseconds SocketConfig::IoWaitTime() const noexcept
{
    return std::chrono::microseconds(GetValue<std::chrono::microseconds::rep>(
        "io_wait_time", m_defaultIoWaitTime));
}

//==============================================================================
char SocketConfig::EndOfMessage() const noexcept
{
    return GetValue<char>("end_of_message", m_defaultEndOfMessage);
}

//==============================================================================
std::size_t SocketConfig::PacketSize() const noexcept
{
    return GetValue<std::size_t>("packet_size", m_defaultPacketSize);
}

} // namespace fly
