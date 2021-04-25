#pragma once

#include "fly/types/string/detail/string_traits.hpp"

#include <type_traits>

namespace fly::detail {

/**
 * Concept that is satisfied when the given type is like a supported std::basic_string
 * specialization. A type is "like" a std::basic_string specialization if it is that specialization
 * itself, a C-string equalivent, or a std::basic_string_view specialization.
 */
template <typename T>
concept IsLikeSupportedString = is_like_supported_string_v<T>;

/**
 * Concept that is satisfied when the given type is a formattable user-defined type.
 */
template <typename T>
concept IsFormatUserDefined = BasicFormatTraits::is_generic_v<T>;

/**
 * Concept that is satisfied when the given type is a formattable pointer type.
 */
template <typename T>
concept IsFormatPointer = BasicFormatTraits::is_pointer_v<T>;

/**
 * Concept that is satisfied when the given type is a formattable integral type, excluding boolean
 * types.
 */
template <typename T>
concept IsFormatIntegral = BasicFormatTraits::is_integral_v<T>;

/**
 * Concept that is satisfied when the given type is a formattable pointer type, excluding character
 * and boolean types.
 */
template <typename T>
concept IsFormatInteger = BasicFormatTraits::is_integer_v<T>;

/**
 * Concept that is satisfied when the given type is a formattable floating point type.
 */
template <typename T>
concept IsFormatFloat = std::is_floating_point_v<T>;

/**
 * Concept that is satisfied when the given type is a formattable boolean type.
 */
template <typename T>
concept IsFormatBoolean = std::is_same_v<T, bool>;

/**
 * Concept that is satisfied when the given type is a default-formatted enumeration type.
 */
template <typename T>
concept IsFormatEnum = BasicFormatTraits::is_default_formatted_enum_v<T>;

} // namespace fly::detail
