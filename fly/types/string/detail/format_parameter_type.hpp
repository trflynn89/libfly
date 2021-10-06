#pragma once

#include "fly/types/string/string_concepts.hpp"

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
    if constexpr (fly::StandardCharacter<T>)
    {
        return ParameterType::Character;
    }
    else if constexpr (fly::FormattableString<T>)
    {
        return ParameterType::String;
    }
    else if constexpr (fly::FormattablePointer<T>)
    {
        return ParameterType::Pointer;
    }
    else if constexpr (fly::FormattableIntegral<T>)
    {
        return ParameterType::Integral;
    }
    else if constexpr (fly::FormattableFloatingPoint<T>)
    {
        return ParameterType::FloatingPoint;
    }
    else if constexpr (fly::FormattableBoolean<T>)
    {
        return ParameterType::Boolean;
    }
    else
    {
        return ParameterType::UserDefined;
    }
}

} // namespace fly::detail
