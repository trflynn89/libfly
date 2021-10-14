#include "fly/net/socket/listen_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/network_config.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/net/socket/socket_service.hpp"
#include "fly/net/socket/tcp_socket.hpp"

namespace fly::net {

//==================================================================================================
template <IPEndpoint EndpointType>
ListenSocket<EndpointType>::ListenSocket(std::shared_ptr<NetworkConfig> config) noexcept :
    ListenSocket(std::move(config), IOMode::Synchronous)
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
ListenSocket<EndpointType>::ListenSocket(
    std::shared_ptr<NetworkConfig> config,
    IOMode mode) noexcept :
    BaseSocket(std::move(config), detail::socket<EndpointType, TcpSocket<EndpointType>>(), mode)
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
ListenSocket<EndpointType>::ListenSocket(
    const std::shared_ptr<SocketService> &service,
    std::shared_ptr<NetworkConfig> config) noexcept :
    BaseSocket(service, std::move(config), detail::socket<EndpointType, TcpSocket<EndpointType>>())
{
}

//==================================================================================================
template <IPEndpoint EndpointType>
ListenSocket<EndpointType>::ListenSocket(ListenSocket &&socket) noexcept :
    BaseSocket(std::move(socket)),
    m_is_listening(socket.m_is_listening)
{
    socket.m_is_listening = false;
}

//==================================================================================================
template <IPEndpoint EndpointType>
auto ListenSocket<EndpointType>::create_socket(
    const std::shared_ptr<SocketService> &service,
    std::shared_ptr<NetworkConfig> config) -> std::shared_ptr<ListenSocket>
{
    // ListenSocket's constructor for socket-service-based sockets is private, thus cannot be used
    // with std::make_shared. This class is used to expose the private constructor locally.
    struct ListenSocketImpl final : public ListenSocket
    {
        ListenSocketImpl(
            const std::shared_ptr<SocketService> &service,
            std::shared_ptr<NetworkConfig> config) noexcept :
            ListenSocket(service, std::move(config))
        {
        }
    };

    return std::make_shared<ListenSocketImpl>(service, std::move(config));
}

//==================================================================================================
template <IPEndpoint EndpointType>
ListenSocket<EndpointType> &ListenSocket<EndpointType>::operator=(ListenSocket &&socket) noexcept
{
    m_is_listening = socket.m_is_listening;
    socket.m_is_listening = false;

    return static_cast<ListenSocket &>(BaseSocket::operator=(std::move(socket)));
}

//==================================================================================================
template <IPEndpoint EndpointType>
bool ListenSocket<EndpointType>::listen()
{
    m_is_listening = detail::listen(handle());
    return is_listening();
}

//==================================================================================================
template <IPEndpoint EndpointType>
bool ListenSocket<EndpointType>::is_listening() const
{
    return m_is_listening;
}

//==================================================================================================
template <IPEndpoint EndpointType>
std::optional<TcpSocket<EndpointType>> ListenSocket<EndpointType>::accept()
{
    EndpointType client_endpoint;
    bool would_block = false;

    if (auto client = detail::accept(handle(), client_endpoint, would_block); client)
    {
        SLOGD(handle(), "Accepted new socket {}", client_endpoint);
        return TcpSocket<EndpointType>(network_config(), *client, io_mode());
    }

    SLOGW(handle(), "Could not accept new socket, closing");
    close();

    return std::nullopt;
}

//==================================================================================================
template <IPEndpoint EndpointType>
bool ListenSocket<EndpointType>::accept_async(AcceptCompletion &&callback)
{
    if (auto service = socket_service(); service && callback)
    {
        service->notify_when_readable(
            this->shared_from_this(),
            [callback = std::move(callback)](std::shared_ptr<ListenSocket> self) mutable {
                self->ready_to_accept(std::move(callback));
            });

        return true;
    }

    return false;
}

//==================================================================================================
template <IPEndpoint EndpointType>
void ListenSocket<EndpointType>::ready_to_accept(AcceptCompletion &&callback)
{
    EndpointType client_endpoint;
    bool would_block = false;

    if (auto client = detail::accept(handle(), client_endpoint, would_block); client)
    {
        SLOGD(handle(), "Accepted new socket {}", client_endpoint);

        auto socket =
            TcpSocket<EndpointType>::create_socket(socket_service(), network_config(), *client);
        std::invoke(std::move(callback), std::move(socket));
    }
    else if (would_block)
    {
        SLOGD(handle(), "Accept would block - will try again later");
        accept_async(std::move(callback));
    }
    else
    {
        SLOGW(handle(), "Could not accept new socket, closing");
        close();

        std::invoke(std::move(callback), nullptr);
    }
}

//==================================================================================================
template class ListenSocket<Endpoint<IPv4Address>>;
template class ListenSocket<Endpoint<IPv6Address>>;

} // namespace fly::net
