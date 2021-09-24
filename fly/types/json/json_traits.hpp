#pragma once

#include "fly/fly.hpp"
#include "fly/types/json/json_types.hpp"

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace fly {

class Json;

/**
 * Traits for basic properties of JSON types defined by https://www.json.org.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 23, 2019
 */
struct JsonTraits
{
    /**
     * Helper SFINAE struct to determine whether a type is a JSON array, and wrapper methods to
     * generically modify an array.
     */
    struct ArrayTraits
    {
        // std::array

        template <typename T, std::size_t N>
        static inline std::size_t size(const std::array<T, N> &array)
        {
            return array.size();
        }

        // std::deque

        template <typename... Args>
        static inline void
        append(std::deque<Args...> &array, typename std::deque<Args...>::value_type &&value)
        {
            array.push_back(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::deque<Args...> &array)
        {
            return array.size();
        }

        // std::forward_list

        template <typename... Args>
        static inline void append(
            std::forward_list<Args...> &array,
            typename std::forward_list<Args...>::value_type &&value)
        {
            auto before_end = array.before_begin();

            for (const auto &element : array)
            {
                FLY_UNUSED(element);
                ++before_end;
            }

            array.insert_after(before_end, std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::forward_list<Args...> &array)
        {
            std::size_t elements = 0;

            for (const auto &element : array)
            {
                FLY_UNUSED(element);
                ++elements;
            }

            return elements;
        }

        // std::list

        template <typename... Args>
        static inline void
        append(std::list<Args...> &array, typename std::list<Args...>::value_type &&value)
        {
            array.push_back(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::list<Args...> &array)
        {
            return array.size();
        }

        // std::multiset

        template <typename... Args>
        static inline void
        append(std::multiset<Args...> &array, typename std::multiset<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::multiset<Args...> &array)
        {
            return array.size();
        }

        // std::set

        template <typename... Args>
        static inline void
        append(std::set<Args...> &array, typename std::set<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::set<Args...> &array)
        {
            return array.size();
        }

        // std::unordered_multiset

        template <typename... Args>
        static inline void append(
            std::unordered_multiset<Args...> &array,
            typename std::unordered_multiset<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::unordered_multiset<Args...> &array)
        {
            return array.size();
        }

        // std::unordered_set

        template <typename... Args>
        static inline void append(
            std::unordered_set<Args...> &array,
            typename std::unordered_set<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::unordered_set<Args...> &array)
        {
            return array.size();
        }

        // std::vector

        template <typename... Args>
        static inline void
        append(std::vector<Args...> &array, typename std::vector<Args...>::value_type &&value)
        {
            array.push_back(std::move(value));
        }

        template <typename... Args>
        static inline std::size_t size(const std::vector<Args...> &array)
        {
            return array.size();
        }
    };
};

} // namespace fly
