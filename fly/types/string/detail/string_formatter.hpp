#pragma once

#include "fly/types/string/detail/string_streamer.hpp"
#include "fly/types/string/detail/string_traits.hpp"

#include <ostream>

namespace fly::detail {

/**
 * Helper class to format and stream generic values into a std::basic_string<>'s output stream type.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version June 6, 2020
 */
template <typename StringType>
class BasicStringFormatter
{
    using traits = detail::BasicStringTraits<StringType>;
    using char_type = typename traits::char_type;
    using ostream_type = typename traits::ostream_type;
    using streamed_type = typename traits::streamer_type::streamed_type;

public:
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
    static streamed_type format(const char_type *fmt, const Args &... args);

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
    static ostream_type &format(ostream_type &ostream, const char_type *fmt, const Args &... args);

    /**
     * Stream the given value into the given stream. If the value is a string-like type, the string
     * is streamed using BasicStringStreamer<>. Or, it the type has an operator<< overload defined,
     * the value is stream using that overload. Otherwise, the memory location of the value is
     * streamed.
     *
     * @param ostream The stream to insert the value into.
     * @param value The value to stream.
     */
    template <typename T>
    static void stream(ostream_type &ostream, const T &value);

private:
    /**
     * Recursively format a string with one argument. The result is streamed into the given ostream.
     */
    template <typename T, typename... Args>
    static void format_internal(
        ostream_type &ostream,
        const char_type *fmt,
        const T &value,
        const Args &... args);

    /**
     * Terminator for the variadic template formatter. Stream the rest of the string into the given
     * ostream.
     */
    static void format_internal(ostream_type &ostream, const char_type *fmt);
};

//==================================================================================================
template <typename StringType>
template <typename... Args>
auto BasicStringFormatter<StringType>::format(const char_type *fmt, const Args &... args)
    -> streamed_type
{
    typename traits::ostringstream_type ostream;
    ostream.precision(6);

    format(ostream, fmt, args...);
    return ostream.str();
}

//==================================================================================================
template <typename StringType>
template <typename... Args>
auto BasicStringFormatter<StringType>::format(
    ostream_type &ostream,
    const char_type *fmt,
    const Args &... args) -> ostream_type &
{
    if (fmt != nullptr)
    {
        format_internal(ostream, fmt, args...);
    }

    return ostream;
}

//==================================================================================================
template <typename StringType>
template <typename T, typename... Args>
void BasicStringFormatter<StringType>::format_internal(
    ostream_type &ostream,
    const char_type *fmt,
    const T &value,
    const Args &... args)
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

            format_internal(ostream, fmt + 2, args...);
            return;
        }

        stream(ostream, *fmt);
    }
}

//==================================================================================================
template <typename StringType>
void BasicStringFormatter<StringType>::format_internal(ostream_type &ostream, const char_type *fmt)
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

//==================================================================================================
template <typename StringType>
template <typename T>
void BasicStringFormatter<StringType>::stream(ostream_type &ostream, const T &value)
{
    if constexpr (
        std::is_same_v<char_type, std::decay_t<T>> || traits::template is_string_like_v<T>)
    {
        detail::BasicStringStreamer<StringType>::stream(ostream, value);
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

} // namespace fly::detail
