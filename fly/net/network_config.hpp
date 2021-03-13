#pragma once

#include "fly/config/config.hpp"

#include <chrono>
#include <cstddef>

namespace fly::net {

/**
 * Class to hold networking-related configuration values.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 6, 2021
 */
class NetworkConfig : public fly::config::Config
{
public:
    static constexpr const char *identifier = "network";

    /**
     * @return Sleep time for socket polling sequence.
     */
    std::chrono::microseconds socket_io_wait_time() const;

    /**
     * @return Size of packet to use for socket IO operations.
     */
    std::size_t packet_size() const;

protected:
    std::chrono::microseconds::rep m_default_socket_io_wait_time {10'000};
    std::size_t m_default_packet_size {4096};
};

} // namespace fly::net
