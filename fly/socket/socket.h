#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>

#include "fly/fly.h"
#include "fly/socket/async_request.h"
#include "fly/socket/socket_config.h"

namespace fly {

DEFINE_CLASS_PTRS(Socket);

/**
 * Virtual interface to represent a network socket. This interface is platform
 * independent - OS dependent implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version November 11, 2013
 */
class Socket
{
public:
    /**
     * Types of supported sockets.
     */
    enum SocketType
    {
        SOCKET_TCP,
        SOCKET_UDP
    };

    /**
     * Enumerated connection state values.
     */
    enum ConnectedState
    {
        NOT_CONNECTED,
        CONNECTING,
        CONNECTED
    };

    /**
     * Default constructor to initialize all values.
     */
    Socket(int, const SocketConfigPtr &);

    /**
     * INADDR_ANY may be different depending on the OS. This function will
     * return the value for the compiled target's OS.
     *
     * @return INADDR_ANY for the target system.
     */
    static int InAddrAny();

    /**
     *  A socket is valid if it's handle has been properly set.
     *
     * @return True if this is a valid socket, false otherwise.
     */
    bool IsValid() const;

    /**
     * Check if there is any errors on the socket.
     */
    virtual bool IsErrorFree() = 0;

    /**
     * Close this socket's handle.
     */
    virtual void Close() = 0;

    /**
     * Set the socket to be asynchronous.
     *
     * @return True if the operation was successful.
     */
    virtual bool SetAsync() = 0;

    /**
     * @return Return this socket's handle.
     */
    size_t GetHandle() const;

    /**
     * @return Return the client IP this socket is connected to.
     */
    int GetClientIp() const;

    /**
     * @return Return the client port this socket is connected to.
     */
    int GetClientPort() const;

    /**
     * @return This socket's ID.
     */
    int GetSocketId() const;

    /**
     * Bind this socket to a server.
     *
     * @param int The server IP to bind to.
     * @param int The server port to bind to.
     */
    virtual bool Bind(int, int) const = 0;

    /**
     * Bind this socket to a server, allowing port to be reused.
     *
     * @param int The server IP to bind to.
     * @param int The server port to bind to.
     */
    virtual bool BindForReuse(int, int) const = 0;

    /**
     * Allow socket to listen for incoming connections.
     */
    virtual bool Listen() = 0;

    /**
     * @return True if this is a TCP socket, false otherwise.
     */
    bool IsTcp() const;

    /**
     * @return True if this is a UDP socket, false otherwise.
     */
    bool IsUdp() const;

    /**
     * @return True if this is an asynchronous socket, false otherwise.
     */
    bool IsAsync() const;

    /**
     * @return True if this socket is a listener socket.
     */
    bool IsListening() const;

    /**
     * @return True if this socket is connecting to a server.
     */
    bool IsConnecting() const;

    /**
     * @return True if this socket is connected to a server.
     */
    bool IsConnected() const;

    /**
     * Connect to a listening socket.
     *
     * @param string The hostname or IPv4 address to connect to.
     * @param int The port to connect to.
     *
     * @param bool True if the connection was successful, false otherwise.
     */
    virtual bool Connect(const std::string &, int) = 0;

    /**
     * Asynchronously connect to a listening socket. The connect may finish
     * immediately, so the connection state is returned rather than a binary
     * boolean. If this is not an asynchronous socket, nothing will occur.
     *
     * @param string The hostname or IPv4 address to connect to.
     * @param int The port to connect to.
     *
     * @return The connection state (not connected, connecting, or connected).
     */
    Socket::ConnectedState ConnectAsync(std::string, int);

    /**
     * After an asynchronous socket in a connecting state becomes available for
     * writing, verify the socket is healthy and store its state as connected.
     *
     * @return True if the socket is healthy and connected.
     */
    bool FinishConnect();

    /**
     * Accept an incoming client connection.
     *
     * @return A Socket on which the actual connection is made.
     */
    virtual SocketPtr Accept() const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     *
     * @return The number of bytes sent.
     */
    virtual size_t Send(const std::string &) const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param bool & Reference to a bool, set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual size_t Send(const std::string &, bool &) const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     *
     * @return The number of bytes sent.
     */
    virtual size_t SendTo(const std::string &, const std::string &, int) const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param bool & Reference to a bool, set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual size_t SendTo(const std::string &, const std::string &, int, bool &) const = 0;

    /**
     * Request data to be written on the socket asynchronously. If this is not
     * an ansynchronous socket, nothing will occur.
     *
     * @param string The data to send.
     *
     * @return True if the request was made.
     */
    bool SendAsync(const std::string &);

    /**
     * Request data to be written on the socket asynchronously. If this is not
     * an ansynchronous socket, nothing will occur.
     *
     * @param string The data to send.
     *
     * @return True if the request was made.
     */
    bool SendToAsync(const std::string &, const std::string &, int);

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    virtual std::string Recv() const = 0;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param bool & Reference to a bool, set to true if the operation would block.
     * @param bool & Reference to a bool, set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string Recv(bool &, bool &) const = 0;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    virtual std::string RecvFrom() const = 0;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param bool & Reference to a bool, set to true if the operation would block.
     * @param bool & Reference to a bool, set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string RecvFrom(bool &, bool &) const = 0;

    /**
     * Iterate thru all pending asynchronous sends. Service each request until
     * one would block, or if some other error occurred (in which case, this
     * socket will be closed).
     *
     * @param RequestQueue Queue of completed sends to post to on success.
     */
    void ServiceSendRequests(AsyncRequest::RequestQueue &);

    /**
     * Read on this socket until a read would block, or some other error occurs
     * (in which case, this socket will be closed).
     *
     * @param RequestQueue Queue of completed received to post to on success.
     */
    void ServiceRecvRequests(AsyncRequest::RequestQueue &);

protected:
    // Socket type
    int m_socketType;

    // Socket config
    const SocketConfigPtr &m_spConfig;

    // End of message character
    const char m_socketEoM;

    // Send/recv packet size
    const size_t m_packetSize;

    // File descriptor for this socket.
    size_t m_socketHandle;

    // Client IP and port this socket is connected to.
    int m_clientIp;
    int m_clientPort;

    // Whether this socket should allow asynchronous operations
    bool m_isAsync;

    // Whether this socket is a listening socket
    bool m_isListening;

    // Whether this socket is not connected, connecting, or connected to a server
    std::atomic<Socket::ConnectedState> m_aConnectedState;

private:
    static std::atomic_int s_aNumSockets;
    int m_socketId;

    AsyncRequest::RequestQueue m_pendingSends;

    std::string m_receiveBuffer;
};

}
