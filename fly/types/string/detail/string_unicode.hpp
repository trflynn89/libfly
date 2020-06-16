#pragma once

#include "fly/types/string/detail/string_formatter.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_exception.hpp"
#include "fly/types/string/string_literal.hpp"

#include <array>
#include <functional>
#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Helper class for decoding and encoding Unicode codepoints in a std::basic_string<>. The exact
 * encoding depends on the template type StringType:
 *
 *     1. std::string - UTF-8 encoding.
 *     2. std::wstring - UTF-16 on Windows, UTF-32 on Linux.
 *     3. std::u16string - UTF-16 encoding.
 *     4. std::u32string - UTF-32 encoding.
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

    using ExceptionFormatter = BasicStringFormatter<std::string>;
    using StringFormatter = BasicStringFormatter<StringType>;

public:
    /**
     * Decode a single Unicode codepoint, starting at the character pointed to by the provided
     * iterator. If successful, after invoking this method, that iterator will point at the first
     * character after the Unicode codepoint in the source string.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return The decoded Unicode codepoint.
     *
     * @throws UnicodeException If the encoded Unicode codepoint is invalid.
     */
    static codepoint_type decode_codepoint(const_iterator &it, const const_iterator &end);

    /**
     * Encode a single Unicode codepoint.
     *
     * @return The Unicode codepoint to encode.
     *
     * @return A string containing the encoded Unicode codepoint.
     */
    static StringType encode_codepoint(codepoint_type codepoint);

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
     * @return A string containing the escaped Unicode codepoint.
     *
     * @throws UnicodeException If the Unicode codepoint could not be escaped.
     */
    template <char UnicodePrefix>
    static StringType escape_codepoint(const_iterator &it, const const_iterator &end);

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
     * @return A string containing the unescaped Unicode codepoint.
     *
     * @throws UnicodeException If the escaped sequence is not a valid Unicode codepoint.
     */
    static StringType unescape_codepoint(const_iterator &it, const const_iterator &end);

private:
    using EncodedByteProvider = std::function<codepoint_type()>;

    /**
     * Escape a single Unicode codepoint.
     *
     * @tparam UnicodePrefix The escaped Unicode prefix character ('u' or 'U').
     *
     * @param codepoint The codepoint to escape.
     *
     * @return The escaped Unicode codepoint.
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
     * @return The parsed Unicode codepoint.
     *
     * @throws UnicodeException If the escaped sequence is not a valid Unicode codepoint.
     */
    template <char UnicodePrefix>
    static codepoint_type unescape_codepoint(const_iterator &it, const const_iterator &end);

    /**
     * Decode a Unicode codepoint from a UTF-8 string.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return The decoded Unicode codepoint.
     *
     * @throws UnicodeException If the Unicode codepoint could not be decoded.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 1), bool> = 0>
    static codepoint_type codepoint_from_string(EncodedByteProvider next_encoded_byte);

    /**
     * Decode a Unicode codepoint from a UTF-16 string.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return The decoded Unicode codepoint.
     *
     * @throws UnicodeException If the Unicode codepoint could not be decoded.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 2), bool> = 0>
    static codepoint_type codepoint_from_string(EncodedByteProvider next_encoded_byte);

    /**
     * Decode a Unicode codepoint from a UTF-32 string.
     *
     * @param next_encoded_byte Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return The decoded Unicode codepoint.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 4), bool> = 0>
    static codepoint_type codepoint_from_string(EncodedByteProvider next_encoded_byte);

    /**
     * Encode a Unicode codepoint into a UTF-8 string.
     *
     * @param codepoint The codepoint to encode.
     *
     * @return The encoded Unicode codepoint.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 1), bool> = 0>
    static StringType codepoint_to_string(codepoint_type codepoint);

    /**
     * Encode a Unicode codepoint into a UTF-16 string.
     *
     * @param codepoint The codepoint to encode.
     *
     * @return The encoded Unicode codepoint.
     */
    template <typename CharType = char_type, std::enable_if_t<(sizeof(CharType) == 2), bool> = 0>
    static StringType codepoint_to_string(codepoint_type codepoint);

    /**
     * Encode a Unicode codepoint into a UTF-32 string.
     *
     * @param codepoint The codepoint to encode.
     *
     * @return The encoded Unicode codepoint.
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
     * @return The created Unicode codepoint.
     *
     * @throws UnicodeException If the Unicode codepoint could not be created.
     */
    static codepoint_type create_codepoint(EncodedByteProvider next_encoded_byte);

    /**
     * Validate a Unicode codepoint is not out-of-range or reserved by the Unicode Standard.
     *
     * @param codepoint The codepoint to validate.
     *
     * @throws UnicodeException If the Unicode codepoint is invalid.
     */
    static void validate_codepoint(codepoint_type codepoint);

    /**
     * Structure to hold static data required for decoding UTF-8 encoded Unicode codepoints.
     */
    struct Utf8Data
    {
        // The value of the UTF-8 encoded leading byte.
        const codepoint_type leading_byte;

        // A bit-mask of the bits in the UTF-8 encoded leading byte reserved for encoding.
        const codepoint_type encoding_mask;

        // A bit-mask of the bits in the UTF-8 encoded leading byte reserved for codepoint data.
        const codepoint_type codepoint_mask;

        // The number of bytes required to decode the codepoint.
        const codepoint_type codepoint_size;
    };

    static constexpr const std::array<Utf8Data, 4> utf8_leading_bytes = {{
        // Codepoint length 1, range [U+0000, U+007f], leading byte 0b0xxx'xxxx.
        {0b0000'0000, 0b1000'0000, 0b0111'1111, 1},

        // Codepoint length 2, range [U+0080, U+07FF], leading byte 0b110x'xxxx.
        {0b1100'0000, 0b1110'0000, 0b0001'1111, 2},

        // Codepoint length 3, range [U+0800, U+FFFF], leading byte 0b1110'xxxx.
        {0b1110'0000, 0b1111'0000, 0b0000'1111, 3},

        // Codepoint length 4, range [U+10000, U+10FFFF], leading byte 0b1111'0xxx.
        {0b1111'0000, 0b1111'1000, 0b0000'0111, 4},
    }};

    static constexpr const Utf8Data utf8_continuation_byte =
        {0b1000'0000, 0b1100'0000, 0b0011'1111, 6};

    static constexpr codepoint_type high_surrogate_min = 0xd800;
    static constexpr codepoint_type high_surrogate_max = 0xdbff;
    static constexpr codepoint_type low_surrogate_min = 0xdc00;
    static constexpr codepoint_type low_surrogate_max = 0xdfff;
    static constexpr codepoint_type max_codepoint = 0x10ffff;

    static constexpr char_type ch_u = FLY_CHR(char_type, 'u');
    static constexpr char_type ch_U = FLY_CHR(char_type, 'U');
    static constexpr char_type ch_0 = FLY_CHR(char_type, '0');
    static constexpr char_type ch_9 = FLY_CHR(char_type, '9');
    static constexpr char_type ch_a = FLY_CHR(char_type, 'a');
    static constexpr char_type ch_A = FLY_CHR(char_type, 'A');
    static constexpr char_type ch_f = FLY_CHR(char_type, 'f');
    static constexpr char_type ch_F = FLY_CHR(char_type, 'F');
};

//==================================================================================================
template <typename StringType>
auto BasicStringUnicode<StringType>::decode_codepoint(const_iterator &it, const const_iterator &end)
    -> codepoint_type
{
    auto next_encoded_byte = [&it, &end]() -> codepoint_type {
        if (it == end)
        {
            throw UnicodeException("Expected another encoded byte");
        }

        return static_cast<codepoint_type>(*(it++));
    };

    const codepoint_type codepoint = codepoint_from_string(std::move(next_encoded_byte));
    validate_codepoint(codepoint);

    return codepoint;
}

//==================================================================================================
template <typename StringType>
StringType BasicStringUnicode<StringType>::encode_codepoint(codepoint_type codepoint)
{
    validate_codepoint(codepoint);

    return codepoint_to_string(codepoint);
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix>
StringType
BasicStringUnicode<StringType>::escape_codepoint(const_iterator &it, const const_iterator &end)
{
    static_assert((UnicodePrefix == 'u') || (UnicodePrefix == 'U'));

    const codepoint_type codepoint = decode_codepoint(it, end);
    return escape_codepoint<UnicodePrefix>(codepoint);
}

//==================================================================================================
template <typename StringType>
StringType
BasicStringUnicode<StringType>::unescape_codepoint(const_iterator &it, const const_iterator &end)
{
    auto escaped_with = [&it, &end](const char_type ch) -> bool {
        if ((it == end) || ((it + 1) == end))
        {
            return false;
        }

        return (*it == '\\') && (*(it + 1) == ch);
    };

    codepoint_type codepoint;

    if (escaped_with(ch_u))
    {
        auto next_codepoint = [&it, &end]() -> codepoint_type {
            return unescape_codepoint<ch_u>(it, end);
        };

        codepoint = create_codepoint(std::move(next_codepoint));
    }
    else if (escaped_with(ch_U))
    {
        codepoint = unescape_codepoint<ch_U>(it, end);
    }
    else
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Escaped Unicode must begin with \\%c or \\%c",
            static_cast<char>(ch_u),
            static_cast<char>(ch_U)));
    }

    return encode_codepoint(codepoint);
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix>
StringType BasicStringUnicode<StringType>::escape_codepoint(codepoint_type codepoint)
{
    StringType result;

    if ((codepoint <= 0x1f) || (codepoint >= 0x7f))
    {
        if (codepoint <= 0xffff)
        {
            result += FLY_CHR(char_type, '\\');
            result += ch_u;
            result += StringFormatter::format_hex(codepoint, 4);
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
                result += ch_U;
                result += StringFormatter::format_hex(codepoint, 8);
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
    const const_iterator &end) -> codepoint_type
{
    static_assert((UnicodePrefix == 'u') || (UnicodePrefix == 'U'));

    if ((it == end) || (*it != '\\') || (++it == end) || (*it != UnicodePrefix))
    {
        throw UnicodeException(
            ExceptionFormatter::format("Expected codepoint to begin with \\%c", UnicodePrefix));
    }

    codepoint_type codepoint = 0;
    ++it;

    static constexpr const codepoint_type expected_digits = (UnicodePrefix == 'u') ? 4 : 8;
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
            throw UnicodeException(ExceptionFormatter::format(
                "Expected %x to be a hexadecimal digit",
                static_cast<codepoint_type>(*it)));
        }
    }

    if (i != expected_digits)
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Expected exactly %u hexadecimals after \\%c, only found %u",
            expected_digits,
            UnicodePrefix,
            i));
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 1), bool>>
auto BasicStringUnicode<StringType>::codepoint_from_string(EncodedByteProvider next_encoded_byte)
    -> codepoint_type
{
    const codepoint_type leading_byte = next_encoded_byte() & 0xff;

    // First find the codepoint length by finding which leading byte matches the first encoded byte.
    auto utf8_it = std::find_if(
        utf8_leading_bytes.begin(),
        utf8_leading_bytes.end(),
        [&leading_byte](const auto &candidate) {
            return (leading_byte & candidate.encoding_mask) == candidate.leading_byte;
        });

    if (utf8_it == utf8_leading_bytes.end())
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Leading byte %x is not a UTF-8 encoded leading byte",
            leading_byte));
    }

    const std::size_t bytes = utf8_it->codepoint_size;
    std::size_t shift = utf8_continuation_byte.codepoint_size * (bytes - 1);

    // Then decode the encoded bytes using the leading and continuation byte masks.
    codepoint_type codepoint = (leading_byte & utf8_it->codepoint_mask) << shift;
    codepoint_type first_continuation_byte = 0;

    for (std::size_t i = 1; i < bytes; ++i)
    {
        const codepoint_type continuation_byte = next_encoded_byte() & 0xff;

        if (i == 1)
        {
            first_continuation_byte = continuation_byte;
        }

        if ((continuation_byte & utf8_continuation_byte.encoding_mask) !=
            utf8_continuation_byte.leading_byte)
        {
            throw UnicodeException(ExceptionFormatter::format(
                "Continuation byte %x is not a UTF-8 encoded continuation byte",
                continuation_byte));
        }

        shift -= utf8_continuation_byte.codepoint_size;
        codepoint |= (continuation_byte & utf8_continuation_byte.codepoint_mask) << shift;
    }

    if ((bytes == 2) && ((leading_byte & 0xfe) == utf8_it->leading_byte))
    {
        throw UnicodeException(
            ExceptionFormatter::format("Encoded 2-byte UTF-8 codepoint %x is overlong", codepoint));
    }
    else if (
        (bytes > 2) && (leading_byte == utf8_it->leading_byte) &&
        ((first_continuation_byte & utf8_it->leading_byte) == utf8_continuation_byte.leading_byte))
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Encoded %u-byte UTF-8 codepoint %x is overlong",
            bytes,
            codepoint));
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 2), bool>>
auto BasicStringUnicode<StringType>::codepoint_from_string(EncodedByteProvider next_encoded_byte)
    -> codepoint_type
{
    return create_codepoint(std::move(next_encoded_byte));
}

//==================================================================================================
template <typename StringType>
template <typename CharType, std::enable_if_t<(sizeof(CharType) == 4), bool>>
auto BasicStringUnicode<StringType>::codepoint_from_string(EncodedByteProvider next_encoded_byte)
    -> codepoint_type
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
        result += static_cast<char_type>(high_surrogate_min | (codepoint >> 10));
        result += static_cast<char_type>(low_surrogate_min | (codepoint & 0x3ff));
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
auto BasicStringUnicode<StringType>::create_codepoint(EncodedByteProvider next_encoded_byte)
    -> codepoint_type
{
    auto is_high_surrogate = [](codepoint_type c) -> bool {
        return (c >= high_surrogate_min) && (c <= high_surrogate_max);
    };
    auto is_low_surrogate = [](codepoint_type c) -> bool {
        return (c >= low_surrogate_min) && (c <= low_surrogate_max);
    };

    const codepoint_type high_surrogate = next_encoded_byte();
    codepoint_type codepoint = high_surrogate;

    if (is_high_surrogate(high_surrogate))
    {
        const codepoint_type low_surrogate = next_encoded_byte();

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
            throw UnicodeException(ExceptionFormatter::format(
                "Expected low surrogate to follow high surrogate %x, found %x",
                high_surrogate,
                low_surrogate));
        }
    }
    else if (is_low_surrogate(high_surrogate))
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Expected high surrogate to preceed low surrogate %x",
            high_surrogate));
    }

    return codepoint;
}

//==================================================================================================
template <typename StringType>
void BasicStringUnicode<StringType>::validate_codepoint(codepoint_type codepoint)
{
    if ((codepoint >= high_surrogate_min) && (codepoint <= low_surrogate_max))
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Codepoint %x is reserved by the Unicode Standard",
            codepoint));
    }
    else if (codepoint > max_codepoint)
    {
        throw UnicodeException(ExceptionFormatter::format(
            "Codepoint %x exceeds the maxium codepoint U+10ffff",
            codepoint));
    }
}

} // namespace fly::detail
