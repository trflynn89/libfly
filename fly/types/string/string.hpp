#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_converter.hpp"
#include "fly/types/string/detail/string_formatter.hpp"
#include "fly/types/string/detail/string_streamer.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/detail/string_unicode.hpp"
#include "fly/types/string/string_literal.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ios>
#include <optional>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace fly {

/**
 * Forward declarations of the supported BasicString specializations.
 */
template <typename StringType>
class BasicString;

using String = BasicString<std::string>;
using WString = BasicString<std::wstring>;
using String8 = BasicString<std::u8string>;
using String16 = BasicString<std::u16string>;
using String32 = BasicString<std::u32string>;

using StringTraits = detail::BasicStringTraits<std::string>;
using WStringTraits = detail::BasicStringTraits<std::wstring>;
using String8Traits = detail::BasicStringTraits<std::u8string>;
using String16Traits = detail::BasicStringTraits<std::u16string>;
using String32Traits = detail::BasicStringTraits<std::u32string>;

/**
 * Static class to provide string utilities not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename StringType>
class BasicString
{
    using formatter = detail::BasicStringFormatter<StringType>;
    using streamer = detail::BasicStringStreamer<StringType>;
    using traits = detail::BasicStringTraits<StringType>;
    using unicode = detail::BasicStringUnicode<StringType>;

public:
    using string_type = typename traits::string_type;
    using size_type = typename traits::size_type;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using codepoint_type = typename traits::codepoint_type;
    using ostream_type = typename traits::ostream_type;
    using streamed_type = typename traits::streamed_type;

    /**
     * Determine the length of any string-like value. Accepts character arrays, std::basic_string
     * specializations, and std::basic_string_view specializations.
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     *
     * @return The length of the string-like value.
     */
    template <typename T, enable_if_all<detail::is_like_supported_string<T>> = 0>
    static size_type size(const T &value);

    /**
     * Split a string into a vector of strings.
     *
     * @param input The string to split.
     * @param delimiter The delimiter to split the string on.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<StringType> split(const StringType &input, char_type delimiter);

    /**
     * Split a string into a vector of strings, up to a maximum size. If the max size is reached,
     * the rest of the string is appended to the last element in the vector.
     *
     * @param input The string to split.
     * @param delimiter The delimiter to split the string on.
     * @param count The maximum return vector size. Zero implies unlimited.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<StringType>
    split(const StringType &input, char_type delimiter, size_type count);

    /**
     * Remove leading and trailing whitespace from a string.
     *
     * @param target The string to trim.
     */
    static void trim(StringType &target);

    /**
     * Replace all instances of a substring in a string with a character.
     *
     * @param target The string container which will be modified.
     * @param search The string to search for and replace.
     * @param replace The replacement character.
     */
    static void replace_all(StringType &target, const StringType &search, const char_type &replace);

    /**
     * Replace all instances of a substring in a string with another string.
     *
     * @param target The string container which will be modified.
     * @param search The string to search for and replace.
     * @param replace The replacement string.
     */
    static void
    replace_all(StringType &target, const StringType &search, const StringType &replace);

    /**
     * Remove all instances of a substring in a string.
     *
     * @param target The string container which will be modified.
     * @param search The string to search for and remove.
     */
    static void remove_all(StringType &target, const StringType &search);

    /**
     * Check if a string begins with a character.
     *
     * @param source The string to check.
     * @param search The beginning to search for.
     *
     * @return True if the string begins with the search character.
     */
    static bool starts_with(const StringType &source, const char_type &search);

    /**
     * Check if a string begins with another string.
     *
     * @param source The string to check.
     * @param search The beginning to search for.
     *
     * @return True if the string begins with the search string.
     */
    static bool starts_with(const StringType &source, const StringType &search);

    /**
     * Check if a string ends with a character.
     *
     * @param source The string to check.
     * @param search The ending to search for.
     *
     * @return True if the string ends with the search character.
     */
    static bool ends_with(const StringType &source, const char_type &search);

    /**
     * Check if a string ends with another string.
     *
     * @param source The string to check.
     * @param search The ending to search for.
     *
     * @return True if the string ends with the search string.
     */
    static bool ends_with(const StringType &source, const StringType &search);

    /**
     * Check if a string matches another string with wildcard expansion.
     *
     * @param source The source string to match against.
     * @param search The wildcard string to search with.
     *
     * @return True if the wildcard string matches the source string.
     */
    static bool wildcard_match(const StringType &source, const StringType &search);

    /**
     * Validate that a string is strictly Unicode compliant.
     *
     * @param value The string to validate.
     *
     * @return True if the string is Unicode compliant.
     */
    static bool validate(const StringType &value);

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
     * @return If successful, the decoded Unicode codepoint. Otherwise, an unitialized value.
     */
    template <typename IteratorType>
    static std::optional<codepoint_type>
    decode_codepoint(IteratorType &it, const IteratorType &end);

    /**
     * Encode a single Unicode codepoint.
     *
     * @param codepoint The Unicode codepoint to encode.
     *
     * @return If successful, a string containing the encoded Unicode codepoint. Otherwise, an
     *         unitialized value.
     */
    static std::optional<StringType> encode_codepoint(codepoint_type codepoint);

    /**
     * Escape all Unicode codepoints in a string.
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
     * @param value The string to escape.
     *
     * @return If successful, a copy of the source string with all Unicode codepoints escaped.
     *         Otherwise, an unitialized value.
     */
    template <char UnicodePrefix = 'U'>
    static std::optional<StringType> escape_all_codepoints(const StringType &value);

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
     *         unitialized value.
     */
    template <char UnicodePrefix = 'U', typename IteratorType>
    static std::optional<StringType> escape_codepoint(IteratorType &it, const IteratorType &end);

    /**
     * Unescape all Unicode codepoints in a string.
     *
     * Accepts escaped sequences of the following forms:
     *
     *     1. \unnnn for Unicode codepoints in the range [U+0000, U+FFFF].
     *     2. \unnnn\unnnn surrogate pairs for Unicode codepoints in the range [U+10000, U+10FFFF].
     *     3. \Unnnnnnnn for all Unicode codepoints.
     *
     * @param value The string containing the escaped character sequence.
     *
     * @return If successful, a copy of the source string with all Unicode codepoints unescaped.
     *         Otherwise, an unitialized value.
     */
    static std::optional<StringType> unescape_all_codepoints(const StringType &value);

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
     *         unitialized value.
     */
    template <typename IteratorType>
    static std::optional<StringType> unescape_codepoint(IteratorType &it, const IteratorType &end);

    /**
     * Format an integer as a hexadecimal string.
     *
     * If the number of bytes required for the string exceeds the provided length, only the least-
     * significant bytes will be written. If the number of bytes required for the string is less
     * than the provided length, the string will be zero-padded.
     *
     * @tparam IntegerType The type of the integer to format.
     *
     * @param value The integer to format.
     * @param length The length of the string to create.
     *
     * @return The created string with only hexacdemical digits.
     */
    template <typename IntegerType>
    static StringType create_hex_string(IntegerType value, size_type length);

    /**
     * Generate a random string of the given length.
     *
     * @param length The length of the string to generate.
     *
     * @return The generated string.
     */
    static StringType generate_random_string(size_type length);

    /**
     * Format a string with variadic template arguments, returning the formatted string.
     *
     * This is type safe in that argument types need not match the format specifier (i.e. there is
     * no error if %s is given an integer). However, specifiers such as %x are still attempted to be
     * handled. That is, if the matching argument for %x is numeric, then it will be converted to a
     * hexadecimal representation.
     *
     * There is also no checking done on the number of format specifiers and the number of
     * arguments. The format specifiers will be replaced one at a time until all arguments are
     * exhausted, then the rest of the string is taken as-is. Any extra specifiers will be in the
     * string. Any extra arguments are dropped.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param fmt The string to format.
     * @param args The variadic list of arguments to be formatted.
     *
     * @return A string that has been formatted with the given arguments.
     */
    template <typename... Args>
    static streamed_type format(const char_type *fmt, const Args &...args);

    /**
     * Format a string with variadic template arguments, inserting the formatted string into a
     * stream.
     *
     * This is type safe in that argument types need not match the format specifier (i.e. there is
     * no error if %s is given an integer). However, specifiers such as %x are still attempted to be
     * handled. That is, if the matching argument for %x is numeric, then it will be converted to a
     * hexadecimal representation.
     *
     * There is also no checking done on the number of format specifiers and the number of
     * arguments. The format specifiers will be replaced one at a time until all arguments are
     * exhausted, then the rest of the string is taken as-is. Any extra specifiers will be in the
     * string. Any extra arguments are dropped.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param ostream The stream to insert the formatted string into.
     * @param fmt The string to format.
     * @param args The variadic list of arguments to be formatted.
     *
     * @return The same stream object.
     */
    template <typename... Args>
    static ostream_type &format(ostream_type &ostream, const char_type *fmt, const Args &...args);

    /**
     * Concatenate a list of objects with the given separator.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param separator Character to use as a separator.
     * @param args The variadic list of arguments to be joined.
     *
     * @return The resulting join of the given arguments.
     */
    template <typename... Args>
    static streamed_type join(const char_type &separator, const Args &...args);

    /**
     * Convert a string to another type. The other type may be a string with a different Unicode
     * encoding or a plain-old-data type, e.g. int or bool.
     *
     * @tparam T The desired type.
     *
     * @param value The string to convert.
     *
     * @return If successful, the string coverted to the specified type. Otherwise, an unitialized
     *         value.
     */
    template <typename T>
    static std::optional<T> convert(const StringType &value);

private:
    /**
     * Recursively join one argument into the given ostream.
     */
    template <typename T, typename... Args>
    static void join_internal(
        ostream_type &ostream,
        const char_type &separator,
        const T &value,
        const Args &...args);

    /**
     * Terminator for the variadic template joiner. Join the last argument into the given ostream.
     */
    template <typename T>
    static void join_internal(ostream_type &ostream, const char_type &separator, const T &value);

    /**
     * A list of alpha-numeric characters in the range [0-9A-Za-z].
     */
    static constexpr const char_type *s_alpha_num =
        FLY_STR(char_type, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    static constexpr size_type s_alpha_num_length =
        std::char_traits<char_type>::length(s_alpha_num);
};

//==================================================================================================
template <typename StringType>
template <typename T, enable_if_all<detail::is_like_supported_string<T>>>
auto BasicString<StringType>::size(const T &value) -> size_type
{
    using U = std::decay_t<T>;

    if constexpr (any_same_v<U, StringType, std::basic_string_view<char_type>>)
    {
        return value.size();
    }
    else
    {
        return std::char_traits<char_type>::length(value);
    }
}

//==================================================================================================
template <typename StringType>
std::vector<StringType> BasicString<StringType>::split(const StringType &input, char_type delimiter)
{
    return split(input, delimiter, 0);
}

//==================================================================================================
template <typename StringType>
std::vector<StringType>
BasicString<StringType>::split(const StringType &input, char_type delimiter, size_type count)
{
    std::vector<StringType> elements;
    size_type num_items = 0;
    StringType item;

    size_type start = 0;
    size_type end = input.find(delimiter);

    auto push_item = [&](const StringType &str)
    {
        if (!str.empty())
        {
            if ((count > 0) && (++num_items > count))
            {
                elements.back() += delimiter;
                elements.back() += str;
            }
            else
            {
                elements.push_back(str);
            }
        }
    };

    while (end != std::string::npos)
    {
        item = input.substr(start, end - start);
        push_item(item);

        start = end + 1;
        end = input.find(delimiter, start);
    }

    item = input.substr(start, end);
    push_item(item);

    return elements;
}

//==================================================================================================
template <typename StringType>
void BasicString<StringType>::trim(StringType &target)
{
    auto is_non_space = [](int ch)
    {
        return !std::isspace(ch);
    };

    // Remove leading whitespace.
    target.erase(target.begin(), std::find_if(target.begin(), target.end(), is_non_space));

    // Remove trailing whitespace.
    target.erase(std::find_if(target.rbegin(), target.rend(), is_non_space).base(), target.end());
}

//==================================================================================================
template <typename StringType>
void BasicString<StringType>::replace_all(
    StringType &target,
    const StringType &search,
    const char_type &replace)
{
    size_type index = target.find(search);

    while (!search.empty() && (index != StringType::npos))
    {
        target.replace(index, search.size(), 1, replace);
        index = target.find(search);
    }
}

//==================================================================================================
template <typename StringType>
void BasicString<StringType>::replace_all(
    StringType &target,
    const StringType &search,
    const StringType &replace)
{
    size_type index = target.find(search);

    while (!search.empty() && (index != StringType::npos))
    {
        target.replace(index, search.size(), replace);
        index = target.find(search);
    }
}

//==================================================================================================
template <typename StringType>
void BasicString<StringType>::remove_all(StringType &target, const StringType &search)
{
    replace_all(target, search, StringType());
}

//==================================================================================================
template <typename StringType>
bool BasicString<StringType>::starts_with(const StringType &source, const char_type &search)
{
    bool result = false;

    if (!source.empty())
    {
        result = source[0] == search;
    }

    return result;
}

//==================================================================================================
template <typename StringType>
bool BasicString<StringType>::starts_with(const StringType &source, const StringType &search)
{
    bool result = false;

    const size_type source_sz = source.size();
    const size_type search_sz = search.size();

    if (source_sz >= search_sz)
    {
        result = source.compare(0, search_sz, search) == 0;
    }

    return result;
}

//==================================================================================================
template <typename StringType>
bool BasicString<StringType>::ends_with(const StringType &source, const char_type &search)
{
    bool result = false;

    const size_type source_sz = source.size();

    if (source_sz > 0)
    {
        result = source[source_sz - 1] == search;
    }

    return result;
}

//==================================================================================================
template <typename StringType>
bool BasicString<StringType>::ends_with(const StringType &source, const StringType &search)
{
    bool result = false;

    const size_type source_sz = source.size();
    const size_type search_sz = search.size();

    if (source_sz >= search_sz)
    {
        result = source.compare(source_sz - search_sz, search_sz, search) == 0;
    }

    return result;
}

//==================================================================================================
template <typename StringType>
bool BasicString<StringType>::wildcard_match(const StringType &source, const StringType &search)
{
    static constexpr char_type s_wildcard = '*';
    bool result = !search.empty();

    const std::vector<StringType> segments = split(search, s_wildcard);
    size_type index = 0;

    if (!segments.empty())
    {
        if (result && (search.front() != s_wildcard))
        {
            result = starts_with(source, segments.front());
        }
        if (result && (search.back() != s_wildcard))
        {
            result = ends_with(source, segments.back());
        }

        for (auto it = segments.begin(); result && (it != segments.end()); ++it)
        {
            index = source.find(*it, index);

            if (index == StringType::npos)
            {
                result = false;
            }
        }
    }

    return result;
}

//==================================================================================================
template <typename StringType>
bool BasicString<StringType>::validate(const StringType &value)
{
    auto it = value.cbegin();
    const auto end = value.cend();

    return unicode::validate_encoding(it, end);
}

//==================================================================================================
template <typename StringType>
template <typename IteratorType>
auto BasicString<StringType>::decode_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<codepoint_type>
{
    return unicode::decode_codepoint(it, end);
}

//==================================================================================================
template <typename StringType>
std::optional<StringType> BasicString<StringType>::encode_codepoint(codepoint_type codepoint)
{
    return unicode::encode_codepoint(codepoint);
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix>
std::optional<StringType> BasicString<StringType>::escape_all_codepoints(const StringType &value)
{
    StringType result;
    result.reserve(value.size());

    const auto end = value.cend();

    for (auto it = value.cbegin(); it != end;)
    {
        if (auto escaped = escape_codepoint<UnicodePrefix>(it, end); escaped)
        {
            result += *std::move(escaped);
        }
        else
        {
            return std::nullopt;
        }
    }

    return result;
}

//==================================================================================================
template <typename StringType>
template <char UnicodePrefix, typename IteratorType>
std::optional<StringType>
BasicString<StringType>::escape_codepoint(IteratorType &it, const IteratorType &end)
{
    return unicode::template escape_codepoint<UnicodePrefix>(it, end);
}

//==================================================================================================
template <typename StringType>
std::optional<StringType> BasicString<StringType>::unescape_all_codepoints(const StringType &value)
{
    StringType result;
    result.reserve(value.size());

    const auto end = value.cend();

    for (auto it = value.cbegin(); it != end;)
    {
        if ((*it == '\\') && ((it + 1) != end))
        {
            switch (*(it + 1))
            {
                case FLY_CHR(char_type, 'u'):
                case FLY_CHR(char_type, 'U'):
                {
                    if (auto unescaped = unescape_codepoint(it, end); unescaped)
                    {
                        result += *std::move(unescaped);
                    }
                    else
                    {
                        return std::nullopt;
                    }

                    break;
                }

                default:
                    result += *(it++);
                    break;
            }
        }
        else
        {
            result += *(it++);
        }
    }

    return result;
}

//==================================================================================================
template <typename StringType>
template <typename IteratorType>
std::optional<StringType>
BasicString<StringType>::unescape_codepoint(IteratorType &it, const IteratorType &end)
{
    return unicode::unescape_codepoint(it, end);
}

//==================================================================================================
template <typename StringType>
template <typename IntegerType>
StringType BasicString<StringType>::create_hex_string(IntegerType value, size_type length)
{
    return formatter::format_hex(value, length);
}

//==================================================================================================
template <typename StringType>
StringType BasicString<StringType>::generate_random_string(size_type length)
{
    using short_distribution = std::uniform_int_distribution<short>;

    constexpr auto limit = static_cast<short_distribution::result_type>(s_alpha_num_length - 1);
    static_assert(limit > 0);

    static thread_local const auto now = std::chrono::system_clock::now().time_since_epoch();
    static thread_local const auto seed = static_cast<std::mt19937::result_type>(now.count());

    static thread_local std::mt19937 engine(seed);
    short_distribution distribution(0, limit);

    StringType result;
    result.reserve(length);

    while (length-- != 0)
    {
        result.push_back(s_alpha_num[distribution(engine)]);
    }

    return result;
}

//==================================================================================================
template <typename StringType>
template <typename... Args>
auto BasicString<StringType>::format(const char_type *fmt, const Args &...args) -> streamed_type
{
    return formatter::format(fmt, args...);
}

//==================================================================================================
template <typename StringType>
template <typename... Args>
auto BasicString<StringType>::format(
    ostream_type &ostream,
    const char_type *fmt,
    const Args &...args) -> ostream_type &
{
    return formatter::format(ostream, fmt, args...);
}

//==================================================================================================
template <typename StringType>
template <typename... Args>
auto BasicString<StringType>::join(const char_type &separator, const Args &...args) -> streamed_type
{
    typename traits::ostringstream_type ostream;
    join_internal(ostream, separator, args...);

    return ostream.str();
}

//==================================================================================================
template <typename StringType>
template <typename T, typename... Args>
void BasicString<StringType>::join_internal(
    ostream_type &ostream,
    const char_type &separator,
    const T &value,
    const Args &...args)
{
    streamer::stream(ostream, value);
    streamer::stream(ostream, separator);

    join_internal(ostream, separator, args...);
}

//==================================================================================================
template <typename StringType>
template <typename T>
void BasicString<StringType>::join_internal(
    ostream_type &ostream,
    const char_type &,
    const T &value)
{
    streamer::stream(ostream, value);
}

//==================================================================================================
template <typename StringType>
template <typename T>
std::optional<T> BasicString<StringType>::convert(const StringType &value)
{
    using U = std::decay_t<T>;

    if constexpr (detail::is_supported_string_v<U>)
    {
        auto it = value.cbegin();
        const auto end = value.cend();

        if (auto result = unicode::template convert_encoding<U>(it, end); result)
        {
            return static_cast<T>(*result);
        }

        return std::nullopt;
    }
    else if constexpr (traits::has_stoi_family_v)
    {
        return detail::BasicStringConverter<StringType, T>::convert(value);
    }
    else
    {
        auto it = value.cbegin();
        const auto end = value.cend();

        if (auto result = unicode::template convert_encoding<streamed_type>(it, end); result)
        {
            return detail::BasicStringConverter<streamed_type, T>::convert(*result);
        }

        return std::nullopt;
    }
}

} // namespace fly
