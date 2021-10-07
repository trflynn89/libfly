#pragma once

#include "fly/concepts/concepts.hpp"

#include <memory>

namespace fly::net {

class IPv4Address;
class IPv6Address;

/**
 * Concept that is satisfied if the provided type is a valid IPv4 or IPv6 address type.
 */
template <typename T>
concept IPAddress = fly::SameAsAny<T, IPv4Address, IPv6Address>;

template <IPAddress IPAddressType>
class Endpoint;

/**
 * Concept that is satisfied if the provided type is a valid IPv4 or IPv6 endpoint type.
 */
template <typename T>
concept IPEndpoint = fly::SameAsAny<T, Endpoint<IPv4Address>, Endpoint<IPv6Address>>;

template <IPEndpoint EndpointType>
class ListenSocket;

template <IPEndpoint EndpointType>
class TcpSocket;

template <IPEndpoint EndpointType>
class UdpSocket;

/**
 * Concept that is satisfied if the provided type is a valid IPv4 or IPv6 socket type.
 */
template <typename T>
concept Socket = fly::SameAsAny<
    T,
    ListenSocket<Endpoint<IPv4Address>>,
    ListenSocket<Endpoint<IPv6Address>>,
    TcpSocket<Endpoint<IPv4Address>>,
    TcpSocket<Endpoint<IPv6Address>>,
    UdpSocket<Endpoint<IPv4Address>>,
    UdpSocket<Endpoint<IPv6Address>>>;

/**
 * Concept that is satisfied if the provided type is a callback which accepts a strong pointer to an
 * IPv4 or IPv6 socket type.
 */
template <typename T, typename SocketType>
concept SocketNotification = requires(T callback)
{
    requires Socket<SocketType>;
    callback(std::declval<std::shared_ptr<SocketType>>());
};

} // namespace fly::net
