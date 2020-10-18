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
 * For std::u16string and std::u32string, the STL does not provide stream types. For a general
 * solution, a copy of the input string is created with UTF-8 encoding for use with std::ostream/
 * std::istream.
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

    using const_iterator = typename traits::const_iterator;

    using streamed_type = typename traits::streamed_type;
    using streamed_char = typename streamed_type::value_type;

    using istream_type = typename traits::istream_type;
    using ostream_type = typename traits::ostream_type;

    using fstream_type = typename traits::fstream_type;
    using ifstream_type = typename traits::ifstream_type;
    using ofstream_type = typename traits::ofstream_type;

    using stringstream_type = typename traits::stringstream_type;
    using istringstream_type = typename traits::istringstream_type;
    using ostringstream_type = typename traits::ostringstream_type;

    /**
     * Stream the given value into the given stream.
     *
     * For all string-like types, any symbol which is not a printable ASCII character is escaped as
     * a Unicode codepoint. For example, the line feed character will be streamed as "\U000c", and
     * U+10f355 will be streamed as "\U0010f355".
     *
     * For all character types, any symbol which is not a printable ASCII character is escaped as
     * hexadecimal. For example, the line feed character will be streamed as "\x0c". The end-of-file
     * character will be streamed as the string "[EOF]".
     *
     * For any other type which has an operator<< overload defined, the value is stream using that
     * overload. Otherwise, the memory location of the value is streamed.
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
     * @param ostream The stream to insert the value into.
     * @param it Pointer to the beginning of the string to stream.
     * @param end Pointer to the end of the string to stream.
     */
    template <
        typename StreamedType = StringType,
        enable_if_all<std::is_same<StreamedType, streamed_type>> = 0>
    static void stream_string(ostream_type &ostream, const_iterator &it, const const_iterator &end);

    /**
     * Convert a string value to UTF-8 and stream that value into the given stream.
     *
     * @param ostream The stream to insert the value into.
     * @param it Pointer to the beginning of the string to stream.
     * @param end Pointer to the end of the string to stream.
     */
    template <
        typename StreamedType = StringType,
        enable_if_none<std::is_same<StreamedType, streamed_type>> = 0>
    static void stream_string(ostream_type &ostream, const_iterator &it, const const_iterator &end);

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

    if constexpr (any_same_v<U, std::string, std::wstring, std::u16string, std::u32string>)
    {
        auto it = value.cbegin();
        const auto end = value.cend();

        stream_string(ostream, it, end);
    }
    else if constexpr (traits::template is_string_like_v<U>)
    {
        // TODO it would be better if this could be a std::string_view. The methods in
        // BasicStringUnicode just need to support any iterator type.
        const StringType copy(value, char_traits::length(value));

        auto it = copy.cbegin();
        const auto end = copy.cend();

        stream_string(ostream, it, end);
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
    typename StreamedType,
    enable_if_all<
        std::is_same<StreamedType, typename BasicStringTraits<StringType>::streamed_type>>>
void BasicStringStreamer<StringType>::stream_string(
    ostream_type &ostream,
    const_iterator &it,
    const const_iterator &end)
{
    while (it != end)
    {
        auto escaped = BasicStringUnicode<StringType>::escape_codepoint(it, end);

        if (escaped)
        {
            ostream << std::move(escaped.value());
        }
    }
}

//==================================================================================================
template <typename StringType>
template <
    typename StreamedType,
    enable_if_none<
        std::is_same<StreamedType, typename BasicStringTraits<StringType>::streamed_type>>>
void BasicStringStreamer<StringType>::stream_string(
    ostream_type &ostream,
    const_iterator &it,
    const const_iterator &end)
{
    auto converted =
        BasicStringUnicode<StringType>::template convert_encoding<streamed_type>(it, end);

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
        const auto value_as_int = static_cast<typename char_traits::int_type>(value);

        if (value_as_int == char_traits::eof())
        {
            ostream << FLY_STR(streamed_char, "[EOF]");
        }
        else
        {
            static constexpr const streamed_char s_fill(FLY_CHR(streamed_char, '0'));

            ostream << FLY_STR(streamed_char, "\\x");
            ostream << std::setfill(s_fill) << std::setw(2);
            ostream << std::hex << (value_as_int & 0xff) << std::dec;
        }
    }
}

} // namespace fly::detail
