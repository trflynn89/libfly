#pragma once

#include "fly/types/numeric/endian.hpp"
#include "fly/types/string/lexer.hpp"

#include <array>
#include <compare>
#include <cstdint>
#include <limits>
#include <optional>
#include <ostream>
#include <string>

namespace fly::net {

/**
 * Class to store an IPv4 address in network order, and to provide convenient access to its data as
 * required by various network APIs.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
class IPv4Address
{
public:
    using address_type = std::array<std::uint8_t, 4>;
    using int_type = std::uint32_t;

    /**
     * Default constructor. Initializes the IPv4 address to 0.0.0.0.
     */
    IPv4Address() = default;

    /**
     * Constructor. Create an IPv4 address from a 4-part array of octets in decimal format. The
     * array should be ordered such that index 0 is the first octet and index 3 is the fourth octet.
     *
     * @param address The 4-part array of octets to initialize the IPv4 address from.
     */
    explicit constexpr IPv4Address(const address_type &address) noexcept;

    /**
     * Constructor. Create an IPv4 address from a network-order 32-bit value.
     *
     * @param address The network-order address to initialize the IPv4 address from.
     */
    explicit constexpr IPv4Address(int_type address) noexcept;

    IPv4Address(const IPv4Address &) = default;
    IPv4Address(IPv4Address &&) = default;

    IPv4Address &operator=(const IPv4Address &) = default;
    IPv4Address &operator=(IPv4Address &&) = default;

    /**
     * @return An IPv4 address representing INADDR_ANY.
     */
    static constexpr IPv4Address in_addr_any();

    /**
     * @return An IPv4 address representing INADDR_BROADCAST.
     */
    static constexpr IPv4Address in_addr_broadcast();

    /**
     * @return An IPv4 address representing INADDR_LOOPBACK.
     */
    static constexpr IPv4Address in_addr_loopback();

    /**
     * Construct an IPv4 address from a string in dot-decimal notation.
     *
     * The provided string must betwen one and four octets, inclusive. If the string contains less
     * than four octets, the last octet is treated as an integer of as many bytes as are required to
     * fill out the address to four octets. Thus, the string "127.65530" is converted to the IPv4
     * address 127.0.255.250.
     *
     * @param address The string in dot-decimal notation to initialize the IPv4 address from.
     *
     * @return If successful, the constructed IPv4 address. Otherwise, an uninitialized value.
     */
    static constexpr std::optional<IPv4Address> from_string(std::string_view address);

    /**
     * Convert the IPv4 address to a four octet string in dot-decimal notation.
     *
     * @return The IPv4 address in dot-decimal notation.
     */
    std::string to_string() const;

    /**
     * @return The IPv4 address as an integer in network order.
     */
    constexpr int_type network_order() const;

    /**
     * @return The IPv4 address as an integer in host order.
     */
    constexpr int_type host_order() const;

    /**
     * Three-way-comparison operator. Defaulted to perform the comparison on the network-order IPv4
     * address.
     */
    auto operator<=>(const IPv4Address &) const = default;

    /**
     * Stream an IPv4 address as a four octet string in dot-decimal notation.
     *
     * @param stream A reference to the output stream.
     * @param address The IPv4 address to stream.
     *
     * @return A reference to the output stream.
     */
    friend std::ostream &operator<<(std::ostream &stream, const IPv4Address &address);

private:
    int_type m_address {0};
};

//==================================================================================================
constexpr IPv4Address::IPv4Address(const address_type &address) noexcept
{
    m_address |= static_cast<int_type>(address[0]);
    m_address |= static_cast<int_type>(address[1] << 8);
    m_address |= static_cast<int_type>(address[2] << 16);
    m_address |= static_cast<int_type>(address[3] << 24);
}

//==================================================================================================
constexpr IPv4Address::IPv4Address(int_type address) noexcept : m_address(address)
{
}

//==================================================================================================
constexpr IPv4Address IPv4Address::in_addr_any()
{
    return IPv4Address(0x00'00'00'00);
}

//==================================================================================================
constexpr IPv4Address IPv4Address::in_addr_broadcast()
{
    return IPv4Address(0xff'ff'ff'ff);
}

//==================================================================================================
constexpr IPv4Address IPv4Address::in_addr_loopback()
{
    return IPv4Address(0x01'00'00'7f);
}

//==================================================================================================
constexpr std::optional<IPv4Address> IPv4Address::from_string(std::string_view address)
{
    constexpr const auto s_max32 = static_cast<std::uint64_t>(std::numeric_limits<int_type>::max());
    constexpr const auto s_decimal = '.';

    fly::Lexer lexer(std::move(address));

    std::array<int_type, 4> parts {};
    std::size_t index = 0;

    do
    {
        if (const auto segment = lexer.consume_number(); segment && (*segment <= s_max32))
        {
            parts[index++] = static_cast<int_type>(*segment);
        }
        else
        {
            return std::nullopt;
        }
    } while ((index < parts.size()) && lexer.consume_if(s_decimal));

    std::optional<int_type> host_address;

    if (index == 1)
    {
        host_address = parts[0];
    }
    else if (index == 2)
    {
        if ((parts[0] <= 0xff) && (parts[1] <= 0xff'ff'ff))
        {
            host_address = (parts[0] << 24) | parts[1];
        }
    }
    else if (index == 3)
    {
        if ((parts[0] <= 0xff) && (parts[1] <= 0xff) && (parts[2] <= 0xff'ff))
        {
            host_address = (parts[0] << 24) | (parts[1] << 16) | parts[2];
        }
    }
    else if (index == 4)
    {
        if ((parts[0] <= 0xff) && (parts[1] <= 0xff) && (parts[2] <= 0xff) && (parts[3] <= 0xff))
        {
            host_address = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
        }
    }

    if (!host_address || lexer.peek())
    {
        return std::nullopt;
    }

    return IPv4Address(fly::endian_swap_if_non_native<std::endian::big>(*host_address));
}

//==================================================================================================
constexpr auto IPv4Address::network_order() const -> int_type
{
    return m_address;
}

//==================================================================================================
constexpr auto IPv4Address::host_order() const -> int_type
{
    return fly::endian_swap_if_non_native<std::endian::big>(m_address);
}

} // namespace fly::net
