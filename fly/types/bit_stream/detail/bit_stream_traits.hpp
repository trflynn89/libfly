#pragma once

#include "fly/types/bit_stream/bit_stream_types.hpp"

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
    using is_unsigned_integer = std::conjunction<
        std::is_integral<std::decay_t<T>>,
        std::is_unsigned<std::decay_t<T>>,
        std::negation<std::is_same<bool, std::decay_t<T>>>>;

    template <typename T>
    constexpr inline static bool is_unsigned_integer_v = is_unsigned_integer<T>::value;

    /**
     * Define a trait for testing if type T is buffer_type.
     */
    template <typename T>
    using is_buffer_type = std::is_same<buffer_type, std::decay_t<T>>;

    template <typename T>
    constexpr inline static bool is_buffer_type_v = is_buffer_type<T>::value;
};

} // namespace fly::detail
