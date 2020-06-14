#include "fly/socket/socket.hpp"

#include "fly/logger/logger.hpp"
#include "fly/socket/socket_config.hpp"
#include "fly/types/string/string.hpp"

namespace fly {

//==================================================================================================
std::atomic_int Socket::s_num_sockets(0);

//==================================================================================================
Socket::Socket(Protocol protocol, const std::shared_ptr<SocketConfig> &config) noexcept :
    m_protocol(protocol),
    m_config(config),
    m_socket_eom(config->end_of_message()),
    m_packet_size(config->packet_size()),
    m_socket_handle(invalid_socket()),
    m_client_ip(0),
    m_client_port(0),
    m_is_async(false),
    m_is_listening(false),
    m_connected_state(ConnectedState::Disconnected),
    m_socket_id(s_num_sockets.fetch_add(1))
{
}

//==================================================================================================
bool Socket::hostname_to_address(const std::string &hostname, address_type &address)
{
    return SocketImpl::hostname_to_address(hostname, address);
}

//==================================================================================================
address_type Socket::in_addr_any()
{
    return SocketImpl::in_addr_any();
}

//==================================================================================================
socket_type Socket::invalid_socket()
{
    return SocketImpl::invalid_socket();
}

//==================================================================================================
bool Socket::is_valid() const
{
    return m_socket_handle != invalid_socket();
}

//==================================================================================================
socket_type Socket::get_handle() const
{
    return m_socket_handle;
}

//==================================================================================================
address_type Socket::get_client_ip() const
{
    return m_client_ip;
}

//==================================================================================================
port_type Socket::get_client_port() const
{
    return m_client_port;
}

//==================================================================================================
int Socket::get_socket_id() const
{
    return m_socket_id;
}

//==================================================================================================
bool Socket::is_tcp() const
{
    return m_protocol == Protocol::TCP;
}

//==================================================================================================
bool Socket::is_udp() const
{
    return m_protocol == Protocol::UDP;
}

//==================================================================================================
bool Socket::is_async() const
{
    return m_is_async;
}

//==================================================================================================
bool Socket::is_listening() const
{
    return m_is_listening;
}

//==================================================================================================
bool Socket::is_connecting() const
{
    return m_connected_state.load() == ConnectedState::Connecting;
}

//==================================================================================================
bool Socket::is_connected() const
{
    return m_connected_state.load() == ConnectedState::Connected;
}

//==================================================================================================
bool Socket::bind(const std::string &hostname, port_type port, BindOption option) const
{
    address_type address = 0;

    if (hostname_to_address(hostname, address))
    {
        return bind(address, port, option);
    }

    return false;
}

//==================================================================================================
bool Socket::connect(const std::string &hostname, port_type port)
{
    address_type address = 0;

    if (hostname_to_address(hostname, address))
    {
        return connect(address, port);
    }

    return false;
}

//==================================================================================================
ConnectedState Socket::connect_async(address_type address, port_type port)
{
    ConnectedState state = ConnectedState::Disconnected;

    if (is_tcp() && is_async())
    {
        if (connect(address, port))
        {
            SLOGD(m_socket_id, "Connected to %d:%d", address, port);
            state = ConnectedState::Connected;
        }
        else if (is_connecting())
        {
            SLOGD(m_socket_id, "Connect to %d:%d in progress", address, port);
            state = ConnectedState::Connecting;
        }
        else
        {
            SLOGW(m_socket_id, "Could not connect to %d:%d, closing socket", address, port);

            close();
        }
    }

    return state;
}

//==================================================================================================
ConnectedState Socket::connect_async(const std::string &hostname, port_type port)
{
    address_type address = 0;

    if (hostname_to_address(hostname, address))
    {
        return connect_async(address, port);
    }

    return ConnectedState::Disconnected;
}

//==================================================================================================
bool Socket::finish_connect()
{
    if (is_valid() & is_connecting() && is_error_free())
    {
        SLOGD(m_socket_id, "Connection completed");
        m_connected_state.store(ConnectedState::Connected);
    }
    else
    {
        SLOGW(m_socket_id, "Could not connect, closing socket");
        m_connected_state.store(ConnectedState::Disconnected);

        close();
    }

    return is_valid() && is_connected();
}

//==================================================================================================
size_t Socket::send(const std::string &message) const
{
    bool would_block = false;
    return send(message, would_block);
}

//==================================================================================================
size_t Socket::send_to(const std::string &message, address_type address, port_type port) const
{
    bool would_block = false;
    return send_to(message, address, port, would_block);
}

//==================================================================================================
size_t
Socket::send_to(const std::string &message, const std::string &hostname, port_type port) const
{
    bool would_block = false;
    return send_to(message, hostname, port, would_block);
}

//==================================================================================================
size_t Socket::send_to(
    const std::string &message,
    const std::string &hostname,
    port_type port,
    bool &would_block) const
{
    address_type address = 0;

    if (hostname_to_address(hostname, address))
    {
        return send_to(message, address, port, would_block);
    }

    return 0;
}

//==================================================================================================
bool Socket::send_async(std::string &&message)
{
    if (is_tcp() && is_async())
    {
        AsyncRequest request(m_socket_id, std::move(message));
        m_pending_sends.push(std::move(request));

        return true;
    }

    return false;
}

//==================================================================================================
bool Socket::send_to_async(std::string &&message, address_type address, port_type port)
{
    if (is_udp() && is_async())
    {
        AsyncRequest request(m_socket_id, std::move(message), address, port);
        m_pending_sends.push(std::move(request));

        return true;
    }

    return false;
}

//==================================================================================================
bool Socket::send_to_async(std::string &&message, const std::string &hostname, port_type port)
{
    address_type address = 0;

    if (hostname_to_address(hostname, address))
    {
        return send_to_async(std::move(message), address, port);
    }

    return false;
}

//==================================================================================================
std::string Socket::recv() const
{
    bool would_block = false, is_complete = false;
    return recv(would_block, is_complete);
}

//==================================================================================================
std::string Socket::recv_from() const
{
    bool would_block = false, is_complete = false;
    return recv_from(would_block, is_complete);
}

//==================================================================================================
void Socket::service_send_requests(AsyncRequest::RequestQueue &completed_sends)
{
    bool would_block = false;

    while (is_valid() && !m_pending_sends.empty() && !would_block)
    {
        AsyncRequest request;
        m_pending_sends.pop(request);

        if (request.is_valid())
        {
            const std::string message = request.get_request_remaining();
            size_t bytes_sent = 0;

            switch (m_protocol)
            {
                case Protocol::TCP:
                    bytes_sent = send(message, would_block);
                    break;

                case Protocol::UDP:
                    bytes_sent =
                        send_to(message, request.get_address(), request.get_port(), would_block);
                    break;
            }

            if (bytes_sent == message.length())
            {
                SLOGD(m_socket_id, "Sent %u bytes", bytes_sent);
                completed_sends.push(std::move(request));
            }
            else if (would_block)
            {
                SLOGI(
                    m_socket_id,
                    "Send would block - sent %u of %u bytes, will finish later",
                    bytes_sent,
                    message.length());

                request.increment_request_offset(bytes_sent);
                m_pending_sends.push(std::move(request));
            }
            else
            {
                SLOGW(m_socket_id, "Can't send, closing socket");
                close();
            }
        }
    }
}

//==================================================================================================
void Socket::service_recv_requests(AsyncRequest::RequestQueue &completed_reads)
{
    bool would_block = false;
    bool is_complete = false;

    while (is_valid() && !would_block)
    {
        std::string received;

        switch (m_protocol)
        {
            case Protocol::TCP:
                received = recv(would_block, is_complete);
                break;

            case Protocol::UDP:
                received = recv_from(would_block, is_complete);
                break;
        }

        if ((received.length() > 0) || is_complete)
        {
            SLOGD(
                m_socket_id,
                "Received %u bytes, %u in buffer",
                received.length(),
                m_receive_buffer.length());

            m_receive_buffer += received;

            if (is_complete)
            {
                SLOGD(m_socket_id, "Completed message, %u bytes", m_receive_buffer.length());

                AsyncRequest request(m_socket_id, std::move(m_receive_buffer));
                completed_reads.push(std::move(request));
                m_receive_buffer.clear();
            }
        }
        else if (would_block)
        {
            SLOGI(
                m_socket_id,
                "Receive would block - received %u bytes, will finish later",
                m_receive_buffer.length());
        }
        else
        {
            SLOGW(m_socket_id, "Can't receive, closing socket");
            close();
        }
    }
}

} // namespace fly
