#pragma once

#include "fly/traits/traits.hpp"

#include <type_traits>

namespace fly {

/**
 * Concept that is satisified if the given type is the provided size.
 */
template <typename T, std::size_t Size>
concept SizeOfTypeIs = size_of_type_is_v<T, Size>;

/**
 * Concept that is satisified if the given type is not the provided size.
 */
template <typename T, std::size_t Size>
concept SizeOfTypeIsNot = std::negation_v<size_of_type_is<T, Size>>;

} // namespace fly
