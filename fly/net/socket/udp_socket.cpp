#include "fly/net/socket/udp_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/network_config.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/net/socket/socket_service.hpp"

namespace fly::net {

//==================================================================================================
template <IPEndpoint EndpointType>
UdpSocket<EndpointType>::UdpSocket(std::shared_ptr<NetworkConfig> config) noexcept :
    UdpSocket(std::move(config), IOMode::Synchronous)
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
UdpSocket<EndpointType>::UdpSocket(std::shared_ptr<NetworkConfig> config, IOMode mode) noexcept :
    BaseSocket(std::move(config), detail::socket<EndpointType, UdpSocket<EndpointType>>(), mode)
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
UdpSocket<EndpointType>::UdpSocket(
    std::shared_ptr<SocketService> const &service,
    std::shared_ptr<NetworkConfig> config) noexcept :
    BaseSocket(service, std::move(config), detail::socket<EndpointType, UdpSocket<EndpointType>>())
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
UdpSocket<EndpointType>::UdpSocket(UdpSocket &&socket) noexcept :
    BaseSocket(std::move(socket))
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
auto UdpSocket<EndpointType>::create_socket(
    std::shared_ptr<SocketService> const &service,
    std::shared_ptr<NetworkConfig> config) -> std::shared_ptr<UdpSocket>
{
    // UdpSocket's constructor for socket-service-based sockets is private, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructor locally.
    struct UdpSocketImpl final : public UdpSocket
    {
        UdpSocketImpl(
            std::shared_ptr<SocketService> const &service,
            std::shared_ptr<NetworkConfig> config) noexcept :
            UdpSocket(service, std::move(config))
        {
        }
    };

    return std::make_shared<UdpSocketImpl>(service, std::move(config));
}

//==================================================================================================
template <IPEndpoint EndpointType>
UdpSocket<EndpointType> &UdpSocket<EndpointType>::operator=(UdpSocket &&socket) noexcept
{
    return static_cast<UdpSocket &>(BaseSocket::operator=(std::move(socket)));
}

//==================================================================================================
template <IPEndpoint EndpointType>
size_t UdpSocket<EndpointType>::send(EndpointType const &endpoint, std::string_view message)
{
    bool would_block = false;

    std::size_t bytes_sent =
        detail::send_to(handle(), endpoint, std::move(message), packet_size(), would_block);

    if (bytes_sent == 0)
    {
        SLOGW(handle(), "Error sending to {}, closing", endpoint);
        close();
    }
    else
    {
        SLOGD(handle(), "Sent {} bytes to {}", bytes_sent, endpoint);
    }

    return bytes_sent;
}

//==================================================================================================
template <IPEndpoint EndpointType>
size_t
UdpSocket<EndpointType>::send(std::string_view hostname, port_type port, std::string_view message)

{
    if (auto address = hostname_to_address(std::move(hostname)); address)
    {
        return send(EndpointType(std::move(*address), port), std::move(message));
    }

    return 0;
}

//==================================================================================================
template <IPEndpoint EndpointType>
bool UdpSocket<EndpointType>::send_async(
    EndpointType const &endpoint,
    std::string_view message,
    SendCompletion &&callback)
{
    if (auto service = socket_service(); service && callback)
    {
        service->notify_when_writable(
            this->shared_from_this(),
            [endpoint, message, callback = std::move(callback)](
                std::shared_ptr<UdpSocket> self) mutable {
                self->ready_to_send(endpoint, message, std::move(callback), 0, message.size());
            });

        return true;
    }

    return false;
}

//==================================================================================================
template <IPEndpoint EndpointType>
bool UdpSocket<EndpointType>::send_async(
    std::string_view hostname,
    port_type port,
    std::string_view message,
    SendCompletion &&callback)
{
    if (auto address = hostname_to_address(std::move(hostname)); address)
    {
        return send_async(
            EndpointType(std::move(*address), port),
            std::move(message),
            std::move(callback));
    }

    return false;
}

//==================================================================================================
template <IPEndpoint EndpointType>
std::string UdpSocket<EndpointType>::receive()
{
    EndpointType endpoint;
    bool would_block = false;

    std::string const received = detail::recv_from(handle(), endpoint, packet_size(), would_block);

    if (received.size() == 0)
    {
        SLOGW(handle(), "Error receiving, closing");
        close();
    }
    else
    {
        SLOGD(handle(), "Received {} bytes from {}", received.size(), endpoint);
    }

    return received;
}

//==================================================================================================
template <IPEndpoint EndpointType>
bool UdpSocket<EndpointType>::receive_async(ReceiveCompletion &&callback)
{
    if (auto service = socket_service(); service && callback)
    {
        service->notify_when_readable(
            this->shared_from_this(),
            [callback = std::move(callback)](std::shared_ptr<UdpSocket> self) mutable {
                self->ready_to_receive(std::move(callback), std::string());
            });

        return true;
    }

    return false;
}

//==================================================================================================
template <IPEndpoint EndpointType>
void UdpSocket<EndpointType>::ready_to_send(
    EndpointType const &endpoint,
    std::string_view message,
    SendCompletion &&callback,
    std::size_t bytes_sent,
    std::size_t total_bytes)
{
    bool would_block = false;

    std::size_t const current_sent =
        detail::send_to(handle(), endpoint, message, packet_size(), would_block);
    bytes_sent += current_sent;

    if (current_sent == message.size())
    {
        SLOGD(handle(), "Completed sending {} bytes to {}", bytes_sent, endpoint);
        std::invoke(std::move(callback), bytes_sent);
    }
    else if (would_block)
    {
        SLOGI(handle(), "Send would block - sent {} of {} bytes", bytes_sent, total_bytes);
        message = message.substr(current_sent);

        socket_service()->notify_when_writable(
            this->shared_from_this(),
            [endpoint, message, callback = std::move(callback), bytes_sent, total_bytes](
                std::shared_ptr<UdpSocket> self) mutable {
                self->ready_to_send(
                    endpoint,
                    message,
                    std::move(callback),
                    bytes_sent,
                    total_bytes);
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
template <IPEndpoint EndpointType>
void UdpSocket<EndpointType>::ready_to_receive(ReceiveCompletion &&callback, std::string received)
{
    EndpointType endpoint;
    bool would_block = false;

    std::string const current_received =
        detail::recv_from(handle(), endpoint, packet_size(), would_block);
    received += current_received;

    if (!current_received.empty())
    {
        SLOGD(handle(), "Received {} bytes from {}", received.size(), endpoint);
        std::invoke(std::move(callback), std::move(received));
    }
    else if (would_block)
    {
        SLOGI(handle(), "Receive would block - received {} bytes", received.length());

        socket_service()->notify_when_readable(
            this->shared_from_this(),
            [callback = std::move(callback),
             received = std::move(received)](std::shared_ptr<UdpSocket> self) mutable {
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
template class UdpSocket<Endpoint<IPv4Address>>;
template class UdpSocket<Endpoint<IPv6Address>>;

} // namespace fly::net
