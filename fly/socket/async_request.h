#pragma once

#include "fly/socket/socket_types.h"
#include "fly/types/concurrent_queue.h"

#include <string>

namespace fly {

/**
 * An asynchronous read/write request.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 28, 2014
 */
class AsyncRequest
{
public:
    typedef fly::ConcurrentQueue<AsyncRequest> RequestQueue;

    /**
     * Default constructor to set the socket ID to an invalid value and the
     * request message to an empty string.
     */
    AsyncRequest();

    /**
     * Constructor to set the ID of the owning socket and the request message.
     */
    AsyncRequest(int, const std::string &);

    /**
     * Constructor to set the ID of the owning socket, the request message, and
     * the address and port of the owning socket.
     */
    AsyncRequest(int, const std::string &, address_type, port_type);

    /**
     * Constructor to set the ID of the owning socket, the request message, and
     * the address and port of the owning socket.
     */
    AsyncRequest(int, const std::string &, const std::string &, port_type);

    /**
     * @return True if the socket ID is valid (i.e. has been explicitly set).
     */
    bool IsValid() const;

    /**
     * @return The ID of the socket who owns this structure.
     */
    int GetSocketId() const;

    /**
     * Increase the current offset into the request message to mark how much
     * data has been sent.
     *
     * @param string::size_type The offset to set.
     */
    void IncrementRequestOffset(std::string::size_type);

    /**
     * @return The request message - the message to be sent or received.
     */
    const std::string &GetRequest() const;

    /**
     * @return The request message starting at its current offset.
     */
    std::string GetRequestRemaining() const;

    /**
     * @return The request address (for UDP sockets).
     */
    address_type GetAddress() const;

    /**
     * @return The request port (for UDP sockets).
     */
    port_type GetPort() const;

private:
    int m_socketId;

    std::string::size_type m_requestOffset;
    std::string m_request;

    address_type m_address;
    port_type m_port;
};

} // namespace fly
