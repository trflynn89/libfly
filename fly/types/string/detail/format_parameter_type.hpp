#pragma once

#include "fly/types/string/detail/string_concepts.hpp"

#include <cstdint>

namespace fly::detail {

/**
 * Enumerated list of supported format parameter types.
 */
enum class ParameterType : std::uint8_t
{
    UserDefined,
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
    if constexpr (StandardCharacter<T>)
    {
        return ParameterType::Character;
    }
    else if constexpr (FormattableString<T>)
    {
        return ParameterType::String;
    }
    else if constexpr (FormattablePointer<T>)
    {
        return ParameterType::Pointer;
    }
    else if constexpr (FormattableIntegral<T>)
    {
        return ParameterType::Integral;
    }
    else if constexpr (FormattableFloatingPoint<T>)
    {
        return ParameterType::FloatingPoint;
    }
    else if constexpr (FormattableBoolean<T>)
    {
        return ParameterType::Boolean;
    }
    else
    {
        return ParameterType::UserDefined;
    }
}

} // namespace fly::detail
