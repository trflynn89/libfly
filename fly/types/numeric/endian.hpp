#pragma once

#include "fly/fly.hpp"
#include "fly/types/numeric/detail/endian_traits.hpp"

#include <bit>
#include <cstdint>
#include <type_traits>

#if defined(FLY_LINUX)
#    include <byteswap.h>
#elif defined(FLY_MACOS)
#    include <libkern/OSByteOrder.h>
#elif defined(FLY_WINDOWS)
#    include "fly/types/numeric/literals.hpp"
#else
#    error Unknown byte swapping includes.
#endif

#if defined(FLY_LINUX)
#    define byte_swap_16(b) __builtin_bswap16(b)
#    define byte_swap_32(b) __builtin_bswap32(b)
#    define byte_swap_64(b) __builtin_bswap64(b)
#elif defined(FLY_MACOS)
#    define byte_swap_16(b) OSSwapInt16(b)
#    define byte_swap_32(b) OSSwapInt32(b)
#    define byte_swap_64(b) OSSwapInt64(b)
#elif defined(FLY_WINDOWS)

// Windows has _byteswap_ushort, _byteswap_ulong, and _byteswap_uint64, but they are non-constexpr.
// So to allow endian swapping to be used at compile time, use custom byte swapping methods.

constexpr std::uint16_t byte_swap_16(std::uint16_t value)
{
    using namespace fly::literals::numeric_literals;

    return ((value & 0xff00_u16) >> 8) | ((value & 0x00ff_u16) << 8);
}

constexpr std::uint32_t byte_swap_32(std::uint32_t value)
{
    using namespace fly::literals::numeric_literals;

    return (
        ((value & 0xff00'0000_u32) >> 24) | ((value & 0x00ff'0000_u32) >> 8) |
        ((value & 0x0000'ff00_u32) << 8) | ((value & 0x0000'00ff_u32) << 24));
}

constexpr std::uint64_t byte_swap_64(std::uint64_t value)
{
    using namespace fly::literals::numeric_literals;

    return (
        ((value & 0xff00'0000'0000'0000_u64) >> 56) | ((value & 0x00ff'0000'0000'0000_u64) >> 40) |
        ((value & 0x0000'ff00'0000'0000_u64) >> 24) | ((value & 0x0000'00ff'0000'0000_u64) >> 8) |
        ((value & 0x0000'0000'ff00'0000_u64) << 8) | ((value & 0x0000'0000'00ff'0000_u64) << 24) |
        ((value & 0x0000'0000'0000'ff00_u64) << 40) | ((value & 0x0000'0000'0000'00ff_u64) << 56));
}

#else
#    error Unknown byte swapping methods.
#endif

namespace fly {

/**
 * Templated wrapper around platform built-in byte swapping macros to change a value's endianness.
 *
 * @tparam T The type of the value to swap.
 *
 * @param value The value to swap.
 *
 * @return The swapped value.
 */
template <typename T>
constexpr T endian_swap(T value)
{
    static_assert(
        detail::EndianTraits::is_supported_integer_v<T>,
        "Value must be an integer type of size 1, 2, 4, or 8 bytes");

    if constexpr (sizeof(T) == 1)
    {
        return value;
    }
    else if constexpr (sizeof(T) == 2)
    {
        return static_cast<T>(byte_swap_16(static_cast<std::uint16_t>(value)));
    }
    else if constexpr (sizeof(T) == 4)
    {
        return static_cast<T>(byte_swap_32(static_cast<std::uint32_t>(value)));
    }
    else if constexpr (sizeof(T) == 8)
    {
        return static_cast<T>(byte_swap_64(static_cast<std::uint64_t>(value)));
    }
}

/**
 * Templated wrapper around platform built-in byte swapping macros to convert a value between system
 * endianness and a desired endianness.
 *
 * @tparam Endian The desired endianness to swap between.
 * @tparam T The type of the value to swap.
 *
 * @param value The value to swap.
 *
 * @return The swapped value.
 */
template <std::endian Endianness, typename T>
constexpr T endian_swap_if_non_native(T value)
{
    if constexpr (Endianness == std::endian::native)
    {
        return value;
    }
    else
    {
        return endian_swap(value);
    }
}

} // namespace fly
