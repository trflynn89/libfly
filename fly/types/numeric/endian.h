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
 * @tparam T The type of the value to swap.
 * @tparam Endian The desired endianness to swap between.
 *
 * @param T The value to swap.
 *
 * @return T The swapped value.
 */
template <Endian endianness, typename T>
inline T endian_swap(T value) noexcept
{
    static_assert(
        detail::EndianTraits::is_unsigned_integer_v<T>,
        "Value must be an unsigned numeric type");

    if constexpr (endianness == Endian::Native)
    {
        return value;
    }
    else if constexpr (std::is_same_v<T, std::uint8_t>)
    {
        return value;
    }
    else if constexpr (std::is_same_v<T, std::uint16_t>)
    {
        return bswap_16(value);
    }
    else if constexpr (std::is_same_v<T, std::uint32_t>)
    {
        return bswap_32(value);
    }
    else if constexpr (std::is_same_v<T, std::uint64_t>)
    {
        return bswap_64(value);
    }
}

} // namespace fly
