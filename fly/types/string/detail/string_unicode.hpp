#pragma once

#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <array>
#include <functional>
#include <optional>
#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Helper class for decoding and encoding Unicode codepoints in a std::basic_string. The assumed
 * Unicode encoding depends on the template type StringType:
 *
 *     1. std::string - UTF-8
 *     2. std::wstring - UTF-16 on Windows, UTF-32 on Linux and macOS
 *     3. std::u8string - UTF-8
 *     4. std::u16string - UTF-16
 *     5. std::u32string - UTF-32
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version June 6, 2020
 */
template <typename StringType>
class BasicStringUnicode
{
    using traits = detail::BasicStringTraits<StringType>;
    using char_type = typename traits::char_type;
    using const_iterator = typename traits::const_iterator;
    using codepoint_type = typename traits::codepoint_type;

public:
    /**
     * Convert the Unicode encoding of a string to another encoding.
     *
     * @tparam DesiredStringType The type of string to convert to.
     *
     * @param it Pointer to the beginning of the encoded Unicode string.
     * @param end Pointer to the end of the encoded Unicode string.
     *
     * @return If successful, a copy of the source string with the desired encoding. Otherwise, an
     *         unitialized value.
     */
    template <typename DesiredStringType>
    static std::optional<DesiredStringType>
    convert_encoding(const_iterator &it, const const_iterator &end);

    /**
     * Decode a single Unicode codepoint, starting at the character pointed to by the provided
     * iterator. If successful, after invoking this method, that iterator will point at the first
     * character after the Unicode codepoint in the source string.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an unitialized value.
     */
    static std::optional<codepoint_type>
    decode_codepoint(const_iterator &it, const const_iterator &end);

    /**
     * Encode a single Unicode codepoint.
     *
     * @return The Unicode codepoint to encode.
     *
     * @return If successful, a string containing the encoded Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    static std::optional<StringType> encode_codepoint(codepoint_type codepoint);

    /**
     * Escape a single Unicode codepoint, starting at the character pointed to by the provided
     * iterator. If successful, after invoking this method, that iterator will point at the first
     * character after the Unicode codepoint in the source string.
     *
     * If the Unicode codepoint is an ASCII, non-control character (i.e. codepoints in the range
     * [U+0020, U+007E]), that character is not escaped.
     *
     * If the Unicode codepoint is non-ASCII or a control character (i.e. codepoints in the range
     * [U+0000, U+001F] or [U+007F, U+10FFFF]), the codepoint is encoded as follows, taking into
     * consideration the provided Unicode prefix character:
     *
     *     1. If the Unicode codepoint is in the range [U+0000, U+001F] or [U+007F, U+FFFF],
     *        regardless of the prefix character, the encoding will be of the form \unnnn.
     *     2. If the codepoint is in the range [U+10000, U+10FFFF], and the prefix character is 'u',
     *        the encoding will be a surrogate pair of the form \unnnn\unnnn.
     *     3. If the codepoint is in the range [U+10000, U+10FFFF], and the prefix character is 'U',
     *        the encoding will of the form \Unnnnnnnn.
     *
     * @tparam UnicodePrefix The Unicode prefix character ('u' or 'U').
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, a string containing the escaped Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    template <char UnicodePrefix = 'U'>
    static std::optional<StringType>
    escape_codepoint(const_iterator &it, const const_iterator &end);

    /**
     * Unescape a single Unicode codepoint, starting at the character pointed to by provided
     * iterator. If successful, after invoking this method, that iterator will point at the first
     * character after the escaped sequence in the source string.
     *
     * Accepts escaped sequences of the following forms:
     *
     *     1. \unnnn for Unicode codepoints in the range [U+0000, U+FFFF].
     *     2. \unnnn\unnnn surrogate pairs for Unicode codepoints in the range [U+10000, U+10FFFF].
     *     3. \Unnnnnnnn for all Unicode codepoints.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return If successful, a string containing the unescaped Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    static std::optional<StringType>
    unescape_codepoint(const_iterator &it, const const_iterator &end);

private:
    using EncodedByteProvider = std::function<std::optional<codepoint_type>()>;

    /**
     * Escape a single Unicode codepoint.
     *
     * @tparam UnicodePrefix The escaped Unicode prefix character ('u' or 'U').
     *
     * @param codepoint The codepoint to escape.
     *
     * @return If successful, a string containing the escaped Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    template <char UnicodePrefix>
    static StringType escape_codepoint(codepoint_type codepoint);

    /**
     * Unescape a sequence of characters to form a single Unicode codepoint.
     *
     * @tparam UnicodePrefix The escaped Unicode prefix character ('u' or 'U').
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return If successful, a string containing the parsed Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    template <char UnicodePrefix>
    static std::optional<codepoint_type>
    unescape_codepoint(const_iterator &it, const const_iterator &end);

    /**
     * Decode a Unicode codepoint from a UTF-8 string.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an unitialized value.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 1), bool> = 0>
    static std::optional<codepoint_type>
    codepoint_from_string(EncodedByteProvider next_encoded_byte);

    /**
     * Decode a Unicode codepoint from a UTF-16 string.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return The decoded Unicode codepoint.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 2), bool> = 0>
    static std::optional<codepoint_type>
    codepoint_from_string(EncodedByteProvider next_encoded_byte);

    /**
     * Decode a Unicode codepoint from a UTF-32 string.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an unitialized value.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 4), bool> = 0>
    static std::optional<codepoint_type>
    codepoint_from_string(EncodedByteProvider next_encoded_byte);

    /**
     * Encode a Unicode codepoint into a UTF-8 string.
     *
     * @param codepoint The codepoint to encode.
     *
     * @return If successful, a string containing the encoded Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 1), bool> = 0>
    static StringType codepoint_to_string(codepoint_type codepoint);

    /**
     * Encode a Unicode codepoint into a UTF-16 string.
     *
     * @param codepoint The codepoint to encode.
     *
     * @return If successful, a string containing the encoded Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 2), bool> = 0>
    static StringType codepoint_to_string(codepoint_type codepoint);

    /**
     * Encode a Unicode codepoint into a UTF-32 string.
     *
     * @param codepoint The codepoint to encode.
     *
     * @return If successful, a string containing the encoded Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 4), bool> = 0>
    static StringType codepoint_to_string(codepoint_type codepoint);

    /**
     * Create a Unicode codepoint from either one complete codepoint or two surrogate halves. The
     * surrogate provider is invoked at least once to retrieve the candidate complete codepoint from
     * the caller. If that codepoint falls in the high surrogate range, the callback is invoked
     * again to retrieve the low surrogate; those two surrogate halves are then combined to form a
     * complete codepoint.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return If successful, the created Unicode codepoint. Otherwise, an unitialized value.
     */
    static std::optional<codepoint_type>
    create_codepoint_from_surrogates(EncodedByteProvider next_encoded_byte);

    /**
     * Validate a Unicode codepoint is not out-of-range or reserved by the Unicode Standard.
     *
     * @param codepoint The codepoint to validate.
     *
     * @return True if the codepoint is valid.
     */
    static bool validate_codepoint(codepoint_type codepoint);

    /**
     * Structure to hold static data required for decoding UTF-8 encoded Unicode codepoints.
     */
    struct Utf8Data
    {
        // The value of the UTF-8 encoded leading byte.
        const codepoint_type m_leading_byte;

        // A bit-mask of the bits in the UTF-8 encoded leading byte reserved for encoding.
        const codepoint_type m_encoding_mask;

        // A bit-mask of the bits in the UTF-8 encoded leading byte reserved for codepoint data.
        const codepoint_type m_codepoint_mask;

        // The number of bytes required to decode the codepoint.
        const codepoint_type m_codepoint_size;
    };

    static constexpr const std::array<Utf8Data, 4> s_utf8_leading_bytes = {{
        // Codepoint length 1, range [U+0000, U+007f], leading byte 0b0xxx'xxxx.
        {0b0000'0000, 0b1000'0000, 0b0111'1111, 1},

        // Codepoint length 2, range [U+0080, U+07FF], leading byte 0b110x'xxxx.
        {0b1100'0000, 0b1110'0000, 0b0001'1111, 2},

        // Codepoint length 3, range [U+0800, U+FFFF], leading byte 0b1110'xxxx.
        {0b1110'0000, 0b1111'0000, 0b0000'1111, 3},

        // Codepoint length 4, range [U+10000, U+10FFFF], leading byte 0b1111'0xxx.
        {0b1111'0000, 0b1111'1000, 0b0000'0111, 4},
    }};

    static constexpr const Utf8Data s_utf8_continuation_byte =
        {0b1000'0000, 0b1100'0000, 0b0011'1111, 6};

    static constexpr codepoint_type s_high_surrogate_min = 0xd800;
    static constexpr codepoint_type s_high_surrogate_max = 0xdbff;
    static constexpr codepoint_type s_low_surrogate_min = 0xdc00;
    static constexpr codepoint_type s_low_surrogate_max = 0xdfff;
    static constexpr codepoint_type s_max_codepoint = 0x10ffff;

    static constexpr char_type s_zero = FLY_CHR(char_type, '0');
    static constexpr char_type s_nine = FLY_CHR(char_type, '9');
    static constexpr char_type s_lower_a = FLY_CHR(char_type, 'a');
    static constexpr char_type s_upper_a = FLY_CHR(char_type, 'A');
    static constexpr char_type s_lower_f = FLY_CHR(char_type, 'f');
    static constexpr char_type s_upper_f = FLY_CHR(char_type, 'F');
    static constexpr char_type s_lower_u = FLY_CHR(char_type, 'u');
    static constexpr char_type s_upper_u = FLY_CHR(char_type, 'U');
};

//==================================================================================================
template <typename StringType>
template <typename DesiredStringType>
std::optional<DesiredStringType>
BasicStringUnicode<StringType>::convert_encoding(const_iterator &it, const const_iterator &end)
{
    using DesiredUnicodeType = BasicStringUnicode<DesiredStringType>;

    DesiredStringType result;
    result.reserve(static_cast<typename StringType::size_type>(std::distance(it, end)));

    while (it != end)
    {
        if (auto codepoint = decode_codepoint(it, end); codepoint)
        {
            if (auto encoded = DesiredUnicodeType::encode_codepoint(codepoint.value()); encoded)
            {
                result += std::move(encoded.value());
                continue;
            }
        }

        return std::nullopt;
    }

    return result;
}

//==================================================================================================
template <typename StringType>
auto BasicStringUnicode<StringType>::decode_codepoint(const_iterator &it, const const_iterator &end)
    -> std::optional<codepoint_type>
{
    auto next_encoded_byte = [&it, &end]() -> std::optional<codepoint_type>
    {
        if (it == end)
        {
            return std::nullopt;
        }

        return static_cast<codepoint_type>(*(it++));
    };

    std::optional<codepoint_type> codepoint = codepoint_from_string(std::move(next_encoded_byte));

    if (codepoint && validate_codepoint(codepoint.value()))
    {
        return codepoint;
    }

    return std::nullopt;
}

//==================================================================================================
template <typename StringType>
std::optional<StringType> BasicStringUnicode<StringType>::encode_codepoint(codepoint_type codepoint)
{
    if (validate_codepoint(codepoint))
    {
        return codepoint_to_string(codepoint);
    }

    return std::nullopt;
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix>
std::optional<StringType>
BasicStringUnicode<StringType>::escape_codepoint(const_iterator &it, const const_iterator &end)
{
    static_assert((UnicodePrefix == 'u') || (UnicodePrefix == 'U'));

    if (auto codepoint = decode_codepoint(it, end); codepoint)
    {
        return escape_codepoint<UnicodePrefix>(codepoint.value());
    }

    return std::nullopt;
}

//==================================================================================================
template <typename StringType>
std::optional<StringType>
BasicStringUnicode<StringType>::unescape_codepoint(const_iterator &it, const const_iterator &end)
{
    auto escaped_with = [&it, &end](const char_type ch) -> bool
    {
        if ((it == end) || ((it + 1) == end))
        {
            return false;
        }

        return (*it == '\\') && (*(it + 1) == ch);
    };

    std::optional<codepoint_type> codepoint;

    if (escaped_with(s_lower_u))
    {
        auto next_codepoint = [&it, &end]() -> std::optional<codepoint_type>
        {
            return unescape_codepoint<s_lower_u>(it, end);
        };

        codepoint = create_codepoint_from_surrogates(std::move(next_codepoint));
    }
    else if (escaped_with(s_upper_u))
    {
        codepoint = unescape_codepoint<s_upper_u>(it, end);
    }

    if (codepoint)
    {
        return encode_codepoint(codepoint.value());
    }

    return std::nullopt;
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix>
StringType BasicStringUnicode<StringType>::escape_codepoint(codepoint_type codepoint)
{
    StringType result;

    // TODO replace this with String::create_hex_string without actually including string.hpp or
    // string_format.hpp.
    auto to_hex = [&codepoint](std::size_t length) -> StringType
    {
        static const auto *s_digits = FLY_STR(char_type, "0123456789abcdef");
        StringType hex(length, FLY_CHR(char_type, '0'));

        for (std::size_t i = 0, j = (length - 1) * 4; i < length; ++i, j -= 4)
        {
            hex[i] = s_digits[(codepoint >> j) & 0x0f];
        }

        return hex;
    };

    if ((codepoint <= 0x1f) || (codepoint >= 0x7f))
    {
        if (codepoint <= 0xffff)
        {
            result += FLY_CHR(char_type, '\\');
            result += s_lower_u;
            result += to_hex(4);
        }
        else
        {
            if constexpr (UnicodePrefix == 'u')
            {
                const codepoint_type high_surrogate = 0xd7c0 + (codepoint >> 10);
                const codepoint_type low_surrogate = 0xdc00 + (codepoint & 0x3ff);

                result += escape_codepoint<UnicodePrefix>(high_surrogate);
                result += escape_codepoint<UnicodePrefix>(low_surrogate);
            }
            else
            {
                result += FLY_CHR(char_type, '\\');
                result += s_upper_u;
                result += to_hex(8);
            }
        }
    }
    else
    {
        result += static_cast<char_type>(codepoint);
    }

    return result;
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix>
auto BasicStringUnicode<StringType>::unescape_codepoint(
    const_iterator &it,
    const const_iterator &end) -> std::optional<codepoint_type>
{
    static_assert((UnicodePrefix == 'u') || (UnicodePrefix == 'U'));

    if ((it == end) || (*it != '\\') || (++it == end) || (*it != UnicodePrefix))
    {
        return std::nullopt;
    }

    codepoint_type codepoint = 0;
    ++it;

    static constexpr const codepoint_type s_expected_digits = (UnicodePrefix == 'u') ? 4 : 8;
    codepoint_type i = 0;

    for (i = 0; (i < s_expected_digits) && (it != end); ++i, ++it)
    {
        const codepoint_type shift = (4 * (s_expected_digits - i - 1));

        if ((*it >= s_zero) && (*it <= s_nine))
        {
            codepoint += static_cast<codepoint_type>(*it - 0x30) << shift;
        }
        else if ((*it >= s_upper_a) && (*it <= s_upper_f))
        {
            codepoint += static_cast<codepoint_type>(*it - 0x37) << shift;
        }
        else if ((*it >= s_lower_a) && (*it <= s_lower_f))
        {
            codepoint += static_cast<codepoint_type>(*it - 0x57) << shift;
        }
        else
        {
            return std::nullopt;
        }
    }

    if (i != s_expected_digits)
    {
        return std::nullopt;
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 1), bool>>
auto BasicStringUnicode<StringType>::codepoint_from_string(EncodedByteProvider next_encoded_byte)
    -> std::optional<codepoint_type>
{
    const std::optional<codepoint_type> maybe_leading_byte = next_encoded_byte();
    if (!maybe_leading_byte)
    {
        return std::nullopt;
    }

    const codepoint_type leading_byte = maybe_leading_byte.value() & 0xff;

    // First find the codepoint length by finding which leading byte matches the first encoded byte.
    auto utf8_it = std::find_if(
        s_utf8_leading_bytes.begin(),
        s_utf8_leading_bytes.end(),
        [&leading_byte](const auto &candidate)
        {
            return (leading_byte & candidate.m_encoding_mask) == candidate.m_leading_byte;
        });

    if (utf8_it == s_utf8_leading_bytes.end())
    {
        return std::nullopt;
    }

    const std::size_t bytes = utf8_it->m_codepoint_size;
    std::size_t shift = s_utf8_continuation_byte.m_codepoint_size * (bytes - 1);

    // Then decode the encoded bytes using the leading and continuation byte masks.
    codepoint_type codepoint = (leading_byte & utf8_it->m_codepoint_mask) << shift;
    codepoint_type first_continuation_byte = 0;

    for (std::size_t i = 1; i < bytes; ++i)
    {
        const std::optional<codepoint_type> maybe_continuation_byte = next_encoded_byte();
        if (!maybe_continuation_byte)
        {
            return std::nullopt;
        }

        const codepoint_type continuation_byte = maybe_continuation_byte.value() & 0xff;

        if (i == 1)
        {
            first_continuation_byte = continuation_byte & 0xff;
        }

        if ((continuation_byte & s_utf8_continuation_byte.m_encoding_mask) !=
            s_utf8_continuation_byte.m_leading_byte)
        {
            return std::nullopt;
        }

        shift -= s_utf8_continuation_byte.m_codepoint_size;
        codepoint |= (continuation_byte & s_utf8_continuation_byte.m_codepoint_mask) << shift;
    }

    // Make sure the encoding was not overlong.
    if ((bytes == 2) && ((leading_byte & 0xfe) == utf8_it->m_leading_byte))
    {
        return std::nullopt;
    }
    else if (bytes > 2)
    {
        if ((leading_byte == utf8_it->m_leading_byte) &&
            ((first_continuation_byte & utf8_it->m_leading_byte) ==
             s_utf8_continuation_byte.m_leading_byte))
        {
            return std::nullopt;
        }
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 2), bool>>
auto BasicStringUnicode<StringType>::codepoint_from_string(EncodedByteProvider next_encoded_byte)
    -> std::optional<codepoint_type>
{
    return create_codepoint_from_surrogates(std::move(next_encoded_byte));
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 4), bool>>
auto BasicStringUnicode<StringType>::codepoint_from_string(EncodedByteProvider next_encoded_byte)
    -> std::optional<codepoint_type>
{
    return next_encoded_byte();
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 1), bool>>
StringType BasicStringUnicode<StringType>::codepoint_to_string(codepoint_type codepoint)
{
    StringType result;

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

    return result;
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 2), bool>>
StringType BasicStringUnicode<StringType>::codepoint_to_string(codepoint_type codepoint)
{
    StringType result;

    if (codepoint < 0x10000)
    {
        result += static_cast<char_type>(codepoint);
    }
    else
    {
        codepoint -= 0x10000;
        result += static_cast<char_type>(s_high_surrogate_min | (codepoint >> 10));
        result += static_cast<char_type>(s_low_surrogate_min | (codepoint & 0x3ff));
    }

    return result;
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 4), bool>>
StringType BasicStringUnicode<StringType>::codepoint_to_string(codepoint_type codepoint)
{
    return StringType(1, static_cast<char_type>(codepoint));
}

//==================================================================================================
template <typename StringType>
auto BasicStringUnicode<StringType>::create_codepoint_from_surrogates(
    EncodedByteProvider next_encoded_byte) -> std::optional<codepoint_type>
{
    auto is_high_surrogate = [](codepoint_type c) -> bool
    {
        return (c >= s_high_surrogate_min) && (c <= s_high_surrogate_max);
    };
    auto is_low_surrogate = [](codepoint_type c) -> bool
    {
        return (c >= s_low_surrogate_min) && (c <= s_low_surrogate_max);
    };

    const std::optional<codepoint_type> high_surrogate = next_encoded_byte();
    if (!high_surrogate)
    {
        return std::nullopt;
    }

    codepoint_type codepoint = high_surrogate.value();

    if (is_high_surrogate(codepoint))
    {
        const std::optional<codepoint_type> low_surrogate = next_encoded_byte();
        if (!low_surrogate)
        {
            return std::nullopt;
        }

        if (is_low_surrogate(low_surrogate.value()))
        {
            // The formula to convert a surrogate pair to a single codepoint is:
            //
            //     C = ((HS - 0xd800) * 0x400) + (LS - 0xdc00) + 0x10000
            //
            // Multiplying by 0x400 is the same as left-shifting 10 bits. The formula then becomes:
            codepoint = (high_surrogate.value() << 10) + low_surrogate.value() - 0x35fdc00;
        }
        else
        {
            return std::nullopt;
        }
    }
    else if (is_low_surrogate(codepoint))
    {
        return std::nullopt;
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
bool BasicStringUnicode<StringType>::validate_codepoint(codepoint_type codepoint)
{
    if ((codepoint >= s_high_surrogate_min) && (codepoint <= s_low_surrogate_max))
    {
        // Reserved codepoint.
        return false;
    }
    else if (codepoint > s_max_codepoint)
    {
        // Out-of-range codepoint.
        return false;
    }

    return true;
}

} // namespace fly::detail
