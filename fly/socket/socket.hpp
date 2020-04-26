#pragma once

#include "fly/fly.hpp"
#include "fly/socket/async_request.hpp"
#include "fly/socket/socket_types.hpp"

#include <atomic>
#include <memory>
#include <string>

namespace fly {

class SocketConfig;

/**
 * Virtual interface to represent a network socket. This interface is platform
 * independent - OS dependent implementations should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
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
    Socket(Protocol, const std::shared_ptr<SocketConfig> &) noexcept;

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
    static bool HostnameToAddress(const std::string &, address_type &) noexcept;

    /**
     * INADDR_ANY may be different depending on the OS. This function will
     * return the value for the compiled target's OS.
     *
     * @return INADDR_ANY for the target system.
     */
    static address_type InAddrAny() noexcept;

    /**
     * Invalid socket handles may be different depending on the OS. This
     * function will return the value for the compiled target's OS.
     *
     * @return Invalid socket handle for the target system.
     */
    static socket_type InvalidSocket() noexcept;

    /**
     * A socket is valid if it's handle has been properly set.
     *
     * @return True if this is a valid socket, false otherwise.
     */
    bool IsValid() const noexcept;

    /**
     * Check if there is any errors on the socket.
     */
    virtual bool IsErrorFree() noexcept = 0;

    /**
     * Close this socket's handle.
     */
    virtual void Close() noexcept = 0;

    /**
     * Set the socket to be asynchronous.
     *
     * @return True if the operation was successful.
     */
    virtual bool SetAsync() noexcept = 0;

    /**
     * @return Return this socket's handle.
     */
    socket_type GetHandle() const noexcept;

    /**
     * @return Return the client IP this socket is connected to.
     */
    address_type GetClientIp() const noexcept;

    /**
     * @return Return the client port this socket is connected to.
     */
    port_type GetClientPort() const noexcept;

    /**
     * @return This socket's ID.
     */
    int GetSocketId() const noexcept;

    /**
     * @return True if this is a TCP socket, false otherwise.
     */
    bool IsTcp() const noexcept;

    /**
     * @return True if this is a UDP socket, false otherwise.
     */
    bool IsUdp() const noexcept;

    /**
     * @return True if this is an asynchronous socket, false otherwise.
     */
    bool IsAsync() const noexcept;

    /**
     * @return True if this socket is a listener socket.
     */
    bool IsListening() const noexcept;

    /**
     * @return True if this socket is connecting to a remote endpoint.
     */
    bool IsConnecting() const noexcept;

    /**
     * @return True if this socket is connected to a remote endpoint.
     */
    bool IsConnected() const noexcept;

    /**
     * Bind this socket to an address.
     *
     * @param address_type The host-order IPv4 address to bind to.
     * @param port_type The port to bind to.
     *
     * @return True if the binding was successful.
     */
    virtual bool Bind(address_type, port_type, BindOption) const noexcept = 0;

    /**
     * Bind this socket to an address.
     *
     * @param string The hostname or IPv4 address to bind to.
     * @param port_type The port to bind to.
     *
     * @return True if the binding was successful.
     */
    bool Bind(const std::string &, port_type, BindOption) const noexcept;

    /**
     * Allow socket to listen for incoming connections.
     */
    virtual bool Listen() noexcept = 0;

    /**
     * Connect to a listening socket.
     *
     * @param address_type The host-order IPv4 address to connect to.
     * @param port_type The port to connect to.
     *
     * @param bool True if the connection was successful, false otherwise.
     */
    virtual bool Connect(address_type, port_type) noexcept = 0;

    /**
     * Connect to a listening socket.
     *
     * @param string The hostname or IPv4 address to connect to.
     * @param port_type The port to connect to.
     *
     * @param bool True if the connection was successful, false otherwise.
     */
    bool Connect(const std::string &, port_type) noexcept;

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
    ConnectedState ConnectAsync(address_type, port_type) noexcept;

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
    ConnectedState ConnectAsync(const std::string &, port_type) noexcept;

    /**
     * After an asynchronous socket in a connecting state becomes available for
     * writing, verify the socket is healthy and store its state as connected.
     *
     * @return True if the socket is healthy and connected.
     */
    bool FinishConnect() noexcept;

    /**
     * Accept an incoming client connection.
     *
     * @return A Socket on which the actual connection is made.
     */
    virtual std::shared_ptr<Socket> Accept() const noexcept = 0;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     *
     * @return The number of bytes sent.
     */
    std::size_t Send(const std::string &) const noexcept;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param address_type The host-order IPv4 address to send data to.
     * @param port_type The port to send data to.
     *
     * @return The number of bytes sent.
     */
    std::size_t
    SendTo(const std::string &, address_type, port_type) const noexcept;

    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param string The hostname or IPv4 address to send data to.
     * @param port_type The port to send data to.
     *
     * @return The number of bytes sent.
     */
    std::size_t
    SendTo(const std::string &, const std::string &, port_type) const noexcept;

    /**
     * Request data to be written on the socket asynchronously. If this is not
     * an ansynchronous socket, nothing will occur.
     *
     * @param string The data to send.
     *
     * @return True if the request was made.
     */
    bool SendAsync(std::string &&) noexcept;

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
    bool SendToAsync(std::string &&, address_type, port_type) noexcept;

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
    bool SendToAsync(std::string &&, const std::string &, port_type) noexcept;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    std::string Recv() const noexcept;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    std::string RecvFrom() const noexcept;

    /**
     * Iterate thru all pending asynchronous sends. Service each request until
     * one would block, or if some other error occurred (in which case, this
     * socket will be closed).
     *
     * @param RequestQueue Queue of completed sends to post to on success.
     */
    void ServiceSendRequests(AsyncRequest::RequestQueue &) noexcept;

    /**
     * Read on this socket until a read would block, or some other error occurs
     * (in which case, this socket will be closed).
     *
     * @param RequestQueue Queue of completed received to post to on success.
     */
    void ServiceRecvRequests(AsyncRequest::RequestQueue &) noexcept;

protected:
    /**
     * Write data on the socket.
     *
     * @param string The data to send.
     * @param bool Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual std::size_t Send(const std::string &, bool &) const noexcept = 0;

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
    virtual std::size_t
    SendTo(const std::string &, address_type, port_type, bool &)
        const noexcept = 0;

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
    std::size_t
    SendTo(const std::string &, const std::string &, port_type, bool &)
        const noexcept;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param bool Boolean set to true if the operation would block.
     * @param bool Boolean set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string Recv(bool &, bool &) const noexcept = 0;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param bool Boolean set to true if the operation would block.
     * @param bool Boolean set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string RecvFrom(bool &, bool &) const noexcept = 0;

    // Socket protocol
    Protocol m_protocol;

    // Socket config
    const std::shared_ptr<SocketConfig> &m_spConfig;

    // End of message character
    const char m_socketEoM;

    // Send/recv packet size
    const std::size_t m_packetSize;

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

} // namespace fly

#include FLY_OS_IMPL_PATH(socket, socket)
