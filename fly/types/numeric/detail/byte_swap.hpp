#pragma once

#include "fly/fly.hpp"

#include <cstdint>

#if defined(FLY_LINUX)
#    include <byteswap.h>
#elif defined(FLY_MACOS)
#    include <libkern/OSByteOrder.h>
#elif defined(FLY_WINDOWS)
#    include "fly/types/numeric/literals.hpp"

#    include <cstdlib>
#    include <type_traits>
#else
#    error Unknown byte swapping includes.
#endif

namespace fly::detail {

#if defined(FLY_LINUX)

constexpr std::uint16_t byte_swap(std::uint16_t value)
{
    return __builtin_bswap16(value);
}

constexpr std::uint32_t byte_swap(std::uint32_t value)
{
    return __builtin_bswap32(value);
}

constexpr std::uint64_t byte_swap(std::uint64_t value)
{
    return __builtin_bswap64(value);
}

#elif defined(FLY_MACOS)

constexpr std::uint16_t byte_swap(std::uint16_t value)
{
    return OSSwapInt16(value);
}

constexpr std::uint32_t byte_swap(std::uint32_t value)
{
    return OSSwapInt32(value);
}

constexpr std::uint64_t byte_swap(std::uint64_t value)
{
    return OSSwapInt64(value);
}

#elif defined(FLY_WINDOWS)

// Built-in byte swapping methods on Windows are non-constexpr. To enable invoking byte swapping at
// compile time, use shift-based byte swapping methods when evaluated in a constant context. See:
//
// https://developercommunity.visualstudio.com/t/constexpr-byte-swapping-optimization/983963
// https://developercommunity.visualstudio.com/t/Mark-some-built-in-functions-as-constexp/362558?entry=myfeedback&refTime=1620723793774&refUserId=f9653689-1bb1-4374-8703-3fb61971d3f5

constexpr std::uint16_t byte_swap(std::uint16_t value)
{
    if (std::is_constant_evaluated())
    {
        using namespace fly::literals::numeric_literals;

        return ((value & 0xff00_u16) >> 8) | ((value & 0x00ff_u16) << 8);
    }
    else
    {
        return _byteswap_ushort(value);
    }
}

constexpr std::uint32_t byte_swap(std::uint32_t value)
{
    if (std::is_constant_evaluated())
    {
        using namespace fly::literals::numeric_literals;

        return (
            ((value & 0xff00'0000_u32) >> 24) | ((value & 0x00ff'0000_u32) >> 8) |
            ((value & 0x0000'ff00_u32) << 8) | ((value & 0x0000'00ff_u32) << 24));
    }
    else
    {
        return _byteswap_ulong(value);
    }
}

constexpr std::uint64_t byte_swap(std::uint64_t value)
{
    if (std::is_constant_evaluated())
    {
        using namespace fly::literals::numeric_literals;

        return (
            ((value & 0xff00'0000'0000'0000_u64) >> 56) |
            ((value & 0x00ff'0000'0000'0000_u64) >> 40) |
            ((value & 0x0000'ff00'0000'0000_u64) >> 24) |
            ((value & 0x0000'00ff'0000'0000_u64) >> 8) |
            ((value & 0x0000'0000'ff00'0000_u64) << 8) |
            ((value & 0x0000'0000'00ff'0000_u64) << 24) |
            ((value & 0x0000'0000'0000'ff00_u64) << 40) |
            ((value & 0x0000'0000'0000'00ff_u64) << 56));
    }
    else
    {
        return _byteswap_uint64(value);
    }
}

#else
#    error Unknown byte swapping methods.
#endif

} // namespace fly::detail
