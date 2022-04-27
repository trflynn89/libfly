#pragma once

#include "fly/concepts/concepts.hpp"
#include "fly/types/string/detail/concepts.hpp"

#include <string>
#include <type_traits>

namespace fly {

/**
 * Alias to map a string-like type to its analogous std::basic_string specialization. A type is
 * "like" a std::basic_string specialization if it is that specialization itself, a C-string
 * equalivent, or a std::basic_string_view specialization. If the provided type is not string-like,
 * this alias becomes std::false_type.
 */
template <typename T>
using StandardStringType = typename detail::StandardStringType<T>::string_type;

/**
 * Alias to map a string-like type to its analogous std::basic_string specialization's character
 * type. A type is "like" a std::basic_string specialization if it is that specialization itself, a
 * C-string equalivent, or a std::basic_string_view specialization. If the provided type is not
 * string-like, this alias becomes std::false_type.
 */
template <typename T>
using StandardCharacterType = typename detail::StandardStringType<T>::char_type;

/**
 * Concept that is satisfied when the given type is a supported std::basic_string specialization.
 */
template <typename T>
concept StandardString =
    fly::SameAsAny<T, std::string, std::wstring, std::u8string, std::u16string, std::u32string>;

/**
 * Concept that is satisfied when the given type is a supported character type.
 */
template <typename T>
concept StandardCharacter = fly::SameAsAny<T, char, wchar_t, char8_t, char16_t, char32_t>;

/**
 * Concept that is satisfied when the given type is like a supported std::basic_string
 * specialization. A type is "like" a std::basic_string specialization if it is that specialization
 * itself, a C-string equalivent, or a std::basic_string_view specialization.
 */
template <typename T>
concept StandardStringLike = !fly::SameAs<StandardStringType<T>, std::false_type>;

/**
 * Concept that is satisfied when a fly::string::Formatter<T, CharType> specialization is defined
 * for a type T, and that specialization implements a |format| method.
 */
template <typename T, typename FormatContext>
concept Formattable = requires(FormatContext context, T const &value)
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
concept FormattableString = StandardStringLike<T>;

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
concept FormattableBoolean = fly::SameAs<T, bool>;

/**
 * Concept that is satisfied when the given type is a formattable integral type, excluding boolean
 * types.
 */
template <typename T>
concept FormattableIntegral = fly::Integral<T>;

/**
 * Concept that is satisfied when the given type is a formattable floating-point type.
 */
template <typename T>
concept FormattableFloatingPoint = fly::FloatingPoint<T>;

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

/**
 * Concept that is satisfied when the given character is a valid Unicode prefix ('u' or 'U').
 */
template <char Ch>
concept UnicodePrefixCharacter = (Ch == 'u') || (Ch == 'U');

} // namespace fly
