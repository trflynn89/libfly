#pragma once

#include "fly/fly.hpp"

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>

namespace fly::detail {

/**
 * Helper struct to convert a std::string to a plain-old-data type, e.g. int or float.
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
template <typename T>
struct Converter
{
    static std::optional<T> convert(const std::string &value)
    {
        const char *begin = value.data();
        const char *end = begin + value.size();

        T converted {};
        auto result = std::from_chars(begin, end, converted);

        if ((result.ptr != end) || (result.ec != std::errc {}))
        {
            return std::nullopt;
        }

        return converted;
    }
};

#if !defined(FLY_COMPILER_SUPPORTS_FP_CHARCONV)

//==================================================================================================
template <>
struct Converter<float>
{
    static std::optional<float> convert(const std::string &value)
    {
        std::size_t index = 0;
        float result {};

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
template <>
struct Converter<double>
{
    static std::optional<double> convert(const std::string &value)
    {
        std::size_t index = 0;
        double result {};

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
template <>
struct Converter<long double>
{
    static std::optional<long double> convert(const std::string &value)
    {
        std::size_t index = 0;
        long double result {};

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

#endif

} // namespace fly::detail
