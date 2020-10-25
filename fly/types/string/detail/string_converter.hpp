#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Helper struct to convert a std::basic_string type to a plain-old-data type, e.g. int or bool.
 *
 * Internally, the std::stoi family of functions is used to handle conversions, so only std::string
 * and std::wstring may be directly used. For std::u8string, std::u16string, and std::u32string,
 * first use BasicStringUnicode::convert_encoding to convert the string to std::string.
 *
 * It is recommended that outside callers use BasicString::convert instead of using this struct
 * directly.
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
    template <typename V = T, std::enable_if_t<(sizeof(V) < 8), bool> = 0>
    static std::optional<T> convert(const StringType &value)
    {
        static constexpr auto s_min = static_cast<long long>(std::numeric_limits<T>::min());
        static constexpr auto s_max = static_cast<long long>(std::numeric_limits<T>::max());

        std::size_t index = 0;
        long long result = 0;

        try
        {
            result = std::stoll(value, &index);
        }
        catch (...)
        {
            return std::nullopt;
        }

        if ((index != value.length()) || (result < s_min) || (result > s_max))
        {
            return std::nullopt;
        }

        return static_cast<T>(result);
    }

    template <typename V = T, std::enable_if_t<(sizeof(V) == 8), bool> = 0>
    static std::optional<T> convert(const StringType &value)
    {
        std::size_t index = 0;
        T result = 0;

        try
        {
            if constexpr (std::is_signed_v<T>)
            {
                result = std::stoll(value, &index);
            }
            else
            {
                result = std::stoull(value, &index);
            }
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
struct BasicStringConverter<StringType, float>
{
    using value_type = float;

    static std::optional<value_type> convert(const StringType &value)
    {
        std::size_t index = 0;
        value_type result = 0;

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
        value_type result = 0;

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
        value_type result = 0;

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
