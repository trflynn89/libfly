#pragma once

#include "fly/traits/concepts.hpp"

#include <concepts>
#include <type_traits>

namespace fly::detail {

/**
 * Concept that is satisfied if the given type is an integral type of a size supported by endian
 * operations.
 */
template <typename T>
concept EndianInteger = requires
{
    requires std::is_integral_v<std::remove_cvref_t<T>>;

    requires !std::same_as<std::remove_cvref_t<T>, bool>;

    requires fly::SizeOfTypeIs<T, 1> || fly::SizeOfTypeIs<T, 2> || fly::SizeOfTypeIs<T, 4> ||
        fly::SizeOfTypeIs<T, 8>;
};

} // namespace fly::detail
