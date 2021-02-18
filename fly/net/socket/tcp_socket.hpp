#pragma once

#include "fly/net/socket/detail/base_socket.hpp"
#include "fly/net/socket/socket_types.hpp"

#include <cstddef>
#include <optional>
#include <string>

namespace fly::net {

template <typename EndpointType>
class ListenSocket;

/**
 * Class to represent a connection-oriented streaming network socket.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <typename EndpointType>
class TcpSocket : public fly::net::detail::BaseSocket<EndpointType>
{
    using BaseSocket = fly::net::detail::BaseSocket<EndpointType>;

public:
    /**
     * Constructor. Open the socket in a synchronous IO processing mode.
     */
    TcpSocket() noexcept;

    /**
     * Constructor. Open the socket in the provided IO processing mode.
     *
     * @param mode IO processing mode to apply to the socket.
     */
    explicit TcpSocket(fly::net::IOMode mode) noexcept;

    /**
     * Move constructor. The provided socket is left in a disconnected, invalid state.
     *
     * @param socket The socket instance to move.
     */
    TcpSocket(TcpSocket &&socket) noexcept;

    /**
     * Move assignment operator. The provided socket is left in a disconnected, invalid state.
     *
     * @param socket The socket instance to move.
     *
     * @return A reference to this socket.
     */
    TcpSocket &operator=(TcpSocket &&socket) noexcept;

    /**
     * Retrieve the remote endpoint to which this socket is connected.
     *
     * @return If successful, the connected endpoint. Otherwise, an uninitialized value.
     */
    std::optional<EndpointType> remote_endpoint() const;

    /**
     * Connect to a remote socket. If this socket was opened in an asynchronous IO processing mode,
     * the connection may not complete immediately. In that case, the connection must be completed
     * via finish_connect() after the socket becomes writable.
     *
     * @param endpoint The remote endpoint to connect to.
     *
     * @return The connection state (disconnected, connecting, or connected).
     */
    ConnectedState connect(const EndpointType &endpoint);

    /**
     * Connect to a remote socket. If this socket was opened in an asynchronous IO processing mode,
     * the connection may not complete immediately. In that case, the connection must be completed
     * via finish_connect() after the socket becomes writable.
     *
     * @param hostname The hostname or IP address string to connect to.
     * @param port The port to connect to.
     *
     * @return The connection state (disconnected, connecting, or connected).
     */
    ConnectedState connect(std::string_view hostname, port_type port);

    /**
     * After an asynchronous socket in a connecting state becomes available for writing, verify the
     * socket is healthy and update its state as connected.
     *
     * @return The connection state (disconnected or connected).
     */
    ConnectedState finish_connect();

    /**
     * @return True if this socket is connecting to a remote endpoint.
     */
    bool is_connecting() const;

    /**
     * @return True if this socket is connected to a remote endpoint.
     */
    bool is_connected() const;

    /**
     * Transmit a message to the connected remote socket.
     *
     * @param message The message to transmit.
     *
     * @return The number of bytes transmitted.
     */
    std::size_t send(std::string_view message) const;

    /**
     * Receive a message from the connected remote socket.
     *
     * @return The message received.
     */
    std::string receive() const;

    using BaseSocket::close;
    using BaseSocket::handle;
    using BaseSocket::hostname_to_endpoint;
    using BaseSocket::is_valid;

private:
    friend ListenSocket<EndpointType>;

    /**
     * Constructor. Create a socket with an already-opened socket handle and the provided IO
     * processing mode.
     *
     * @param handle Native socket handle opened by the concrete socket type.
     * @param mode IO processing mode to apply to the socket.
     */
    TcpSocket(socket_type handle, fly::net::IOMode mode) noexcept;

    TcpSocket(const TcpSocket &) = delete;
    TcpSocket &operator=(const TcpSocket &) = delete;

    using BaseSocket::m_packet_size;

    std::atomic<ConnectedState> m_connected_state {ConnectedState::Disconnected};
};

} // namespace fly::net
