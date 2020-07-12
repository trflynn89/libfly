#pragma once

#include "fly/config/config.hpp"

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
     * @return Sleep time for socket IO thread.
     */
    std::chrono::microseconds io_wait_time() const;

    /**
     * @return Character signifying the end of a message received over a socket.
     */
    char end_of_message() const;

    /**
     * Size of packet to use for send/receive operations.
     */
    std::size_t packet_size() const;

protected:
    std::chrono::microseconds::rep m_default_io_wait_time {10000};
    char m_default_end_of_message {0x04};
    std::size_t m_default_packet_size {4096};
};

} // namespace fly
