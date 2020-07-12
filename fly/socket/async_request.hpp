#pragma once

#include "fly/socket/socket_types.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

#include <string>

namespace fly {

/**
 * An asynchronous read/write request.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version December 28, 2014
 */
class AsyncRequest
{
public:
    using RequestQueue = fly::ConcurrentQueue<AsyncRequest>;

    /**
     * Default constructor to set the socket ID to an invalid value and the request message to an
     * empty string.
     */
    AsyncRequest() = default;

    /**
     * Move constructor. The moved request is invalidated.
     */
    AsyncRequest(AsyncRequest &&request) noexcept;

    /**
     * Constructor to set the ID of the owning socket and the request message.
     */
    AsyncRequest(int socket_id, std::string &&request) noexcept;

    /**
     * Constructor to set the ID of the owning socket, the request message, and the address and port
     * of the owning socket.
     */
    AsyncRequest(
        int socket_id,
        std::string &&request,
        address_type address,
        port_type port) noexcept;

    /**
     * Move assignment operator. The moved request is invalidated.
     */
    AsyncRequest &operator=(AsyncRequest &&) noexcept;

    /**
     * @return True if the socket ID is valid (i.e. has been explicitly set).
     */
    bool is_valid() const;

    /**
     * @return The ID of the socket who owns this structure.
     */
    int get_socket_id() const;

    /**
     * Increase the current offset into the request message to mark how much data has been sent.
     *
     * @param offset The offset to set.
     */
    void increment_request_offset(std::string::size_type offset);

    /**
     * @return The request message - the message to be sent or received.
     */
    const std::string &get_request() const;

    /**
     * @return The request message starting at its current offset.
     */
    std::string get_request_remaining() const;

    /**
     * @return The request address (for UDP sockets).
     */
    address_type get_address() const;

    /**
     * @return The request port (for UDP sockets).
     */
    port_type get_port() const;

private:
    static constexpr int s_invalid_id = -1;
    int m_socket_id {s_invalid_id};

    std::string::size_type m_request_offset {0};
    std::string m_request;

    address_type m_address {0};
    port_type m_port {0};
};

} // namespace fly
