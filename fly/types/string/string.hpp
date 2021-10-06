#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/string/detail/classifier.hpp"
#include "fly/types/string/detail/converter.hpp"
#include "fly/types/string/detail/format_context.hpp"
#include "fly/types/string/detail/format_parameters.hpp"
#include "fly/types/string/detail/format_parse_context.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/format_string.hpp"
#include "fly/types/string/detail/string_concepts.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/detail/unicode.hpp"
#include "fly/types/string/formatters.hpp"
#include "fly/types/string/literals.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ios>
#include <iterator>
#include <optional>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace fly {

template <typename CharType>
class BasicString;

using String = BasicString<char>;
using WString = BasicString<wchar_t>;
using String8 = BasicString<char8_t>;
using String16 = BasicString<char16_t>;
using String32 = BasicString<char32_t>;

/**
 * Static class to provide string utilities not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename CharType>
class BasicString
{
    using traits = detail::BasicStringTraits<CharType>;
    using unicode = detail::BasicUnicode<CharType>;

public:
    using string_type = typename traits::string_type;
    using size_type = typename traits::size_type;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using int_type = typename traits::int_type;
    using codepoint_type = typename traits::codepoint_type;

    template <typename... ParameterTypes>
    using FormatString =
        detail::BasicFormatString<char_type, std::type_identity_t<ParameterTypes>...>;

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
    template <detail::StandardStringLike T>
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
     * The STL's std:toupper and std::towupper require that the provided character fits into an
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
     * The STL's std:tolower and std::towlower require that the provided character fits into an
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
     * Checks if the given character is a whitespace character as classified by the default C
     * locale.
     *
     * The STL's std::isspace and std::iswspace require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is a whitespace character.
     */
    static constexpr bool is_space(char_type ch);

    /**
     * Split a string into a vector of strings.
     *
     * @param input The string to split.
     * @param delimiter The delimiter to split the string on.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<string_type> split(view_type input, char_type delimiter);

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
    static std::vector<string_type> split(view_type input, char_type delimiter, size_type count);

    /**
     * Remove leading and trailing whitespace from a string.
     *
     * @param target The string to trim.
     */
    static void trim(string_type &target);

    /**
     * Replace all instances of a substring in a string with a character.
     *
     * @param target The string container which will be modified.
     * @param search The string to search for and replace.
     * @param replace The replacement character.
     */
    static void replace_all(string_type &target, view_type search, char_type replace);

    /**
     * Replace all instances of a substring in a string with another string.
     *
     * @param target The string container which will be modified.
     * @param search The string to search for and replace.
     * @param replace The replacement string.
     */
    static void replace_all(string_type &target, view_type search, view_type replace);

    /**
     * Remove all instances of a substring in a string.
     *
     * @param target The string container which will be modified.
     * @param search The string to search for and remove.
     */
    static void remove_all(string_type &target, view_type search);

    /**
     * Check if a string matches another string with wildcard expansion.
     *
     * @param source The source string to match against.
     * @param search The wildcard string to search with.
     *
     * @return True if the wildcard string matches the source string.
     */
    static bool wildcard_match(view_type source, view_type search);

    /**
     * Validate that a string is strictly Unicode compliant.
     *
     * @param value The string to validate.
     *
     * @return True if the string is Unicode compliant.
     */
    static bool validate(view_type value);

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
    static std::optional<string_type> encode_codepoint(codepoint_type codepoint);

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
    static std::optional<string_type> escape_all_codepoints(view_type value);

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
    static std::optional<string_type> unescape_all_codepoints(view_type value);

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

    /**
     * Generate a random string of the given length.
     *
     * @param length The length of the string to generate.
     *
     * @return The generated string.
     */
    static string_type generate_random_string(size_type length);

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
     * For a detailed description of replacement fields, see fly::detail::BasicFormatSpecifier.
     *
     * This implementation differs from std::format in the following ways:
     *
     *    1. All standard string types are supported as format strings.
     *
     *    2. All standard string types are supported as format parameters, even if that type differs
     *       from the format string type. If the type differs, the format parameter is transcoded to
     *       the type of the format string.
     *
     *    3. This implementation is exceptionless. Any error encountered (such as failed transcoding
     *       in (2) above) results in the format parameter that caused the error to be dropped.
     *
     *    4. Locale-specific form is not supported. If the option appears in the format string, it
     *       will be parsed, but will be ignored.
     *
     * The format string type is implicitly constructed from a C-string literal. Callers should only
     * invoke this method accordingly:
     *
     *     fly::String::format("Format {:d}", 1);
     *
     * On compilers that support immediate functions (consteval), the format string is validated at
     * compile time against the types of the format parameters. If the format string is invalid, a
     * compile error with a diagnostic message will be raised. On other compilers, the error message
     * will be returned rather than a formatted string.
     *
     * Replacement fields for user-defined types are parsed at runtime. To format a user-defined
     * type, a fly::Formatter specialization must be defined, analagous to std::formatter. The
     * specialization may extend a standard fly::Formatter, for example:
     *
     *     template <typename CharType>
     *     struct fly::Formatter<MyType, CharType> : public fly::Formatter<int, CharType>
     *     {
     *         template <typename FormatContext>
     *         void format(const MyType &value, FormatContext &context)
     *         {
     *             fly::Formatter<int, CharType>::format(value.as_int(), context);
     *         }
     *     };
     *
     * Or, the formatter may be defined without without inheritence:
     *
     *     template <typename CharType>
     *     struct fly::Formatter<MyType, CharType>
     *     {
     *         bool m_option {false};
     *
     *         template <typename FormatParseContext>
     *         constexpr void parse(FormatParseContext &context)
     *         {
     *             if (context.lexer().consume_if(FLY_CHR(CharType, 'o')))
     *             {
     *                 m_option = true;
     *             }
     *             if (!context.lexer().consume_if(FLY_CHR(CharType, '}')))
     *             {
     *                 context.on_error("UserDefinedTypeWithParser error!");
     *             }
     *         }
     *
     *         template <typename FormatContext>
     *         void format(const MyType &value, FormatContext &context)
     *         {
     *             fly::BasicString<CharType>::format_to(context.out(), "{}", value.as_int());
     *         }
     *     };
     *
     * The |parse| method is optional. If defined, it is provided a BasicFormatParseContext which
     * contains a lexer that may be used to parse the format string. The lexer is positioned such
     * that it is pointed at the first character after the ":" in the replacement field (if there is
     * one), or after the opening "{" character. The |parse| method is expected to consume up to and
     * including the closing "}" character. It may indicate any parsing errors through the parsing
     * context; if an error occurs, the error is written to the formatted string, and formatting
     * will halt.
     *
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param fmt The string to format.
     * @param parameters The variadic list of format parameters to be formatted.
     *
     * @return A string that has been formatted with the given format parameters.
     */
    template <typename... ParameterTypes>
    static string_type
    format(FormatString<ParameterTypes...> &&fmt, ParameterTypes &&...parameters);

    /**
     * Format a string with a set of format parameters to an existing output iterator. Based
     * strongly upon: https://en.cppreference.com/w/cpp/utility/format/format.
     *
     * For a detailed description of string formatting, see fly::BasicString::format.
     *
     * @tparam OutputIterator The type of the output iterator.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param output The output iterator to write to.
     * @param fmt The string to format.
     * @param parameters The variadic list of format parameters to be formatted.
     *
     * @return A string that has been formatted with the given format parameters.
     */
    template <typename OutputIterator, typename... ParameterTypes>
    static void format_to(
        OutputIterator output,
        FormatString<ParameterTypes...> &&fmt,
        ParameterTypes &&...parameters);

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
    static string_type join(char_type separator, Args &&...args);

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
    static std::optional<T> convert(const string_type &value);

private:
    /**
     * Recursively join one argument into the given string.
     */
    template <typename T, typename... Args>
    static void join_internal(string_type &result, char_type separator, T &&value, Args &&...args);

    /**
     * Terminator for the variadic template joiner. Join the last argument into the given string.
     */
    template <typename T>
    static void join_internal(string_type &result, char_type separator, T &&value);

    /**
     * A list of alpha-numeric characters in the range [0-9A-Za-z].
     */
    static constexpr const char_type *s_alpha_num =
        FLY_STR(char_type, "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    static constexpr size_type s_alpha_num_length =
        std::char_traits<char_type>::length(s_alpha_num);

    static constexpr const auto s_left_brace = FLY_CHR(char_type, '{');
    static constexpr const auto s_right_brace = FLY_CHR(char_type, '}');
};

//==================================================================================================
template <typename CharType>
template <detail::StandardStringLike T>
constexpr auto BasicString<CharType>::size(T &&value) -> size_type
{
    return detail::BasicClassifier<char_type>::size(std::forward<T>(value));
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicString<CharType>::is_alpha(char_type ch)
{
    return detail::BasicClassifier<char_type>::is_alpha(ch);
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicString<CharType>::is_upper(char_type ch)
{
    return detail::BasicClassifier<char_type>::is_upper(ch);
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicString<CharType>::is_lower(char_type ch)
{
    return detail::BasicClassifier<char_type>::is_lower(ch);
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicString<CharType>::is_digit(char_type ch)
{
    return detail::BasicClassifier<char_type>::is_digit(ch);
}

//==================================================================================================
template <typename CharType>
constexpr auto BasicString<CharType>::to_upper(char_type ch) -> char_type
{
    return detail::BasicClassifier<char_type>::to_upper(ch);
}

//==================================================================================================
template <typename CharType>
constexpr auto BasicString<CharType>::to_lower(char_type ch) -> char_type
{
    return detail::BasicClassifier<char_type>::to_lower(ch);
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicString<CharType>::is_x_digit(char_type ch)
{
    return detail::BasicClassifier<char_type>::is_x_digit(ch);
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicString<CharType>::is_space(char_type ch)
{
    return detail::BasicClassifier<char_type>::is_space(ch);
}

//==================================================================================================
template <typename CharType>
auto BasicString<CharType>::split(view_type input, char_type delimiter) -> std::vector<string_type>
{
    return split(input, delimiter, 0);
}

//==================================================================================================
template <typename CharType>
auto BasicString<CharType>::split(view_type input, char_type delimiter, size_type count)
    -> std::vector<string_type>
{
    std::vector<string_type> elements;
    string_type item;

    size_type start = 0;
    size_type end = input.find(delimiter);

    auto push_item = [&elements, &count, &delimiter](view_type str)
    {
        if (!str.empty())
        {
            if ((count > 0) && (elements.size() == count))
            {
                elements.back() += delimiter;
                elements.back() += str;
            }
            else
            {
                elements.push_back(string_type(str));
            }
        }
    };

    while (end != string_type::npos)
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
template <typename CharType>
void BasicString<CharType>::trim(string_type &target)
{
    auto is_non_space = [](auto ch)
    {
        return !is_space(ch);
    };

    // Remove leading whitespace.
    target.erase(target.begin(), std::find_if(target.begin(), target.end(), is_non_space));

    // Remove trailing whitespace.
    target.erase(std::find_if(target.rbegin(), target.rend(), is_non_space).base(), target.end());
}

//==================================================================================================
template <typename CharType>
void BasicString<CharType>::replace_all(string_type &target, view_type search, char_type replace)
{
    size_type index = target.find(search);

    while (!search.empty() && (index != string_type::npos))
    {
        target.replace(index, search.size(), 1, replace);
        index = target.find(search);
    }
}

//==================================================================================================
template <typename CharType>
void BasicString<CharType>::replace_all(string_type &target, view_type search, view_type replace)
{
    size_type index = target.find(search);

    while (!search.empty() && (index != string_type::npos))
    {
        target.replace(index, search.size(), replace);
        index = target.find(search);
    }
}

//==================================================================================================
template <typename CharType>
void BasicString<CharType>::remove_all(string_type &target, view_type search)
{
    replace_all(target, search, view_type {});
}

//==================================================================================================
template <typename CharType>
bool BasicString<CharType>::wildcard_match(view_type source, view_type search)
{
    static constexpr char_type s_wildcard = '*';
    bool result = !search.empty();

    const std::vector<string_type> segments = split(search, s_wildcard);
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

            if (index == string_type::npos)
            {
                result = false;
            }
        }
    }

    return result;
}

//==================================================================================================
template <typename CharType>
inline bool BasicString<CharType>::validate(view_type value)
{
    auto it = value.cbegin();
    const auto end = value.cend();

    return unicode::validate_encoding(it, end);
}

//==================================================================================================
template <typename CharType>
template <typename IteratorType>
inline auto BasicString<CharType>::decode_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<codepoint_type>
{
    return unicode::decode_codepoint(it, end);
}

//==================================================================================================
template <typename CharType>
inline auto BasicString<CharType>::encode_codepoint(codepoint_type codepoint)
    -> std::optional<string_type>
{
    return unicode::encode_codepoint(codepoint);
}

//==================================================================================================
template <typename CharType>
template <char UnicodePrefix>
auto BasicString<CharType>::escape_all_codepoints(view_type value) -> std::optional<string_type>
{
    string_type result;
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
template <typename CharType>
template <char UnicodePrefix, typename IteratorType>
inline auto BasicString<CharType>::escape_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<string_type>
{
    return unicode::template escape_codepoint<UnicodePrefix>(it, end);
}

//==================================================================================================
template <typename CharType>
auto BasicString<CharType>::unescape_all_codepoints(view_type value) -> std::optional<string_type>
{
    string_type result;
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
template <typename CharType>
template <typename IteratorType>
inline auto BasicString<CharType>::unescape_codepoint(IteratorType &it, const IteratorType &end)
    -> std::optional<string_type>
{
    return unicode::unescape_codepoint(it, end);
}

//==================================================================================================
template <typename CharType>
auto BasicString<CharType>::generate_random_string(size_type length) -> string_type
{
    using short_distribution = std::uniform_int_distribution<short>;

    constexpr auto limit = static_cast<short_distribution::result_type>(s_alpha_num_length - 1);
    static_assert(limit > 0);

    static thread_local const auto s_now = std::chrono::system_clock::now().time_since_epoch();
    static thread_local const auto s_seed = static_cast<std::mt19937::result_type>(s_now.count());

    static thread_local std::mt19937 s_engine(s_seed);
    short_distribution distribution(0, limit);

    string_type result;
    result.reserve(length);

    while (length-- != 0)
    {
        result.push_back(s_alpha_num[distribution(s_engine)]);
    }

    return result;
}

//==================================================================================================
template <typename CharType>
template <typename... ParameterTypes>
inline auto
BasicString<CharType>::format(FormatString<ParameterTypes...> &&fmt, ParameterTypes &&...parameters)
    -> string_type
{
    string_type formatted;
    formatted.reserve(fmt.context().view().size() * 2);

    format_to(
        std::back_inserter(formatted),
        std::move(fmt),
        std::forward<ParameterTypes>(parameters)...);

    return formatted;
}

//==================================================================================================
template <typename CharType>
template <typename OutputIterator, typename... ParameterTypes>
void BasicString<CharType>::format_to(
    OutputIterator output,
    FormatString<ParameterTypes...> &&fmt,
    ParameterTypes &&...parameters)
{
    using FormatContext = detail::BasicFormatContext<OutputIterator, char_type>;
    using FormatParseContext = detail::BasicFormatParseContext<char_type>;

    FormatParseContext &parse_context = fmt.context();
    const view_type view = parse_context.view();

    if (parse_context.has_error())
    {
        format_to(
            output,
            FLY_ARR(char_type, "Ignored invalid formatter: {}"),
            parse_context.error());

        return;
    }

    auto params =
        detail::make_format_parameters<FormatContext>(std::forward<ParameterTypes>(parameters)...);
    FormatContext context(output, params);

    for (std::size_t pos = 0; pos < view.size();)
    {
        switch (const auto &ch = view[pos])
        {
            case s_left_brace:
                if (view[pos + 1] == s_left_brace)
                {
                    *context.out()++ = ch;
                    pos += 2;
                }
                else
                {
                    auto specifier = *std::move(fmt.next_specifier());
                    pos += specifier.m_size;

                    const auto parameter = context.arg(specifier.m_position);
                    parameter.format(parse_context, context, std::move(specifier));

                    if (parse_context.has_error())
                    {
                        format_to(output, FLY_ARR(char_type, "{}"), parse_context.error());
                        return;
                    }
                }
                break;

            case s_right_brace:
                *context.out()++ = ch;
                pos += 2;
                break;

            default:
                *context.out()++ = ch;
                ++pos;
                break;
        }
    }
}

//==================================================================================================
template <typename CharType>
template <typename... Args>
inline auto BasicString<CharType>::join(char_type separator, Args &&...args) -> string_type
{
    string_type result;
    join_internal(result, separator, std::forward<Args>(args)...);

    return result;
}

//==================================================================================================
template <typename CharType>
template <typename T, typename... Args>
inline void BasicString<CharType>::join_internal(
    string_type &result,
    char_type separator,
    T &&value,
    Args &&...args)
{
    result += format(FLY_ARR(char_type, "{}{}"), std::forward<T>(value), separator);
    join_internal(result, separator, std::forward<Args>(args)...);
}

//==================================================================================================
template <typename CharType>
template <typename T>
inline void BasicString<CharType>::join_internal(string_type &result, char_type, T &&value)
{
    result += format(FLY_ARR(char_type, "{}"), std::forward<T>(value));
}

//==================================================================================================
template <typename CharType>
template <typename T>
std::optional<T> BasicString<CharType>::convert(const string_type &value)
{
    if constexpr (detail::StandardString<T>)
    {
        return unicode::template convert_encoding<T>(value);
    }
    else if constexpr (fly::SameAs<char_type, char>)
    {
        return detail::Converter<T>::convert(value);
    }
    else
    {
        if (auto result = unicode::template convert_encoding<std::string>(value); result)
        {
            return detail::Converter<T>::convert(*result);
        }

        return std::nullopt;
    }
}

} // namespace fly
