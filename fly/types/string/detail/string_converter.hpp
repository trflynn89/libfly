#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace fly::detail {

/**
 * Helper struct to convert a std::basic_string<> type to a plain-old-data type,
 * e.g. int or bool.
 *
 * Internally, the std::stoi family of functions is used to handle conversions,
 * so only std::string and std::wstring may be directly used. For std::u16string
 * and std::u32string, first use BasicStringStreamer<>::stream to convert the
 * string to std::string.
 *
 * It is recommended that outside callers use BasicString<>::convert<> instead
 * of using this struct directly.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename StringType, typename T>
struct BasicStringConverter;

//==============================================================================
template <typename StringType, typename T>
struct BasicStringConverter
{
    static T convert(const StringType &value) noexcept(false)
    {
        static constexpr long long s_min = std::numeric_limits<T>::min();
        static constexpr long long s_max = std::numeric_limits<T>::max();

        std::size_t index = 0;
        long long result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("Entire string was not consumed");
        }
        else if ((result < s_min) || (result > s_max))
        {
            throw std::out_of_range("Conversion is out-of-range of given type");
        }

        return static_cast<T>(result);
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::int64_t>
{
    using value_type = std::int64_t;

    static value_type convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stoll(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("Entire string was not consumed");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, std::uint64_t>
{
    using value_type = std::uint64_t;

    static value_type convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stoull(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("Entire string was not consumed");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, float>
{
    using value_type = float;

    static value_type convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stof(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("Entire string was not consumed");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, double>
{
    using value_type = double;

    static value_type convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stod(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("Entire string was not consumed");
        }

        return result;
    }
};

//==============================================================================
template <typename StringType>
struct BasicStringConverter<StringType, long double>
{
    using value_type = long double;

    static value_type convert(const StringType &value) noexcept(false)
    {
        std::size_t index = 0;
        value_type result = std::stold(value, &index);

        if (index != value.length())
        {
            throw std::invalid_argument("Entire string was not consumed");
        }

        return result;
    }
};

} // namespace fly::detail
