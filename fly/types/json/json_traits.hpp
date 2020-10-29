#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <string>
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
     * Aliases for JSON types. These could be specified as template parameters to the JsonTraits
     * class to allow callers to override the types. But these are reasonable default types for now,
     * and the Json class constructors allow for type flexibility.
     */
    using null_type = std::nullptr_t;
    using string_type = std::string;
    using boolean_type = bool;
    using signed_type = std::intmax_t;
    using unsigned_type = std::uintmax_t;
    using float_type = long double;
    using object_type = std::map<string_type, Json>;
    using array_type = std::vector<Json>;

    /**
     * Define a trait for testing if type T is a JSON string.
     */
    template <typename T>
    using is_string = detail::is_like_supported_string<T>;

    template <typename T>
    using is_string_t = typename is_string<T>::type;

    template <typename T>
    inline static constexpr bool is_string_v = is_string<T>::value;

    /**
     * Define a trait for testing if type T is a JSON boolean.
     */
    template <typename T>
    using is_boolean = std::is_same<bool, std::decay_t<T>>;

    template <typename T>
    inline static constexpr bool is_boolean_v = is_boolean<T>::value;

    /**
     * Define a trait for testing if type T is a signed JSON number.
     */
    template <typename T>
    using is_signed_integer = std::conjunction<
        std::is_integral<std::decay_t<T>>,
        std::is_signed<std::decay_t<T>>,
        std::negation<std::is_same<bool, std::decay_t<T>>>>;

    template <typename T>
    inline static constexpr bool is_signed_integer_v = is_signed_integer<T>::value;

    /**
     * Define a trait for testing if type T is an unsigned JSON number.
     */
    template <typename T>
    using is_unsigned_integer = std::conjunction<
        std::is_integral<std::decay_t<T>>,
        std::is_unsigned<std::decay_t<T>>,
        std::negation<std::is_same<bool, std::decay_t<T>>>>;

    template <typename T>
    inline static constexpr bool is_unsigned_integer_v = is_unsigned_integer<T>::value;

    /**
     * Define a trait for testing if type T is a floating-point JSON number.
     */
    template <typename T>
    using is_floating_point = std::is_floating_point<std::decay_t<T>>;

    template <typename T>
    inline static constexpr bool is_floating_point_v = is_floating_point<T>::value;

    /**
     * Define a trait for testing if type T is any JSON number type.
     */
    template <typename T>
    using is_number = std::disjunction<
        is_signed_integer<std::decay_t<T>>,
        is_unsigned_integer<std::decay_t<T>>,
        is_floating_point<std::decay_t<T>>>;

    template <typename T>
    inline static constexpr bool is_number_v = is_number<T>::value;

    /**
     * Helper SFINAE struct to determine whether a type is a JSON object.
     */
    struct ObjectTraits
    {
        template <typename Key>
        using is_string_key = std::bool_constant<is_string_v<Key>>;

        template <typename>
        struct IsObject : std::false_type
        {
        };

        template <typename Key, typename... Args>
        struct IsObject<std::map<Key, Args...>> : is_string_key<Key>
        {
        };

        template <typename Key, typename... Args>
        struct IsObject<std::multimap<Key, Args...>> : is_string_key<Key>
        {
        };

        template <typename Key, typename... Args>
        struct IsObject<std::unordered_map<Key, Args...>> : is_string_key<Key>
        {
        };

        template <typename Key, typename... Args>
        struct IsObject<std::unordered_multimap<Key, Args...>> : is_string_key<Key>
        {
        };
    };

    /**
     * Define a trait for testing if type T is a JSON object.
     */
    template <typename T>
    using is_object = ObjectTraits::IsObject<std::decay_t<T>>;

    template <typename T>
    inline static constexpr bool is_object_v = is_object<T>::value;

    /**
     * Helper SFINAE struct to determine whether a type is a JSON array, and wrapper methods to
     * generically modify an array.
     */
    struct ArrayTraits
    {
        template <typename>
        struct IsArray : std::false_type
        {
        };

        // std::array

        template <typename T, std::size_t N>
        struct IsArray<std::array<T, N>> : std::true_type
        {
        };

        // std::deque

        template <typename... Args>
        struct IsArray<std::deque<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void
        append(std::deque<Args...> &array, typename std::deque<Args...>::value_type &&value)
        {
            array.push_back(std::move(value));
        }

        // std::forward_list

        template <typename... Args>
        struct IsArray<std::forward_list<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void append(
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

        // std::list

        template <typename... Args>
        struct IsArray<std::list<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void
        append(std::list<Args...> &array, typename std::list<Args...>::value_type &&value)
        {
            array.push_back(std::move(value));
        }

        // std::multiset

        template <typename... Args>
        struct IsArray<std::multiset<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void
        append(std::multiset<Args...> &array, typename std::multiset<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        // std::set

        template <typename... Args>
        struct IsArray<std::set<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void append(std::set<Args...> &array, typename std::set<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        // std::unordered_multiset

        template <typename... Args>
        struct IsArray<std::unordered_multiset<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void append(
            std::unordered_multiset<Args...> &array,
            typename std::unordered_multiset<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        // std::unordered_set

        template <typename... Args>
        struct IsArray<std::unordered_set<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void append(
            std::unordered_set<Args...> &array,
            typename std::unordered_set<Args...>::value_type &&value)
        {
            array.insert(std::move(value));
        }

        // std::vector

        template <typename... Args>
        struct IsArray<std::vector<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        static void
        append(std::vector<Args...> &array, typename std::vector<Args...>::value_type &&value)
        {
            array.push_back(std::move(value));
        }
    };

    /**
     * Define a trait for testing if type T is a JSON array.
     */
    template <typename T>
    using is_array = ArrayTraits::IsArray<std::decay_t<T>>;

    template <typename T>
    inline static constexpr bool is_array_v = is_array<T>::value;

    /**
     * Define a trait for testing if type T is an iterable JSON type.
     */
    template <typename T>
    using is_iterable = std::disjunction<is_object<T>, is_array<T>>;

    template <typename T>
    inline static constexpr bool is_iterable_v = is_iterable<T>::value;
};

} // namespace fly
