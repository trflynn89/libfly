#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/lexer.hpp"
#include "fly/types/string/string.hpp"

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>

namespace fly::net {

/**
 * Class to store an IPv6 address in an array, and to provide convenient access to its data as
 * required by various network APIs.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 13, 2021
 */
class IPv6Address
{
public:
    using address_type = std::array<std::uint8_t, 16>;

    /**
     * Default constructor. Initializes the IPv6 address to ::.
     */
    IPv6Address() = default;

    /**
     * Constructor. Create an IPv6 address from a 16-part array of octets. The array should be
     * ordered such that index 0 is the first octet and index 15 is the sixteenth octet.
     *
     * @param address The 16-part array of octets to initialize the IPv6 address from.
     */
    explicit constexpr IPv6Address(const address_type &address) noexcept;

    /**
     * Constructor. Create an IPv6 address from a 16-part array of octets. The array should be
     * ordered such that index 0 is the first octet and index 15 is the sixteenth octet.
     *
     * @param address The 16-part array of octets to initialize the IPv6 address from.
     */
    explicit constexpr IPv6Address(address_type &&address) noexcept;

    /**
     * Constructor. Create an IPv6 address from a 16-part array of octets. The array should be
     * ordered such that index 0 is the first octet and index 15 is the sixteenth octet.
     *
     * @param address The 16-part array of octets to initialize the IPv6 address from.
     */
    explicit constexpr IPv6Address(const address_type::value_type (&address)[16]) noexcept;

    IPv6Address(const IPv6Address &) = default;
    IPv6Address(IPv6Address &&) = default;

    IPv6Address &operator=(const IPv6Address &) = default;
    IPv6Address &operator=(IPv6Address &&) = default;

    /**
     * @return An IPv6 address representing IN6ADDR_ANY.
     */
    static constexpr IPv6Address in_addr_any();

    /**
     * @return An IPv6 address representing IN6ADDR_LOOPBACK.
     */
    static constexpr IPv6Address in_addr_loopback();

    /**
     * Construct an IPv6 address from a string of hexadectets.
     *
     * The provided string must be fully formed or use shorthand form. In shorthand form, each
     * hexadectet may have leading zeros removed (that is, the hexadectet "001a" may be provided as
     * just "1a"). Further, consecutive hexadectets of zeros may be replaced with two colons (that
     * is, the hexadectets "1:0:0:1" may be provided as "1::1"); this replacement may only be used
     * once in the string.
     *
     * @param address The string to initialize the IPv6 address from.
     *
     * @return If successful, the constructed IPv6 address. Otherwise, an uninitialized value.
     */
    static constexpr std::optional<IPv6Address> from_string(std::string_view address);

    /**
     * Copy the IPv6 address into a 16-part array.
     *
     * @param address The 16-part array of octets to copy the IPv6 address into.
     */
    constexpr void copy(address_type::value_type (&address)[16]) const;

#if defined(FLY_MACOS)
    /**
     * Comparison operators. Apple's Clang does not fully support the three-way comparison operator,
     * so these must be manually defined.
     */
    constexpr bool operator==(const IPv6Address &address) const;
    constexpr bool operator!=(const IPv6Address &address) const;
    constexpr bool operator<(const IPv6Address &address) const;
    constexpr bool operator<=(const IPv6Address &address) const;
    constexpr bool operator>(const IPv6Address &address) const;
    constexpr bool operator>=(const IPv6Address &address) const;
#else
    /**
     * Three-way-comparison operator. Defaulted to perform the comparison on the IPv6 array data.
     */
    auto operator<=>(const IPv6Address &) const = default;
#endif

private:
    static constexpr std::size_t s_address_size = std::tuple_size_v<address_type>;
    address_type m_address {};
};

//==================================================================================================
constexpr IPv6Address::IPv6Address(const address_type &address) noexcept : m_address {address}
{
}

//==================================================================================================
constexpr IPv6Address::IPv6Address(address_type &&address) noexcept : m_address {std::move(address)}
{
}

//==================================================================================================
constexpr IPv6Address::IPv6Address(const address_type::value_type (&address)[16]) noexcept
{
    std::copy(std::begin(address), std::end(address), m_address.begin());
}

//==================================================================================================
constexpr IPv6Address IPv6Address::in_addr_any()
{
    address_type address {};
    return IPv6Address(std::move(address));
}

//==================================================================================================
constexpr IPv6Address IPv6Address::in_addr_loopback()
{
    address_type address {};
    address.back() = 0x01;

    return IPv6Address(std::move(address));
}

//==================================================================================================
constexpr std::optional<IPv6Address> IPv6Address::from_string(std::string_view address)
{
    constexpr const auto s_max16 =
        static_cast<std::uint64_t>(std::numeric_limits<std::uint16_t>::max());
    constexpr const auto s_colon = ':';

    fly::Lexer lexer(std::move(address));
    address_type parts {};

    std::optional<std::size_t> index_after_short_form;
    std::size_t index = 0;

    do
    {
        if (lexer.consume_if(s_colon))
        {
            if (lexer.consume_if(s_colon))
            {
                if (index_after_short_form)
                {
                    return std::nullopt;
                }

                index_after_short_form = index;
            }
        }
        else if (const auto segment = lexer.consume_hex_number(); segment && (*segment <= s_max16))
        {
            parts[index++] = static_cast<address_type::value_type>((*segment >> 8) & 0xff);
            parts[index++] = static_cast<address_type::value_type>(*segment & 0xff);
        }
        else
        {
            return std::nullopt;
        }
    } while (lexer.peek() && (index < s_address_size));

    if (lexer.peek() || ((index != s_address_size) && !index_after_short_form))
    {
        return std::nullopt;
    }
    else if (index_after_short_form)
    {
        const auto start = static_cast<std::ptrdiff_t>(s_address_size - index);
        const auto end = static_cast<std::ptrdiff_t>(*index_after_short_form);

        std::rotate(parts.rbegin(), parts.rbegin() + start, parts.rend() - end);
    }

    return IPv6Address(std::move(parts));
}

//==================================================================================================
constexpr void IPv6Address::copy(address_type::value_type (&address)[16]) const
{
    std::copy(m_address.begin(), m_address.end(), std::begin(address));
}

#if defined(FLY_MACOS)

//==================================================================================================
constexpr bool IPv6Address::operator==(const IPv6Address &address) const
{
    return m_address == address.m_address;
}

//==================================================================================================
constexpr bool IPv6Address::operator!=(const IPv6Address &address) const
{
    return m_address != address.m_address;
}

//==================================================================================================
constexpr bool IPv6Address::operator<(const IPv6Address &address) const
{
    return m_address < address.m_address;
}

//==================================================================================================
constexpr bool IPv6Address::operator<=(const IPv6Address &address) const
{
    return m_address <= address.m_address;
}

//==================================================================================================
constexpr bool IPv6Address::operator>(const IPv6Address &address) const
{
    return m_address > address.m_address;
}

//==================================================================================================
constexpr bool IPv6Address::operator>=(const IPv6Address &address) const
{
    return m_address >= address.m_address;
}

#endif

} // namespace fly::net

//==================================================================================================
template <>
struct fly::Formatter<fly::net::IPv6Address>
{
    /**
     * Format an IPv6 address as a string in shorthand form.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param address The IPv6 address to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(const fly::net::IPv6Address &address, FormatContext &context)
    {
        static constexpr std::size_t s_address_size =
            std::tuple_size_v<fly::net::IPv6Address::address_type>;

        fly::net::IPv6Address::address_type::value_type parts[s_address_size] {};
        address.copy(parts);

        auto join_segments = [&parts](std::size_t index) -> std::uint16_t
        {
            return (parts[index] << 8) | parts[index + 1];
        };

        bool used_short_form = false;

        for (std::size_t i = 0; i < s_address_size;)
        {
            const std::uint16_t segment = join_segments(i);

            if ((segment == 0) && !used_short_form)
            {
                do
                {
                    i += 2;
                } while ((i < s_address_size) && (join_segments(i) == 0));

                fly::String::format_to(context.out(), "{}", (i < s_address_size) ? ":" : "::");
                used_short_form = true;
            }
            else
            {
                fly::String::format_to(context.out(), "{:.{}}{:x}", ":", (i > 0) ? 1 : 0, segment);
                i += 2;
            }
        }
    }
};
