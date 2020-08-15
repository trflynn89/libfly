#pragma once

#include "fly/fly.hpp"
#include "fly/types/numeric/detail/endian_traits.hpp"

#include <cstdint>
#include <type_traits>

#if defined(FLY_LINUX)
#    include <byteswap.h>
#elif defined(FLY_MACOS)
#    include <libkern/OSByteOrder.h>
#    include <machine/endian.h>
#elif defined(FLY_WINDOWS)
#    include <cstdlib>
#else
#    error Unknown byte swapping includes.
#endif

#if defined(FLY_LINUX)
#    define byte_swap_16(b) bswap_16(b)
#    define byte_swap_32(b) bswap_32(b)
#    define byte_swap_64(b) bswap_64(b)
#elif defined(FLY_MACOS)
#    define byte_swap_16(b) OSSwapInt16(b)
#    define byte_swap_32(b) OSSwapInt32(b)
#    define byte_swap_64(b) OSSwapInt64(b)
#elif defined(FLY_WINDOWS)
#    define byte_swap_16(b) _byteswap_ushort(b)
#    define byte_swap_32(b) _byteswap_ulong(b)
#    define byte_swap_64(b) _byteswap_uint64(b)
#else
#    error Unknown byte swapping methods.
#endif

namespace fly {

/**
 * Enumeration to detect system endianness. Can be replaced by std::endian when available in C++20.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
enum class Endian : std::uint16_t
{
#if defined(FLY_LINUX)
    Little = __ORDER_LITTLE_ENDIAN__,
    Big = __ORDER_BIG_ENDIAN__,
    Native = __BYTE_ORDER__,
#elif defined(FLY_MACOS)
    Little = LITTLE_ENDIAN,
    Big = BIG_ENDIAN,
    Native = BYTE_ORDER,
#elif defined(FLY_WINDOWS)
    Little = 0,
    Big = 1,
    Native = Little,
#else
#    error Unknown system endianness.
#endif
};

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
inline T endian_swap(T value)
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
template <Endian Endianness, typename T>
inline T endian_swap_if_non_native(T value)
{
    if constexpr (Endianness == Endian::Native)
    {
        return value;
    }
    else
    {
        return endian_swap(value);
    }
}

} // namespace fly
