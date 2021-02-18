#pragma once

#include "fly/net/socket/detail/base_socket.hpp"
#include "fly/net/socket/socket_types.hpp"

#include <cstddef>
#include <string>

namespace fly::net {

/**
 * Class to represent a connectionless datagram network socket.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <typename EndpointType>
class UdpSocket : public fly::net::detail::BaseSocket<EndpointType>
{
    using BaseSocket = fly::net::detail::BaseSocket<EndpointType>;

public:
    /**
     * Constructor. Open the socket in a synchronous IO processing mode.
     */
    UdpSocket() noexcept;

    /**
     * Constructor. Open the socket in the provided IO processing mode.
     *
     * @param mode IO processing mode to apply to the socket.
     */
    explicit UdpSocket(fly::net::IOMode mode) noexcept;

    /**
     * Move constructor. The provided socket is left in an invalid state.
     *
     * @param socket The socket instance to move.
     */
    UdpSocket(UdpSocket &&socket) noexcept;

    /**
     * Move assignment operator. The provided socket is left in an invalid state.
     *
     * @param socket The socket instance to move.
     *
     * @return A reference to this socket.
     */
    UdpSocket &operator=(UdpSocket &&socket) noexcept;

    /**
     * Transmit a message to a specific remote endpoint.
     *
     * @param endpoint The remote endpoint to transmit to.
     * @param message The message to transmit.
     *
     * @return The number of bytes transmitted.
     */
    std::size_t send(const EndpointType &endpoint, std::string_view message) const;

    /**
     * Transmit a message to a specific remote endpoint.
     *
     * @param hostname The hostname or IP address string to transmit to.
     * @param port The port to transmit to.
     * @param message The message to transmit.
     *
     * @return The number of bytes transmitted.
     */
    std::size_t send(std::string_view hostname, port_type port, std::string_view message) const;

    /**
     * Receive a message from an unspecified remote endpoint.
     *
     * @return The message received.
     */
    std::string receive() const;

    using BaseSocket::handle;
    using BaseSocket::hostname_to_endpoint;

private:
    UdpSocket(const UdpSocket &) = delete;
    UdpSocket &operator=(const UdpSocket &) = delete;

    using BaseSocket::m_packet_size;
};

} // namespace fly::net
