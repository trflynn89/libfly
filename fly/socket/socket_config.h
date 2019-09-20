#pragma once

#include "fly/config/config.h"

#include <chrono>

namespace fly {

/**
 * Class to hold networking-related configuration values.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 19, 2016
 */
class SocketConfig : public Config
{
public:
    static constexpr const char *identifier = "socket";

    /**
     * Constructor.
     */
    SocketConfig() noexcept;

    /**
     * @return Sleep time for socket IO thread.
     */
    std::chrono::microseconds IoWaitTime() const noexcept;

    /**
     * @return Character signifying the end of a message received over a socket.
     */
    char EndOfMessage() const noexcept;

    /**
     * Size of packet to use for send/receive operations.
     */
    std::size_t PacketSize() const noexcept;

protected:
    std::chrono::microseconds::rep m_defaultIoWaitTime;
    char m_defaultEndOfMessage;
    std::size_t m_defaultPacketSize;
};

} // namespace fly
