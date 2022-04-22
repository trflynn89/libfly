#pragma once

#include "fly/fly.hpp"
#include "fly/net/ipv4_address.hpp"
#include "fly/net/ipv6_address.hpp"
#include "fly/net/socket/concepts.hpp"
#include "fly/net/socket/types.hpp"
#include "fly/types/string/lexer.hpp"
#include "fly/types/string/string.hpp"

#include <compare>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>

namespace fly::net {

/**
 * Class to store a version-independent IP address and port, and to provide convenient access to its
 * data as required by various network APIs.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
template <IPAddress IPAddressType>
class Endpoint
{
public:
    using address_type = IPAddressType;

    /**
     * Default constructor.
     */
    Endpoint() = default;

    /**
     * Constructor. Create an endpoint from an IP address and port.
     *
     * @param address The IP address to initialize the endpoint with.
     * @param port The port to initialize the endpoint with.
     */
    constexpr Endpoint(IPAddressType const &address, port_type port) noexcept;

    /**
     * Constructor. Create an endpoint from an IP address and port.
     *
     * @param address The IP address to initialize the endpoint with.
     * @param port The port to initialize the endpoint with.
     */
    constexpr Endpoint(IPAddressType &&address, port_type port) noexcept;

    Endpoint(Endpoint const &) = default;
    Endpoint(Endpoint &&) = default;

    Endpoint &operator=(Endpoint const &) = default;
    Endpoint &operator=(Endpoint &&) = default;

    /**
     * @return True if this is an IPv4 endpoint.
     */
    static constexpr bool is_ipv4();

    /**
     * @return True if this is an IPv6 endpoint.
     */
    static constexpr bool is_ipv6();

    /**
     * Construct an endpoint from a string containing an IP address and a port.
     *
     * The provided string should begin with the IP address and end with the port, separated by a
     * single colon. IPv6 addressses should be surrounded by square brackets (e.g. "[::1]:80");
     *
     * @param endpoint The string to initialize the endpoint from.
     *
     * @return If successful, the constructed endpoint. Otherwise, an uninitialized value.
     */
    static constexpr std::optional<Endpoint> from_string(std::string_view endpoint);

    /**
     * @return The endoint's IP address.
     */
    constexpr IPAddressType const &address() const;

    /**
     * @return The endoint's IP port.
     */
    constexpr port_type port() const;

#if defined(FLY_MACOS)
    /**
     * Comparison operators. Apple's Clang does not fully support the three-way comparison operator,
     * so these must be manually defined.
     */
    constexpr bool operator==(Endpoint const &endpoint) const;
    constexpr bool operator!=(Endpoint const &endpoint) const;
    constexpr bool operator<(Endpoint const &endpoint) const;
    constexpr bool operator<=(Endpoint const &endpoint) const;
    constexpr bool operator>(Endpoint const &endpoint) const;
    constexpr bool operator>=(Endpoint const &endpoint) const;
#else
    /**
     * Three-way-comparison operator. Defaulted to perform the comparison on the underlying IP
     * address and port.
     */
    auto operator<=>(Endpoint const &) const = default;
#endif

private:
    IPAddressType m_address {};
    port_type m_port {0};
};

//==================================================================================================
template <IPAddress IPAddressType>
constexpr Endpoint<IPAddressType>::Endpoint(IPAddressType const &address, port_type port) noexcept :
    m_address(address),
    m_port(port)
{
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr Endpoint<IPAddressType>::Endpoint(IPAddressType &&address, port_type port) noexcept :
    m_address(address),
    m_port(port)
{
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::is_ipv4()
{
    return std::is_same_v<IPAddressType, IPv4Address>;
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::is_ipv6()
{
    return std::is_same_v<IPAddressType, IPv6Address>;
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr std::optional<Endpoint<IPAddressType>>
Endpoint<IPAddressType>::from_string(std::string_view endpoint)
{
    constexpr auto const s_max16 =
        static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max());
    constexpr auto const s_colon = ':';

    std::size_t const separator = endpoint.find_last_of(s_colon);

    if ((separator == std::string_view::npos) || (separator == 0) || (separator == endpoint.size()))
    {
        return std::nullopt;
    }

    auto address_view = endpoint.substr(0, separator);
    auto port_view = endpoint.substr(separator + 1);

    if constexpr (is_ipv6())
    {
        constexpr auto const s_left_bracket = '[';
        constexpr auto const s_right_bracket = ']';

        if (address_view.starts_with(s_left_bracket) && address_view.ends_with(s_right_bracket))
        {
            address_view = address_view.substr(1, address_view.size() - 2);
        }
        else
        {
            return std::nullopt;
        }
    }

    fly::Lexer lexer(std::move(port_view));

    auto address = IPAddressType::from_string(std::move(address_view));
    auto port = lexer.consume_number();

    if (!address || !port || (*port > s_max16) || lexer.peek())
    {
        return std::nullopt;
    }

    return Endpoint(*std::move(address), static_cast<port_type>(*port));
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr IPAddressType const &Endpoint<IPAddressType>::address() const
{
    return m_address;
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr port_type Endpoint<IPAddressType>::port() const
{
    return m_port;
}

#if defined(FLY_MACOS)

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::operator==(Endpoint<IPAddressType> const &endpoint) const
{
    return (m_address == endpoint.m_address) && (m_port == endpoint.m_port);
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::operator!=(Endpoint<IPAddressType> const &endpoint) const
{
    return !(*this == endpoint);
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::operator<(Endpoint<IPAddressType> const &endpoint) const
{
    if (m_address < endpoint.m_address)
    {
        return true;
    }

    return m_port < endpoint.m_port;
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::operator<=(Endpoint<IPAddressType> const &endpoint) const
{
    return !(endpoint < *this);
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::operator>(Endpoint<IPAddressType> const &endpoint) const
{
    return !(*this <= endpoint);
}

//==================================================================================================
template <IPAddress IPAddressType>
constexpr bool Endpoint<IPAddressType>::operator>=(Endpoint<IPAddressType> const &endpoint) const
{
    return !(*this < endpoint);
}

#endif

} // namespace fly::net

//==================================================================================================
template <>
struct fly::Formatter<fly::net::Endpoint<fly::net::IPv4Address>>
{
    /**
     * Format an IPv4 endpoint.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param endpoint The IPv4 endpoint to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(fly::net::Endpoint<fly::net::IPv4Address> const &endpoint, FormatContext &context)
    {
        fly::String::format_to(context.out(), "{}:{}", endpoint.address(), endpoint.port());
    }
};

//==================================================================================================
template <>
struct fly::Formatter<fly::net::Endpoint<fly::net::IPv6Address>>
{
    /**
     * Format an IPv6 endpoint.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param endpoint The IPv6 endpoint to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(fly::net::Endpoint<fly::net::IPv6Address> const &endpoint, FormatContext &context)
    {
        fly::String::format_to(context.out(), "[{}]:{}", endpoint.address(), endpoint.port());
    }
};
