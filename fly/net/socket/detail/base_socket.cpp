#include "fly/net/socket/detail/base_socket.hpp"

#include "fly/net/endpoint.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/network_config.hpp"
#include "fly/net/socket/detail/socket_operations.hpp"
#include "fly/net/socket/socket_service.hpp"

#include <atomic>

namespace fly::net::detail {

namespace {

    std::atomic_uint64_t s_num_sockets {0};

} // namespace

//==================================================================================================
template <typename EndpointType>
BaseSocket<EndpointType>::BaseSocket(
    std::shared_ptr<fly::net::NetworkConfig> config,
    socket_type handle,
    fly::net::IOMode mode) noexcept :
    m_config(std::move(config)),
    m_socket_handle(handle),
    m_socket_id(s_num_sockets.fetch_add(1)),
    m_mode(mode)
{
    set_io_mode(mode);
}

//==================================================================================================
template <typename EndpointType>
BaseSocket<EndpointType>::BaseSocket(
    const std::shared_ptr<fly::net::SocketService> &service,
    std::shared_ptr<fly::net::NetworkConfig> config,
    socket_type handle) noexcept :
    BaseSocket(std::move(config), handle, fly::net::IOMode::Asynchronous)
{
    m_weak_socket_service = service;
}

//==================================================================================================
template <typename EndpointType>
BaseSocket<EndpointType>::BaseSocket(BaseSocket &&socket) noexcept :
    m_weak_socket_service(std::move(socket.m_weak_socket_service)),
    m_config(std::move(socket.m_config)),
    m_socket_handle(socket.m_socket_handle),
    m_socket_id(socket.m_socket_id),
    m_mode(socket.m_mode)
{
    socket.m_socket_handle = fly::net::detail::invalid_socket();
}

//==================================================================================================
template <typename EndpointType>
BaseSocket<EndpointType>::~BaseSocket() noexcept
{
    close();
}

//==================================================================================================
template <typename EndpointType>
BaseSocket<EndpointType> &BaseSocket<EndpointType>::operator=(BaseSocket &&socket) noexcept
{
    m_weak_socket_service = std::move(socket.m_weak_socket_service);
    m_config = std::move(socket.m_config);
    m_socket_handle = socket.m_socket_handle;
    m_socket_id = socket.m_socket_id;
    m_mode = socket.m_mode;

    socket.m_socket_handle = fly::net::detail::invalid_socket();

    return *this;
}

//==================================================================================================
template <typename EndpointType>
auto BaseSocket<EndpointType>::hostname_to_address(std::string_view hostname)
    -> std::optional<address_type>
{
    if (auto address = detail::hostname_to_address<address_type>(std::move(hostname)); address)
    {
        LOGD("Resolved hostname {} to {}", hostname, *address);
        return address;
    }

    return std::nullopt;
}

//==================================================================================================
template <typename EndpointType>
bool BaseSocket<EndpointType>::is_open() const
{
    return m_socket_handle != fly::net::detail::invalid_socket();
}

//==================================================================================================
template <typename EndpointType>
socket_type BaseSocket<EndpointType>::handle() const
{
    return m_socket_handle;
}

//==================================================================================================
template <typename EndpointType>
std::uint64_t BaseSocket<EndpointType>::socket_id() const
{
    return m_socket_id;
}

//==================================================================================================
template <typename EndpointType>
bool BaseSocket<EndpointType>::set_io_mode(fly::net::IOMode mode)
{
    if (fly::net::detail::set_io_mode(m_socket_handle, mode))
    {
        m_mode = mode;
    }
    else
    {
        close();
    }

    return is_open();
}

//==================================================================================================
template <typename EndpointType>
fly::net::IOMode BaseSocket<EndpointType>::io_mode() const
{
    return m_mode;
}

//==================================================================================================
template <typename EndpointType>
std::optional<EndpointType> BaseSocket<EndpointType>::local_endpoint() const
{
    return fly::net::detail::local_endpoint<EndpointType>(m_socket_handle);
}

//==================================================================================================
template <typename EndpointType>
void BaseSocket<EndpointType>::close()
{
    if (is_open())
    {
        if (auto service = socket_service(); service)
        {
            service->remove_socket(m_socket_handle);
        }

        fly::net::detail::close(m_socket_handle);
        m_socket_handle = fly::net::detail::invalid_socket();
    }
}

//==================================================================================================
template <typename EndpointType>
bool BaseSocket<EndpointType>::bind(const EndpointType &endpoint, BindMode option) const
{
    return fly::net::detail::bind(m_socket_handle, endpoint, option);
}

//==================================================================================================
template <typename EndpointType>
bool BaseSocket<EndpointType>::bind(std::string_view hostname, port_type port, BindMode option)
    const
{
    if (auto address = hostname_to_address(std::move(hostname)); address)
    {
        return bind(EndpointType(std::move(*address), port), option);
    }

    return false;
}

//==================================================================================================
template <typename EndpointType>
std::shared_ptr<fly::net::SocketService> BaseSocket<EndpointType>::socket_service() const
{
    return m_weak_socket_service.lock();
}

//==================================================================================================
template <typename EndpointType>
std::shared_ptr<fly::net::NetworkConfig> BaseSocket<EndpointType>::network_config() const
{
    return m_config;
}

//==================================================================================================
template <typename EndpointType>
std::size_t BaseSocket<EndpointType>::packet_size() const
{
    return m_config->packet_size();
}

//==================================================================================================
template class BaseSocket<fly::net::Endpoint<fly::net::IPv4Address>>;
template class BaseSocket<fly::net::Endpoint<fly::net::IPv6Address>>;

} // namespace fly::net::detail
