#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/detail/string_unicode.hpp"
#include "fly/types/string/string_literal.hpp"

#include <fstream>
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Helper struct to stream a std::basic_string to a std::basic_ostream, and to define aliases for
 * what file/string stream types should be used for that std::basic_string type.
 *
 * For std::string and std::wstring, the "normal" stream types are used (std::istream/std::ostream
 * and std::wistream/std::wostream, and their children, respectively).
 *
 * For std::u8string, std::u16string, and std::u32string, the STL does not provide stream types. For
 * a general solution, a copy of the input string is created with UTF-8 encoding for use with
 * std::ostream/std::istream.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename StringType>
struct BasicStringStreamer
{
    using traits = BasicStringTraits<StringType>;

    using char_type = typename traits::char_type;
    using char_traits = std::char_traits<char_type>;

    using ostream_type = typename traits::ostream_type;
    using streamed_type = typename traits::streamed_type;
    using streamed_char = typename streamed_type::value_type;

    /**
     * Stream the given value into the given stream.
     *
     * For all string-like types, any symbol which is not a printable ASCII character is escaped as
     * a Unicode codepoint. For example, the line feed character will be streamed as "\u000c", and
     * U+10f355 will be streamed as "\U0010f355".
     *
     * For all character types, any symbol which is not a printable ASCII character is escaped as
     * hexadecimal. For example, the line feed character will be streamed as "\x0c". The end-of-file
     * character will be streamed as the string "[EOF]".
     *
     * For any other type which has an operator<< overload defined, the value is stream using that
     * overload. Otherwise, the memory location of the value is streamed.
     *
     * @tparam T The type of the value to stream.
     *
     * @param ostream The stream to insert the value into.
     * @param value The value to stream.
     */
    template <typename T>
    static void stream(ostream_type &ostream, const T &value);

private:
    /**
     * Stream a string into the given stream.
     *
     * @tparam T The type of the string to stream.
     *
     * @param ostream The stream to insert the value into.
     * @param value The string to stream.
     */
    template <typename T, enable_if_all<std::is_same<T, streamed_type>> = 0>
    static void stream_string(ostream_type &ostream, const T &value);

    /**
     * Convert a string value to UTF-8 and stream that value into the given stream.
     *
     * @tparam T The type of the string to stream.
     *
     * @param ostream The stream to insert the value into.
     * @param value The string to stream.
     */
    template <typename T, enable_if_none<std::is_same<T, streamed_type>> = 0>
    static void stream_string(ostream_type &ostream, const T &value);

    /**
     * Stream a character into the given stream.
     *
     * @param ostream The stream to insert the value into.
     * @param value The character to stream.
     */
    static void stream_char(ostream_type &ostream, const char_type value);
};

//==================================================================================================
template <typename StringType>
template <typename T>
void BasicStringStreamer<StringType>::stream(ostream_type &ostream, const T &value)
{
    using U = std::decay_t<T>;

    if constexpr (detail::is_supported_string_v<U>)
    {
        stream_string(ostream, value);
    }
    else if constexpr (detail::is_like_supported_string_v<U>)
    {
        using string_type = detail::is_like_supported_string_t<U>;
        using string_traits = std::char_traits<typename string_type::value_type>;

        // TODO it would be better if this could be a std::string_view. The methods in
        // BasicStringUnicode just need to support any iterator type.
        const string_type copy(value, string_traits::length(value));
        stream_string(ostream, copy);
    }
    else if constexpr (std::is_same_v<U, char_type>)
    {
        stream_char(ostream, value);
    }
    else if constexpr (traits::OstreamTraits::template is_declared_v<U>)
    {
        ostream << std::boolalpha << value;
    }
    else
    {
        ostream << '[' << std::hex << &value << std::dec << ']';
    }
}

//==================================================================================================
template <typename StringType>
template <
    typename T,
    enable_if_all<std::is_same<T, typename BasicStringTraits<StringType>::streamed_type>>>
void BasicStringStreamer<StringType>::stream_string(ostream_type &ostream, const T &value)
{
    auto it = value.cbegin();
    const auto end = value.cend();

    while (it != end)
    {
        auto escaped = BasicStringUnicode<T>::escape_codepoint(it, end);

        if (escaped)
        {
            ostream << std::move(escaped.value());
        }
    }
}

//==================================================================================================
template <typename StringType>
template <
    typename T,
    enable_if_none<std::is_same<T, typename BasicStringTraits<StringType>::streamed_type>>>
void BasicStringStreamer<StringType>::stream_string(ostream_type &ostream, const T &value)
{
    auto it = value.cbegin();
    const auto end = value.cend();

    auto converted = BasicStringUnicode<T>::template convert_encoding<streamed_type>(it, end);

    if (converted)
    {
        BasicStringStreamer<streamed_type>::stream(ostream, converted.value());
    }
}

//==================================================================================================
template <typename StringType>
void BasicStringStreamer<StringType>::stream_char(ostream_type &ostream, const char_type value)
{
    if ((value > 0x1f) && (value < 0x7f))
    {
        ostream << static_cast<typename streamed_type::value_type>(value);
    }
    else
    {
        static constexpr const auto s_eof_as_char = static_cast<char_type>(char_traits::eof());

        if (value == s_eof_as_char)
        {
            ostream << FLY_STR(streamed_char, "[EOF]");
        }
        else
        {
            static constexpr const streamed_char s_fill(FLY_CHR(streamed_char, '0'));
            const auto value_as_int = static_cast<typename char_traits::int_type>(value);

            ostream << FLY_STR(streamed_char, "\\x");
            ostream << std::setfill(s_fill) << std::setw(2);
            ostream << std::hex << (value_as_int & 0xff) << std::dec;
        }
    }
}

} // namespace fly::detail
