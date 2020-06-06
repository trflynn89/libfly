#pragma once

#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_exception.hpp"

namespace fly::detail {

/**
 * Helper class to parse escaped unicode character sequences in a
 * std::basic_string<>.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version June 6, 2020
 */
template <typename StringType>
class BasicStringUnicode
{
    using traits = detail::BasicStringTraits<StringType>;
    using char_type = typename traits::char_type;

    using codepoint_type = std::uint32_t;

    static constexpr codepoint_type high_surrogate_min = 0xd800;
    static constexpr codepoint_type high_surrogate_max = 0xdbff;

    static constexpr codepoint_type low_surrogate_min = 0xdc00;
    static constexpr codepoint_type low_surrogate_max = 0xdfff;

public:
    /**
     * Parse an escaped sequence of unicode characters. Accepts UTF-8 encodings
     * and UTF-16 paired surrogate encodings.
     *
     * Input sequences must be of the form: (\u[0-9a-fA-F]{4}){1,2}
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return The parsed unicode character.
     *
     * @throws UnicodeException If the interpreted unicode character is not
     *         valid or there weren't enough available bytes.
     */
    static StringType parse_character(
        typename StringType::const_iterator &it,
        const typename StringType::const_iterator &end) noexcept(false);

private:
    /**
     * Parse a single escaped unicode character. Convert the character to a
     * 32-bit codepoint.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return The parsed unicode codepoint.
     *
     * @throws UnicodeException If the interpreted unicode character is not
     *         valid or there weren't enough available bytes.
     */
    static codepoint_type parse_codepoint(
        typename StringType::const_iterator &it,
        const typename StringType::const_iterator &end) noexcept(false);

    /**
     * Convert a unicode codepoint to a unicode string.
     *
     * @param codepoint The codepoint to convert.
     *
     * @return The converted unicode character.
     */
    static StringType convert_codepoint(codepoint_type codepoint) noexcept;
};

//==============================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::parse_character(
    typename StringType::const_iterator &it,
    const typename StringType::const_iterator &end) noexcept(false)
{
    auto is_high_surrogate = [](codepoint_type c) -> bool {
        return (c >= high_surrogate_min) && (c <= high_surrogate_max);
    };
    auto is_low_surrogate = [](codepoint_type c) -> bool {
        return (c >= low_surrogate_min) && (c <= low_surrogate_max);
    };

    const codepoint_type high_surrogate = parse_codepoint(it, end);
    codepoint_type codepoint = high_surrogate;

    if (is_high_surrogate(high_surrogate))
    {
        const codepoint_type low_surrogate = parse_codepoint(it, end);

        if (is_low_surrogate(low_surrogate))
        {
            // The formula to convert a surrogate pair to a single codepoint is:
            //
            //     C = ((HS - 0xd800) * 0x400) + (LS - 0xdc00) + 0x10000
            //
            // Multiplying by 0x400 (1024) is the same as bit-shifting left by
            // 10 bits. The formula then simplifies to:
            codepoint = (high_surrogate << 10) + low_surrogate - 0x35fdc00;
        }
        else
        {
            throw UnicodeException(
                "Expected low surrogate to follow high surrogate %x, found %x",
                high_surrogate,
                low_surrogate);
        }
    }
    else if (is_low_surrogate(high_surrogate))
    {
        throw UnicodeException(
            "Expected high surrogate to preceed low surrogate %x",
            high_surrogate);
    }

    return convert_codepoint(codepoint);
}

//==============================================================================
template <typename StringType>
auto BasicStringUnicode<StringType>::parse_codepoint(
    typename StringType::const_iterator &it,
    const typename StringType::const_iterator &end) noexcept(false)
    -> codepoint_type
{
    if ((it == end) || (*it != '\\') || (++it == end) || (*it != 'u'))
    {
        throw UnicodeException("Expected codepoint to begin with \\u");
    }

    codepoint_type codepoint = 0;
    codepoint_type i = 0;
    ++it;

    for (i = 0; (i < 4) && (it != end); ++i, ++it)
    {
        const codepoint_type shift = (4 * (3 - i));

        if ((*it >= '0') && (*it <= '9'))
        {
            codepoint += static_cast<codepoint_type>((*it - 0x30) << shift);
        }
        else if ((*it >= 'A') && (*it <= 'F'))
        {
            codepoint += static_cast<codepoint_type>((*it - 0x37) << shift);
        }
        else if ((*it >= 'a') && (*it <= 'f'))
        {
            codepoint += static_cast<codepoint_type>((*it - 0x57) << shift);
        }
        else
        {
            throw UnicodeException(
                "Expected %x to be a hexadecimal digit",
                static_cast<codepoint_type>(*it));
        }
    }

    if (i != 4)
    {
        throw UnicodeException(
            "Expected exactly 4 hexadecimals after \\u, only found %u",
            i);
    }

    return codepoint;
}

//==============================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::convert_codepoint(
    codepoint_type codepoint) noexcept
{
    StringType result;

    if constexpr (sizeof(char_type) == 1)
    {
        if (codepoint < 0x80)
        {
            result += char_type(codepoint);
        }
        else if (codepoint < 0x800)
        {
            result += char_type(0xc0 | (codepoint >> 6));
            result += char_type(0x80 | (codepoint & 0x3f));
        }
        else if (codepoint < 0x10000)
        {
            result += char_type(0xe0 | (codepoint >> 12));
            result += char_type(0x80 | ((codepoint >> 6) & 0x3f));
            result += char_type(0x80 | (codepoint & 0x3f));
        }
        else
        {
            result += char_type(0xf0 | (codepoint >> 18));
            result += char_type(0x80 | ((codepoint >> 12) & 0x3f));
            result += char_type(0x80 | ((codepoint >> 6) & 0x3f));
            result += char_type(0x80 | (codepoint & 0x3f));
        }
    }
    else if constexpr (sizeof(char_type) == 2)
    {
        if (codepoint < 0x10000)
        {
            result += char_type(codepoint);
        }
        else
        {
            codepoint -= 0x10000;
            result += char_type(high_surrogate_min | (codepoint >> 10));
            result += char_type(low_surrogate_min | (codepoint & 0x3ff));
        }
    }
    else if constexpr (sizeof(char_type) == 4)
    {
        result += char_type(codepoint);
    }

    return result;
}

} // namespace fly::detail
