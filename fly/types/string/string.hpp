#pragma once

#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_converter.hpp"
#include "fly/types/string/detail/string_formatter.hpp"
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
    using classifier = detail::BasicStringClassifier<StringType>;
    using traits = detail::BasicStringTraits<StringType>;
    using unicode = detail::BasicStringUnicode<StringType>;

public:
    using string_type = typename traits::string_type;
    using size_type = typename traits::size_type;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using int_type = typename traits::int_type;
    using codepoint_type = typename traits::codepoint_type;

    template <typename... ParameterTypes>
    using FormatString =
        detail::BasicFormatString<StringType, std::type_identity_t<ParameterTypes>...>;

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
    template <typename T, enable_if<detail::is_like_supported_string<T>> = 0>
    static constexpr size_type size(T &&value);

    /**
     * Checks if the given character is an alphabetic character as classified by the default C
     * locale.
     *
     * The STL's std::isalpha and std::iswalpha require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is an alphabetic character.
     */
    static constexpr bool is_alpha(char_type ch);

    /**
     * Checks if the given character is an upper-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std::isupper and std::iswupper require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is an alphabetic character.
     */
    static constexpr bool is_upper(char_type ch);

    /**
     * Checks if the given character is a lower-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std::islower and std::iswlower require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is an alphabetic character.
     */
    static constexpr bool is_lower(char_type ch);

    /**
     * Converts the given character to an upper-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std:tosupper and std::towupper require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to convert.
     *
     * @return The converted character.
     */
    static constexpr char_type to_upper(char_type ch);

    /**
     * Converts the given character to a lower-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std:toslower and std::towlower require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to convert.
     *
     * @return The converted character.
     */
    static constexpr char_type to_lower(char_type ch);

    /**
     * Checks if the given character is a decimal digit character.
     *
     * The STL's std::isdigit and std::iswdigit require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is a decimal digit character.
     */
    static constexpr bool is_digit(char_type ch);

    /**
     * Checks if the given character is a hexadecimal digit character.
     *
     * The STL's std::isxdigit and std::iswxdigit require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is a hexadecimal digit character.
     */
    static constexpr bool is_x_digit(char_type ch);

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
     * @return If successful, the decoded Unicode codepoint. Otherwise, an uninitialized value.
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
     *         uninitialized value.
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
     *         Otherwise, an uninitialized value.
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
     *         uninitialized value.
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
     *         Otherwise, an uninitialized value.
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
     *         uninitialized value.
     */
    template <typename IteratorType>
    static std::optional<StringType> unescape_codepoint(IteratorType &it, const IteratorType &end);

    /**
     * Generate a random string of the given length.
     *
     * @param length The length of the string to generate.
     *
     * @return The generated string.
     */
    static StringType generate_random_string(size_type length);

    /**
     * Format a string with a set of format parameters, returning the formatted string. Based
     * strongly upon: https://en.cppreference.com/w/cpp/utility/format/format.
     *
     * A format string consists of:
     *
     *     1. Any character other than "{" or "}", which are copied unchanged to the output.
     *     2. Escape sequences "{{" and "}}", which are replaced with "{" and "}" in the output.
     *     3. Replacement fields.
     *
     * Replacement fields may be of the form:
     *
     *     1. An introductory "{" character.
     *     2. An optional non-negative position.
     *     3. An optional colon ":" following by formatting options.
     *     4. A final "}" character.
     *
     * For a detailed description of replacement fields see fly::detail::BasicFormatSpecifier.
     *
     * This implementation differs from std::format in the following ways:
     *
     *    1. All standard string types are supported as format strings.
     *
     *    2. All standard string types are supported as format parameters, even if that type differs
     *       from the format string type. If the type differs, the format parameter is transcoded to
     *       the type of the format string.
     *
     *    3. Any generic type for which an operator<< overload is defined will be converted to a
     *       string using that overload.
     *
     *    4. Formatting of strong enumeration types defaults to the format of the enumeration's
     *       underlying type. However, if an overload of operator<< is defined, the type is treated
     *       as a generic type according to (3) above.
     *
     *    5. This implementation is exceptionless. Any error encountered (such as failed transcoding
     *       in (2) above) results in the format parameter that caused the error to be dropped.
     *
     *    6. Locale-specific form is not supported. If the option appears in the format string, it
     *       will be parsed, but will be ignored.
     *
     * The format string type is implicitly constructed from a C-string literal. Callers should only
     * invoke this method accordingly:
     *
     *     format("Format {:d}", 1);
     *
     * On compilers that support immediate functions (consteval), the format string is validated at
     * compile time against the types of the format parameters. If the format string is invalid, a
     * compile error with a diagnostic message will be raised. On other compilers, the error message
     * will be returned rather than a formatted string.
     *
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param fmt The string to format.
     * @param parameters The variadic list of format parameters to be formatted.
     *
     * @return A string that has been formatted with the given format parameters.
     */
    template <typename... ParameterTypes>
    static StringType format(FormatString<ParameterTypes...> &&fmt, ParameterTypes &&...parameters);

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
    static StringType join(char_type separator, Args &&...args);

    /**
     * Convert a string to another type. The other type may be a string with a different Unicode
     * encoding or a plain-old-data type, e.g. int or bool.
     *
     * @tparam T The desired type.
     *
     * @param value The string to convert.
     *
     * @return If successful, the string coverted to the specified type. Otherwise, an uninitialized
     *         value.
     */
    template <typename T>
    static std::optional<T> convert(const StringType &value);

private:
    /**
     * Recursively join one argument into the given string.
     */
    template <typename T, typename... Args>
    static void join_internal(StringType &result, char_type separator, T &&value, Args &&...args);

    /**
     * Terminator for the variadic template joiner. Join the last argument into the given string.
     */
    template <typename T>
    static void join_internal(StringType &result, char_type separator, T &&value);

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
template <typename T, enable_if<detail::is_like_supported_string<T>>>
constexpr auto BasicString<StringType>::size(T &&value) -> size_type
{
    return classifier::size(std::forward<T>(value));
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicString<StringType>::is_alpha(char_type ch)
{
    return classifier::is_alpha(ch);
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicString<StringType>::is_upper(char_type ch)
{
    return classifier::is_upper(ch);
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicString<StringType>::is_lower(char_type ch)
{
    return classifier::is_lower(ch);
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicString<StringType>::is_digit(char_type ch)
{
    return classifier::is_digit(ch);
}

//==================================================================================================
template <typename StringType>
constexpr inline auto BasicString<StringType>::to_upper(char_type ch) -> char_type
{
    return classifier::to_upper(ch);
}

//==================================================================================================
template <typename StringType>
constexpr inline auto BasicString<StringType>::to_lower(char_type ch) -> char_type
{
    return classifier::to_lower(ch);
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicString<StringType>::is_x_digit(char_type ch)
{
    return classifier::is_x_digit(ch);
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
            result = source.starts_with(segments.front());
        }
        if (result && (search.back() != s_wildcard))
        {
            result = source.ends_with(segments.back());
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
inline bool BasicString<StringType>::validate(const StringType &value)
{
    auto it = value.cbegin();
    const auto end = value.cend();

    return unicode::validate_encoding(it, end);
}

//==================================================================================================
template <typename StringType>
template <typename IteratorType>
inline auto BasicString<StringType>::decode_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<codepoint_type>
{
    return unicode::decode_codepoint(it, end);
}

//==================================================================================================
template <typename StringType>
inline std::optional<StringType> BasicString<StringType>::encode_codepoint(codepoint_type codepoint)
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
inline std::optional<StringType>
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
inline std::optional<StringType>
BasicString<StringType>::unescape_codepoint(IteratorType &it, const IteratorType &end)
{
    return unicode::unescape_codepoint(it, end);
}

//==================================================================================================
template <typename StringType>
StringType BasicString<StringType>::generate_random_string(size_type length)
{
    using short_distribution = std::uniform_int_distribution<short>;

    constexpr auto limit = static_cast<short_distribution::result_type>(s_alpha_num_length - 1);
    static_assert(limit > 0);

    static thread_local const auto s_now = std::chrono::system_clock::now().time_since_epoch();
    static thread_local const auto s_seed = static_cast<std::mt19937::result_type>(s_now.count());

    static thread_local std::mt19937 s_engine(s_seed);
    short_distribution distribution(0, limit);

    StringType result;
    result.reserve(length);

    while (length-- != 0)
    {
        result.push_back(s_alpha_num[distribution(s_engine)]);
    }

    return result;
}

//==================================================================================================
template <typename StringType>
template <typename... ParameterTypes>
inline StringType BasicString<StringType>::format(
    FormatString<ParameterTypes...> &&fmt,
    ParameterTypes &&...parameters)
{
    if (fmt.has_error())
    {
        return format(FLY_ARR(char_type, "Ignored invalid formatter: {}"), fmt.error());
    }

    detail::BasicStringFormatter<StringType, ParameterTypes...> formatter(
        std::forward<ParameterTypes>(parameters)...);

    return formatter.format(std::forward<FormatString<ParameterTypes...>>(fmt));
}

//==================================================================================================
template <typename StringType>
template <typename... Args>
inline StringType BasicString<StringType>::join(char_type separator, Args &&...args)
{
    StringType result;
    join_internal(result, separator, std::forward<Args>(args)...);

    return result;
}

//==================================================================================================
template <typename StringType>
template <typename T, typename... Args>
inline void BasicString<StringType>::join_internal(
    StringType &result,
    char_type separator,
    T &&value,
    Args &&...args)
{
    result += format(FLY_ARR(char_type, "{}{}"), std::forward<T>(value), separator);
    join_internal(result, separator, std::forward<Args>(args)...);
}

//==================================================================================================
template <typename StringType>
template <typename T>
inline void BasicString<StringType>::join_internal(StringType &result, char_type, T &&value)
{
    result += format(FLY_ARR(char_type, "{}"), std::forward<T>(value));
}

//==================================================================================================
template <typename StringType>
template <typename T>
std::optional<T> BasicString<StringType>::convert(const StringType &value)
{
    if constexpr (detail::is_supported_string_v<T>)
    {
        return unicode::template convert_encoding<T>(value);
    }
    else if constexpr (std::is_same_v<char_type, char>)
    {
        return detail::BasicStringConverter<StringType, T>::convert(value);
    }
    else
    {
        if (auto result = unicode::template convert_encoding<std::string>(value); result)
        {
            return detail::BasicStringConverter<std::string, T>::convert(*result);
        }

        return std::nullopt;
    }
}

} // namespace fly
