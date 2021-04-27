#pragma once

#include "fly/types/string/detail/string_traits.hpp"

#include <cstdint>

namespace fly::detail {

/**
 * Enumerated list of supported format parameter types.
 */
enum class ParameterType : std::uint8_t
{
    Generic,
    Character,
    String,
    Pointer,
    Integral,
    FloatingPoint,
    Boolean,
};

/**
 * Map a format parameter type to the ParameterType enumeration.
 *
 * @tparam T The format parameter type.
 *
 * @return The mapped ParameterType enumeration.
 */
template <typename T>
constexpr ParameterType infer_parameter_type()
{
    using U = std::remove_cvref_t<T>;

    if constexpr (is_supported_character_v<U>)
    {
        return ParameterType::Character;
    }
    else if constexpr (is_like_supported_string_v<U>)
    {
        return ParameterType::String;
    }
    else if constexpr (std::is_pointer_v<U> || std::is_null_pointer_v<U>)
    {
        return ParameterType::Pointer;
    }
    else if constexpr (
        BasicFormatTraits::is_integer_v<U> || BasicFormatTraits::is_default_formatted_enum_v<U>)
    {
        return ParameterType::Integral;
    }
    else if constexpr (std::is_floating_point_v<U>)
    {
        return ParameterType::FloatingPoint;
    }
    else if constexpr (std::is_same_v<U, bool>)
    {
        return ParameterType::Boolean;
    }
    else
    {
        return ParameterType::Generic;
    }
}

} // namespace fly::detail
