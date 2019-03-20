#pragma once

#include "fly/fly.h"
#include "fly/traits/traits.h"

#include <cstdint>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

namespace fly {

/**
 * Static class to provide string utilities not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 16, 2016
 */
class String
{
public:
    /**
     * Split a string into a vector of strings.
     *
     * @param string The string to split.
     * @param char The delimiter to split the string on.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<std::string> Split(const std::string &, char) noexcept;

    /**
     * Split a string into a vector of strings, up to a maximum size. If the max
     * size is reached, the rest of the string is appended to the last element
     * in the vector.
     *
     * @param string The string to split.
     * @param char The delimiter to split the string on.
     * @param uint32_t The maximum return vector size. Zero implies unlimited.
     *
     * @return A vector containing the split strings.
     */
    static std::vector<std::string>
    Split(const std::string &, char, std::uint32_t) noexcept;

    /**
     * Remove leading and trailing whitespace from a string.
     *
     * @param string The string to trim.
     */
    static void Trim(std::string &) noexcept;

    /**
     * Replace all instances of a substring in a string with a character.
     *
     * @param string The string container which will be modified.
     * @param string The string to search for and replace.
     * @param char The replacement character.
     */
    static void
    ReplaceAll(std::string &, const std::string &, const char &) noexcept;

    /**
     * Replace all instances of a substring in a string with another string.
     *
     * @param string The string container which will be modified.
     * @param string The string to search for and replace.
     * @param string The replacement string.
     */
    static void ReplaceAll(
        std::string &,
        const std::string &,
        const std::string &) noexcept;

    /**
     * Remove all instances of a substring in a string.
     *
     * @param string The string container which will be modified.
     * @param string The string to search for and remove.
     */
    static void RemoveAll(std::string &, const std::string &) noexcept;

    /**
     * Check if a string begins with a character.
     *
     * @param string The string to check.
     * @param char The beginning to search for.
     *
     * @return True if the string begins with the search character.
     */
    static bool StartsWith(const std::string &, const char &) noexcept;

    /**
     * Check if a string begins with another string.
     *
     * @param string The string to check.
     * @param string The beginning to search for.
     *
     * @return True if the string begins with the search string.
     */
    static bool StartsWith(const std::string &, const std::string &) noexcept;

    /**
     * Check if a string ends with a character.
     *
     * @param string The string to check.
     * @param string The ending to search for.
     *
     * @return True if the string ends with the search character.
     */
    static bool EndsWith(const std::string &, const char &) noexcept;

    /**
     * Check if a string ends with another string.
     *
     * @param string The string to check.
     * @param string The ending to search for.
     *
     * @return True if the string ends with the search string.
     */
    static bool EndsWith(const std::string &, const std::string &) noexcept;

    /**
     * Check if a string matches another string with wildcard expansion.
     *
     * @param string The source string to match against.
     * @param string The wildcard string to search with.
     *
     * @return True if the wildcard string matches the source string.
     */
    static bool
    WildcardMatch(const std::string &, const std::string &) noexcept;

    /**
     * Generate a random string of the given size.
     *
     * @param size_type The length of the string to generate.
     *
     * @return The generated string.
     */
    static std::string
    GenerateRandomString(const std::string::size_type) noexcept;

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
     * @param char* The string to format.
     * @param Args The variadic list of arguments to be formatted.
     *
     * @return A string that has been formatted with the given arguments.
     */
    template <typename... Args>
    static std::string Format(const char *, const Args &...) noexcept;

    /**
     * Concatenate a list of objects with the given separator.
     *
     * @tparam Args Variadic template arguments.
     *
     * @param char Character to use as a separator.
     * @param Args The variadic list of arguments to be joined.
     *
     * @return The resulting join of the given arguments.
     */
    template <typename... Args>
    static std::string Join(const char &, const Args &...) noexcept;

    /**
     * Convert a string to a basic type, e.g. int or bool.
     *
     * @tparam T The desired basic type.
     *
     * @param string The string to convert.
     *
     * @return The string coverted to the specified type.
     *
     * @throws std::invalid_argument Conversion could not be performed.
     * @throws std::out_of_range Converted value is out of range of result type.
     */
    template <typename T>
    static T Convert(const std::string &) noexcept(
        std::is_same_v<std::string, std::decay_t<T>>);

private:
    /**
     * Recursively format a string with one argument. The result is streamed
     * into the given ostream.
     */
    template <typename T, typename... Args>
    static void
    format(std::ostream &, const char *, const T &, const Args &...) noexcept;

    /**
     * Terminator for the variadic template formatter. Stream the rest of the
     * string into the given ostream.
     */
    static void format(std::ostream &, const char *) noexcept;

    /**
     * Recursively join one argument into the given ostream.
     */
    template <typename T, typename... Args>
    static void
    join(std::ostream &, const char &, const T &, const Args &...) noexcept;

    /**
     * Terminator for the variadic template joiner. Join the last argument
     * into the given ostream.
     */
    template <typename T>
    static void join(std::ostream &, const char &, const T &) noexcept;

    /**
     * Stream the given value into the given stream.
     */
    template <typename T, if_ostream::enabled<T> = 0>
    static void getValue(std::ostream &, const T &) noexcept;

    /**
     * Stream the hash of the given value into the given stream.
     */
    template <typename T, if_ostream::disabled<T> = 0>
    static void getValue(std::ostream &, const T &) noexcept;
};

//==============================================================================
template <typename... Args>
std::string String::Format(const char *fmt, const Args &... args) noexcept
{
    std::ostringstream stream;
    stream.precision(6);

    if (fmt != NULL)
    {
        format(stream, fmt, args...);
    }

    return stream.str();
}

//==============================================================================
template <typename T, typename... Args>
void String::format(
    std::ostream &stream,
    const char *fmt,
    const T &value,
    const Args &... args) noexcept
{
    for (; *fmt != '\0'; ++fmt)
    {
        if (*fmt == '%')
        {
            char type = *(fmt + 1);

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
template <typename... Args>
std::string String::Join(const char &separator, const Args &... args) noexcept
{
    std::ostringstream stream;
    join(stream, separator, args...);

    return stream.str();
}

//==============================================================================
template <typename T, typename... Args>
void String::join(
    std::ostream &stream,
    const char &separator,
    const T &value,
    const Args &... args) noexcept
{
    getValue(stream, value);
    stream << separator;

    join(stream, separator, args...);
}

//==============================================================================
template <typename T>
void String::join(std::ostream &stream, const char &, const T &value) noexcept
{
    getValue(stream, value);
}

//==============================================================================
template <typename T, if_ostream::enabled<T>>
void String::getValue(std::ostream &stream, const T &value) noexcept
{
    stream << std::boolalpha << value;
}

//==============================================================================
template <typename T, if_ostream::disabled<T>>
void String::getValue(std::ostream &stream, const T &value) noexcept
{
    static std::hash<T *> hasher;
    stream << "[0x" << std::hex << hasher(&value) << std::dec << ']';
}

} // namespace fly
