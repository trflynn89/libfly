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
 * Virtual interface to represent a network socket. This interface is platform independent - OS
 * dependent implementations should inherit from this class.
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
     * @param protocol The communication protocol of the socket.
     * @param config Reference to the socket configuration.
     */
    Socket(Protocol protocol, const std::shared_ptr<SocketConfig> &config) noexcept;

    /**
     * Destructor.
     */
    virtual ~Socket() = default;

    /**
     * Convert a string hostname or IPv4 address to a host-order numeric IPv4 address.
     *
     * @param hostname The hostname or IPv4 address to convert.
     * @param address The location to store the converted address.
     *
     * @return True if the hostname/address string could be converted.
     */
    static bool hostname_to_address(const std::string &hostname, address_type &address);

    /**
     * INADDR_ANY may be different depending on the OS. This function will return the value for the
     * compiled target's OS.
     *
     * @return INADDR_ANY for the target system.
     */
    static address_type in_addr_any();

    /**
     * Invalid socket handles may be different depending on the OS. This function will return the
     * value for the compiled target's OS.
     *
     * @return Invalid socket handle for the target system.
     */
    static socket_type invalid_socket();

    /**
     * A socket is valid if it's handle has been properly set.
     *
     * @return True if this is a valid socket.
     */
    bool is_valid() const;

    /**
     * Check if there is any errors on the socket.
     */
    virtual bool is_error_free() = 0;

    /**
     * Close this socket's handle.
     */
    virtual void close() = 0;

    /**
     * Set the socket to be asynchronous.
     *
     * @return True if the operation was successful.
     */
    virtual bool set_async() = 0;

    /**
     * @return Return this socket's handle.
     */
    socket_type get_handle() const;

    /**
     * @return Return the client IP this socket is connected to.
     */
    address_type get_client_ip() const;

    /**
     * @return Return the client port this socket is connected to.
     */
    port_type get_client_port() const;

    /**
     * @return This socket's ID.
     */
    int get_socket_id() const;

    /**
     * @return True if this is a TCP socket.
     */
    bool is_tcp() const;

    /**
     * @return True if this is a UDP socket.
     */
    bool is_udp() const;

    /**
     * @return True if this is an asynchronous socket.
     */
    bool is_async() const;

    /**
     * @return True if this socket is a listener socket.
     */
    bool is_listening() const;

    /**
     * @return True if this socket is connecting to a remote endpoint.
     */
    bool is_connecting() const;

    /**
     * @return True if this socket is connected to a remote endpoint.
     */
    bool is_connected() const;

    /**
     * Bind this socket to an address.
     *
     * @param address The host-order IPv4 address to bind to.
     * @param port The port to bind to.
     * @param option Option to apply to the socket before binding.
     *
     * @return True if the binding was successful.
     */
    virtual bool bind(address_type address, port_type port, BindOption option) const = 0;

    /**
     * Bind this socket to an address.
     *
     * @param hostname The hostname or IPv4 address to bind to.
     * @param port The port to bind to.
     * @param option Option to apply to the socket before binding.
     *
     * @return True if the binding was successful.
     */
    bool bind(const std::string &hostname, port_type port, BindOption option) const;

    /**
     * Allow socket to listen for incoming connections.
     */
    virtual bool listen() = 0;

    /**
     * Connect to a listening socket.
     *
     * @param address The host-order IPv4 address to connect to.
     * @param port The port to connect to.
     *
     * @return True if the connection was successful.
     */
    virtual bool connect(address_type address, port_type port) = 0;

    /**
     * Connect to a listening socket.
     *
     * @param hostname The hostname or IPv4 address to connect to.
     * @param port The port to connect to.
     *
     * @return True if the connection was successful.
     */
    bool connect(const std::string &hostname, port_type port);

    /**
     * Asynchronously connect to a listening socket. The connect may finish immediately, so the
     * connection state is returned rather than a binary boolean. If this is not an asynchronous
     * socket, nothing will occur.
     *
     * @param address The host-order IPv4 address to connect to.
     * @param port The port to connect to.
     *
     * @return The connection state (not connected, connecting, or connected).
     */
    ConnectedState connect_async(address_type address, port_type port);

    /**
     * Asynchronously connect to a listening socket. The connect may finish immediately, so the
     * connection state is returned rather than a binary boolean. If this is not an asynchronous
     * socket, nothing will occur.
     *
     * @param hostname The hostname or IPv4 address to connect to.
     * @param port The port to connect to.
     *
     * @return The connection state (not connected, connecting, or connected).
     */
    ConnectedState connect_async(const std::string &hostname, port_type port);

    /**
     * After an asynchronous socket in a connecting state becomes available for writing, verify the
     * socket is healthy and store its state as connected.
     *
     * @return True if the socket is healthy and connected.
     */
    bool finish_connect();

    /**
     * Accept an incoming client connection.
     *
     * @return The accepted client socket.
     */
    virtual std::shared_ptr<Socket> accept() const = 0;

    /**
     * Write data on the socket.
     *
     * @param message The data to send.
     *
     * @return The number of bytes sent.
     */
    std::size_t send(const std::string &message) const;

    /**
     * Write data on the socket.
     *
     * @param message The data to send.
     * @param address The host-order IPv4 address to send data to.
     * @param port The port to send data to.
     *
     * @return The number of bytes sent.
     */
    std::size_t send_to(const std::string &message, address_type address, port_type port) const;

    /**
     * Write data on the socket.
     *
     * @param message The data to send.
     * @param hostname The hostname or IPv4 address to send data to.
     * @param port The port to send data to.
     *
     * @return The number of bytes sent.
     */
    std::size_t
    send_to(const std::string &message, const std::string &hostname, port_type port) const;

    /**
     * Request data to be written on the socket asynchronously. If this is not an ansynchronous
     * socket, nothing will occur.
     *
     * @param message The data to send.
     *
     * @return True if the request was made.
     */
    bool send_async(std::string &&message);

    /**
     * Request data to be written on the socket asynchronously. If this is not an ansynchronous
     * socket, nothing will occur.
     *
     * @param message The data to send.
     * @param address The host-order IPv4 address to send data to.
     * @param port The port to send data to.
     *
     * @return True if the request was made.
     */
    bool send_to_async(std::string &&message, address_type address, port_type port);

    /**
     * Request data to be written on the socket asynchronously. If this is not an ansynchronous
     * socket, nothing will occur.
     *
     * @param message The data to send.
     * @param hostname The hostname or IPv4 address to send data to.
     * @param port The port to send data to.
     *
     * @return True if the request was made.
     */
    bool send_to_async(std::string &&message, const std::string &hostname, port_type port);

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    std::string recv() const;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @return The data received.
     */
    std::string recv_from() const;

    /**
     * Iterate thru all pending asynchronous sends. Service each request until one would block, or
     * if some other error occurred (in which case, this socket will be closed).
     *
     * @param completed_sends Queue of completed sends to post to on success.
     */
    void service_send_requests(AsyncRequest::RequestQueue &completed_sends);

    /**
     * Read on this socket until a read would block, or some other error occurs (in which case, this
     * socket will be closed).
     *
     * @param completed_reads Queue of completed received to post to on success.
     */
    void service_recv_requests(AsyncRequest::RequestQueue &completed_reads);

protected:
    /**
     * Write data on the socket.
     *
     * @param message The data to send.
     * @param would_block Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual std::size_t send(const std::string &message, bool &would_block) const = 0;

    /**
     * Write data on the socket.
     *
     * @param message The data to send.
     * @param address The host-order IPv4 address to send data to.
     * @param port The port to send data to.
     * @param would_block Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    virtual std::size_t
    send_to(const std::string &message, address_type address, port_type port, bool &would_block)
        const = 0;

    /**
     * Write data on the socket.
     *
     * @param message The data to send.
     * @param hostname The hostname or IPv4 address to send data to.
     * @param port The port to send data to.
     * @param would_block Boolean set to true if the operation would block.
     *
     * @return The number of bytes sent.
     */
    std::size_t send_to(
        const std::string &message,
        const std::string &hostname,
        port_type port,
        bool &would_block) const;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param would_block Boolean set to true if the operation would block.
     * @param is_complete Boolean set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string recv(bool &would_block, bool &is_complete) const = 0;

    /**
     * Read data on this socket until the end-of-message character is received.
     *
     * @param would_block Boolean set to true if the operation would block.
     * @param is_complete Boolean set to true if the EoM char was received.
     *
     * @return The data received.
     */
    virtual std::string recv_from(bool &would_block, bool &is_complete) const = 0;

    // Socket protocol.
    Protocol m_protocol;

    // Socket config.
    const std::shared_ptr<SocketConfig> &m_config;

    // End of message character.
    const char m_socket_eom;

    // Send/recv packet size.
    const std::size_t m_packet_size;

    // File descriptor for this socket..
    socket_type m_socket_handle;

    // Client IP and port this socket is connected to..
    address_type m_client_ip;
    port_type m_client_port;

    // Whether this socket should allow asynchronous operations.
    bool m_is_async;

    // Whether this socket is a listening socket.
    bool m_is_listening;

    // Whether this socket is not connected, connecting, or connected.
    std::atomic<ConnectedState> m_connected_state;

private:
    static std::atomic_int s_num_sockets;
    int m_socket_id;

    AsyncRequest::RequestQueue m_pending_sends;

    std::string m_receive_buffer;
};

} // namespace fly

#include FLY_OS_IMPL_PATH(socket, socket)
