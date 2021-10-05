#pragma once

#include "fly/traits/traits.hpp"

#include <concepts>
#include <type_traits>

namespace fly {

/**
 * Concept that is satisfied if the provided types are the same types, after removing cv-qualifiers.
 */
template <typename T, typename U>
concept SameAs = std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

/**
 * Concept that is satisfied if any type in a sequence of types are the same as a specific type.
 * Examples:
 *
 *     fly::SameAsAny<int, int, int> // Satisfied.
 *     fly::SameAsAny<int, int, const int &> // Satisfied.
 *     fly::SameAsAny<int, int, bool> // Satisfied.
 *     fly::SameAsAny<int, bool, bool> // Unsatisfied.
 */
template <typename T, typename... Us>
concept SameAsAny = (fly::SameAs<T, Us> || ...);

/**
 * Concept that is satisfied if all types in a sequence of types are the same as a specific type.
 * Examples:
 *
 *     fly::SameAsAll<int, int, int> // Satisfied.
 *     fly::SameAsAll<int, int, const int &> // Satisfied.
 *     fly::SameAsAll<int, int, bool> // Unsatisfied.
 *     fly::SameAsAll<int, bool, bool> // Unsatisfied.
 */
template <typename T, typename... Us>
concept SameAsAll = (fly::SameAs<T, Us> && ...);

/**
 * Concept that is satisified if the given type is the provided size.
 */
template <typename T, std::size_t Size>
concept SizeOfTypeIs = (sizeof(T) == Size);

/**
 * Concept that is satisified if the given type is an integral, non-boolean type.
 */
template <typename T>
concept Integral = std::is_integral_v<std::remove_cvref_t<T>> && !fly::SameAs<T, bool>;

/**
 * Concept that is satisified if the given type is a signed integral, non-boolean type.
 */
template <typename T>
concept SignedIntegral = fly::Integral<T> && std::is_signed_v<std::remove_cvref_t<T>>;

/**
 * Concept that is satisified if the given type is an unsigned integral, non-boolean type.
 */
template <typename T>
concept UnsignedIntegral = fly::Integral<T> && std::is_unsigned_v<std::remove_cvref_t<T>>;

/**
 * Concept that is satisified if the given type is a floating-point type.
 */
template <typename T>
concept FloatingPoint = std::is_floating_point_v<std::remove_cvref_t<T>>;

} // namespace fly
