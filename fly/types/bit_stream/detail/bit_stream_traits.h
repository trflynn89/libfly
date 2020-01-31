#pragma once

#include "fly/types/bit_stream/bit_stream_types.h"

#include <type_traits>

namespace fly::detail {

/**
 * Traits for type safety in the BitStream template methods.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
struct BitStreamTraits
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

    /**
     * Define a trait for testing if type T is buffer_type.
     */
    template <typename T>
    using is_buffer_type =
        std::bool_constant<std::is_same_v<buffer_type, std::decay_t<T>>>;

    template <typename T>
    constexpr inline static bool is_buffer_type_v = is_buffer_type<T>::value;
};

} // namespace fly::detail
