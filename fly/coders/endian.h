#pragma once

#include "fly/fly.h"

#include <cstdint>

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
 * @author Timothy Flynn (trflynn89@gmail.com)
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
#    error Unknown byte swapping methods.
#endif
};

/**
 * Templated wrapper around platform built-in byte swapping macros to convert a
 * byte to network-order.
 *
 * @tparam T The type of the byte to swap.
 *
 * @param T The byte to swap.
 *
 * @return T The swapped byte.
 */
template <typename T>
inline T byte_swap(const T &) noexcept;

//==============================================================================
template <>
inline std::uint8_t byte_swap(const std::uint8_t &byte) noexcept
{
    return byte;
}

//==============================================================================
template <>
inline std::uint16_t byte_swap(const std::uint16_t &byte) noexcept
{
    if constexpr (Endian::Native == Endian::Little)
    {
        return bswap_16(byte);
    }
    else
    {
        return byte;
    }
}

//==============================================================================
template <>
inline std::uint32_t byte_swap(const std::uint32_t &byte) noexcept
{
    if constexpr (Endian::Native == Endian::Little)
    {
        return bswap_32(byte);
    }
    else
    {
        return byte;
    }
}

//==============================================================================
template <>
inline std::uint64_t byte_swap(const std::uint64_t &byte) noexcept
{
    if constexpr (Endian::Native == Endian::Little)
    {
        return bswap_64(byte);
    }
    else
    {
        return byte;
    }
}

} // namespace fly
