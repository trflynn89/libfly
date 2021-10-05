#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/bit_stream/bit_stream_types.hpp"

#include <concepts>
#include <type_traits>

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
concept BitStreamBuffer = std::same_as<std::remove_cvref_t<T>, buffer_type>;

} // namespace fly::detail
