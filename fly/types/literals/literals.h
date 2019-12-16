#pragma once

#include "fly/types/literals/detail/literal_parser.h"

#include <cstddef>
#include <cstdint>

/**
 * Fixed-width integer literal suffixes not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
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
