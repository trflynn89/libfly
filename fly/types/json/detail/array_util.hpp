#pragma once

#include "fly/fly.hpp"

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <unordered_set>
#include <vector>

namespace fly::detail {

// std::array

template <typename T, std::size_t N>
inline std::size_t json_array_size(std::array<T, N> const &array)
{
    return array.size();
}

// std::deque

template <typename... Args>
inline void
json_array_append(std::deque<Args...> &array, typename std::deque<Args...>::value_type &&value)
{
    array.push_back(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::deque<Args...> const &array)
{
    return array.size();
}

// std::forward_list

template <typename... Args>
inline void json_array_append(
    std::forward_list<Args...> &array,
    typename std::forward_list<Args...>::value_type &&value)
{
    auto before_end = array.before_begin();

    for (auto const &element : array)
    {
        FLY_UNUSED(element);
        ++before_end;
    }

    array.insert_after(before_end, std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::forward_list<Args...> const &array)
{
    std::size_t elements = 0;

    for (auto const &element : array)
    {
        FLY_UNUSED(element);
        ++elements;
    }

    return elements;
}

// std::list

template <typename... Args>
inline void
json_array_append(std::list<Args...> &array, typename std::list<Args...>::value_type &&value)
{
    array.push_back(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::list<Args...> const &array)
{
    return array.size();
}

// std::multiset

template <typename... Args>
inline void json_array_append(
    std::multiset<Args...> &array,
    typename std::multiset<Args...>::value_type &&value)
{
    array.insert(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::multiset<Args...> const &array)
{
    return array.size();
}

// std::set

template <typename... Args>
inline void
json_array_append(std::set<Args...> &array, typename std::set<Args...>::value_type &&value)
{
    array.insert(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::set<Args...> const &array)
{
    return array.size();
}

// std::unordered_multiset

template <typename... Args>
inline void json_array_append(
    std::unordered_multiset<Args...> &array,
    typename std::unordered_multiset<Args...>::value_type &&value)
{
    array.insert(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::unordered_multiset<Args...> const &array)
{
    return array.size();
}

// std::unordered_set

template <typename... Args>
inline void json_array_append(
    std::unordered_set<Args...> &array,
    typename std::unordered_set<Args...>::value_type &&value)
{
    array.insert(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::unordered_set<Args...> const &array)
{
    return array.size();
}

// std::vector

template <typename... Args>
inline void
json_array_append(std::vector<Args...> &array, typename std::vector<Args...>::value_type &&value)
{
    array.push_back(std::move(value));
}

template <typename... Args>
inline std::size_t json_array_size(std::vector<Args...> const &array)
{
    return array.size();
}

} // namespace fly::detail
