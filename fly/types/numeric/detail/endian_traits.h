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
     * Define a trait for testing if type T is an unsigned integral type.
     */
    template <typename T>
    using is_unsigned_integer = std::bool_constant<
        std::is_integral_v<std::decay_t<T>> &&
        std::is_unsigned_v<std::decay_t<T>> &&
        !std::is_same_v<bool, std::decay_t<T>>>;

    template <typename T>
    constexpr inline static bool is_unsigned_integer_v =
        is_unsigned_integer<T>::value;
};

} // namespace fly::detail
