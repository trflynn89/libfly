#pragma once

#include "fly/types/literals/detail/literal_parser.h"

#include <cstddef>
#include <cstdint>

/**
 * Type-safe, fixed-width integer literal suffixes not provided by the STL.
 *
 * The expression that precedes the literal suffix is parsed and validated at
 * compile time. Compilation will fail if any of the following error conditions
 * are met:
 *
 * 1. The expression preceding the literal suffix is invalid. All standard
 *    integer literals are accepted.
 * 2. The value represented by the preceding expression does not fit in the type
 *    specified by the suffix.
 * 3. A character in the preceding expression does not match the corresponding
 *    base (e.g. 0b2 is an invalid expression).
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version December 15, 2019
 */

template <char... Digits>
constexpr inline std::int8_t operator"" _i8()
{
    return fly::detail::literal<std::int8_t, Digits...>();
}

template <char... Digits>
constexpr inline std::int16_t operator"" _i16()
{
    return fly::detail::literal<std::int16_t, Digits...>();
}

template <char... Digits>
constexpr inline std::int32_t operator"" _i32()
{
    return fly::detail::literal<std::int32_t, Digits...>();
}

template <char... Digits>
constexpr inline std::int64_t operator"" _i64()
{
    return fly::detail::literal<std::int64_t, Digits...>();
}

template <char... Digits>
constexpr inline std::uint8_t operator"" _u8()
{
    return fly::detail::literal<std::uint8_t, Digits...>();
}

template <char... Digits>
constexpr inline std::uint16_t operator"" _u16()
{
    return fly::detail::literal<std::uint16_t, Digits...>();
}

template <char... Digits>
constexpr inline std::uint32_t operator"" _u32()
{
    return fly::detail::literal<std::uint32_t, Digits...>();
}

template <char... Digits>
constexpr inline std::uint64_t operator"" _u64()
{
    return fly::detail::literal<std::uint64_t, Digits...>();
}

template <char... Digits>
constexpr inline std::size_t operator"" _zu()
{
    return fly::detail::literal<std::size_t, Digits...>();
}
