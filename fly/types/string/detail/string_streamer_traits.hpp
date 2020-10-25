#pragma once

#include <fstream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace fly::detail {

/**
 * Traits for streaming properties of standard std::basic_string specializations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename StringType>
struct BasicStringStreamerTraits;

//==================================================================================================
template <>
struct BasicStringStreamerTraits<std::string>
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
};

//==================================================================================================
template <>
struct BasicStringStreamerTraits<std::wstring>
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
};

//==================================================================================================
template <>
struct BasicStringStreamerTraits<std::u8string>
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
};

//==================================================================================================
template <>
struct BasicStringStreamerTraits<std::u16string>
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
};

//==================================================================================================
template <>
struct BasicStringStreamerTraits<std::u32string>
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
};

} // namespace fly::detail
