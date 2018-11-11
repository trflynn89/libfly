#include "fly/socket/socket_config.h"

#include "fly/fly.h"

namespace fly {

//==============================================================================
SocketConfig::SocketConfig() :
    m_defaultIoWaitTime(I64(10000)),
    m_defaultEndOfMessage(0x04),
    m_defaultPacketSize(4096)
{
}

//==============================================================================
std::string SocketConfig::GetName()
{
    return "socket";
}

//==============================================================================
std::chrono::microseconds SocketConfig::IoWaitTime() const
{
    return std::chrono::microseconds(
        GetValue<std::chrono::microseconds::rep>(
            "io_wait_time", m_defaultIoWaitTime
        )
    );
}

//==============================================================================
char SocketConfig::EndOfMessage() const
{
    return GetValue<char>("end_of_message", m_defaultEndOfMessage);
}

//==============================================================================
size_t SocketConfig::PacketSize() const
{
    return GetValue<size_t>("packet_size", m_defaultPacketSize);
}

}
