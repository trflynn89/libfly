#pragma once

#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace fly {

/**
 * Helper struct to stream a std::basic_string<> to a std::basic_ostream<>, and
 * to define aliases for what file/string stream types should be used for that
 * std::basic_string<> type.
 *
 * For std::string and std::wstring, the "normal" std::*stream types are used
 * (std::istream/std::ostream and std::wistream/std::wostream, and their
 * children, respectively).
 *
 * For std::u16string and std::u32string, the STL does not provide stream types.
 * In some cases, specializing std::basic_*stream<> may work, but that is not
 * the case for all stream types. So for a general solution, these string types
 * use std::istream/std::ostream. For each character in the given string, if the
 * character is ASCII, it is first casted to std::string::value_type, and then
 * streamed. Otherwise, it is streamed as a hexadecimal.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version March 21, 2019
 */
template <typename StringType>
struct BasicStringStreamer;

//==============================================================================
template <>
struct BasicStringStreamer<std::string>
{
    using streamed_type = std::string;

    using istream_type = std::istream;
    using ostream_type = std::ostream;

    using fstream_type = std::fstream;
    using ifstream_type = std::ifstream;
    using ofstream_type = std::ofstream;

    using stringstream_type = std::stringstream;
    using istringstream_type = std::istringstream;
    using ostringstream_type = std::ostringstream;

    static void Stream(ostream_type &stream, const std::string &value) noexcept
    {
        stream << value;
    }

    static void Stream(ostream_type &stream, const char *value) noexcept
    {
        stream << value;
    }

    static void Stream(ostream_type &stream, const char value) noexcept
    {
        stream << value;
    }
};

//==============================================================================
template <>
struct BasicStringStreamer<std::wstring>
{
    using streamed_type = std::wstring;

    using istream_type = std::wistream;
    using ostream_type = std::wostream;

    using fstream_type = std::wfstream;
    using ifstream_type = std::wifstream;
    using ofstream_type = std::wofstream;

    using stringstream_type = std::wstringstream;
    using istringstream_type = std::wistringstream;
    using ostringstream_type = std::wostringstream;

    static void Stream(ostream_type &stream, const std::wstring &value) noexcept
    {
        stream << value;
    }

    static void Stream(ostream_type &stream, const wchar_t *value) noexcept
    {
        stream << value;
    }

    static void Stream(ostream_type &stream, const wchar_t value) noexcept
    {
        stream << value;
    }
};

//==============================================================================
template <>
struct BasicStringStreamer<std::u16string>
{
    using streamed_type = std::string;

    using istream_type = std::istream;
    using ostream_type = std::ostream;

    using fstream_type = std::fstream;
    using ifstream_type = std::ifstream;
    using ofstream_type = std::ofstream;

    using stringstream_type = std::stringstream;
    using istringstream_type = std::istringstream;
    using ostringstream_type = std::ostringstream;

    static void
    Stream(ostream_type &stream, const std::u16string &value) noexcept
    {
        for (const auto &ch : value)
        {
            Stream(stream, ch);
        }
    }

    static void Stream(ostream_type &stream, const char16_t *value) noexcept
    {
        const std::size_t size = std::char_traits<char16_t>::length(value);

        for (std::size_t i = 0; i < size; ++i)
        {
            Stream(stream, value[i]);
        }
    }

    static void Stream(ostream_type &stream, const char16_t value) noexcept
    {
        if (value <= 127)
        {
            stream << static_cast<ostream_type::char_type>(value);
        }
        else
        {
            stream << "[0x" << std::hex << value << ']';
        }
    }
};

//==============================================================================
template <>
struct BasicStringStreamer<std::u32string>
{
    using streamed_type = std::string;

    using istream_type = std::istream;
    using ostream_type = std::ostream;

    using fstream_type = std::fstream;
    using ifstream_type = std::ifstream;
    using ofstream_type = std::ofstream;

    using stringstream_type = std::stringstream;
    using istringstream_type = std::istringstream;
    using ostringstream_type = std::ostringstream;

    static void
    Stream(ostream_type &stream, const std::u32string &value) noexcept
    {
        for (const auto &ch : value)
        {
            Stream(stream, ch);
        }
    }

    static void Stream(ostream_type &stream, const char32_t *value) noexcept
    {
        const std::size_t size = std::char_traits<char32_t>::length(value);

        for (std::size_t i = 0; i < size; ++i)
        {
            Stream(stream, value[i]);
        }
    }

    static void Stream(ostream_type &stream, const char32_t value) noexcept
    {
        if (value <= 127)
        {
            stream << static_cast<ostream_type::char_type>(value);
        }
        else
        {
            stream << "[0x" << std::hex << value << ']';
        }
    }
};

} // namespace fly
