#pragma once

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
     * Aliases for JSON types. These could be specified as template parameters
     * to the JsonTraits class to allow callers to override the types. But these
     * are reasonable default types for now, and the Json class constructors
     * allow for type flexibility.
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
    using is_string =
        typename BasicString<string_type>::traits::template is_string_like<T>;

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
    using is_signed_integer = std::bool_constant<
        std::is_integral_v<std::decay_t<T>> &&
        std::is_signed_v<std::decay_t<T>> &&
        !std::is_same_v<bool, std::decay_t<T>>>;

    template <typename T>
    inline static constexpr bool is_signed_integer_v =
        is_signed_integer<T>::value;

    /**
     * Define a trait for testing if type T is an unsigned JSON number.
     */
    template <typename T>
    using is_unsigned_integer = std::bool_constant<
        std::is_integral_v<std::decay_t<T>> &&
        std::is_unsigned_v<std::decay_t<T>> &&
        !std::is_same_v<bool, std::decay_t<T>>>;

    template <typename T>
    inline static constexpr bool is_unsigned_integer_v =
        is_unsigned_integer<T>::value;

    /**
     * Define a trait for testing if type T is a floating-point JSON number.
     */
    template <typename T>
    using is_floating_point = std::is_floating_point<std::decay_t<T>>;

    template <typename T>
    inline static constexpr bool is_floating_point_v =
        is_floating_point<T>::value;

    /**
     * Define a trait for testing if type T is any JSON number type.
     */
    template <typename T>
    using is_number = std::bool_constant<
        is_signed_integer_v<std::decay_t<T>> ||
        is_unsigned_integer_v<std::decay_t<T>> ||
        is_floating_point_v<std::decay_t<T>>>;

    template <typename T>
    inline static constexpr bool is_number_v = is_number<T>::value;

    /**
     * Helper SFINAE struct to determine whether a type is a JSON object.
     */
    struct __object__
    {
        template <typename>
        struct is_object : std::false_type
        {
        };

        template <typename... Args>
        struct is_object<std::map<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_object<std::multimap<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_object<std::unordered_map<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_object<std::unordered_multimap<Args...>> : std::true_type
        {
        };
    };

    /**
     * Define a trait for testing if type T is a JSON object.
     */
    template <typename T>
    using is_object = __object__::is_object<T>;

    template <typename T>
    inline static constexpr bool is_object_v = is_object<T>::value;

    /**
     * Helper SFINAE struct to determine whether a type is a JSON array.
     */
    struct __array__
    {
        template <typename>
        struct is_array : std::false_type
        {
        };

        template <typename T, std::size_t N>
        struct is_array<std::array<T, N>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::deque<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::forward_list<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::list<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::multiset<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::set<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::unordered_multiset<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::unordered_set<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_array<std::vector<Args...>> : std::true_type
        {
        };
    };

    /**
     * Define a trait for testing if type T is a JSON array.
     */
    template <typename T>
    using is_array = __array__::is_array<T>;

    template <typename T>
    inline static constexpr bool is_array_v = is_array<T>::value;
};

} // namespace fly
