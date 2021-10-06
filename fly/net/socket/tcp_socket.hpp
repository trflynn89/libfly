#pragma once

#include "fly/net/socket/concepts.hpp"
#include "fly/net/socket/detail/base_socket.hpp"
#include "fly/net/socket/types.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace fly::net {

class NetworkConfig;
class SocketService;

template <IPEndpoint EndpointType>
class ListenSocket;

/**
 * Class to represent a connection-oriented streaming network socket.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <IPEndpoint EndpointType>
class TcpSocket :
    public detail::BaseSocket<EndpointType>,
    public std::enable_shared_from_this<TcpSocket<EndpointType>>
{
    using BaseSocket = detail::BaseSocket<EndpointType>;

    using ConnectCompletion = std::function<void(ConnectedState)>;
    using SendCompletion = std::function<void(std::size_t)>;
    using ReceiveCompletion = std::function<void(std::string)>;

public:
    /**
     * Constructor. Open the socket in a synchronous IO processing mode.
     *
     * @param config Reference to network configuration.
     */
    explicit TcpSocket(std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * Constructor. Open the socket in the provided IO processing mode.
     *
     * @param config Reference to network configuration.
     * @param mode IO processing mode to apply to the socket.
     */
    TcpSocket(std::shared_ptr<NetworkConfig> config, IOMode mode) noexcept;

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
     * Asynchronously connect to a remote socket. May only be used if this socket was created
     * through a socket service.
     *
     * A connection attempt will be made immediately. If successful, the provided callback will not
     * invoked; rather, the appropriate connection state will be returned.
     *
     * If the immediate attempt fails because the operation would have blocked, the attempt will be
     * completed asynchronously later. The provided callback will be invoked upon completion with
     * the new connection state.
     *
     * @param endpoint The remote endpoint to connect to.
     * @param callback The callback to invoke if the operation completes asynchronously.
     *
     * @return The connection state (disconnected, connecting, or connected).
     */
    ConnectedState connect_async(const EndpointType &endpoint, ConnectCompletion &&callback);

    /**
     * Asynchronously connect to a remote socket. May only be used if this socket was created
     * through a socket service.
     *
     * A connection attempt will be made immediately. If successful, the provided callback will not
     * invoked; rather, the appropriate connection state will be returned.
     *
     * If the immediate attempt fails because the operation would have blocked, the attempt will be
     * completed asynchronously later. The provided callback will be invoked upon completion with
     * the new connection state.
     *
     * @param hostname The hostname or IP address string to connect to.
     * @param port The port to connect to.
     * @param callback The callback to invoke if the operation completes asynchronously.
     *
     * @return The connection state (disconnected, connecting, or connected).
     */
    ConnectedState
    connect_async(std::string_view hostname, port_type port, ConnectCompletion &&callback);

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
     * Transmit a message to the connected remote socket. If an error occurs on the socket, the
     * socket will be closed.
     *
     * @param message The message to transmit.
     *
     * @return The number of bytes transmitted.
     */
    std::size_t send(std::string_view message);

    /**
     * Asynchronously transmit a message to the connected remote socket. May only be used if this
     * socket was created through a socket service.
     *
     * Upon completion, the provided callback will be invoked with the number of bytes that were
     * transmitted. If an error occurs on the socket, the callback will still be invoked with the
     * number of bytes successfully transmitted, but the socket will also be closed before the
     * invocation.
     *
     * @param message The message to transmit.
     * @param callback The callback to invoke when the operation has completed.
     *
     * @return True if the socket service and the provided callback are valid.
     */
    bool send_async(std::string_view message, SendCompletion &&callback);

    /**
     * Receive a message from the connected remote socket. If an error occurs on the socket, the
     * socket will be closed.
     *
     * @return The message received.
     */
    std::string receive();

    /**
     * Asynchronously receive a message from the connected remote socket. May only be used if this
     * socket was created through a socket service.
     *
     * Upon completion, the provided callback will be invoked with the message received. If an error
     * occurs on the socket, the callback will still be invoked with any message partially received,
     * but the socket will also be closed before the invocation.
     *
     * @param callback The callback to invoke when the operation has completed.
     *
     * @return True if the socket service and the provided callback are valid.
     */
    bool receive_async(ReceiveCompletion &&callback);

    using BaseSocket::close;
    using BaseSocket::handle;
    using BaseSocket::hostname_to_address;
    using BaseSocket::is_open;

private:
    friend ListenSocket<EndpointType>;
    friend SocketService;

    /**
     * Create an asynchronous socket armed with a socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     *
     * @return The created socket.
     */
    static std::shared_ptr<TcpSocket> create_socket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config);

    /**
     * Create an asynchronous socket with an already-opened socket handle armed with a socket
     * service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     * @param handle Native socket handle opened by the calling listening socket.
     *
     * @return The created socket.
     */
    static std::shared_ptr<TcpSocket> create_socket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config,
        socket_type handle);

    /**
     * Constructor. Open the socket in an asynchronous IO processing mode armed with the provided
     * socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     */
    TcpSocket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * Constructor. Create a socket with an already-opened socket handle and the provided IO
     * processing mode.
     *
     * @param config Reference to network configuration.
     * @param handle Native socket handle opened by the calling listening socket.
     * @param mode IO processing mode to apply to the socket.
     */
    TcpSocket(std::shared_ptr<NetworkConfig> config, socket_type handle, IOMode mode) noexcept;

    /**
     * Constructor. Create an asynchronous socket with an already-opened socket handle armed with a
     * socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     * @param handle Native socket handle opened by the calling listening socket.
     */
    TcpSocket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config,
        socket_type handle) noexcept;

    TcpSocket(const TcpSocket &) = delete;
    TcpSocket &operator=(const TcpSocket &) = delete;

    /**
     * When the socket service indicates the socket is available for writing, attempt to transmit
     * the provided message to the connected remote socket. If successful, the provided callback
     * will be invoked with the number of bytes transmitted. If unsuccessful because the operation
     * would still block, queue another attempt. Otherwise, the socket will be closed and the
     * callback will be invoked with the number of bytes successfully transmitted.
     *
     * @param message The message to transmit.
     * @param callback The callback to invoke when the operation has completed.
     * @param bytes_sent The number of bytes successfully transmitted so far.
     * @param total_bytes The total number of bytes expected to be transmitted.
     */
    void ready_to_send(
        std::string_view message,
        SendCompletion &&callback,
        std::size_t bytes_sent,
        std::size_t total_bytes);

    /**
     * When the socket service indicates the socket is available for reading, attempt to receive
     * a message from the connected remote socket. If successful, the provided callback will be
     * invoked with the received message. If unsuccessful because the operation would still block,
     * queue another attempt. Otherwise, the socket will be closed and the callback will be invoked
     * with any message partially received.
     *
     * @param callback The callback to invoke when the operation has completed.
     * @param message The message successfully received so far.
     */
    void ready_to_receive(ReceiveCompletion &&callback, std::string received);

    using BaseSocket::packet_size;
    using BaseSocket::socket_service;

    std::atomic<ConnectedState> m_connected_state {ConnectedState::Disconnected};
};

} // namespace fly::net
