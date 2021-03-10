#pragma once

#include "fly/net/socket/socket_types.hpp"

#include <cstddef>
#include <optional>
#include <set>
#include <string>

namespace fly::net::detail {

/**
 * Perform any platform-specific actions needed to initialize network services.
 */
void initialize();

/**
 * Perform any platform-specific actions needed to deinitialize network services.
 */
void deinitialize();

/**
 * @return Invalid socket handle for the target system.
 */
fly::net::socket_type invalid_socket();

/**
 * Convert a hostname or IP address string to an endpoint.
 *
 * @param hostname The hostname or IP address string to convert.
 *
 * @return If successful, the converted endpoint. Otherwise, an uninitialized value.
 */
template <typename EndpointType>
std::optional<EndpointType> hostname_to_endpoint(std::string_view hostname);

/**
 * Open a socket with the appropriate flags for the endpoint and socket types.
 *
 * @tparam EndpointType The endpoint type corresponding to the socket's domain.
 * @tparam SocketType The socket type corresponding to the socket's protocol.
 *
 * @return If successful, the opened socket handle. Otherwise, an invalid socket handle.
 */
template <typename EndpointType, typename SocketType>
fly::net::socket_type socket();

/**
 * Close a socket.
 *
 * @param handle The socket handle.
 */
void close(fly::net::socket_type handle);

/**
 * Check if any error has occurred on a socket.
 *
 * @param handle The socket handle.
 *
 * @return True if the socket is error free.
 */
bool is_error_free(fly::net::socket_type handle);

/**
 * Configure a socket to operate in the provided IO processing mode.
 *
 * @param handle The socket handle.
 * @param mode IO processing mode to apply to the socket.
 *
 * @return True if the operation was successful.
 */
bool set_io_mode(fly::net::socket_type handle, fly::net::IOMode mode);

/**
 * Retrieve the local endpoint to which a socket is bound.
 *
 * @param handle The socket handle.
 *
 * @return If successful, the bound endpoint. Otherwise, an uninitialized value.
 */
template <typename EndpointType>
std::optional<EndpointType> local_endpoint(fly::net::socket_type handle);

/**
 * Retrieve the remote endpoint to which a socket is connected.
 *
 * @param handle The socket handle.
 *
 * @return If successful, the connected endpoint. Otherwise, an uninitialized value.
 */
template <typename EndpointType>
std::optional<EndpointType> remote_endpoint(fly::net::socket_type handle);

/**
 * Bind a socket to a local endpoint.
 *
 * @param handle The socket handle.
 * @param endpoint The local endpoint to bind to.
 * @param mode Binding mode to apply to the socket before binding.
 *
 * @return True if the operation was successful.
 */
template <typename EndpointType>
bool bind(fly::net::socket_type handle, const EndpointType &endpoint, fly::net::BindMode mode);

/**
 * Configure a socket to be used to accept incoming connections.
 *
 * @param handle The socket handle.
 *
 * @return True if the operation was successful.
 */
bool listen(fly::net::socket_type handle);

/**
 * Accept an incoming connection on a listening socket.
 *
 * @param handle The listening socket handle.
 * @param endpoint Location to store the connected remote endpoint.
 * @param would_block Location to store if the operation would have blocked.
 *
 * @return If successful, the accepted socket handle. Otherwise, an uninitialized value.
 */
template <typename EndpointType>
std::optional<fly::net::socket_type>
accept(fly::net::socket_type handle, EndpointType &endpoint, bool &would_block);

/**
 * Connect to a remote socket. If this socket was opened in an asynchronous IO processing mode,
 * the connection may not complete immediately.
 *
 * @param handle The socket handle.
 * @param endpoint The remote endpoint to connect to.
 *
 * @return The connection state (disconnected, connecting, or connected).
 */
template <typename EndpointType>
fly::net::ConnectedState connect(fly::net::socket_type handle, const EndpointType &endpoint);

/**
 * Transmit a message to a connected remote socket.
 *
 * @param handle The socket handle.
 * @param message The message to transmit.
 * @param would_block Location to store if the operation would have blocked.
 *
 * @return The number of bytes transmitted.
 */
std::size_t send(fly::net::socket_type handle, std::string_view message, bool &would_block);

/**
 * Transmit a message to a specific remote socket.
 *
 * @param handle The socket handle.
 * @param endpoint The remote endpoint to transmit to.
 * @param message The message to transmit.
 * @param packet_size The maxiumum message size to transmit at once.
 * @param would_block Location to store if the operation would have blocked.
 *
 * @return The number of bytes transmitted.
 */
template <typename EndpointType>
std::size_t send_to(
    fly::net::socket_type handle,
    const EndpointType &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block);

/**
 * Receive a message from a connected remote socket.
 *
 * @param handle The socket handle.
 * @param packet_size The maxiumum message size to receive.
 * @param would_block Location to store if the operation would have blocked.
 *
 * @return The message received.
 */
std::string recv(fly::net::socket_type handle, std::size_t packet_size, bool &would_block);

/**
 * Receive a message from an unspecified remote socket.
 *
 * @param handle The socket handle.
 * @param endpoint Location to store the remote endpoint from which a message was received.
 * @param packet_size The maxiumum message size to receive.
 * @param would_block Location to store if the operation would have blocked.
 *
 * @return The message received.
 */
template <typename EndpointType>
std::string recv_from(
    fly::net::socket_type handle,
    EndpointType &endpoint,
    std::size_t packet_size,
    bool &would_block);

/**
 * Monitor a set of socket handles for IO readiness. The provided sets are modified to only contain
 * the socket handles that are ready for IO.
 *
 * @param writing_handles The maxiumum message size to receive.
 * @param reading_handles Location to store if the operation would have blocked.
 */
void select(
    std::set<fly::net::socket_type> &writing_handles,
    std::set<fly::net::socket_type> &reading_handles);

} // namespace fly::net::detail
