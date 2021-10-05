#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/string/detail/string_traits.hpp"

#include <concepts>
#include <type_traits>

namespace fly::detail {

/**
 * Concept that is satisfied when the given type is like a supported std::basic_string
 * specialization. A type is "like" a std::basic_string specialization if it is that specialization
 * itself, a C-string equalivent, or a std::basic_string_view specialization.
 */
template <typename T>
concept StringLike = fly::detail::is_like_supported_string_v<T>;

/**
 * Concept that is satisfied when a fly::Formatter<T, CharType> specialization is defined for a type
 * T, and that specialization implements a |format| method.
 */
template <typename T, typename FormatContext>
concept Formattable = requires(FormatContext context, const T &value)
{
    typename FormatContext::template formatter_type<T>;

    std::declval<typename FormatContext::template formatter_type<T>>().format(value, context);
};

/**
 * Concept that is satisfied when the given formatter defines a |parse| method.
 */
template <typename FormatParseContext, typename Formatter>
concept FormattableWithParsing = requires(FormatParseContext parse_context, Formatter formatter)
{
    formatter.parse(parse_context);
};

/**
 * Concept that is satisfied when the given type is a formattable string type.
 */
template <typename T>
concept FormattableString = StringLike<T>;

/**
 * Concept that is satisfied when the given type is a formattable pointer type.
 */
template <typename T>
concept FormattablePointer = requires
{
    requires std::is_pointer_v<std::remove_cvref_t<T>> ||
        std::is_null_pointer_v<std::remove_cvref_t<T>>;
    requires !FormattableString<T>;
};

/**
 * Concept that is satisfied when the given type is a formattable boolean type.
 */
template <typename T>
concept FormattableBoolean = std::same_as<bool, std::remove_cvref_t<T>>;

/**
 * Concept that is satisfied when the given type is a formattable integral type, excluding boolean
 * types.
 */
template <typename T>
concept FormattableIntegral = std::is_integral_v<std::remove_cvref_t<T>> && !FormattableBoolean<T>;

/**
 * Concept that is satisfied when the given type is a formattable floating-point type.
 */
template <typename T>
concept FormattableFloatingPoint = std::is_floating_point_v<std::remove_cvref_t<T>>;

/**
 * Concept that is satisfied when the given type is a formattable user-defined type.
 */
template <typename T>
concept FormattableUserDefined = requires
{
    requires !FormattableString<T>;
    requires !FormattablePointer<T>;
    requires !FormattableIntegral<T>;
    requires !FormattableFloatingPoint<T>;
    requires !FormattableBoolean<T>;
};

} // namespace fly::detail
