#pragma once

#include "fly/types/string/detail/string_converter.h"
#include "fly/types/string/detail/string_streamer.h"
#include "fly/types/string/detail/string_traits.h"
#include "fly/types/string/string_literal.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ios>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

namespace fly {

/**
 * Forward declarations of the supported BasicString<> specializations.
 */
template <typename StringType>
class BasicString;

using String = BasicString<std::string>;
using WString = BasicString<std::wstring>;
using String16 = BasicString<std::u16string>;
using String32 = BasicString<std::u32string>;

/**
 * Static class to provide string utilities not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version March 21, 2019
 */
template <typename StringType>
class BasicString
{
public:
    // Forward some aliases from detail::BasicStringTraits<> for convenience.
    using traits = detail::BasicStringTraits<StringType>;
    using size_type = typename traits::size_type;
    using char_type = typename traits::char_type;
    using ostream_type = typename traits::ostream_type;
    using streamed_type = typename traits::streamer_type::streamed_type;

    /**
     * Split a string into a vector of strings.
     *
     * @param StringType The string to split.
     * @param char_type The delimiter to split the string on.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<StringType>
    Split(const StringType &, char_type) noexcept;

    /**
     * Split a string into a vector of strings, up to a maximum size. If the max
     * size is reached, the rest of the string is appended to the last element
     * in the vector.
     *
     * @param StringType The string to split.
     * @param char_type The delimiter to split the string on.
     * @param uint32_t The maximum return vector size. Zero implies unlimited.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<StringType>
    Split(const StringType &, char_type, std::uint32_t) noexcept;

    /**
     * Remove leading and trailing whitespace from a string.
     *
     * @param StringType The string to trim.
     */
    static void Trim(StringType &) noexcept;

    /**
     * Replace all instances of a substring in a string with a character.
     *
     * @param StringType The string container which will be modified.
     * @param StringType The string to search for and replace.
     * @param char_type The replacement character.
     */
    static void
    ReplaceAll(StringType &, const StringType &, const char_type &) noexcept;

    /**
     * Replace all instances of a substring in a string with another string.
     *
     * @param StringType The string container which will be modified.
     * @param StringType The string to search for and replace.
     * @param StringType The replacement string.
     */
    static void
    ReplaceAll(StringType &, const StringType &, const StringType &) noexcept;

    /**
     * Remove all instances of a substring in a string.
     *
     * @param StringType The string container which will be modified.
     * @param StringType The string to search for and remove.
     */
    static void RemoveAll(StringType &, const StringType &) noexcept;

    /**
     * Check if a string begins with a character.
     *
     * @param StringType The string to check.
     * @param char_type The beginning to search for.
     *
     * @return True if the string begins with the search character.
     */
    static bool StartsWith(const StringType &, const char_type &) noexcept;

    /**
     * Check if a string begins with another string.
     *
     * @param StringType The string to check.
     * @param StringType The beginning to search for.
     *
     * @return True if the string begins with the search string.
     */
    static bool StartsWith(const StringType &, const StringType &) noexcept;

    /**
     * Check if a string ends with a character.
     *
     * @param StringType The string to check.
     * @param char_type The ending to search for.
     *
     * @return True if the string ends with the search character.
     */
    static bool EndsWith(const StringType &, const char_type &) noexcept;

    /**
     * Check if a string ends with another string.
     *
     * @param StringType The string to check.
     * @param StringType The ending to search for.
     *
     * @return True if the string ends with the search string.
     */
    static bool EndsWith(const StringType &, const StringType &) noexcept;

    /**
     * Check if a string matches another string with wildcard expansion.
     *
     * @param StringType The source string to match against.
     * @param StringType The wildcard string to search with.
     *
     * @return True if the wildcard string matches the source string.
     */
    static bool WildcardMatch(const StringType &, const StringType &) noexcept;

    /**
     * Generate a random string of the given size.
     *
     * @param size_type The length of the string to generate.
     *
     * @return The generated string.
     */
    static StringType GenerateRandomString(const size_type) noexcept;

    /**
     * Format a string with variadic template arguments, returning the formatted
     * string.
     *
     * This is type safe in that argument types need not match the format
     * specifier (i.e. there is no error if %s is given an integer). However,
     * specifiers such as %x are still attempted to be handled. That is, if the
     * matching argument for %x is numeric, then it will be converted to a
     * hexadecimal representation.
     *
     * There is also no checking done on the number of format specifiers and
     * the number of arguments. The format specifiers will be replaced one at a
     * time until all arguments are exhausted, then the rest of the string is
     * taken as-is. Any extra specifiers will be in the string. Any extra
     * arguments are dropped.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param char_type* The string to format.
     * @param Args The variadic list of arguments to be formatted.
     *
     * @return A string that has been formatted with the given arguments.
     */
    template <typename... Args>
    static streamed_type Format(const char_type *, const Args &...) noexcept;

    /**
     * Format a string with variadic template arguments, inserting the formatted
     * string into a stream.
     *
     * This is type safe in that argument types need not match the format
     * specifier (i.e. there is no error if %s is given an integer). However,
     * specifiers such as %x are still attempted to be handled. That is, if the
     * matching argument for %x is numeric, then it will be converted to a
     * hexadecimal representation.
     *
     * There is also no checking done on the number of format specifiers and
     * the number of arguments. The format specifiers will be replaced one at a
     * time until all arguments are exhausted, then the rest of the string is
     * taken as-is. Any extra specifiers will be in the string. Any extra
     * arguments are dropped.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param ostream The stream to insert the formatted string into.
     * @param char* The string to format.
     * @param Args The variadic list of arguments to be formatted.
     *
     * @return The same stream object.
     */
    template <typename... Args>
    static ostream_type &
    Format(ostream_type &, const char_type *, const Args &...) noexcept;

    /**
     * Concatenate a list of objects with the given separator.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param char_type Character to use as a separator.
     * @param Args The variadic list of arguments to be joined.
     *
     * @return The resulting join of the given arguments.
     */
    template <typename... Args>
    static streamed_type Join(const char_type &, const Args &...) noexcept;

    /**
     * Convert a string to a plain-old-data type, e.g. int or bool.
     *
     * @tparam T The desired plain-old-data type.
     *
     * @param StringType The string to convert.
     *
     * @return The string coverted to the specified type.
     *
     * @throws std::invalid_argument Conversion could not be performed.
     * @throws std::out_of_range Converted value is out of range of result type.
     */
    template <typename T>
    static T Convert(const StringType &) noexcept(
        std::is_same_v<StringType, std::decay_t<T>>);

private:
    /**
     * Recursively format a string with one argument. The result is streamed
     * into the given ostream.
     */
    template <typename T, typename... Args>
    static void format(
        ostream_type &,
        const char_type *,
        const T &,
        const Args &...) noexcept;

    /**
     * Terminator for the variadic template formatter. Stream the rest of the
     * string into the given ostream.
     */
    static void format(ostream_type &, const char_type *) noexcept;

    /**
     * Recursively join one argument into the given ostream.
     */
    template <typename T, typename... Args>
    static void join(
        ostream_type &,
        const char_type &,
        const T &,
        const Args &...) noexcept;

    /**
     * Terminator for the variadic template joiner. Join the last argument
     * into the given ostream.
     */
    template <typename T>
    static void join(ostream_type &, const char_type &, const T &) noexcept;

    /**
     * Stream the given value into the given stream.
     */
    template <typename T>
    static void stream(ostream_type &, const T &) noexcept;

    /**
     * A list of alpha-numeric characters in the range [0-9A-Za-z].
     */
    static constexpr const char_type *s_alphaNum = FLY_STR(
        char_type,
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz");

    static constexpr std::size_t s_alphaNumLength =
        std::char_traits<char_type>::length(s_alphaNum);
};

//==============================================================================
template <typename StringType>
std::vector<StringType> BasicString<StringType>::Split(
    const StringType &input,
    char_type delimiter) noexcept
{
    return Split(input, delimiter, 0);
}

//==============================================================================
template <typename StringType>
std::vector<StringType> BasicString<StringType>::Split(
    const StringType &input,
    char_type delimiter,
    std::uint32_t max) noexcept
{
    std::vector<StringType> elems;
    std::uint32_t numItems = 0;
    StringType item;

    size_type start = 0;
    size_type end = input.find(delimiter);

    auto push_item = [&](const StringType &str) {
        if (!str.empty())
        {
            if ((max > 0) && (++numItems > max))
            {
                elems.back() += delimiter;
                elems.back() += str;
            }
            else
            {
                elems.push_back(str);
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

    return elems;
}

//==============================================================================
template <typename StringType>
void BasicString<StringType>::Trim(StringType &str) noexcept
{
    auto is_non_space = [](int ch) { return !std::isspace(ch); };

    // Remove leading whitespace
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), is_non_space));

    // Remove trailing whitespace
    str.erase(
        std::find_if(str.rbegin(), str.rend(), is_non_space).base(), str.end());
}

//==============================================================================
template <typename StringType>
void BasicString<StringType>::ReplaceAll(
    StringType &target,
    const StringType &search,
    const char_type &replace) noexcept
{
    size_type pos = target.find(search);

    while (!search.empty() && (pos != StringType::npos))
    {
        target.replace(pos, search.length(), 1, replace);
        pos = target.find(search);
    }
}

//==============================================================================
template <typename StringType>
void BasicString<StringType>::ReplaceAll(
    StringType &target,
    const StringType &search,
    const StringType &replace) noexcept
{
    size_type pos = target.find(search);

    while (!search.empty() && (pos != StringType::npos))
    {
        target.replace(pos, search.length(), replace);
        pos = target.find(search);
    }
}

//==============================================================================
template <typename StringType>
void BasicString<StringType>::RemoveAll(
    StringType &target,
    const StringType &search) noexcept
{
    ReplaceAll(target, search, StringType());
}

//==============================================================================
template <typename StringType>
bool BasicString<StringType>::StartsWith(
    const StringType &source,
    const char_type &search) noexcept
{
    bool ret = false;

    if (!source.empty())
    {
        ret = (source[0] == search);
    }

    return ret;
}

//==============================================================================
template <typename StringType>
bool BasicString<StringType>::StartsWith(
    const StringType &source,
    const StringType &search) noexcept
{
    bool ret = false;

    const size_type sourceSz = source.length();
    const size_type searchSz = search.length();

    if (sourceSz >= searchSz)
    {
        ret = (source.compare(0, searchSz, search) == 0);
    }

    return ret;
}

//==============================================================================
template <typename StringType>
bool BasicString<StringType>::EndsWith(
    const StringType &source,
    const char_type &search) noexcept
{
    bool ret = false;

    const size_type sourceSz = source.length();

    if (sourceSz > 0)
    {
        ret = (source[sourceSz - 1] == search);
    }

    return ret;
}

//==============================================================================
template <typename StringType>
bool BasicString<StringType>::EndsWith(
    const StringType &source,
    const StringType &search) noexcept
{
    bool ret = false;

    const size_type sourceSz = source.length();
    const size_type searchSz = search.length();

    if (sourceSz >= searchSz)
    {
        ret = (source.compare(sourceSz - searchSz, searchSz, search) == 0);
    }

    return ret;
}

//==============================================================================
template <typename StringType>
bool BasicString<StringType>::WildcardMatch(
    const StringType &source,
    const StringType &search) noexcept
{
    static constexpr char_type wildcard = '*';
    bool ret = !search.empty();

    const std::vector<StringType> segments = Split(search, wildcard);
    size_type pos = 0;

    if (!segments.empty())
    {
        if (ret && (search.front() != wildcard))
        {
            ret = StartsWith(source, segments.front());
        }
        if (ret && (search.back() != wildcard))
        {
            ret = EndsWith(source, segments.back());
        }

        for (auto it = segments.begin(); ret && (it != segments.end()); ++it)
        {
            pos = source.find(*it, pos);

            if (pos == StringType::npos)
            {
                ret = false;
            }
        }
    }

    return ret;
}

//==============================================================================
template <typename StringType>
StringType
BasicString<StringType>::GenerateRandomString(const size_type len) noexcept
{
    using short_distribution = std::uniform_int_distribution<short>;

    constexpr auto limit =
        static_cast<short_distribution::result_type>(s_alphaNumLength - 1);
    static_assert(limit > 0);

    short_distribution distribution(0, limit);

    const auto now = std::chrono::system_clock::now().time_since_epoch();
    const auto seed = static_cast<std::mt19937::result_type>(now.count());
    std::mt19937 engine(seed);

    StringType ret;
    ret.reserve(len);

    for (size_type i = 0; i < len; ++i)
    {
        ret += s_alphaNum[distribution(engine)];
    }

    return ret;
}

//==============================================================================
template <typename StringType>
template <typename... Args>
auto BasicString<StringType>::Format(
    const char_type *fmt,
    const Args &... args) noexcept -> streamed_type
{
    typename traits::ostringstream_type ostream;
    ostream.precision(6);

    Format(ostream, fmt, args...);
    return ostream.str();
}

//==============================================================================
template <typename StringType>
template <typename... Args>
auto BasicString<StringType>::Format(
    ostream_type &ostream,
    const char_type *fmt,
    const Args &... args) noexcept -> ostream_type &
{
    if (fmt != nullptr)
    {
        format(ostream, fmt, args...);
    }

    return ostream;
}

//==============================================================================
template <typename StringType>
template <typename T, typename... Args>
void BasicString<StringType>::format(
    ostream_type &ostream,
    const char_type *fmt,
    const T &value,
    const Args &... args) noexcept
{
    const std::ios_base::fmtflags flags(ostream.flags());

    for (; *fmt != '\0'; ++fmt)
    {
        if (*fmt == '%')
        {
            const char_type type = *(fmt + 1);

            switch (type)
            {
                case '\0':
                    stream(ostream, *fmt);
                    return;

                case '%':
                    stream(ostream, *(++fmt));
                    continue;

                case 'x':
                    ostream << "0x" << std::hex << std::nouppercase;
                    break;
                case 'X':
                    ostream << "0X" << std::hex << std::uppercase;
                    break;

                case 'o':
                    ostream << '0' << std::oct;
                    break;

                case 'a':
                    ostream << std::hexfloat << std::nouppercase;
                    break;
                case 'A':
                    ostream << std::hexfloat << std::uppercase;
                    break;

                case 'f':
                    ostream << std::fixed << std::nouppercase;
                    break;
                case 'F':
                    ostream << std::fixed << std::uppercase;
                    break;

                case 'g':
                    ostream << std::nouppercase;
                    break;
                case 'G':
                    ostream << std::uppercase;
                    break;

                case 'e':
                    ostream << std::scientific << std::nouppercase;
                    break;
                case 'E':
                    ostream << std::scientific << std::uppercase;
                    break;

                default:
                    break;
            }

            stream(ostream, value);
            ostream.flags(flags);

            format(ostream, fmt + 2, args...);
            return;
        }

        stream(ostream, *fmt);
    }
}

//==============================================================================
template <typename StringType>
void BasicString<StringType>::format(
    ostream_type &ostream,
    const char_type *fmt) noexcept
{
    for (; *fmt != '\0'; ++fmt)
    {
        if ((*fmt == '%') && (*(fmt + 1) == '%'))
        {
            stream(ostream, *(++fmt));
        }
        else
        {
            stream(ostream, *fmt);
        }
    }
}

//==============================================================================
template <typename StringType>
template <typename... Args>
auto BasicString<StringType>::Join(
    const char_type &separator,
    const Args &... args) noexcept -> streamed_type
{
    typename traits::ostringstream_type ostream;
    join(ostream, separator, args...);

    return ostream.str();
}

//==============================================================================
template <typename StringType>
template <typename T, typename... Args>
void BasicString<StringType>::join(
    ostream_type &ostream,
    const char_type &separator,
    const T &value,
    const Args &... args) noexcept
{
    stream(ostream, value);
    stream(ostream, separator);

    join(ostream, separator, args...);
}

//==============================================================================
template <typename StringType>
template <typename T>
void BasicString<StringType>::join(
    ostream_type &ostream,
    const char_type &,
    const T &value) noexcept
{
    stream(ostream, value);
}

//==============================================================================
template <typename StringType>
template <typename T>
void BasicString<StringType>::stream(
    ostream_type &ostream,
    const T &value) noexcept
{
    if constexpr (
        std::is_same_v<char_type, std::decay_t<T>> ||
        traits::template is_string_like_v<T>)
    {
        detail::BasicStringStreamer<StringType>::Stream(ostream, value);
    }
    else if constexpr (traits::OstreamTraits::template is_declared_v<T>)
    {
        ostream << std::boolalpha << value;
    }
    else
    {
        ostream << '[' << std::hex << &value << std::dec << ']';
    }
}

//==============================================================================
template <typename StringType>
template <typename T>
T BasicString<StringType>::Convert(const StringType &value) noexcept(
    std::is_same_v<StringType, std::decay_t<T>>)
{
    if constexpr (std::is_same_v<StringType, std::decay_t<T>>)
    {
        return value;
    }
    else if constexpr (traits::has_stoi_family_v)
    {
        return detail::BasicStringConverter<StringType, T>::Convert(value);
    }
    else
    {
        typename traits::ostringstream_type ostream;
        stream(ostream, value);

        return detail::BasicStringConverter<streamed_type, T>::Convert(
            ostream.str());
    }
}

} // namespace fly
