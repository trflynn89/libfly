#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "fly/fly.h"
#include "fly/socket/async_request.h"
#include "fly/socket/socket_types.h"

namespace fly {

class SocketConfig;

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
     * Constructor.
     *
     * @param Protocol The communication protocol of the socket.
     * @param SocketConfig Reference to the socket configuration.
     */
    Socket(Protocol, const std::shared_ptr<SocketConfig> &);

    /**
     * Destructor.
     */
    virtual ~Socket() = default;

    /**
     * Convert a string hostname or IPv4 address to a host-order numeric IPv4
     * address.
     *
     * @param string The hostname or IPv4 address to convert.
     * @param address_type The location to store the converted address.
     *
     * @return bool True if the hostname/address string could be converted.
     */
    static bool HostnameToAddress(const std::string &, address_type &);

    /**
     * INADDR_ANY may be different depending on the OS. This function will
     * return the value for the compiled target's OS.
     *
     * @return INADDR_ANY for the target system.
     */
    static address_type InAddrAny();

    /**
     * Invalid socket handles may be different depending on the OS. This
     * function will return the value for the compiled target's OS.
     *
     * @return Invalid socket handle for the target system.
     */
    static socket_type InvalidSocket();

    /**
     * A socket is valid if it's handle has been properly set.
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
    socket_type GetHandle() const;

    /**
     * @return Return the client IP this socket is connected to.
     */
    address_type GetClientIp() const;

    /**
     * @return Return the client port this socket is connected to.
     */
    port_type GetClientPort() const;

    /**
     * @return This socket's ID.
     */
    int GetSocketId() const;

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
     * @return True if this socket is connecting to a remote endpoint.
     */
    bool IsConnecting() const;

    /**
     * @return True if this socket is connected to a remote endpoint.
     */
    bool IsConnected() const;

    /**
     * Bind this socket to an address.
     *
     * @param address_type The host-order IPv4 address to bind to.
     * @param port_type The port to bind to.
     *
     * @return True if the binding was successful.
     */
    virtual bool Bind(address_type, port_type, BindOption) const = 0;

    /**
     * Bind this socket to an address.
     *
     * @param string The hostname or IPv4 address to bind to.
     * @param port_type The port to bind to.
     *
     * @return True if the binding was successful.
     */
    bool Bind(const std::string &, port_type, BindOption) const;

    /**
     * Allow socket to listen for incoming connections.
     */
    virtual bool Listen() = 0;

    /**
     * Connect to a listening socket.
     *
     * @param address_type The host-order IPv4 address to connect to.
     * @param port_type The port to connect to.
     *
     * @param bool True if the connection was successful, false otherwise.
     */
    virtual bool Connect(address_type, port_type) = 0;

    /**
     * Connect to a listening socket.
     *
     * @param string The hostname or IPv4 address to connect to.
     * @param port_type The port to connect to.
     *
     * @param bool True if the connection was successful, false otherwise.
     */
    bool Connect(const std::string &, port_type);

    /**
     * Asynchronously connect to a listening socket. The connect may finish
     * immediately, so the connection state is returned rather than a binary
     * boolean. If this is not an asynchronous socket, nothing will occur.
     *
     * @param address_type The host-order IPv4 address to connect to.
     * @param port_type The port to connect to.
     *
     * @return The connection state (not connected, connecting, or connected).
     */
    ConnectedState ConnectAsync(address_type, port_type);

    /**
     * Asynchronously connect to a listening socket. The connect may finish
     * immediately, so the connection state is returned rather than a binary
     * boolean. If this is not an asynchronous socket, nothing will occur.
     *
     * @param string The hostname or IPv4 address to connect to.
     * @param port_type The port to connect to.
     *
     * @return The connection state (not connected, connecting, or connected).
     */
    ConnectedState ConnectAsync(const std::string &, port_type);

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
    virtual std::shared_ptr<Socket> Accept() const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     *
     * @return The number of bytes sent.
     */
    size_t Send(const std::string &) const;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param address_type The host-order IPv4 address to send data to.
     * @param port_type The port to send data to.
     *
     * @return The number of bytes sent.
     */
    size_t SendTo(const std::string &, address_type, port_type) const;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param string The hostname or IPv4 address to send data to.
     * @param port_type The port to send data to.
     *
     * @return The number of bytes sent.
     */
    size_t SendTo(const std::string &, const std::string &, port_type) const;

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
     * @param string The host-order IPv4 address to send data to.
     * @param port_type The port to send data to.
     *
     * @return True if the request was made.
     */
    bool SendToAsync(const std::string &, address_type, port_type);

    /**
     * Request data to be written on the socket asynchronously. If this is not
     * an ansynchronous socket, nothing will occur.
     *
     * @param string The data to send.
     * @param string The hostname or IPv4 address to send data to.
     * @param port_type The port to send data to.
     *
     * @return True if the request was made.
     */
    bool SendToAsync(const std::string &, const std::string &, port_type);

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    std::string Recv() const;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    std::string RecvFrom() const;

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
    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param bool Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual size_t Send(const std::string &, bool &) const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param address_type The host-order IPv4 address to send data to.
     * @param port_type The port to send data to.
     * @param bool Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual size_t SendTo(
        const std::string &,
        address_type,
        port_type,
        bool &
    ) const = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param string The hostname or IPv4 address to send data to.
     * @param port_type The port to send data to.
     * @param bool Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    size_t SendTo(
        const std::string &,
        const std::string &,
        port_type,
        bool &
    ) const;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param bool Boolean set to true if the operation would block.
     * @param bool Boolean set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string Recv(bool &, bool &) const = 0;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param bool Boolean set to true if the operation would block.
     * @param bool Boolean set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string RecvFrom(bool &, bool &) const = 0;

    // Socket protocol
    Protocol m_protocol;

    // Socket config
    const std::shared_ptr<SocketConfig> &m_spConfig;

    // End of message character
    const char m_socketEoM;

    // Send/recv packet size
    const size_t m_packetSize;

    // File descriptor for this socket.
    socket_type m_socketHandle;

    // Client IP and port this socket is connected to.
    address_type m_clientIp;
    port_type m_clientPort;

    // Whether this socket should allow asynchronous operations
    bool m_isAsync;

    // Whether this socket is a listening socket
    bool m_isListening;

    // Whether this socket is not connected, connecting, or connected
    std::atomic<ConnectedState> m_aConnectedState;

private:
    static std::atomic_int s_aNumSockets;
    int m_socketId;

    AsyncRequest::RequestQueue m_pendingSends;

    std::string m_receiveBuffer;
};

}

#include FLY_OS_IMPL_PATH(socket, socket)
