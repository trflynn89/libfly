#include "fly/net/socket/detail/socket_operations.hpp"

#include "fly/logger/logger.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <type_traits>

namespace fly::net {

template <typename EndpointType>
class TcpSocket;

template <typename EndpointType>
class UdpSocket;

} // namespace fly::net

namespace fly::net::detail {

namespace {

    template <typename EndpointType>
    using socket_address_type =
        std::conditional_t<EndpointType::is_ipv4(), sockaddr_in, sockaddr_in6>;

    using IPv4Endpoint = fly::net::Endpoint<fly::net::IPv4Address>;
    using IPv6Endpoint = fly::net::Endpoint<fly::net::IPv6Address>;

    sockaddr_in endpoint_to_address(const fly::net::Endpoint<fly::net::IPv4Address> &endpoint)
    {
        sockaddr_in socket_address {};

        socket_address.sin_family = AF_INET;
        socket_address.sin_addr.s_addr = endpoint.address().network_order();
        socket_address.sin_port = htons(endpoint.port());

        return socket_address;
    }

    sockaddr_in6 endpoint_to_address(const fly::net::Endpoint<fly::net::IPv6Address> &endpoint)
    {
        sockaddr_in6 socket_address {};

        socket_address.sin6_family = AF_INET6;
        endpoint.address().copy(socket_address.sin6_addr.s6_addr);
        socket_address.sin6_port = htons(endpoint.port());

        return socket_address;
    }

    fly::net::Endpoint<fly::net::IPv4Address> address_to_endpoint(const sockaddr_in &socket_address)
    {
        fly::net::IPv4Address address(socket_address.sin_addr.s_addr);
        fly::net::port_type port = ntohs(socket_address.sin_port);

        return fly::net::Endpoint(std::move(address), port);
    }

    fly::net::Endpoint<fly::net::IPv6Address>
    address_to_endpoint(const sockaddr_in6 &socket_address)
    {
        fly::net::IPv6Address address(socket_address.sin6_addr.s6_addr);
        fly::net::port_type port = ntohs(socket_address.sin6_port);

        return fly::net::Endpoint(std::move(address), port);
    }

    template <typename SocketAddressType>
    sockaddr *base(SocketAddressType &address)
    {
        return reinterpret_cast<sockaddr *>(&address);
    }

    template <typename SocketAddressType>
    const sockaddr *base(const SocketAddressType &address)
    {
        return reinterpret_cast<const sockaddr *>(&address);
    }

} // namespace

//==================================================================================================
fly::net::socket_type invalid_socket()
{
    return -1;
}

//==================================================================================================
template <typename EndpointType>
std::optional<EndpointType> hostname_to_endpoint(std::string_view hostname)
{
    addrinfo *results = nullptr;

    addrinfo hints = {};
    hints.ai_family = EndpointType::is_ipv4() ? AF_INET : AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Copying to a string is required because the hostname might not be null terminated.
    const std::string hostname_copy(hostname.data(), hostname.size());

    if (int error = ::getaddrinfo(hostname_copy.c_str(), nullptr, &hints, &results); error != 0)
    {
        LOGS("Error resolving {}: ({}) {}", hostname, error, ::gai_strerror(error));
        return std::nullopt;
    }

    auto *address = reinterpret_cast<socket_address_type<EndpointType> *>(results->ai_addr);
    const auto endpoint = address_to_endpoint(*address);
    ::freeaddrinfo(results);

    return endpoint;
}

template std::optional<IPv4Endpoint> hostname_to_endpoint(std::string_view hostname);
template std::optional<IPv6Endpoint> hostname_to_endpoint(std::string_view hostname);

//==================================================================================================
template <>
fly::net::socket_type socket<IPv4Endpoint, fly::net::TcpSocket<IPv4Endpoint>>()
{
    return ::socket(AF_INET, SOCK_STREAM, 0);
}

//==================================================================================================
template <>
fly::net::socket_type socket<IPv4Endpoint, fly::net::UdpSocket<IPv4Endpoint>>()
{
    return ::socket(AF_INET, SOCK_DGRAM, 0);
}

//==================================================================================================
template <>
fly::net::socket_type socket<IPv6Endpoint, fly::net::TcpSocket<IPv6Endpoint>>()
{
    return ::socket(AF_INET6, SOCK_STREAM, 0);
}

//==================================================================================================
template <>
fly::net::socket_type socket<IPv6Endpoint, fly::net::UdpSocket<IPv6Endpoint>>()
{
    return ::socket(AF_INET6, SOCK_DGRAM, 0);
}

//==================================================================================================
void close(fly::net::socket_type handle)
{
    ::close(handle);
}

//==================================================================================================
bool is_error_free(fly::net::socket_type handle)
{
    int option_value = -1;
    socklen_t option_size = sizeof(option_value);

    if (::getsockopt(handle, SOL_SOCKET, SO_ERROR, &option_value, &option_size) == -1)
    {
        SLOGS(handle, "Error getting error flag");
    }

    return option_value == 0;
}

//==================================================================================================
bool set_io_mode(fly::net::socket_type handle, fly::net::IOMode mode)
{
    int flags = ::fcntl(handle, F_GETFL, 0);

    if (flags == -1)
    {
        SLOGS(handle, "Error getting socket flags");
        return false;
    }

    flags = (mode == fly::net::IOMode::Synchronous) ? flags & ~O_NONBLOCK : flags | O_NONBLOCK;

    if (::fcntl(handle, F_SETFL, flags) == -1)
    {
        SLOGS(handle, "Error setting IO mode to {}", mode);
        return false;
    }

    return true;
}

//==================================================================================================
template <typename EndpointType>
std::optional<EndpointType> local_endpoint(fly::net::socket_type handle)
{
    socket_address_type<EndpointType> address;
    socklen_t address_size = sizeof(address);

    if (::getsockname(handle, base(address), &address_size) == -1)
    {
        SLOGS(handle, "Error getting bound endpoint");
        return std::nullopt;
    }

    return address_to_endpoint(address);
}

template std::optional<IPv4Endpoint> local_endpoint(fly::net::socket_type handle);
template std::optional<IPv6Endpoint> local_endpoint(fly::net::socket_type handle);

//==================================================================================================
template <typename EndpointType>
std::optional<EndpointType> remote_endpoint(fly::net::socket_type handle)
{
    socket_address_type<EndpointType> address;
    socklen_t address_size = sizeof(address);

    if (::getpeername(handle, base(address), &address_size) == -1)
    {
        SLOGS(handle, "Error getting remote endpoint");
        return std::nullopt;
    }

    return address_to_endpoint(address);
}

template std::optional<IPv4Endpoint> remote_endpoint(fly::net::socket_type handle);
template std::optional<IPv6Endpoint> remote_endpoint(fly::net::socket_type handle);

//==================================================================================================
template <typename EndpointType>
bool bind(fly::net::socket_type handle, const EndpointType &endpoint, fly::net::BindMode mode)
{
    static constexpr const int s_bind_reuse = 1;
    static constexpr const socklen_t s_bind_reuse_size = sizeof(s_bind_reuse);

    if (mode == fly::net::BindMode::AllowReuse)
    {
        if (::setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &s_bind_reuse, s_bind_reuse_size) == -1)
        {
            SLOGS(handle, "Error setting reuse flag");
            return false;
        }
    }

    const auto address = endpoint_to_address(endpoint);

    if (::bind(handle, base(address), sizeof(address)) == -1)
    {
        SLOGS(handle, "Error binding to {}", endpoint);
        return false;
    }

    return true;
}

template bool
bind(fly::net::socket_type handle, const IPv4Endpoint &endpoint, fly::net::BindMode mode);

template bool
bind(fly::net::socket_type handle, const IPv6Endpoint &endpoint, fly::net::BindMode mode);

//==================================================================================================
bool listen(fly::net::socket_type handle)
{
    if (::listen(handle, 1024) == -1)
    {
        SLOGS(handle, "Error listening");
        return false;
    }

    return true;
}

//==================================================================================================
template <typename EndpointType>
std::optional<fly::net::socket_type>
accept(fly::net::socket_type handle, EndpointType &endpoint, bool &would_block)
{
    socket_address_type<EndpointType> address;
    socklen_t address_size = sizeof(address);

    const fly::net::socket_type client = ::accept(handle, base(address), &address_size);
    would_block = false;

    if (client == invalid_socket())
    {
        would_block = fly::System::get_error_code() == EWOULDBLOCK;
        SLOGS(handle, "Error accepting");

        return std::nullopt;
    }

    endpoint = address_to_endpoint(address);
    return client;
}

template std::optional<fly::net::socket_type>
accept(fly::net::socket_type handle, IPv4Endpoint &endpoint, bool &would_block);

template std::optional<fly::net::socket_type>
accept(fly::net::socket_type handle, IPv6Endpoint &endpoint, bool &would_block);

//==================================================================================================
template <typename EndpointType>
fly::net::ConnectedState connect(fly::net::socket_type handle, const EndpointType &endpoint)
{
    const auto address = endpoint_to_address(endpoint);

    if (::connect(handle, base(address), sizeof(address)) == -1)
    {
        const int error = fly::System::get_error_code();
        SLOGS(handle, "Error connecting");

        if ((error == EINTR) || (error == EINPROGRESS))
        {
            return fly::net::ConnectedState::Connecting;
        }

        return fly::net::ConnectedState::Disconnected;
    }

    return fly::net::ConnectedState::Connected;
}

template fly::net::ConnectedState
connect(fly::net::socket_type handle, const IPv4Endpoint &endpoint);

template fly::net::ConnectedState
connect(fly::net::socket_type handle, const IPv6Endpoint &endpoint);

//==================================================================================================
std::size_t send(fly::net::socket_type handle, std::string_view message, bool &would_block)
{
    std::size_t bytes_sent = 0;
    would_block = false;

    bool keep_sending = !message.empty();

    while (keep_sending)
    {
        const ssize_t status = ::send(handle, message.data(), message.size(), MSG_NOSIGNAL);

        if (status > 0)
        {
            const auto bytes = static_cast<std::size_t>(status);
            bytes_sent += bytes;

            message = message.substr(bytes, std::string::npos);
            keep_sending = !message.empty();
        }
        else
        {
            keep_sending = false;

            if (status == -1)
            {
                would_block = fly::System::get_error_code() == EWOULDBLOCK;
                SLOGS(handle, "Error sending");
            }
        }
    }

    return bytes_sent;
}

//==================================================================================================
template <typename EndpointType>
std::size_t send_to(
    fly::net::socket_type handle,
    const EndpointType &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block)
{
    std::size_t bytes_sent = 0;
    would_block = false;

    const auto address = endpoint_to_address(endpoint);
    bool keep_sending = !message.empty();

    while (keep_sending)
    {
        const auto size = std::min(packet_size, message.size());

        const ssize_t status =
            ::sendto(handle, message.data(), size, 0, base(address), sizeof(address));

        if (status > 0)
        {
            const auto bytes = static_cast<std::size_t>(status);
            bytes_sent += bytes;

            message = message.substr(bytes, std::string::npos);
            keep_sending = !message.empty();
        }
        else
        {
            keep_sending = false;

            if (status == -1)
            {
                would_block = fly::System::get_error_code() == EWOULDBLOCK;
                SLOGS(handle, "Error sending");
            }
        }
    }

    return bytes_sent;
}

template std::size_t send_to(
    fly::net::socket_type handle,
    const IPv4Endpoint &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block);

template std::size_t send_to(
    fly::net::socket_type handle,
    const IPv6Endpoint &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block);

//==================================================================================================
std::string recv(fly::net::socket_type handle, std::size_t packet_size, bool &would_block)
{
    std::string result(packet_size, 0);
    would_block = false;

    const ssize_t status = ::recv(handle, result.data(), result.size(), 0);

    if (status > 0)
    {
        result.resize(static_cast<std::size_t>(status));
    }
    else
    {
        if (status == -1)
        {
            would_block = fly::System::get_error_code() == EWOULDBLOCK;
            SLOGS(handle, "Error receiving");
        }

        result.clear();
    }

    return result;
}

//==================================================================================================
template <typename EndpointType>
std::string recv_from(
    fly::net::socket_type handle,
    EndpointType &endpoint,
    std::size_t packet_size,
    bool &would_block)
{
    std::string result(packet_size, 0);
    would_block = false;

    socket_address_type<EndpointType> address;
    socklen_t address_size = sizeof(address);

    const ssize_t status =
        ::recvfrom(handle, result.data(), result.size(), 0, base(address), &address_size);

    if (status > 0)
    {
        result.resize(static_cast<std::size_t>(status));
        endpoint = address_to_endpoint(address);
    }
    else
    {
        if (status == -1)
        {
            would_block = fly::System::get_error_code() == EWOULDBLOCK;
            SLOGS(handle, "Error receiving");
        }

        result.clear();
    }

    return result;
}

template std::string recv_from(
    fly::net::socket_type handle,
    IPv4Endpoint &endpoint,
    std::size_t packet_size,
    bool &would_block);

template std::string recv_from(
    fly::net::socket_type handle,
    IPv6Endpoint &endpoint,
    std::size_t packet_size,
    bool &would_block);

} // namespace fly::net::detail
