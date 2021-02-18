#include "fly/net/socket/tcp_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"

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
TcpSocket<EndpointType>::TcpSocket(socket_type handle, fly::net::IOMode mode) noexcept :
    BaseSocket(handle, mode),
    m_connected_state(ConnectedState::Connected)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType>::TcpSocket(TcpSocket &&socket) noexcept :
    BaseSocket(std::move(socket)),
    m_connected_state(socket.m_connected_state.exchange(ConnectedState::Disconnected)),
    m_is_listening(socket.m_is_listening)
{
}

//==================================================================================================
template <typename EndpointType>
TcpSocket<EndpointType> &TcpSocket<EndpointType>::operator=(TcpSocket &&socket) noexcept
{
    m_is_listening = socket.m_is_listening;
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
bool TcpSocket<EndpointType>::listen()
{
    m_is_listening = fly::net::detail::listen(handle());
    return is_listening();
}

//==================================================================================================
template <typename EndpointType>
bool TcpSocket<EndpointType>::is_listening() const
{
    return m_is_listening;
}

//==================================================================================================
template <typename EndpointType>
std::optional<TcpSocket<EndpointType>> TcpSocket<EndpointType>::accept() const
{
    EndpointType client_endpoint;

    if (auto client = fly::net::detail::accept(handle(), client_endpoint); client)
    {
        return TcpSocket(*client, io_mode());
    }

    return std::nullopt;
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
ConnectedState TcpSocket<EndpointType>::finish_connect()
{
    ConnectedState state = ConnectedState::Disconnected;

    if (is_valid() & is_connecting() && fly::net::detail::is_error_free(handle()))
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
std::size_t TcpSocket<EndpointType>::send(std::string_view message) const
{
    bool would_block = false;
    return fly::net::detail::send(handle(), std::move(message), would_block);
}

//==================================================================================================
template <typename EndpointType>
std::string TcpSocket<EndpointType>::receive() const
{
    bool would_block = false;
    return fly::net::detail::recv(handle(), m_packet_size, would_block);
}

//==================================================================================================
template class TcpSocket<Endpoint<IPv4Address>>;
template class TcpSocket<Endpoint<IPv6Address>>;

} // namespace fly::net
