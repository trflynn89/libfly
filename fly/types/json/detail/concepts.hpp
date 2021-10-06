#pragma once

#include "fly/concepts/concepts.hpp"

#include <array>
#include <type_traits>

namespace fly::detail {

template <template <typename...> class, typename>
struct IsContainer : std::false_type
{
};

template <template <typename...> class T, typename... Args>
struct IsContainer<T, T<Args...>> : std::true_type
{
};

template <typename T, template <typename...> class Container>
concept SameAsContainerType = IsContainer<Container, T>::value;

template <typename>
struct IsFixedArray : std::false_type
{
};

template <typename T, std::size_t N>
struct IsFixedArray<std::array<T, N>> : std::true_type
{
};

template <typename T>
concept SameAsFixedArray = IsFixedArray<T>::value;

} // namespace fly::detail
