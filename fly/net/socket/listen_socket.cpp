#include "fly/net/socket/listen_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/net/socket/tcp_socket.hpp"

namespace fly::net {

//==================================================================================================
template <typename EndpointType>
ListenSocket<EndpointType>::ListenSocket() noexcept : ListenSocket(fly::net::IOMode::Synchronous)
{
}

//==================================================================================================
template <typename EndpointType>
ListenSocket<EndpointType>::ListenSocket(fly::net::IOMode mode) noexcept :
    BaseSocket(fly::net::detail::socket<EndpointType, TcpSocket<EndpointType>>(), mode)
{
}

//==================================================================================================
template <typename EndpointType>
ListenSocket<EndpointType>::ListenSocket(ListenSocket &&socket) noexcept :
    BaseSocket(std::move(socket)),
    m_is_listening(socket.m_is_listening)
{
    socket.m_is_listening = false;
}

//==================================================================================================
template <typename EndpointType>
ListenSocket<EndpointType> &ListenSocket<EndpointType>::operator=(ListenSocket &&socket) noexcept
{
    m_is_listening = socket.m_is_listening;
    socket.m_is_listening = false;

    return static_cast<ListenSocket &>(BaseSocket::operator=(std::move(socket)));
}

//==================================================================================================
template <typename EndpointType>
bool ListenSocket<EndpointType>::listen()
{
    m_is_listening = fly::net::detail::listen(handle());
    return is_listening();
}

//==================================================================================================
template <typename EndpointType>
bool ListenSocket<EndpointType>::is_listening() const
{
    return m_is_listening;
}

//==================================================================================================
template <typename EndpointType>
std::optional<TcpSocket<EndpointType>> ListenSocket<EndpointType>::accept() const
{
    EndpointType client_endpoint;
    bool would_block = false;

    if (auto client = fly::net::detail::accept(handle(), client_endpoint, would_block); client)
    {
        SLOGD(handle(), "Accepted new socket {}", client_endpoint);
        return TcpSocket<EndpointType>(*client, io_mode());
    }

    return std::nullopt;
}

//==================================================================================================
template class ListenSocket<Endpoint<IPv4Address>>;
template class ListenSocket<Endpoint<IPv6Address>>;

} // namespace fly::net
