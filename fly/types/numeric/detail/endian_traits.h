#pragma once

#include <type_traits>

namespace fly::detail {

/**
 * Traits for type safety in the endian byte swapping template methods.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version December 30, 2019
 */
struct EndianTraits
{
    /**
     * Define a trait for testing if type T is a supported byte size.
     */
    template <typename T>
    using is_supported_size = std::bool_constant<
        (sizeof(T) == 1) || (sizeof(T) == 2) || (sizeof(T) == 4) ||
        (sizeof(T) == 8)>;

    template <typename T>
    constexpr inline static bool is_supported_size_v =
        is_supported_size<T>::value;

    /**
     * Define a trait for testing if type T is an integral type.
     */
    template <typename T>
    using is_supported_integer = std::bool_constant<
        std::is_integral_v<std::decay_t<T>> &&
        !std::is_same_v<bool, std::decay_t<T>> &&
        is_supported_size_v<std::decay_t<T>>>;

    template <typename T>
    constexpr inline static bool is_supported_integer_v =
        is_supported_integer<T>::value;
};

} // namespace fly::detail
