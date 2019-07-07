#pragma once

#include <fly/fly.h>

#include <cstdint>

#if defined(FLY_WINDOWS)
#    include <cstdlib>
#    define bswap_16(b) _byteswap_ushort(b)
#    define bswap_32(b) _byteswap_ulong(b)
#    define bswap_64(b) _byteswap_uint64(b)
#elif defined(FLY_LINUX)
#    include <byteswap.h>
#endif

namespace fly {

/**
 * Templated wrapper around platform built-in byte swapping macros.
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
    return bswap_16(byte);
}

//==============================================================================
template <>
inline std::uint32_t byte_swap(const std::uint32_t &byte) noexcept
{
    return bswap_32(byte);
}

//==============================================================================
template <>
inline std::uint64_t byte_swap(const std::uint64_t &byte) noexcept
{
    return bswap_64(byte);
}

} // namespace fly
