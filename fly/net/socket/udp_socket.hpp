#pragma once

#include "fly/net/socket/concepts.hpp"
#include "fly/net/socket/detail/base_socket.hpp"
#include "fly/net/socket/types.hpp"

#include <cstddef>
#include <functional>
#include <memory>
#include <string>

namespace fly::net {

class NetworkConfig;
class SocketService;

/**
 * Class to represent a connectionless datagram network socket.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <IPEndpoint EndpointType>
class UdpSocket :
    public detail::BaseSocket<EndpointType>,
    public std::enable_shared_from_this<UdpSocket<EndpointType>>
{
    using BaseSocket = detail::BaseSocket<EndpointType>;

    using SendCompletion = std::function<void(std::size_t)>;
    using ReceiveCompletion = std::function<void(std::string)>;

public:
    /**
     * Constructor. Open the socket in a synchronous IO processing mode.
     *
     * @param config Reference to network configuration.
     */
    explicit UdpSocket(std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * Constructor. Open the socket in the provided IO processing mode.
     *
     * @param config Reference to network configuration.
     * @param mode IO processing mode to apply to the socket.
     */
    UdpSocket(std::shared_ptr<NetworkConfig> config, IOMode mode) noexcept;

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
     * Transmit a message to a specific remote endpoint. If an error occurs on the socket, the
     * socket will be closed.
     *
     * @param endpoint The remote endpoint to transmit to.
     * @param message The message to transmit.
     *
     * @return The number of bytes transmitted.
     */
    std::size_t send(const EndpointType &endpoint, std::string_view message);

    /**
     * Transmit a message to a specific remote endpoint. If an error occurs on the socket, the
     * socket will be closed.
     *
     * @param hostname The hostname or IP address string to transmit to.
     * @param port The port to transmit to.
     * @param message The message to transmit.
     *
     * @return The number of bytes transmitted.
     */
    std::size_t send(std::string_view hostname, port_type port, std::string_view message);

    /**
     * Asynchronously transmit a message to a specific remote endpoint. May only be used if this
     * socket was created through a socket service.
     *
     * Upon completion, the provided callback will be invoked with the number of bytes that were
     * transmitted. If an error occurs on the socket, the callback will still be invoked with the
     * number of bytes successfully transmitted, but the socket will also be closed before the
     * invocation.
     *
     * @param endpoint The remote endpoint to transmit to.
     * @param message The message to transmit.
     * @param callback The callback to invoke when the operation has completed.
     *
     * @return True if the socket service and the provided callback are valid.
     */
    bool
    send_async(const EndpointType &endpoint, std::string_view message, SendCompletion &&callback);

    /**
     * Asynchronously transmit a message to a specific remote endpoint. May only be used if this
     * socket was created through a socket service.
     *
     * Upon completion, the provided callback will be invoked with the number of bytes that were
     * transmitted. If an error occurs on the socket, the callback will still be invoked with the
     * number of bytes successfully transmitted, but the socket will also be closed before the
     * invocation.
     *
     * @param hostname The hostname or IP address string to transmit to.
     * @param port The port to transmit to.
     * @param message The message to transmit.
     * @param callback The callback to invoke when the operation has completed.
     *
     * @return True if the socket service and the provided callback are valid.
     */
    bool send_async(
        std::string_view hostname,
        port_type port,
        std::string_view message,
        SendCompletion &&callback);

    /**
     * Receive a message from an unspecified remote endpoint. If an error occurs on the socket, the
     * socket will be closed.
     *
     * @return The message received.
     */
    std::string receive();

    /**
     * Asynchronously receive a message from an unspecified remote endpoint. May only be used if
     * this socket was created through a socket service.
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
    static std::shared_ptr<UdpSocket> create_socket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config);

    /**
     * Constructor. Open the socket in an asynchronous IO processing mode armed with the provided
     * socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     */
    UdpSocket(
        const std::shared_ptr<SocketService> &service,
        std::shared_ptr<NetworkConfig> config) noexcept;

    /**
     * When the socket service indicates the socket is available for writing, attempt to transmit
     * the provided message to the specified remote endpoint. If successful, the provided callback
     * will be invoked with the number of bytes transmitted. If unsuccessful because the operation
     * would still block, queue another attempt. Otherwise, the socket will be closed and the
     * callback will be invoked with the number of bytes successfully transmitted.
     *
     * @param endpoint The remote endpoint to transmit to.
     * @param message The message to transmit.
     * @param callback The callback to invoke when the operation has completed.
     * @param bytes_sent The number of bytes successfully transmitted so far.
     * @param total_bytes The total number of bytes expected to be transmitted.
     */
    void ready_to_send(
        const EndpointType &endpoint,
        std::string_view message,
        SendCompletion &&callback,
        std::size_t bytes_sent,
        std::size_t total_bytes);

    /**
     * When the socket service indicates the socket is available for reading, attempt to receive
     * a message from an unspecified remote endpoint. If successful, the provided callback will be
     * invoked with the received message. If unsuccessful because the operation would still block,
     * queue another attempt. Otherwise, the socket will be closed and the callback will be invoked
     * with any message partially received.
     *
     * @param callback The callback to invoke when the operation has completed.
     * @param message The message successfully received so far.
     */
    void ready_to_receive(ReceiveCompletion &&callback, std::string received);

    UdpSocket(const UdpSocket &) = delete;
    UdpSocket &operator=(const UdpSocket &) = delete;

    using BaseSocket::packet_size;
    using BaseSocket::socket_service;
};

} // namespace fly::net
