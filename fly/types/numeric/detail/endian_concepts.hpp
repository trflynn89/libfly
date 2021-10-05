#pragma once

#include "fly/traits/concepts.hpp"

namespace fly::detail {

/**
 * Concept that is satisfied if the given type is an integral type of a size supported by endian
 * operations.
 */
template <typename T>
concept EndianInteger = requires
{
    requires fly::Integral<T>;

    requires fly::SizeOfTypeIs<T, 1> || fly::SizeOfTypeIs<T, 2> || fly::SizeOfTypeIs<T, 4> ||
        fly::SizeOfTypeIs<T, 8>;
};

} // namespace fly::detail
