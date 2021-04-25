#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/literals.hpp"

#include <array>
#include <functional>
#include <iterator>
#include <optional>
#include <string>

namespace fly::detail {

/**
 * Helper class for decoding and encoding Unicode codepoints in a std::basic_string. The assumed
 * Unicode encoding depends on the template type character type:
 *
 *     1. char - UTF-8
 *     2. wchar_t - UTF-16 on Windows, UTF-32 on Linux and macOS
 *     3. char8_t - UTF-8
 *     4. char16_t - UTF-16
 *     5. char32_t - UTF-32
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version June 6, 2020
 */
template <typename CharType>
class BasicUnicode
{
    using traits = detail::BasicStringTraits<CharType>;
    using string_type = typename traits::string_type;
    using view_type = typename traits::view_type;
    using codepoint_type = typename traits::codepoint_type;

public:
    /**
     * Validate that a string is strictly Unicode compliant.
     *
     * @tparam IteratorType The type of the encoded Unicode string's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode string.
     * @param end Pointer to the end of the encoded Unicode string.
     *
     * @return True if the string is Unicode compliant.
     */
    template <typename IteratorType>
    static bool validate_encoding(IteratorType &it, const IteratorType &end);

    /**
     * Convert the Unicode encoding of a string to another encoding.
     *
     * @tparam DesiredStringType The type of string to convert to.
     *
     * @param value The encoded Unicode string to convert.
     *
     * @return If successful, a copy of the source string with the desired encoding. Otherwise, an
     *         uninitialized value.
     */
    template <typename DesiredStringType>
    static std::optional<DesiredStringType> convert_encoding(view_type value);

    /**
     * Convert the Unicode encoding of a string to another encoding, inserting the result into the
     * provided output iterator.
     *
     * @tparam DesiredStringType The type of string to convert to.
     * @tparam OutputIteratorType The type of the output iterator to insert the result into.
     *
     * @param it Pointer to the beginning of the encoded Unicode string.
     * @param end Pointer to the end of the encoded Unicode string.
     * @param out The output iterator to insert the result into.
     *
     * @return Whether the conversion was successful.
     */
    template <typename DesiredStringType, typename OutputIteratorType>
    static bool convert_encoding_into(view_type value, OutputIteratorType out);

    /**
     * Decode a single Unicode codepoint, starting at the character pointed to by the provided
     * iterator. If successful, after invoking this method, that iterator will point at the first
     * character after the Unicode codepoint in the source string.
     *
     * @tparam IteratorType The type of the encoded Unicode codepoint's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an uninitialized value.
     */
    template <typename IteratorType>
    static std::optional<codepoint_type>
    decode_codepoint(IteratorType &it, const IteratorType &end);

    /**
     * Encode a single Unicode codepoint.
     *
     * @return The Unicode codepoint to encode.
     *
     * @return If successful, a string containing the encoded Unicode codepoint. Otherwise, an
     *         uninitialized value.
     */
    static std::optional<string_type> encode_codepoint(codepoint_type codepoint);

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
     * @tparam IteratorType The type of the encoded Unicode codepoint's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, a string containing the escaped Unicode codepoint. Otherwise, an
     *         uninitialized value.
     */
    template <char UnicodePrefix = 'U', typename IteratorType>
    static std::optional<string_type> escape_codepoint(IteratorType &it, const IteratorType &end);

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
     * @tparam IteratorType The type of the escaped Unicode string's iterator.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return If successful, a string containing the unescaped Unicode codepoint. Otherwise, an
     *         uninitialized value.
     */
    template <typename IteratorType>
    static std::optional<string_type> unescape_codepoint(IteratorType &it, const IteratorType &end);

private:
    friend BasicUnicode<char>;
    friend BasicUnicode<wchar_t>;
    friend BasicUnicode<char8_t>;
    friend BasicUnicode<char16_t>;
    friend BasicUnicode<char32_t>;

    /**
     * Escape a single Unicode codepoint.
     *
     * @tparam UnicodePrefix The escaped Unicode prefix character ('u' or 'U').
     *
     * @param codepoint The codepoint to escape.
     *
     * @return If successful, a string containing the escaped Unicode codepoint. Otherwise, an
     *         uninitialized value.
     */
    template <char UnicodePrefix>
    static string_type escape_codepoint(codepoint_type codepoint);

    /**
     * Unescape a sequence of characters to form a single Unicode codepoint.
     *
     * @tparam UnicodePrefix The escaped Unicode prefix character ('u' or 'U').
     * @tparam IteratorType The type of the escaped Unicode string's iterator.
     *
     * @param it Pointer to the beginning of the escaped character sequence.
     * @param end Pointer to the end of the escaped character sequence.
     *
     * @return If successful, a string containing the parsed Unicode codepoint. Otherwise, an
     *         invalid codepoint.
     */
    template <char UnicodePrefix, typename IteratorType>
    static codepoint_type unescape_codepoint(IteratorType &it, const IteratorType &end);

    /**
     * Decode a Unicode codepoint from a UTF-8 string.
     *
     * @tparam IteratorType The type of the encoded Unicode codepoint's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an invalid codepoint.
     */
    template <typename IteratorType>
    requires fly::SizeOfTypeIs<CharType, 1>
    static codepoint_type codepoint_from_string(IteratorType &it, const IteratorType &end);

    /**
     * Decode a Unicode codepoint from a UTF-16 string.
     *
     * @tparam IteratorType The type of the encoded Unicode codepoint's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an invalid codepoint.
     */
    template <typename IteratorType>
    requires fly::SizeOfTypeIs<CharType, 2>
    static codepoint_type codepoint_from_string(IteratorType &it, const IteratorType &end);

    /**
     * Decode a Unicode codepoint from a UTF-32 string.
     *
     * @tparam IteratorType The type of the encoded Unicode codepoint's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return If successful, the decoded Unicode codepoint. Otherwise, an invalid codepoint.
     */
    template <typename IteratorType>
    requires fly::SizeOfTypeIs<CharType, 4>
    static codepoint_type codepoint_from_string(IteratorType &it, const IteratorType &end);

    /**
     * Encode a Unicode codepoint into a UTF-8 string.
     *
     * @tparam OutputIteratorType The type of the output iterator to insert the result into.
     *
     * @param codepoint The codepoint to encode.
     * @param out The output iterator to insert the result into.
     */
    template <typename OutputIteratorType>
    requires fly::SizeOfTypeIs<CharType, 1>
    static void codepoint_to_string(codepoint_type codepoint, OutputIteratorType out);

    /**
     * Encode a Unicode codepoint into a UTF-16 string.
     *
     * @tparam OutputIteratorType The type of the output iterator to insert the result into.
     *
     * @param codepoint The codepoint to encode.
     * @param out The output iterator to insert the result into.
     */
    template <typename OutputIteratorType>
    requires fly::SizeOfTypeIs<CharType, 2>
    static void codepoint_to_string(codepoint_type codepoint, OutputIteratorType out);

    /**
     * Encode a Unicode codepoint into a UTF-32 string.
     *
     * @tparam OutputIteratorType The type of the output iterator to insert the result into.
     *
     * @param codepoint The codepoint to encode.
     * @param out The output iterator to insert the result into.
     */
    template <typename OutputIteratorType>
    requires fly::SizeOfTypeIs<CharType, 4>
    static void codepoint_to_string(codepoint_type codepoint, OutputIteratorType out);

    /**
     * Create a Unicode codepoint from either one complete codepoint or two surrogate halves. The
     * surrogate provider is invoked at least once to retrieve the candidate complete codepoint from
     * the caller. If that codepoint falls in the high surrogate range, the callback is invoked
     * again to retrieve the low surrogate; those two surrogate halves are then combined to form a
     * complete codepoint.
     *
     * @param next_codepoint Callback for the method to invoke to retrieve the next encoded byte.
     *
     * @return If successful, the created Unicode codepoint. Otherwise, an invalid codepoint.
     */
    static codepoint_type
    create_codepoint_from_surrogates(std::function<codepoint_type()> next_codepoint);

    /**
     * Validate a Unicode codepoint is not out-of-range or reserved by the Unicode Standard.
     *
     * @param codepoint The codepoint to validate.
     *
     * @return True if the codepoint is valid.
     */
    static bool validate_codepoint(codepoint_type codepoint);

    /**
     * Retrieve the next byte of an Unicode codepoint. If the provided iterator has reached its end,
     * returns an invalid codepoint.
     *
     * @tparam IteratorType The type of the encoded Unicode codepoint's iterator.
     *
     * @param it Pointer to the beginning of the encoded Unicode codepoint.
     * @param end Pointer to the end of the encoded Unicode codepoint.
     *
     * @return The next byte of the encoded Unicode codepoint.
     */
    template <typename IteratorType>
    static codepoint_type next_encoded_byte(IteratorType &it, const IteratorType &end);

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

    static constexpr const codepoint_type s_high_surrogate_min = 0xd800;
    static constexpr const codepoint_type s_high_surrogate_max = 0xdbff;
    static constexpr const codepoint_type s_low_surrogate_min = 0xdc00;
    static constexpr const codepoint_type s_low_surrogate_max = 0xdfff;
    static constexpr const codepoint_type s_max_codepoint = 0x10ffff;
    static constexpr const codepoint_type s_invalid_codepoint = 0xffffffff;

    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
    static constexpr const auto s_nine = FLY_CHR(CharType, '9');
    static constexpr const auto s_lower_a = FLY_CHR(CharType, 'a');
    static constexpr const auto s_upper_a = FLY_CHR(CharType, 'A');
    static constexpr const auto s_lower_f = FLY_CHR(CharType, 'f');
    static constexpr const auto s_upper_f = FLY_CHR(CharType, 'F');
    static constexpr const auto s_lower_u = FLY_CHR(CharType, 'u');
    static constexpr const auto s_upper_u = FLY_CHR(CharType, 'U');
};

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
bool BasicUnicode<CharType>::validate_encoding(IteratorType &it, const IteratorType &end)
{
    while (it != end)
    {
        if (!decode_codepoint(it, end))
        {
            return false;
        }
    }

    return true;
}

//==================================================================================================
template <typename CharType>
template <typename DesiredStringType>
inline std::optional<DesiredStringType> BasicUnicode<CharType>::convert_encoding(view_type value)
{
    DesiredStringType result;
    result.reserve(static_cast<typename DesiredStringType::size_type>(value.size()));

    if (convert_encoding_into<DesiredStringType>(std::move(value), std::back_inserter(result)))
    {
        return result;
    }

    return std::nullopt;
}

//==================================================================================================
template <typename CharType>
template <typename DesiredStringType, typename OutputIteratorType>
bool BasicUnicode<CharType>::convert_encoding_into(view_type value, OutputIteratorType out)
{
    using DesiredUnicodeType = BasicUnicode<typename DesiredStringType::value_type>;

    auto it = value.cbegin();
    const auto end = value.cend();

    while (it != end)
    {
        if (auto codepoint = decode_codepoint(it, end); codepoint)
        {
            DesiredUnicodeType::codepoint_to_string(*codepoint, out);
            continue;
        }

        return false;
    }

    return true;
}

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
auto BasicUnicode<CharType>::decode_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<codepoint_type>
{
    const codepoint_type codepoint = codepoint_from_string(it, end);

    if (validate_codepoint(codepoint))
    {
        return codepoint;
    }

    return std::nullopt;
}

//==================================================================================================
template <typename CharType>
auto BasicUnicode<CharType>::encode_codepoint(codepoint_type codepoint)
    -> std::optional<string_type>
{
    if (validate_codepoint(codepoint))
    {
        string_type result;
        codepoint_to_string(codepoint, std::back_inserter(result));

        return result;
    }

    return std::nullopt;
}

//==================================================================================================
template <typename CharType>
template <char UnicodePrefix, typename IteratorType>
auto BasicUnicode<CharType>::escape_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<string_type>
{
    static_assert((UnicodePrefix == 'u') || (UnicodePrefix == 'U'));

    if (auto codepoint = decode_codepoint(it, end); codepoint)
    {
        return escape_codepoint<UnicodePrefix>(*codepoint);
    }

    return std::nullopt;
}

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
auto BasicUnicode<CharType>::unescape_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<string_type>
{
    auto escaped_with = [&it, &end](const CharType ch) -> bool
    {
        if ((it == end) || ((it + 1) == end))
        {
            return false;
        }

        return (*it == '\\') && (*(it + 1) == ch);
    };

    codepoint_type codepoint = s_invalid_codepoint;

    if (escaped_with(s_lower_u))
    {
        auto next_codepoint = [&it, &end]() -> codepoint_type
        {
            return unescape_codepoint<s_lower_u>(it, end);
        };

        codepoint = create_codepoint_from_surrogates(std::move(next_codepoint));
    }
    else if (escaped_with(s_upper_u))
    {
        codepoint = unescape_codepoint<s_upper_u>(it, end);
    }

    return encode_codepoint(codepoint);
}

//==================================================================================================
template <typename CharType>
template <char UnicodePrefix>
auto BasicUnicode<CharType>::escape_codepoint(codepoint_type codepoint) -> string_type
{
    string_type result;

    // TODO: Replace this with BasicString::format without actually including string_formatters.hpp.
    auto to_hex = [&codepoint](std::size_t length) -> string_type
    {
        static const auto *s_digits = FLY_STR(CharType, "0123456789abcdef");
        string_type hex(length, FLY_CHR(CharType, '0'));

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
            result += FLY_CHR(CharType, '\\');
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
                result += FLY_CHR(CharType, '\\');
                result += s_upper_u;
                result += to_hex(8);
            }
        }
    }
    else
    {
        result += static_cast<CharType>(codepoint);
    }

    return result;
}

//==================================================================================================
template <typename CharType>
template <char UnicodePrefix, typename IteratorType>
auto BasicUnicode<CharType>::unescape_codepoint(IteratorType &it, const IteratorType &end)
    -> codepoint_type
{
    static_assert((UnicodePrefix == 'u') || (UnicodePrefix == 'U'));

    if ((it == end) || (*it != '\\') || (++it == end) || (*it != UnicodePrefix))
    {
        return s_invalid_codepoint;
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
            return s_invalid_codepoint;
        }
    }

    return (i == s_expected_digits) ? codepoint : s_invalid_codepoint;
}

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
requires SizeOfTypeIs<CharType, 1>
auto BasicUnicode<CharType>::codepoint_from_string(IteratorType &it, const IteratorType &end)
    -> codepoint_type
{
    const codepoint_type leading_byte = next_encoded_byte(it, end);

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
        return s_invalid_codepoint;
    }

    const std::size_t bytes = utf8_it->m_codepoint_size;
    std::size_t shift = s_utf8_continuation_byte.m_codepoint_size * (bytes - 1);

    // Then decode the encoded bytes using the leading and continuation byte masks.
    codepoint_type codepoint = (leading_byte & utf8_it->m_codepoint_mask) << shift;

    for (std::size_t i = 1; i < bytes; ++i)
    {
        const codepoint_type continuation_byte = next_encoded_byte(it, end);

        if ((continuation_byte & s_utf8_continuation_byte.m_encoding_mask) !=
            s_utf8_continuation_byte.m_leading_byte)
        {
            return s_invalid_codepoint;
        }

        shift -= s_utf8_continuation_byte.m_codepoint_size;
        codepoint |= (continuation_byte & s_utf8_continuation_byte.m_codepoint_mask) << shift;
    }

    // Finally, make sure the encoding was not overlong.
    if (((codepoint < 0x80) && (bytes != 1)) ||
        ((codepoint >= 0x80) && (codepoint < 0x800) && (bytes != 2)) ||
        ((codepoint >= 0x800) && (codepoint < 0x10000) && (bytes != 3)))
    {
        return s_invalid_codepoint;
    }

    return codepoint;
}

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
requires SizeOfTypeIs<CharType, 2>
auto BasicUnicode<CharType>::codepoint_from_string(IteratorType &it, const IteratorType &end)
    -> codepoint_type
{
    auto next_codepoint = [&it, &end]() -> codepoint_type
    {
        return next_encoded_byte(it, end);
    };

    return create_codepoint_from_surrogates(std::move(next_codepoint));
}

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
requires SizeOfTypeIs<CharType, 4>
auto BasicUnicode<CharType>::codepoint_from_string(IteratorType &it, const IteratorType &end)
    -> codepoint_type
{
    return next_encoded_byte(it, end);
}

//==================================================================================================
template <typename CharType>
template <typename OutputIteratorType>
requires SizeOfTypeIs<CharType, 1>
void BasicUnicode<CharType>::codepoint_to_string(codepoint_type codepoint, OutputIteratorType out)
{
    if (codepoint < 0x80)
    {
        *out++ = static_cast<CharType>(codepoint);
    }
    else if (codepoint < 0x800)
    {
        *out++ = static_cast<CharType>(0xc0 | (codepoint >> 6));
        *out++ = static_cast<CharType>(0x80 | (codepoint & 0x3f));
    }
    else if (codepoint < 0x10000)
    {
        *out++ = static_cast<CharType>(0xe0 | (codepoint >> 12));
        *out++ = static_cast<CharType>(0x80 | ((codepoint >> 6) & 0x3f));
        *out++ = static_cast<CharType>(0x80 | (codepoint & 0x3f));
    }
    else
    {
        *out++ = static_cast<CharType>(0xf0 | (codepoint >> 18));
        *out++ = static_cast<CharType>(0x80 | ((codepoint >> 12) & 0x3f));
        *out++ = static_cast<CharType>(0x80 | ((codepoint >> 6) & 0x3f));
        *out++ = static_cast<CharType>(0x80 | (codepoint & 0x3f));
    }
}

//==================================================================================================
template <typename CharType>
template <typename OutputIteratorType>
requires SizeOfTypeIs<CharType, 2>
void BasicUnicode<CharType>::codepoint_to_string(codepoint_type codepoint, OutputIteratorType out)
{
    if (codepoint < 0x10000)
    {
        *out++ = static_cast<CharType>(codepoint);
    }
    else
    {
        codepoint -= 0x10000;
        *out++ = static_cast<CharType>(s_high_surrogate_min | (codepoint >> 10));
        *out++ = static_cast<CharType>(s_low_surrogate_min | (codepoint & 0x3ff));
    }
}

//==================================================================================================
template <typename CharType>
template <typename OutputIteratorType>
requires SizeOfTypeIs<CharType, 4>
void BasicUnicode<CharType>::codepoint_to_string(codepoint_type codepoint, OutputIteratorType out)
{
    *out++ = static_cast<CharType>(codepoint);
}

//==================================================================================================
template <typename CharType>
auto BasicUnicode<CharType>::create_codepoint_from_surrogates(
    std::function<codepoint_type()> next_codepoint) -> codepoint_type
{
    auto is_high_surrogate = [](codepoint_type c) -> bool
    {
        return (c >= s_high_surrogate_min) && (c <= s_high_surrogate_max);
    };
    auto is_low_surrogate = [](codepoint_type c) -> bool
    {
        return (c >= s_low_surrogate_min) && (c <= s_low_surrogate_max);
    };

    codepoint_type codepoint = next_codepoint();

    if (is_high_surrogate(codepoint))
    {
        const codepoint_type low_surrogate = next_codepoint();

        if (is_low_surrogate(low_surrogate))
        {
            // The formula to convert a surrogate pair to a single codepoint is:
            //
            //     C = ((HS - 0xd800) * 0x400) + (LS - 0xdc00) + 0x10000
            //
            // Multiplying by 0x400 is the same as left-shifting 10 bits. The formula then becomes:
            codepoint = (codepoint << 10) + low_surrogate - 0x35fdc00;
        }
        else
        {
            return s_invalid_codepoint;
        }
    }
    else if (is_low_surrogate(codepoint))
    {
        return s_invalid_codepoint;
    }

    return codepoint;
}

//==================================================================================================
template <typename CharType>
bool BasicUnicode<CharType>::validate_codepoint(codepoint_type codepoint)
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

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
inline auto BasicUnicode<CharType>::next_encoded_byte(IteratorType &it, const IteratorType &end)
    -> codepoint_type
{
    return (it == end) ? s_invalid_codepoint : static_cast<codepoint_type>(*(it++));
}

} // namespace fly::detail
