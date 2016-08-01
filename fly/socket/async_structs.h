#pragma once

#include <functional>
#include <memory>
#include <string>

#include <fly/concurrency/concurrent_queue.h>

namespace fly {

/**
 * Base class for data common to all asynchronous data structures.
 *
 * Stores data pertintent to all asynchronous data. All constructors are
 * protected to prevent instantiation.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 28, 2014
 */
class AsyncBase
{
public:
    /**
     * @return True if the socket ID is valid (i.e. has been explicitly set).
     */
    bool IsValid() const;

    /**
     * @return The ID of the socket who owns this structure.
     */
    int GetSocketId() const;

protected:
    /**
     * Default constructor to set the socket ID to an invalid value.
     */
    AsyncBase();

    /**
     * Constructor to set the ID of the owning socket.
     */
    AsyncBase(int socketId);

    int m_socketId;
};

/**
 * An asynchronous read/write request.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 28, 2014
 */
class AsyncRequest : public AsyncBase
{
public:
    typedef fly::ConcurrentQueue<AsyncRequest> RequestQueue;

    /**
     * Default constructor to set the socket ID to an invalid value and the
     * request message to an empty string.
     */
    AsyncRequest();

    /**
     * Constructor to set the ID of the owning socket, while setting the
     * request message to an empty string.
     */
    AsyncRequest(int);

    /**
     * Constructor to set the ID of the owning socket and the request message.
     */
    AsyncRequest(int, const std::string &);

    /**
     * Constructor to set the ID of the owning socket and the request message.
     */
    AsyncRequest(int, const std::string &, const std::string &, int);

    /**
     * @return The request message - the message to be sent or received.
     */
    std::string GetRequest() const;

    /**
     * @return The request hostname (for UDP sockets).
     */
    std::string GetHostname() const;

    /**
     * @return The request port (for UDP sockets).
     */
    int GetPort() const;

private:
    std::string m_request;
    std::string m_hostname;
    int m_port;
};

/**
 * An asynchronous connect request.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version December 28, 2014
 */
class AsyncConnect : public AsyncBase
{
public:
    typedef fly::ConcurrentQueue<AsyncConnect> ConnectQueue;

    /**
     * Default constructor to set the socket ID to an invalid value and the
     * hostname/port to invalid values.
     */
    AsyncConnect();

    /**
     * Constructor to set the ID of the owning socket and the hostname/port to
     * connect to.
     */
    AsyncConnect(int, std::string, int);

    /**
     * @return The hostname to connect to.
     */
    std::string GetHostname() const;

    /**
     * @return The port to connect to.
     */
    int GetPort() const;

private:
    std::string m_hostname;
    int m_port;
};

}
