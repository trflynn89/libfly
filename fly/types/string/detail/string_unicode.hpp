#pragma once

#include "fly/types/string/detail/string_formatter.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_exception.hpp"
#include "fly/types/string/string_literal.hpp"

#include <string>

namespace fly::detail {

/**
 * Helper class to parse escaped unicode character sequences in a std::basic_string<>.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version June 6, 2020
 */
template <typename StringType>
class BasicStringUnicode
{
    using traits = detail::BasicStringTraits<StringType>;
    using char_type = typename traits::char_type;

    using StringFormatter = BasicStringFormatter<std::string>;

    using codepoint_type = std::uint32_t;

    static constexpr codepoint_type high_surrogate_min = 0xd800;
    static constexpr codepoint_type high_surrogate_max = 0xdbff;

    static constexpr codepoint_type low_surrogate_min = 0xdc00;
    static constexpr codepoint_type low_surrogate_max = 0xdfff;

public:
    /**
     * Parse an escaped sequence of unicode characters. Accepts the following unicode encodings:
     *
     *     UTF-8 encodings of the form: \unnnn
     *     UTF-16 paried surrogate encodings of the form: \unnnn\unnnn
     *     UTF-32 encodings of the form: \Unnnnnnnn
     *
     * Where each character n is a hexadecimal digit.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return The parsed unicode character.
     *
     * @throws UnicodeException If the escaped sequence is not a valid unicode character.
     */
    static StringType parse_character(
        typename StringType::const_iterator &it,
        const typename StringType::const_iterator &end) noexcept(false);

private:
    /**
     * Parse an escaped sequence of unicode characters. Accepts UTF-8 encodings and UTF-16 paired
     * surrogate encodings.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return The parsed unicode character.
     *
     * @throws UnicodeException If the escaped sequence is not a valid unicode character.
     */
    static StringType parse_utf8_or_utf16(
        typename StringType::const_iterator &it,
        const typename StringType::const_iterator &end) noexcept(false);

    /**
     * Parse an escaped sequence of unicode characters. Accepts UTF-32 encodings.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return The parsed unicode character.
     *
     * @throws UnicodeException If the escaped sequence is not a valid unicode character.
     */
    static StringType parse_utf32(
        typename StringType::const_iterator &it,
        const typename StringType::const_iterator &end) noexcept(false);

    /**
     * Parse a single escaped unicode character. Convert the character to a 32-bit codepoint.
     *
     * Note: the template type is StringType::value_type rather than char_type because of an MSVC
     * bug. MSVC fails trying to match the following declaration/definition pair:
     *
     *    template <typename StringType>
     *    class BasicStringUnicode
     *    {
     *        using char_type = typename traits::char_type;
     *
     *        template <char_type UnicodePrefix>
     *        static codepoint_type parse_codepoint();
     *    };
     *
     *    template <typename StringType>
     *    template <typename BasicStringUnicode<StringType>::char_type UnicodePrefix>
     *    auto BasicStringUnicode<StringType>::parse_codepoint() {}
     *
     * See:
     * https://stackoverflow.com/questions/49521073/msvcerror-c2244-unable-to-match-function-definition-to-an-existing-declaratio
     * https://developercommunity.visualstudio.com/content/problem/225941/error-c2244-unable-to-match-function-definition-to.html
     *
     * @tparam UnicodePrefix The escaped unicode prefix character ('u' or 'U').
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return The parsed unicode codepoint.
     *
     * @throws UnicodeException If the escaped sequence is not a valid unicode character.
     */
    template <typename StringType::value_type UnicodePrefix>
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

    static constexpr char_type utf8 = FLY_CHR(char_type, 'u');
    static constexpr char_type utf32 = FLY_CHR(char_type, 'U');

    static constexpr char_type ch_0 = FLY_CHR(char_type, '0');
    static constexpr char_type ch_9 = FLY_CHR(char_type, '9');
    static constexpr char_type ch_a = FLY_CHR(char_type, 'a');
    static constexpr char_type ch_A = FLY_CHR(char_type, 'A');
    static constexpr char_type ch_f = FLY_CHR(char_type, 'f');
    static constexpr char_type ch_F = FLY_CHR(char_type, 'F');
};

//==================================================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::parse_character(
    typename StringType::const_iterator &it,
    const typename StringType::const_iterator &end) noexcept(false)
{
    auto begins_with = [&it, &end](const char_type ch) -> bool {
        if ((it == end) || ((it + 1) == end))
        {
            return false;
        }

        return (*it == '\\') && (*(it + 1) == ch);
    };

    if (begins_with(utf8))
    {
        return parse_utf8_or_utf16(it, end);
    }
    else if (begins_with(utf32))
    {
        return parse_utf32(it, end);
    }

    throw UnicodeException(StringFormatter::format(
        "Escaped unicode must begin with \\%c or \\%c",
        static_cast<char>(utf8),
        static_cast<char>(utf32)));
}

//==================================================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::parse_utf8_or_utf16(
    typename StringType::const_iterator &it,
    const typename StringType::const_iterator &end) noexcept(false)
{
    auto is_high_surrogate = [](codepoint_type c) -> bool {
        return (c >= high_surrogate_min) && (c <= high_surrogate_max);
    };
    auto is_low_surrogate = [](codepoint_type c) -> bool {
        return (c >= low_surrogate_min) && (c <= low_surrogate_max);
    };

    const codepoint_type high_surrogate = parse_codepoint<utf8>(it, end);
    codepoint_type codepoint = high_surrogate;

    if (is_high_surrogate(high_surrogate))
    {
        const codepoint_type low_surrogate = parse_codepoint<utf8>(it, end);

        if (is_low_surrogate(low_surrogate))
        {
            // The formula to convert a surrogate pair to a single codepoint is:
            //
            //     C = ((HS - 0xd800) * 0x400) + (LS - 0xdc00) + 0x10000
            //
            // Multiplying by 0x400 is the same as left-shifting 10 bits. The formula then becomes:
            codepoint = (high_surrogate << 10) + low_surrogate - 0x35fdc00;
        }
        else
        {
            throw UnicodeException(StringFormatter::format(
                "Expected low surrogate to follow high surrogate %x, found %x",
                high_surrogate,
                low_surrogate));
        }
    }
    else if (is_low_surrogate(high_surrogate))
    {
        throw UnicodeException(StringFormatter::format(
            "Expected high surrogate to preceed low surrogate %x",
            high_surrogate));
    }

    return convert_codepoint(codepoint);
}

//==================================================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::parse_utf32(
    typename StringType::const_iterator &it,
    const typename StringType::const_iterator &end) noexcept(false)
{
    const codepoint_type codepoint = parse_codepoint<utf32>(it, end);
    return convert_codepoint(codepoint);
}

//==================================================================================================
template <typename StringType>
template <typename StringType::value_type UnicodePrefix>
auto BasicStringUnicode<StringType>::parse_codepoint(
    typename StringType::const_iterator &it,
    const typename StringType::const_iterator &end) noexcept(false) -> codepoint_type
{
    static_assert((UnicodePrefix == utf8) || (UnicodePrefix == utf32));

    if ((it == end) || (*it != '\\') || (++it == end) || (*it != UnicodePrefix))
    {
        throw UnicodeException(StringFormatter::format(
            "Expected codepoint to begin with \\%c",
            static_cast<char>(UnicodePrefix)));
    }

    codepoint_type codepoint = 0;
    ++it;

    static constexpr const codepoint_type expected_digits = (UnicodePrefix == utf8) ? 4 : 8;
    codepoint_type i = 0;

    for (i = 0; (i < expected_digits) && (it != end); ++i, ++it)
    {
        const codepoint_type shift = (4 * (expected_digits - i - 1));

        if ((*it >= ch_0) && (*it <= ch_9))
        {
            codepoint += static_cast<codepoint_type>(*it - 0x30) << shift;
        }
        else if ((*it >= ch_A) && (*it <= ch_F))
        {
            codepoint += static_cast<codepoint_type>(*it - 0x37) << shift;
        }
        else if ((*it >= ch_a) && (*it <= ch_f))
        {
            codepoint += static_cast<codepoint_type>(*it - 0x57) << shift;
        }
        else
        {
            throw UnicodeException(StringFormatter::format(
                "Expected %x to be a hexadecimal digit",
                static_cast<codepoint_type>(*it)));
        }
    }

    if (i != expected_digits)
    {
        throw UnicodeException(StringFormatter::format(
            "Expected exactly %u hexadecimals after \\%c, only found %u",
            expected_digits,
            static_cast<char>(UnicodePrefix),
            i));
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::convert_codepoint(codepoint_type codepoint) noexcept
{
    StringType result;

    static_assert((sizeof(char_type) == 1) || (sizeof(char_type) == 2) || (sizeof(char_type) == 4));

    if constexpr (sizeof(char_type) == 1)
    {
        if (codepoint < 0x80)
        {
            result += static_cast<char_type>(codepoint);
        }
        else if (codepoint < 0x800)
        {
            result += static_cast<char_type>(0xc0 | (codepoint >> 6));
            result += static_cast<char_type>(0x80 | (codepoint & 0x3f));
        }
        else if (codepoint < 0x10000)
        {
            result += static_cast<char_type>(0xe0 | (codepoint >> 12));
            result += static_cast<char_type>(0x80 | ((codepoint >> 6) & 0x3f));
            result += static_cast<char_type>(0x80 | (codepoint & 0x3f));
        }
        else
        {
            result += static_cast<char_type>(0xf0 | (codepoint >> 18));
            result += static_cast<char_type>(0x80 | ((codepoint >> 12) & 0x3f));
            result += static_cast<char_type>(0x80 | ((codepoint >> 6) & 0x3f));
            result += static_cast<char_type>(0x80 | (codepoint & 0x3f));
        }
    }
    else if constexpr (sizeof(char_type) == 2)
    {
        if (codepoint < 0x10000)
        {
            result += static_cast<char_type>(codepoint);
        }
        else
        {
            codepoint -= 0x10000;
            result += static_cast<char_type>(high_surrogate_min | (codepoint >> 10));
            result += static_cast<char_type>(low_surrogate_min | (codepoint & 0x3ff));
        }
    }
    else
    {
        result += static_cast<char_type>(codepoint);
    }

    return result;
}

} // namespace fly::detail
