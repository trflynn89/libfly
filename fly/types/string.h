#pragma once

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <string_view>
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

/**
 * Static class to provide string utilities not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 16, 2016
 */
template <typename StringType>
class BasicString
{
public:
    using string_type = StringType;

    using size_type = typename StringType::size_type;
    using value_type = typename StringType::value_type;

    using istream_type = std::basic_istream<value_type>;
    using ostream_type = std::basic_ostream<value_type>;

    using fstream_type = std::basic_fstream<value_type>;
    using ifstream_type = std::basic_ifstream<value_type>;
    using ofstream_type = std::basic_ofstream<value_type>;

    using sstream_type = std::basic_stringstream<value_type>;
    using isstream_type = std::basic_istringstream<value_type>;
    using osstream_type = std::basic_ostringstream<value_type>;

    using view_type = std::basic_string_view<value_type>;

    /**
     * Split a string into a vector of strings.
     *
     * @param StringType The string to split.
     * @param value_type The delimiter to split the string on.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<StringType>
    Split(const StringType &, value_type) noexcept;

    /**
     * Split a string into a vector of strings, up to a maximum size. If the max
     * size is reached, the rest of the string is appended to the last element
     * in the vector.
     *
     * @param StringType The string to split.
     * @param value_type The delimiter to split the string on.
     * @param uint32_t The maximum return vector size. Zero implies unlimited.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<StringType>
    Split(const StringType &, value_type, std::uint32_t) noexcept;

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
     * @param value_type The replacement character.
     */
    static void
    ReplaceAll(StringType &, const StringType &, const value_type &) noexcept;

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
     * @param value_type The beginning to search for.
     *
     * @return True if the string begins with the search character.
     */
    static bool StartsWith(const StringType &, const value_type &) noexcept;

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
     * @param StringType The ending to search for.
     *
     * @return True if the string ends with the search character.
     */
    static bool EndsWith(const StringType &, const value_type &) noexcept;

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
     * Format a string with variadic template arguments. This is type safe in
     * in that the type doesn't matter with the format specifier (e.g. there's
     * no error if %s is given an integer). However, specifiers such as %x are
     * still attempted to be handled. That is, if the matching argument for %x
     * is numeric, then it will be printed in hexadecimal.
     *
     * There is also no checking done on the number of format specifiers and
     * the number of arguments. The format specifiers will be replaced one at a
     * time until all arguments are exhausted, then the rest of the string is
     * taken as-is. Any extra specifiers will be in the string. Any extra
     * arguments are dropped.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param value_type* The string to format.
     * @param Args The variadic list of arguments to be formatted.
     *
     * @return A string that has been formatted with the given arguments.
     */
    template <typename... Args>
    static StringType Format(const value_type *, const Args &...) noexcept;

    /**
     * Concatenate a list of objects with the given separator.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param value_type Character to use as a separator.
     * @param Args The variadic list of arguments to be joined.
     *
     * @return The resulting join of the given arguments.
     */
    template <typename... Args>
    static StringType Join(const value_type &, const Args &...) noexcept;

    /**
     * Convert a string to a basic type, e.g. int or bool.
     *
     * @tparam T The desired basic type.
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
        const value_type *,
        const T &,
        const Args &...) noexcept;

    /**
     * Terminator for the variadic template formatter. Stream the rest of the
     * string into the given ostream.
     */
    static void format(ostream_type &, const value_type *) noexcept;

    /**
     * Recursively join one argument into the given ostream.
     */
    template <typename T, typename... Args>
    static void join(
        ostream_type &,
        const value_type &,
        const T &,
        const Args &...) noexcept;

    /**
     * Terminator for the variadic template joiner. Join the last argument
     * into the given ostream.
     */
    template <typename T>
    static void join(ostream_type &, const value_type &, const T &) noexcept;

    /**
     * Stream the given value into the given stream.
     */
    template <typename T>
    static void getValue(ostream_type &, const T &) noexcept;

    /**
     * A list of alpha-numeric characters in the range [0-9a-zA-Z].
     */
    static constexpr std::array<value_type, 62> s_alphaNum = {{
        std::char_traits<value_type>::to_char_type(0x30), // 0
        std::char_traits<value_type>::to_char_type(0x31), // 1
        std::char_traits<value_type>::to_char_type(0x32), // 2
        std::char_traits<value_type>::to_char_type(0x33), // 3
        std::char_traits<value_type>::to_char_type(0x34), // 4
        std::char_traits<value_type>::to_char_type(0x35), // 5
        std::char_traits<value_type>::to_char_type(0x36), // 6
        std::char_traits<value_type>::to_char_type(0x37), // 7
        std::char_traits<value_type>::to_char_type(0x38), // 8
        std::char_traits<value_type>::to_char_type(0x39), // 9

        std::char_traits<value_type>::to_char_type(0x41), // A
        std::char_traits<value_type>::to_char_type(0x42), // B
        std::char_traits<value_type>::to_char_type(0x43), // C
        std::char_traits<value_type>::to_char_type(0x44), // D
        std::char_traits<value_type>::to_char_type(0x45), // E
        std::char_traits<value_type>::to_char_type(0x46), // F
        std::char_traits<value_type>::to_char_type(0x47), // G
        std::char_traits<value_type>::to_char_type(0x48), // H
        std::char_traits<value_type>::to_char_type(0x49), // I
        std::char_traits<value_type>::to_char_type(0x4a), // J
        std::char_traits<value_type>::to_char_type(0x4b), // K
        std::char_traits<value_type>::to_char_type(0x4c), // L
        std::char_traits<value_type>::to_char_type(0x4d), // M
        std::char_traits<value_type>::to_char_type(0x4e), // N
        std::char_traits<value_type>::to_char_type(0x4f), // O
        std::char_traits<value_type>::to_char_type(0x50), // P
        std::char_traits<value_type>::to_char_type(0x51), // Q
        std::char_traits<value_type>::to_char_type(0x52), // R
        std::char_traits<value_type>::to_char_type(0x53), // S
        std::char_traits<value_type>::to_char_type(0x54), // T
        std::char_traits<value_type>::to_char_type(0x55), // U
        std::char_traits<value_type>::to_char_type(0x56), // V
        std::char_traits<value_type>::to_char_type(0x57), // W
        std::char_traits<value_type>::to_char_type(0x58), // X
        std::char_traits<value_type>::to_char_type(0x59), // Y
        std::char_traits<value_type>::to_char_type(0x5a), // Z

        std::char_traits<value_type>::to_char_type(0x61), // a
        std::char_traits<value_type>::to_char_type(0x62), // b
        std::char_traits<value_type>::to_char_type(0x63), // c
        std::char_traits<value_type>::to_char_type(0x64), // d
        std::char_traits<value_type>::to_char_type(0x65), // e
        std::char_traits<value_type>::to_char_type(0x66), // f
        std::char_traits<value_type>::to_char_type(0x67), // g
        std::char_traits<value_type>::to_char_type(0x68), // h
        std::char_traits<value_type>::to_char_type(0x69), // i
        std::char_traits<value_type>::to_char_type(0x6a), // j
        std::char_traits<value_type>::to_char_type(0x6b), // k
        std::char_traits<value_type>::to_char_type(0x6c), // l
        std::char_traits<value_type>::to_char_type(0x6d), // m
        std::char_traits<value_type>::to_char_type(0x6e), // n
        std::char_traits<value_type>::to_char_type(0x6f), // o
        std::char_traits<value_type>::to_char_type(0x70), // p
        std::char_traits<value_type>::to_char_type(0x71), // q
        std::char_traits<value_type>::to_char_type(0x72), // r
        std::char_traits<value_type>::to_char_type(0x73), // s
        std::char_traits<value_type>::to_char_type(0x74), // t
        std::char_traits<value_type>::to_char_type(0x75), // u
        std::char_traits<value_type>::to_char_type(0x76), // v
        std::char_traits<value_type>::to_char_type(0x77), // w
        std::char_traits<value_type>::to_char_type(0x78), // x
        std::char_traits<value_type>::to_char_type(0x79), // y
        std::char_traits<value_type>::to_char_type(0x7a), // z
    }};
};

/**
 * Helper class to convert a string type to a basic type, e.g. int or bool. This
 * class is only provided so specializations for each basic type can be defined.
 * Outside callers should use BasicString<StringType>::Convert().
 */
template <typename StringType, typename T>
struct BasicStringConverter
{
    static T Convert(const StringType &) noexcept(
        std::is_same_v<StringType, std::decay_t<T>>);
};

//==============================================================================
template <typename StringType>
std::vector<StringType> BasicString<StringType>::Split(
    const StringType &input,
    value_type delim) noexcept
{
    return Split(input, delim, 0);
}

//==============================================================================
template <typename StringType>
std::vector<StringType> BasicString<StringType>::Split(
    const StringType &input,
    value_type delim,
    std::uint32_t max) noexcept
{
    StringType item;
    sstream_type ss(input);
    std::vector<StringType> elems;
    std::uint32_t numItems = 0;

    while (std::getline(ss, item, delim))
    {
        if (!item.empty())
        {
            if ((max > 0) && (++numItems > max))
            {
                elems.back() += delim;
                elems.back() += item;
            }
            else
            {
                elems.push_back(item);
            }
        }
    }

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
    const value_type &replace) noexcept
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
    const value_type &search) noexcept
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
    const value_type &search) noexcept
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
    static constexpr value_type wildcard = '*';
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
        static_cast<short_distribution::result_type>(s_alphaNum.size() - 1);
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
StringType BasicString<StringType>::Format(
    const value_type *fmt,
    const Args &... args) noexcept
{
    osstream_type stream;
    stream.precision(6);

    if (fmt != NULL)
    {
        format(stream, fmt, args...);
    }

    return stream.str();
}

//==============================================================================
template <typename StringType>
template <typename T, typename... Args>
void BasicString<StringType>::format(
    ostream_type &stream,
    const value_type *fmt,
    const T &value,
    const Args &... args) noexcept
{
    for (; *fmt != '\0'; ++fmt)
    {
        if (*fmt == '%')
        {
            value_type type = *(fmt + 1);

            switch (type)
            {
                case '\0':
                    stream << *fmt;
                    return;

                case 'x':
                case 'X':
                    stream << "0x" << std::hex;
                    getValue(stream, value);
                    stream << std::dec;
                    break;

                case 'f':
                case 'F':
                case 'g':
                case 'G':
                    stream << std::fixed;
                    getValue(stream, value);
                    stream.unsetf(std::ios_base::fixed);
                    break;

                case 'e':
                case 'E':
                    stream << std::scientific;
                    getValue(stream, value);
                    stream.unsetf(std::ios_base::scientific);
                    break;

                default:
                    getValue(stream, value);
                    break;
            }

            format(stream, fmt + 2, args...);
            return;
        }

        stream << *fmt;
    }
}

//==============================================================================
template <typename StringType>
void BasicString<StringType>::format(
    ostream_type &stream,
    const value_type *fmt) noexcept
{
    stream << fmt;
}

//==============================================================================
template <typename StringType>
template <typename... Args>
StringType BasicString<StringType>::Join(
    const value_type &separator,
    const Args &... args) noexcept
{
    osstream_type stream;
    join(stream, separator, args...);

    return stream.str();
}

//==============================================================================
template <typename StringType>
template <typename T, typename... Args>
void BasicString<StringType>::join(
    ostream_type &stream,
    const value_type &separator,
    const T &value,
    const Args &... args) noexcept
{
    getValue(stream, value);
    stream << separator;

    join(stream, separator, args...);
}

//==============================================================================
template <typename StringType>
template <typename T>
void BasicString<StringType>::join(
    ostream_type &stream,
    const value_type &,
    const T &value) noexcept
{
    getValue(stream, value);
}

//==============================================================================
template <typename StringType>
template <typename T>
void BasicString<StringType>::getValue(
    ostream_type &stream,
    const T &value) noexcept
{
    stream << std::boolalpha << value;
}

//==============================================================================
template <typename StringType>
template <typename T>
T BasicString<StringType>::Convert(const StringType &value) noexcept(
    std::is_same_v<StringType, std::decay_t<T>>)
{
    return BasicStringConverter<StringType, T>::Convert(value);
}

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, StringType>
{
    static StringType Convert(const StringType &value) noexcept
    {
        return value;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, bool>
{
    using value_type = bool;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("bool");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("bool");
        }

        return (result != 0);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, typename StringType::value_type>
{
    using value_type = typename StringType::value_type;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("char");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("char");
        }

        return static_cast<value_type>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::int8_t>
{
    using value_type = std::int8_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::int8_t");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("std::int8_t");
        }

        return static_cast<value_type>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::uint8_t>
{
    using value_type = std::uint8_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::uint8_t");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("std::uint8_t");
        }

        return static_cast<value_type>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::int16_t>
{
    using value_type = std::int16_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::int16_t");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("std::int16_t");
        }

        return static_cast<value_type>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::uint16_t>
{
    using value_type = std::uint16_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::uint16_t");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("std::uint16_t");
        }

        return static_cast<value_type>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::int32_t>
{
    using value_type = std::int32_t;

    static auto Convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stoi(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::int32_t");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::uint32_t>
{
    using value_type = std::uint32_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        static constexpr long long min = std::numeric_limits<value_type>::min();
        static constexpr long long max = std::numeric_limits<value_type>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::uint32_t");
        }
        else if ((result < min) || (result > max))
        {
            throw std::out_of_range("std::uint32_t");
        }

        return static_cast<value_type>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::int64_t>
{
    using value_type = std::int64_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::int64_t");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::uint64_t>
{
    using value_type = std::uint64_t;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stoull(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("std::uint64_t");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, float>
{
    using value_type = float;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stof(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("float");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, double>
{
    using value_type = double;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stod(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("double");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, long double>
{
    using value_type = long double;

    static value_type Convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stold(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("ldouble");
        }

        return result;
    }
};

} // namespace fly
