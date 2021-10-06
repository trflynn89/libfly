#pragma once

#include "fly/net/socket/detail/base_socket.hpp"
#include "fly/net/socket/socket_concepts.hpp"

#include <functional>
#include <memory>
#include <optional>

namespace fly::net {

class NetworkConfig;
class SocketService;

template <IPEndpoint EndpointType>
class TcpSocket;

/**
 * Class to represent a listening socket for accepting incoming network connection requests from
 * connection-oriented sockets.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <IPEndpoint EndpointType>
class ListenSocket :
    public detail::BaseSocket<EndpointType>,
    public std::enable_shared_from_this<ListenSocket<EndpointType>>
{
    using BaseSocket = detail::BaseSocket<EndpointType>;

    using AcceptCompletion = std::function<void(std::shared_ptr<TcpSocket<EndpointType>>)>;

public:
    /**
     * Constructor. Open the socket in a synchronous IO processing mode.
     *
     * @param config Reference to network configuration.
     */
    explicit ListenSocket(std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * Constructor. Open the socket in the provided IO processing mode.
     *
     * @param config Reference to network configuration.
     * @param mode IO processing mode to apply to the socket.
     */
    ListenSocket(std::shared_ptr<NetworkConfig> config, IOMode mode) noexcept;

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
     * Accept an incoming connection on this listening socket. If an error occurs on the socket, the
     * socket will be closed.
     *
     * @return If successful, the accepted socket. Otherwise, an uninitialized value.
     */
    std::optional<TcpSocket<EndpointType>> accept();

    /**
     * Asynchronously accept an incoming connection on this listening socket. May only be used if
     * this socket was created through a socket service.
     *
     * If successful, the provided callback will be invoked with the accepted socket upon
     * completion. Otherwise, the provided callback will be invoked with an uninitialized value, and
     * the socket will be closed before the invocation.
     *
     * @param callback The callback to invoke when the operation has completed.
     *
     * @return True if the socket service and the provided callback are valid.
     */
    bool accept_async(AcceptCompletion &&callback);

    using BaseSocket::close;
    using BaseSocket::handle;
    using BaseSocket::io_mode;

private:
    friend SocketService;

    /**
     * Create an asynchronous socket armed with a socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     *
     * @return The created socket.
     */
    static std::shared_ptr<ListenSocket> create_socket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config);

    /**
     * Constructor. Open the socket in an asynchronous IO processing mode armed with the provided
     * socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     */
    ListenSocket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * When the socket service indicates the socket is available for reading, attempt to accept an
     * incoming connection. If successful, the provided callback will be invoked with the accepted
     * socket. If unsuccessful because the operation would still block, queue another attempt.
     * Otherwise, the socket will be closed and the callback will be invoked with an uninitialized
     * value.
     *
     * @param callback The callback to invoke when the operation has completed.
     */
    void ready_to_accept(AcceptCompletion &&callback);

    using BaseSocket::network_config;
    using BaseSocket::socket_service;

    bool m_is_listening {false};
};

} // namespace fly::net
