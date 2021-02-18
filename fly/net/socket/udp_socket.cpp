#include "fly/net/socket/udp_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"

namespace fly::net {

//==================================================================================================
template <typename EndpointType>
UdpSocket<EndpointType>::UdpSocket() noexcept : UdpSocket(fly::net::IOMode::Synchronous)
{
}

//==================================================================================================
template <typename EndpointType>
UdpSocket<EndpointType>::UdpSocket(fly::net::IOMode mode) noexcept :
    BaseSocket(fly::net::detail::socket<EndpointType, UdpSocket<EndpointType>>(), mode)
{
}

//==================================================================================================
template <typename EndpointType>
UdpSocket<EndpointType>::UdpSocket(UdpSocket &&socket) noexcept : BaseSocket(std::move(socket))
{
}

//==================================================================================================
template <typename EndpointType>
UdpSocket<EndpointType> &UdpSocket<EndpointType>::operator=(UdpSocket &&socket) noexcept
{
    return static_cast<UdpSocket &>(BaseSocket::operator=(std::move(socket)));
}

//==================================================================================================
template <typename EndpointType>
size_t UdpSocket<EndpointType>::send(const EndpointType &endpoint, std::string_view message) const
{
    bool would_block = false;

    return fly::net::detail::send_to(
        handle(),
        endpoint,
        std::move(message),
        m_packet_size,
        would_block);
}

//==================================================================================================
template <typename EndpointType>
size_t
UdpSocket<EndpointType>::send(std::string_view hostname, port_type port, std::string_view message)
    const
{
    if (auto endpoint = hostname_to_endpoint(std::move(hostname)); endpoint)
    {
        endpoint->set_port(port);
        return send(*endpoint, std::move(message));
    }

    return 0;
}

//==================================================================================================
template <typename EndpointType>
std::string UdpSocket<EndpointType>::receive() const
{
    EndpointType endpoint;
    bool would_block = false;

    return fly::net::detail::recv_from(handle(), endpoint, m_packet_size, would_block);
}

//==================================================================================================
template class UdpSocket<Endpoint<IPv4Address>>;
template class UdpSocket<Endpoint<IPv6Address>>;

} // namespace fly::net
