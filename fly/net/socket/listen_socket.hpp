#pragma once

#include "fly/net/socket/detail/base_socket.hpp"

#include <optional>

namespace fly::net {

template <typename EndpointType>
class TcpSocket;

/**
 * Class to represent a listening socket for accepting incoming network connection requests from
 * connection-oriented sockets.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <typename EndpointType>
class ListenSocket : public fly::net::detail::BaseSocket<EndpointType>
{
    using BaseSocket = fly::net::detail::BaseSocket<EndpointType>;

public:
    /**
     * Constructor. Open the socket in a synchronous IO processing mode.
     */
    ListenSocket() noexcept;

    /**
     * Constructor. Open the socket in the provided IO processing mode.
     *
     * @param mode IO processing mode to apply to the socket.
     */
    explicit ListenSocket(fly::net::IOMode mode) noexcept;

    /**
     * Move constructor. The provided socket is left in a non-listening, invalid state.
     *
     * @param socket The socket instance to move.
     */
    ListenSocket(ListenSocket &&socket) noexcept;

    /**
     * Move assignment operator. The provided socket is left in a non-listening, invalid state.
     *
     * @param socket The socket instance to move.
     *
     * @return A reference to this socket.
     */
    ListenSocket &operator=(ListenSocket &&socket) noexcept;

    /**
     * Configure this socket to be used to accept incoming connections.
     *
     * @return True if the operation was successful.
     */
    bool listen();

    /**
     * @return True if this socket is listening for incoming connections.
     */
    bool is_listening() const;

    /**
     * Accept an incoming connection on this listening socket.
     *
     * @return If successful, the accepted socket. Otherwise, an uninitialized value.
     */
    std::optional<TcpSocket<EndpointType>> accept() const;

    using BaseSocket::handle;
    using BaseSocket::io_mode;

private:
    bool m_is_listening {false};
};

} // namespace fly::net
