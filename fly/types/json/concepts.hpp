#pragma once

#include "fly/concepts/concepts.hpp"
#include "fly/types/json/detail/concepts.hpp"
#include "fly/types/json/types.hpp"
#include "fly/types/string/concepts.hpp"

#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace fly {

/**
 * Concept that is satisfied when the given type is a JSON null type.
 */
template <typename T>
concept JsonNull = fly::SameAs<T, json_null_type>;

/**
 * Concept that is satisfied when the given type is a supported JSON string type.
 */
template <typename T>
concept JsonString = fly::StandardString<T>;

/**
 * Concept that is satisfied when the given type is a supported JSON string type.
 */
template <typename T>
concept JsonStringLike = fly::StandardStringLike<T>;

/**
 * Concept that is satisfied when the given type is a supported JSON object type.
 */
template <typename T>
concept JsonObject = requires
{
    typename std::remove_cvref_t<T>::key_type;
    typename std::remove_cvref_t<T>::mapped_type;

    requires JsonStringLike<typename std::remove_cvref_t<T>::key_type>;
    // TODO: Restrict T::mapped_type to allowed JSON types.

    requires detail::SameAsContainerType<std::remove_cvref_t<T>, std::map> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::multimap> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::unordered_map> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::unordered_multimap>;
};

/**
 * Concept that is satisfied when the given type is a supported JSON array type.
 */
template <typename T>
concept JsonArray = requires
{
    typename std::remove_cvref_t<T>::value_type;

    // TODO: Restrict U::value_type to allowed JSON types.

    requires detail::SameAsFixedArray<std::remove_cvref_t<T>> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::deque> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::forward_list> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::list> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::multiset> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::set> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::unordered_multiset> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::unordered_set> ||
        detail::SameAsContainerType<std::remove_cvref_t<T>, std::vector>;
};

/**
 * Concept that is satisfied when the given type is a JSON container type.
 */
template <typename T>
concept JsonContainer = JsonString<T> || JsonObject<T> || JsonArray<T>;

/**
 * Concept that is satisfied when the given type is a JSON Boolean type.
 */
template <typename T>
concept JsonBoolean = fly::SameAs<T, json_boolean_type>;

/**
 * Concept that is satisfied when the given type is a signed JSON number type.
 */
template <typename T>
concept JsonSignedInteger = fly::SignedIntegral<T>;

/**
 * Concept that is satisfied when the given type is an unsigned JSON number type.
 */
template <typename T>
concept JsonUnsignedInteger = fly::UnsignedIntegral<T>;

/**
 * Concept that is satisfied when the given type is a JSON floating-point number type.
 */
template <typename T>
concept JsonFloatingPoint = fly::FloatingPoint<T>;

/**
 * Concept that is satisfied when the given type is a JSON number type.
 */
template <typename T>
concept JsonNumber = JsonSignedInteger<T> || JsonUnsignedInteger<T> || JsonFloatingPoint<T>;

/**
 * Concept that is satisfied when the given type is an iterable JSON type.
 */
template <typename T>
concept JsonIterable = JsonObject<T> || JsonArray<T>;

} // namespace fly
