#include "fly/net/socket/detail/socket_operations.hpp"

#include "fly/logger/logger.hpp"
#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <type_traits>

namespace fly::net {

template <IPEndpoint EndpointType>
class TcpSocket;

template <IPEndpoint EndpointType>
class UdpSocket;

} // namespace fly::net

namespace fly::net::detail {

namespace {

    template <fly::net::IPEndpoint EndpointType>
    using socket_address_type =
        std::conditional_t<EndpointType::is_ipv4(), sockaddr_in, sockaddr_in6>;

    using IPv4Endpoint = fly::net::Endpoint<fly::net::IPv4Address>;
    using IPv6Endpoint = fly::net::Endpoint<fly::net::IPv6Address>;

    fly::net::IPv4Address create_address(sockaddr_in const &socket_address)
    {
        return fly::net::IPv4Address(socket_address.sin_addr.s_addr);
    }

    fly::net::IPv6Address create_address(sockaddr_in6 const &socket_address)
    {
        return fly::net::IPv6Address(socket_address.sin6_addr.s6_addr);
    }

    sockaddr_in endpoint_to_address(fly::net::Endpoint<fly::net::IPv4Address> const &endpoint)
    {
        sockaddr_in socket_address {};

        socket_address.sin_family = AF_INET;
        socket_address.sin_addr.s_addr = endpoint.address().network_order();
        socket_address.sin_port = htons(endpoint.port());

        return socket_address;
    }

    sockaddr_in6 endpoint_to_address(fly::net::Endpoint<fly::net::IPv6Address> const &endpoint)
    {
        sockaddr_in6 socket_address {};

        socket_address.sin6_family = AF_INET6;
        endpoint.address().copy(socket_address.sin6_addr.s6_addr);
        socket_address.sin6_port = htons(endpoint.port());

        return socket_address;
    }

    fly::net::Endpoint<fly::net::IPv4Address> address_to_endpoint(sockaddr_in const &socket_address)
    {
        fly::net::IPv4Address address = create_address(socket_address);
        fly::net::port_type port = ntohs(socket_address.sin_port);

        return fly::net::Endpoint(std::move(address), port);
    }

    fly::net::Endpoint<fly::net::IPv6Address>
    address_to_endpoint(sockaddr_in6 const &socket_address)
    {
        fly::net::IPv6Address address = create_address(socket_address);
        fly::net::port_type port = ntohs(socket_address.sin6_port);

        return fly::net::Endpoint(std::move(address), port);
    }

    template <typename SocketAddressType>
    sockaddr *base(SocketAddressType &address)
    {
        return reinterpret_cast<sockaddr *>(&address);
    }

    template <typename SocketAddressType>
    sockaddr const *base(SocketAddressType const &address)
    {
        return reinterpret_cast<sockaddr const *>(&address);
    }

} // namespace

//==================================================================================================
void initialize()
{
}

//==================================================================================================
void deinitialize()
{
}

//==================================================================================================
fly::net::socket_type invalid_socket()
{
    return -1;
}

//==================================================================================================
template <fly::net::IPAddress IPAddressType>
std::optional<IPAddressType> hostname_to_address(std::string_view hostname)
{
    using EndpointType = fly::net::Endpoint<IPAddressType>;

    addrinfo *results = nullptr;

    addrinfo hints = {};
    hints.ai_family = EndpointType::is_ipv4() ? AF_INET : AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Copying to a string is required because the hostname might not be null terminated.
    std::string const hostname_copy(hostname.data(), hostname.size());

    if (int error = ::getaddrinfo(hostname_copy.c_str(), nullptr, &hints, &results); error != 0)
    {
        LOGS("Error resolving {}: ({}) {}", hostname, error, ::gai_strerror(error));
        return std::nullopt;
    }

    auto *address = reinterpret_cast<socket_address_type<EndpointType> *>(results->ai_addr);
    auto const ip_address = create_address(*address);
    ::freeaddrinfo(results);

    return ip_address;
}

template std::optional<fly::net::IPv4Address> hostname_to_address(std::string_view hostname);
template std::optional<fly::net::IPv6Address> hostname_to_address(std::string_view hostname);

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
template <fly::net::IPEndpoint EndpointType>
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
template <fly::net::IPEndpoint EndpointType>
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
template <fly::net::IPEndpoint EndpointType>
bool bind(fly::net::socket_type handle, EndpointType const &endpoint, fly::net::BindMode mode)
{
    static constexpr int const s_bind_reuse = 1;
    static constexpr socklen_t const s_bind_reuse_size = sizeof(s_bind_reuse);

    if (mode == fly::net::BindMode::AllowReuse)
    {
        if (::setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &s_bind_reuse, s_bind_reuse_size) == -1)
        {
            SLOGS(handle, "Error setting reuse flag");
            return false;
        }
    }

    auto const address = endpoint_to_address(endpoint);

    if (::bind(handle, base(address), sizeof(address)) == -1)
    {
        SLOGS(handle, "Error binding to {}", endpoint);
        return false;
    }

    return true;
}

template bool
bind(fly::net::socket_type handle, IPv4Endpoint const &endpoint, fly::net::BindMode mode);

template bool
bind(fly::net::socket_type handle, IPv6Endpoint const &endpoint, fly::net::BindMode mode);

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
template <fly::net::IPEndpoint EndpointType>
std::optional<fly::net::socket_type>
accept(fly::net::socket_type handle, EndpointType &endpoint, bool &would_block)
{
    socket_address_type<EndpointType> address;
    socklen_t address_size = sizeof(address);

    fly::net::socket_type const client = ::accept(handle, base(address), &address_size);
    would_block = false;

    if (client == invalid_socket())
    {
        would_block = fly::system::get_error_code() == EWOULDBLOCK;
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
template <fly::net::IPEndpoint EndpointType>
fly::net::ConnectedState connect(fly::net::socket_type handle, EndpointType const &endpoint)
{
    auto const address = endpoint_to_address(endpoint);

    if (::connect(handle, base(address), sizeof(address)) == -1)
    {
        int const error = fly::system::get_error_code();
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
connect(fly::net::socket_type handle, IPv4Endpoint const &endpoint);

template fly::net::ConnectedState
connect(fly::net::socket_type handle, IPv6Endpoint const &endpoint);

//==================================================================================================
std::size_t send(fly::net::socket_type handle, std::string_view message, bool &would_block)
{
    std::size_t bytes_sent = 0;
    would_block = false;

    bool keep_sending = !message.empty();

    while (keep_sending)
    {
        ssize_t const status = ::send(handle, message.data(), message.size(), MSG_NOSIGNAL);

        if (status > 0)
        {
            auto const bytes = static_cast<std::size_t>(status);
            bytes_sent += bytes;

            message = message.substr(bytes, std::string::npos);
            keep_sending = !message.empty();
        }
        else
        {
            keep_sending = false;

            if (status == -1)
            {
                would_block = fly::system::get_error_code() == EWOULDBLOCK;
                SLOGS(handle, "Error sending");
            }
        }
    }

    return bytes_sent;
}

//==================================================================================================
template <fly::net::IPEndpoint EndpointType>
std::size_t send_to(
    fly::net::socket_type handle,
    EndpointType const &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block)
{
    std::size_t bytes_sent = 0;
    would_block = false;

    auto const address = endpoint_to_address(endpoint);
    bool keep_sending = !message.empty();

    while (keep_sending)
    {
        auto const size = std::min(packet_size, message.size());

        ssize_t const status =
            ::sendto(handle, message.data(), size, 0, base(address), sizeof(address));

        if (status > 0)
        {
            auto const bytes = static_cast<std::size_t>(status);
            bytes_sent += bytes;

            message = message.substr(bytes, std::string::npos);
            keep_sending = !message.empty();
        }
        else
        {
            keep_sending = false;

            if (status == -1)
            {
                would_block = fly::system::get_error_code() == EWOULDBLOCK;
                SLOGS(handle, "Error sending");
            }
        }
    }

    return bytes_sent;
}

template std::size_t send_to(
    fly::net::socket_type handle,
    IPv4Endpoint const &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block);

template std::size_t send_to(
    fly::net::socket_type handle,
    IPv6Endpoint const &endpoint,
    std::string_view message,
    std::size_t packet_size,
    bool &would_block);

//==================================================================================================
std::string recv(fly::net::socket_type handle, std::size_t packet_size, bool &would_block)
{
    std::string result(packet_size, 0);
    would_block = false;

    ssize_t const status = ::recv(handle, result.data(), result.size(), 0);

    if (status > 0)
    {
        result.resize(static_cast<std::size_t>(status));
    }
    else
    {
        if (status == -1)
        {
            would_block = fly::system::get_error_code() == EWOULDBLOCK;
            SLOGS(handle, "Error receiving");
        }

        result.clear();
    }

    return result;
}

//==================================================================================================
template <fly::net::IPEndpoint EndpointType>
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

    ssize_t const status =
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
            would_block = fly::system::get_error_code() == EWOULDBLOCK;
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

//==================================================================================================
void select(
    std::chrono::microseconds timeout,
    std::set<fly::net::socket_type> &writing_handles,
    std::set<fly::net::socket_type> &reading_handles)
{
    fd_set write_set, read_set;
    FD_ZERO(&write_set);
    FD_ZERO(&read_set);

    for (fly::net::socket_type handle : writing_handles)
    {
        FD_SET(handle, &write_set);
    }
    for (fly::net::socket_type handle : reading_handles)
    {
        FD_SET(handle, &read_set);
    }

    socket_type const max_handle = std::max(
        writing_handles.empty() ? invalid_socket() : *writing_handles.rbegin(),
        reading_handles.empty() ? invalid_socket() : *reading_handles.rbegin());

    suseconds_t const usec = static_cast<suseconds_t>(timeout.count());
    timeval tv {0, usec};

    int const status = ::select(max_handle + 1, &read_set, &write_set, nullptr, &tv);

    if (status > 0)
    {
        auto is_not_set = [](fd_set *set, socket_type handle) {
            return !FD_ISSET(handle, set);
        };

        std::erase_if(writing_handles, std::bind(is_not_set, &write_set, std::placeholders::_1));
        std::erase_if(reading_handles, std::bind(is_not_set, &read_set, std::placeholders::_1));
    }
    else
    {
        if (status == -1)
        {
            LOGS(
                "Error polling {} writing, {} reading sockets",
                writing_handles.size(),
                reading_handles.size());
        }

        writing_handles.clear();
        reading_handles.clear();
    }
}

} // namespace fly::net::detail
