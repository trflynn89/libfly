#include "fly/net/socket/tcp_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/net/socket/socket_service.hpp"

namespace fly::net {

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket() noexcept : TcpSocket(fly::net::IOMode::Synchronous)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket(fly::net::IOMode mode) noexcept :
    BaseSocket(fly::net::detail::socket<EndpointType, TcpSocket<EndpointType>>(), mode)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket(const std::shared_ptr<SocketService> &service) noexcept :
    BaseSocket(fly::net::detail::socket<EndpointType, TcpSocket<EndpointType>>(), service)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket(socket_type socket_handle, fly::net::IOMode mode) noexcept :
    BaseSocket(socket_handle, mode),
    m_connected_state(ConnectedState::Connected)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket(
    socket_type socket_handle,
    const std::shared_ptr<SocketService> &service) noexcept :
    BaseSocket(socket_handle, service),
    m_connected_state(ConnectedState::Connected)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket(TcpSocket &&socket) noexcept :
    BaseSocket(std::move(socket)),
    m_connected_state(socket.m_connected_state.exchange(ConnectedState::Disconnected))
{
}

//==================================================================================================
template <typename EndpointType>
auto TcpSocket<EndpointType>::create_socket(const std::shared_ptr<SocketService> &service)
    -> std::shared_ptr<TcpSocket>
{
    // TcpSocket's constructor for socket-service-based sockets is private, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructor locally.
    struct TcpSocketImpl final : public TcpSocket
    {
        explicit TcpSocketImpl(const std::shared_ptr<SocketService> &service) noexcept :
            TcpSocket(service)
        {
        }
    };

    return std::make_shared<TcpSocketImpl>(service);
}

//==================================================================================================
template <typename EndpointType>
auto TcpSocket<EndpointType>::create_accepted_socket(
    socket_type socket_handle,
    const std::shared_ptr<SocketService> &service) -> std::shared_ptr<TcpSocket>
{
    // TcpSocket's constructors for accepted sockets are private, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructors locally.
    struct TcpSocketImpl final : public TcpSocket
    {
        TcpSocketImpl(
            socket_type socket_handle,
            const std::shared_ptr<SocketService> &service) noexcept :
            TcpSocket(socket_handle, service)
        {
        }
    };

    return std::make_shared<TcpSocketImpl>(socket_handle, service);
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType> &TcpSocket<EndpointType>::operator=(TcpSocket &&socket) noexcept
{
    m_connected_state = socket.m_connected_state.exchange(ConnectedState::Disconnected);

    return static_cast<TcpSocket &>(BaseSocket::operator=(std::move(socket)));
}

//==================================================================================================
template <typename EndpointType>
std::optional<EndpointType> TcpSocket<EndpointType>::remote_endpoint() const
{
    return fly::net::detail::remote_endpoint<EndpointType>(handle());
}

//==================================================================================================
template <typename EndpointType>
ConnectedState TcpSocket<EndpointType>::connect(const EndpointType &endpoint)
{
    const auto state = fly::net::detail::connect(handle(), endpoint);

    switch (state)
    {
        case ConnectedState::Connected:
            SLOGD(handle(), "Connected to {}", endpoint);
            break;

        case ConnectedState::Connecting:
            SLOGD(handle(), "Connection to {} in progress", endpoint);
            break;

        case ConnectedState::Disconnected:
            SLOGW(handle(), "Could not connect to {}, closing socket", endpoint);
            close();
            break;
    }

    m_connected_state.store(state);
    return state;
}

//==================================================================================================
template <typename EndpointType>
ConnectedState TcpSocket<EndpointType>::connect(std::string_view hostname, port_type port)
{
    if (auto endpoint = hostname_to_endpoint(std::move(hostname)); endpoint)
    {
        endpoint->set_port(port);
        return connect(*endpoint);
    }

    return ConnectedState::Disconnected;
}

//==================================================================================================
template <typename EndpointType>
ConnectedState
TcpSocket<EndpointType>::connect_async(const EndpointType &endpoint, ConnectCompletion &&callback)
{
    if (auto service = socket_service(); service && callback)
    {
        const auto state = connect(endpoint);

        if (state == ConnectedState::Connecting)
        {
            service->notify_when_writable(
                this->shared_from_this(),
                [callback = std::move(callback)](std::shared_ptr<TcpSocket> self) mutable
                {
                    std::invoke(std::move(callback), self->finish_connect());
                });
        }

        return state;
    }

    return ConnectedState::Disconnected;
}

//==================================================================================================
template <typename EndpointType>
ConnectedState TcpSocket<EndpointType>::connect_async(
    std::string_view hostname,
    port_type port,
    ConnectCompletion &&callback)
{
    if (auto endpoint = hostname_to_endpoint(std::move(hostname)); endpoint)
    {
        endpoint->set_port(port);
        return connect_async(*endpoint, std::move(callback));
    }

    return ConnectedState::Disconnected;
}

//==================================================================================================
template <typename EndpointType>
ConnectedState TcpSocket<EndpointType>::finish_connect()
{
    ConnectedState state = ConnectedState::Disconnected;

    if (is_open() & is_connecting() && fly::net::detail::is_error_free(handle()))
    {
        SLOGD(handle(), "Connection complete");
        state = ConnectedState::Connected;
    }
    else
    {
        SLOGW(handle(), "Could not complete connection, closing socket");
        close();
    }

    m_connected_state.store(state);
    return state;
}

//==================================================================================================
template <typename EndpointType>
bool TcpSocket<EndpointType>::is_connecting() const
{
    return m_connected_state.load() == ConnectedState::Connecting;
}

//==================================================================================================
template <typename EndpointType>
bool TcpSocket<EndpointType>::is_connected() const
{
    return m_connected_state.load() == ConnectedState::Connected;
}

//==================================================================================================
template <typename EndpointType>
std::size_t TcpSocket<EndpointType>::send(std::string_view message)
{
    bool would_block = false;
    std::size_t bytes_sent = fly::net::detail::send(handle(), std::move(message), would_block);

    if (bytes_sent == 0)
    {
        SLOGW(handle(), "Error sending, closing");
        close();
    }
    else
    {
        SLOGD(handle(), "Sent {} bytes", bytes_sent);
    }

    return bytes_sent;
}

//==================================================================================================
template <typename EndpointType>
bool TcpSocket<EndpointType>::send_async(std::string_view message, SendCompletion &&callback)
{
    if (auto service = socket_service(); service && callback)
    {
        service->notify_when_writable(
            this->shared_from_this(),
            [message, callback = std::move(callback)](std::shared_ptr<TcpSocket> self) mutable
            {
                self->ready_to_send(message, std::move(callback), 0, message.size());
            });

        return true;
    }

    return false;
}

//==================================================================================================
template <typename EndpointType>
std::string TcpSocket<EndpointType>::receive()
{
    bool would_block = false;
    const std::string received = fly::net::detail::recv(handle(), m_packet_size, would_block);

    if (received.size() == 0)
    {
        SLOGW(handle(), "Error receiving, closing");
        close();
    }
    else
    {
        SLOGD(handle(), "Received {} bytes", received.size());
    }

    return received;
}

//==================================================================================================
template <typename EndpointType>
bool TcpSocket<EndpointType>::receive_async(ReceiveCompletion &&callback)
{
    if (auto service = socket_service(); service && callback)
    {
        service->notify_when_readable(
            this->shared_from_this(),
            [callback = std::move(callback)](std::shared_ptr<TcpSocket> self) mutable
            {
                self->ready_to_receive(std::move(callback), std::string());
            });

        return true;
    }

    return false;
}

//==================================================================================================
template <typename EndpointType>
void TcpSocket<EndpointType>::ready_to_send(
    std::string_view message,
    SendCompletion &&callback,
    std::size_t bytes_sent,
    std::size_t total_bytes)
{
    bool would_block = false;

    const std::size_t current_sent = fly::net::detail::send(handle(), message, would_block);
    bytes_sent += current_sent;

    if (current_sent == message.size())
    {
        SLOGD(handle(), "Completed sending {} bytes", bytes_sent);
        std::invoke(std::move(callback), bytes_sent);
    }
    else if (would_block)
    {
        SLOGI(handle(), "Send would block - sent {} of {} bytes", bytes_sent, total_bytes);
        message = message.substr(current_sent);

        socket_service()->notify_when_writable(
            this->shared_from_this(),
            [message, callback = std::move(callback), bytes_sent, total_bytes](
                std::shared_ptr<TcpSocket> self) mutable
            {
                self->ready_to_send(message, std::move(callback), bytes_sent, total_bytes);
            });
    }
    else
    {
        SLOGW(handle(), "Error after sending {} of {} bytes, closing", bytes_sent, total_bytes);
        close();

        std::invoke(std::move(callback), bytes_sent);
    }
}

//==================================================================================================
template <typename EndpointType>
void TcpSocket<EndpointType>::ready_to_receive(ReceiveCompletion &&callback, std::string received)
{
    bool would_block = false;

    const std::string current_received =
        fly::net::detail::recv(handle(), m_packet_size, would_block);
    received += current_received;

    if (!current_received.empty())
    {
        SLOGD(handle(), "Received {} bytes", received.size());
        std::invoke(std::move(callback), std::move(received));
    }
    else if (would_block)
    {
        SLOGI(handle(), "Receive would block - received {} bytes", received.length());

        socket_service()->notify_when_readable(
            this->shared_from_this(),
            [callback = std::move(callback),
             received = std::move(received)](std::shared_ptr<TcpSocket> self) mutable
            {
                self->ready_to_receive(std::move(callback), std::move(received));
            });
    }
    else
    {
        SLOGW(handle(), "Error after receiving {} bytes, closing", received.length());
        close();

        std::invoke(std::move(callback), std::move(received));
    }
}

//==================================================================================================
template class TcpSocket<Endpoint<IPv4Address>>;
template class TcpSocket<Endpoint<IPv6Address>>;

} // namespace fly::net
