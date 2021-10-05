#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/bit_stream/bit_stream_types.hpp"

namespace fly::detail {

/**
 * Concept that is satisfied if the given type is an unsigned integral type.
 */
template <typename T>
concept BitStreamInteger = fly::UnsignedIntegral<T>;

/**
 * Concept that is satisfied if the given type is the bit stream buffer type.
 */
template <typename T>
concept BitStreamBuffer = fly::SameAs<T, buffer_type>;

} // namespace fly::detail
