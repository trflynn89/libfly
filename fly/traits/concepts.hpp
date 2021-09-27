#pragma once

#include "fly/traits/traits.hpp"

#include <concepts>
#include <type_traits>

namespace fly {

/**
 * Concept that is satisfied when any type in a sequence of types are the same as a specific type.
 * Examples:
 *
 *     fly::SameAsAny<int, int, int> // Satisfied.
 *     fly::SameAsAny<int, int, const int &> // Satisfied.
 *     fly::SameAsAny<int, int, bool> // Satisfied.
 *     fly::SameAsAny<int, bool, bool> // Unsatisfied.
 */
template <typename T, typename... Ts>
concept SameAsAny = (std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>> || ...);

/**
 * Concept that is satisfied when all types in a sequence of types are the same as a specific type.
 * Examples:
 *
 *     fly::SameAsAll<int, int, int> // Satisfied.
 *     fly::SameAsAll<int, int, const int &> // Satisfied.
 *     fly::SameAsAll<int, int, bool> // Unsatisfied.
 *     fly::SameAsAll<int, bool, bool> // Unsatisfied.
 */
template <typename T, typename... Ts>
concept SameAsAll = (std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<Ts>> && ...);

/**
 * Concept that is satisified if the given type is the provided size.
 */
template <typename T, std::size_t Size>
concept SizeOfTypeIs = fly::size_of_type_is_v<T, Size>;

/**
 * Concept that is satisified if the given type is a signed integral type.
 *
 * This concept will be removed once Apple's Clang supports std::signed_integral.
 */
template <typename T>
concept SignedIntegral = std::is_integral_v<T> && std::is_signed_v<T>;

/**
 * Concept that is satisified if the given type is an unsigned integral type.
 *
 * This concept will be removed once Apple's Clang supports std::unsigned_integral.
 */
template <typename T>
concept UnsignedIntegral = std::is_integral_v<T> && std::is_unsigned_v<T>;

/**
 * Concept that is satisified if the given type is a floating-point type.
 *
 * This concept will be removed once Apple's Clang supports std::floating_point.
 */
template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

} // namespace fly
