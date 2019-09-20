#pragma once

#include <cstddef>
#include <cstdint>

/**
 * Fixed-width integer literal suffixes not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */

constexpr inline std::int8_t operator"" _i8(unsigned long long int value)
{
    return static_cast<std::int8_t>(value);
}

constexpr inline std::int16_t operator"" _i16(unsigned long long int value)
{
    return static_cast<std::int16_t>(value);
}

constexpr inline std::int32_t operator"" _i32(unsigned long long int value)
{
    return static_cast<std::int32_t>(value);
}

constexpr inline std::int64_t operator"" _i64(unsigned long long int value)
{
    return static_cast<std::int64_t>(value);
}

constexpr inline std::uint8_t operator"" _u8(unsigned long long int value)
{
    return static_cast<std::uint8_t>(value);
}

constexpr inline std::uint16_t operator"" _u16(unsigned long long int value)
{
    return static_cast<std::uint16_t>(value);
}

constexpr inline std::uint32_t operator"" _u32(unsigned long long int value)
{
    return static_cast<std::uint32_t>(value);
}

constexpr inline std::uint64_t operator"" _u64(unsigned long long int value)
{
    return static_cast<std::uint64_t>(value);
}

constexpr inline std::size_t operator"" _zu(unsigned long long int value)
{
    return static_cast<std::size_t>(value);
}
