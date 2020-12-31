#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>

namespace fly::detail {

/**
 * Helper struct to convert a std::basic_string type to a plain-old-data type, e.g. int or float.
 *
 * Ideally, this entire helper can be removed when the STL supports floating point types with
 * std::from_chars. However, only integral types are currently supported. Thus, integral types will
 * use std::from_chars, and floating point types will use the std::stof family of functions.
 *
 * https://en.cppreference.com/w/cpp/compiler_support
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 21, 2019
 */
template <typename StringType, typename T>
struct BasicStringConverter;

//==================================================================================================
template <typename StringType, typename T>
struct BasicStringConverter
{
    static std::optional<T> convert(const StringType &value)
    {
        const auto *begin = value.data();
        const auto *end = begin + value.size();

        T converted {};
        auto result = std::from_chars(begin, end, converted);

        if ((result.ptr != end) || (result.ec != std::errc()))
        {
            return std::nullopt;
        }

        return converted;
    }
};

//==================================================================================================
template <typename StringType>
struct BasicStringConverter<StringType, float>
{
    using value_type = float;

    static std::optional<value_type> convert(const StringType &value)
    {
        std::size_t index = 0;
        value_type result {};

        try
        {
            result = std::stof(value, &index);
        }
        catch (...)
        {
            return std::nullopt;
        }

        if (index != value.length())
        {
            return std::nullopt;
        }

        return result;
    }
};

//==================================================================================================
template <typename StringType>
struct BasicStringConverter<StringType, double>
{
    using value_type = double;

    static std::optional<value_type> convert(const StringType &value)
    {
        std::size_t index = 0;
        value_type result {};

        try
        {
            result = std::stod(value, &index);
        }
        catch (...)
        {
            return std::nullopt;
        }

        if (index != value.length())
        {
            return std::nullopt;
        }

        return result;
    }
};

//==================================================================================================
template <typename StringType>
struct BasicStringConverter<StringType, long double>
{
    using value_type = long double;

    static std::optional<value_type> convert(const StringType &value)
    {
        std::size_t index = 0;
        value_type result {};

        try
        {
            result = std::stold(value, &index);
        }
        catch (...)
        {
            return std::nullopt;
        }

        if (index != value.length())
        {
            return std::nullopt;
        }

        return result;
    }
};

} // namespace fly::detail
