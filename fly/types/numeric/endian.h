#pragma once

#include "fly/fly.h"
#include "fly/types/numeric/detail/endian_traits.h"

#include <cstdint>
#include <type_traits>

#if defined(FLY_WINDOWS)
#    include <cstdlib>
#    define bswap_16(b) _byteswap_ushort(b)
#    define bswap_32(b) _byteswap_ulong(b)
#    define bswap_64(b) _byteswap_uint64(b)
#elif defined(FLY_LINUX)
#    include <byteswap.h>
#else
#    error Unknown byte swapping methods.
#endif

namespace fly {

/**
 * Enumeration to detect system endianness. Can be replaced by std::endian when
 * available in C++20.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
enum class Endian : std::uint16_t
{
#if defined(FLY_WINDOWS)
    Little = 0,
    Big = 1,
    Native = Little
#elif defined(FLY_LINUX)
    Little = __ORDER_LITTLE_ENDIAN__,
    Big = __ORDER_BIG_ENDIAN__,
    Native = __BYTE_ORDER__
#else
#    error Unknown system endianness.
#endif
};

/**
 * Templated wrapper around platform built-in byte swapping macros to convert a
 * value between system endianness and a desired endianness.
 *
 * @tparam Endian The desired endianness to swap between.
 * @tparam T The type of the value to swap.
 *
 * @param T The value to swap.
 *
 * @return T The swapped value.
 */
template <Endian endianness, typename T>
inline T endian_swap(T value) noexcept
{
    static_assert(
        detail::EndianTraits::is_supported_integer_v<T>,
        "Value must be an integer type of size 1, 2, 4, or 8 bytes");

    if constexpr ((endianness == Endian::Native) || (sizeof(T) == 1))
    {
        return value;
    }
    else if constexpr (sizeof(T) == 2)
    {
        return static_cast<T>(bswap_16(static_cast<std::uint16_t>(value)));
    }
    else if constexpr (sizeof(T) == 4)
    {
        return static_cast<T>(bswap_32(static_cast<std::uint32_t>(value)));
    }
    else if constexpr (sizeof(T) == 8)
    {
        return static_cast<T>(bswap_64(static_cast<std::uint64_t>(value)));
    }
}

} // namespace fly
