#pragma once

#include "fly/concepts/concepts.hpp"
#include "fly/types/numeric/detail/byte_swap.hpp"
#include "fly/types/numeric/detail/endian_concepts.hpp"

#include <bit>
#include <cstdint>

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
template <detail::EndianInteger T>
constexpr T endian_swap(T value)
{
    if constexpr (fly::SizeOfTypeIs<T, 1>)
    {
        return value;
    }
    else if constexpr (fly::SizeOfTypeIs<T, 2>)
    {
        return static_cast<T>(detail::byte_swap(static_cast<std::uint16_t>(value)));
    }
    else if constexpr (fly::SizeOfTypeIs<T, 4>)
    {
        return static_cast<T>(detail::byte_swap(static_cast<std::uint32_t>(value)));
    }
    else if constexpr (fly::SizeOfTypeIs<T, 8>)
    {
        return static_cast<T>(detail::byte_swap(static_cast<std::uint64_t>(value)));
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
template <std::endian Endianness, detail::EndianInteger T>
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
