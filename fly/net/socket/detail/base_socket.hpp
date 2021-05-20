#pragma once

#include "fly/net/socket/socket_types.hpp"
#include "fly/traits/traits.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <type_traits>

namespace fly::net {

class IPv4Address;
class IPv6Address;
class NetworkConfig;
class SocketService;

template <typename IPAddressType>
class Endpoint;

} // namespace fly::net

namespace fly::net::detail {

/**
 * Base class to represent a network socket and provide functionality needed by all concrete socket
 * types.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <typename EndpointType>
class BaseSocket
{
    static_assert(
        fly::any_same_v<
            EndpointType,
            fly::net::Endpoint<fly::net::IPv4Address>,
            fly::net::Endpoint<fly::net::IPv6Address>>,
        "Sockets may only be created with IPv4 or IPv6 endpoints");

public:
    using endpoint_type = EndpointType;
    using address_type = typename EndpointType::address_type;

    /**
     * @return True if this is an IPv4 socket.
     */
    static constexpr bool is_ipv4();

    /**
     * @return True if this is an IPv6 socket.
     */
    static constexpr bool is_ipv6();

    /**
     * Convert a hostname or IP address string to an endpoint.
     *
     * @param hostname The hostname or IP address string to convert.
     *
     * @return If successful, the converted endpoint. Otherwise, an uninitialized value.
     */
    static std::optional<EndpointType> hostname_to_endpoint(std::string_view hostname);

    /**
     * @return True if the socket handle is opened.
     */
    bool is_open() const;

    /**
     * @return This socket's native handle.
     */
    socket_type handle() const;

    /**
     * @return This socket's unique ID.
     */
    std::uint64_t socket_id() const;

    /**
     * Configure the socket to operate in the provided IO processing mode.
     *
     * @param mode IO processing mode to apply to the socket.
     *
     * @return True if the operation was successful.
     */
    bool set_io_mode(fly::net::IOMode mode);

    /**
     * @return This socket's IO processing mode.
     */
    fly::net::IOMode io_mode() const;

    /**
     * Retrieve the local endpoint to which this socket is bound.
     *
     * @return If successful, the bound endpoint. Otherwise, an uninitialized value.
     */
    std::optional<EndpointType> local_endpoint() const;

    /**
     * If this socket is valid, close this socket.
     */
    void close();

    /**
     * Bind this socket to a local endpoint.
     *
     * @param endpoint The local endpoint to bind to.
     * @param mode Binding mode to apply to the socket before binding.
     *
     * @return True if the binding was successful.
     */
    bool bind(const EndpointType &endpoint, BindMode mode) const;

    /**
     * Bind this socket to a local endpoint.
     *
     * @param hostname The hostname or IP address string to bind to.
     * @param port The port to bind to.
     * @param mode Binding mode to apply to the socket before binding.
     *
     * @return True if the binding was successful.
     */
    bool bind(std::string_view hostname, port_type port, BindMode mode) const;

protected:
    /**
     * Constructor. Initialize the socket in the provided IO processing mode.
     *
     * @param config Reference to network configuration.
     * @param handle Native socket handle opened by the concrete socket type.
     * @param mode IO processing mode to apply to the socket.
     */
    BaseSocket(
        std::shared_ptr<fly::net::NetworkConfig> config,
        socket_type handle,
        fly::net::IOMode mode) noexcept;

    /**
     * Constructor. Initialize the socket in an asynchronous IO processing mode armed with the
     * provided socket service for performing IO operations.
     *
     * @param service The socket service for performing IO operations.
     * @param config Reference to network configuration.
     * @param handle Native socket handle opened by the concrete socket type.
     */
    BaseSocket(
        const std::shared_ptr<fly::net::SocketService> &service,
        std::shared_ptr<fly::net::NetworkConfig> config,
        socket_type handle) noexcept;

    /**
     * Move constructor. The provided socket is left in an invalid state.
     *
     * @param socket The socket instance to move.
     */
    BaseSocket(BaseSocket &&socket) noexcept;

    /**
     * Destructor. If needed, close the socket.
     */
    virtual ~BaseSocket() noexcept;

    /**
     * Move assignment operator. The provided socket is left in an invalid state.
     *
     * @param socket The socket instance to move.
     *
     * @return A reference to this socket.
     */
    BaseSocket &operator=(BaseSocket &&socket) noexcept;

    /**
     * @return A strong (possibly null) pointer to the socket service.
     */
    std::shared_ptr<fly::net::SocketService> socket_service() const;

    /**
     * @return A strong pointer to the network configuration.
     */
    std::shared_ptr<fly::net::NetworkConfig> network_config() const;

    /**
     * @return Size of packet to use for IO operations.
     */
    std::size_t packet_size() const;

private:
    BaseSocket(const BaseSocket &) = delete;
    BaseSocket &operator=(const BaseSocket &) = delete;

    std::weak_ptr<fly::net::SocketService> m_weak_socket_service;

    std::shared_ptr<fly::net::NetworkConfig> m_config;

    socket_type m_socket_handle;
    std::uint64_t m_socket_id;
    fly::net::IOMode m_mode;
};

//==================================================================================================
template <typename EndpointType>
constexpr bool BaseSocket<EndpointType>::is_ipv4()
{
    return EndpointType::is_ipv4();
}

//==================================================================================================
template <typename EndpointType>
constexpr bool BaseSocket<EndpointType>::is_ipv6()
{
    return EndpointType::is_ipv6();
}

} // namespace fly::net::detail
