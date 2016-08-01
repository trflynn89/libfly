#include "socket_config.h"

namespace fly {

//==============================================================================
SocketConfig::SocketConfig()
{
}

//==============================================================================
SocketConfig::~SocketConfig()
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
        GetValue<std::chrono::microseconds::rep>("io_wait_time", I64(10000))
    );
}

//==============================================================================
char SocketConfig::EndOfMessage() const
{
    return GetValue<char>("end_of_message", 0x04);
}

//==============================================================================
size_t SocketConfig::PacketSize() const
{
    return GetValue<size_t>("packet_size", 4096);
}

}
