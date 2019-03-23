#pragma once

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

/**
 * Define SFINAE tests for whether a function is declared for a type.
 *
 * For example, to test if a class of type T declares a function called Foo:
 *
 *      DECL_TESTS(foo, T, std::declval<const T &>().Foo());
 *
 * This will define these wrappers around std::enable_if:
 *
 *      if_foo::enabled<T>
 *      if_foo::disabled<T>
 *
 * And may be used as SFINAE tests to, e.g., define a function depending on
 * whether or not Foo() was declared:
 *
 *      template <typename T, if_foo::enabled<T> = 0>
 *      void foo_wrapper(const T &arg) { arg.Foo(); }
 *
 *      template <typename T, if_foo::disabled<T> = 0>
 *      void foo_wrapper(const T &) { }
 *
 * @param label The name to give the test.
 * @param Type The type to be tested.
 * @param functor The function to test for.
 */
#define FLY_DECLARATION_TESTS(label, Type, functor)                            \
    struct if_##label                                                          \
    {                                                                          \
        struct __##label                                                       \
        {                                                                      \
            template <typename Type>                                           \
            static constexpr auto test_##label(Type *) -> decltype(functor);   \
                                                                               \
            template <typename Type>                                           \
            static constexpr auto test_##label(...) -> std::false_type;        \
                                                                               \
            template <typename Type>                                           \
            inline static constexpr bool is_undefined = std::is_same<          \
                decltype(test_##label<Type>(0)),                               \
                std::false_type>::value;                                       \
        };                                                                     \
                                                                               \
        template <typename Type>                                               \
        using enabled =                                                        \
            std::enable_if_t<!__##label::is_undefined<Type>, bool>;            \
                                                                               \
        template <typename Type>                                               \
        using disabled =                                                       \
            std::enable_if_t<__##label::is_undefined<Type>, bool>;             \
    };

/**
 * Type traits not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 28, 2017
 */
namespace fly {

/**
 * Wrapper around std::enable_if for testing that all coditions in a sequence of
 * traits are true. Example:
 *
 *      template <
 *          typename T,
 *          fly::enable_if_all<std::is_class<T>, std::is_empty<T>> = 0>
 *      void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_all = std::enable_if_t<std::conjunction_v<Conditions...>, bool>;

/**
 * Wrapper around std::enable_if for testing that any codition in a sequence of
 * traits is true. Example:
 *
 *      template <
 *          typename T,
 *          fly::enable_if_any<std::is_class<T>, std::is_empty<T>> = 0>
 *      void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_any = std::enable_if_t<std::disjunction_v<Conditions...>, bool>;

/**
 * Wrapper around std::enable_if for testing that all coditions in a sequence of
 * traits are false. Example:
 *
 *      template <
 *          typename T,
 *          fly::enable_if_none<std::is_class<T>, std::is_empty<T>> = 0>
 *      void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_none =
    std::enable_if_t<std::negation_v<std::disjunction<Conditions...>>, bool>;

/**
 * Wrapper around std::enable_if for testing that any codition in a sequence of
 * traits is false. Example:
 *
 *      template <
 *          typename T,
 *          fly::enable_if_not_all<std::is_class<T>, std::is_empty<T>> = 0>
 *      void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_not_all =
    std::enable_if_t<std::negation_v<std::conjunction<Conditions...>>, bool>;

/**
 * Tests for whether a given type is a signed integer.
 */
struct if_signed_integer
{
    struct __
    {
        template <typename T>
        inline static constexpr bool is_signed_integer = std::bool_constant<
            std::is_integral_v<std::decay_t<T>> &&
            std::is_signed_v<std::decay_t<T>> &&
            !std::is_same_v<bool, std::decay_t<T>>>::value;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_signed_integer<T>, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_signed_integer<T>, bool>;
};

/**
 * Tests for whether a given type is an unsigned integer.
 */
struct if_unsigned_integer
{
    struct __
    {
        template <typename T>
        inline static constexpr bool is_unsigned_integer = std::bool_constant<
            std::is_integral_v<std::decay_t<T>> &&
            std::is_unsigned_v<std::decay_t<T>> &&
            !std::is_same_v<bool, std::decay_t<T>>>::value;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_unsigned_integer<T>, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_unsigned_integer<T>, bool>;
};

/**
 * Tests for whether a given type is a float.
 */
struct if_floating_point
{
    struct __
    {
        template <typename T>
        inline static constexpr bool is_floating_point = std::bool_constant<
            std::is_floating_point_v<std::decay_t<T>>>::value;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_floating_point<T>, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_floating_point<T>, bool>;
};

/**
 * Tests for whether a given type is numeric.
 */
struct if_numeric
{
    struct __
    {
        template <typename T>
        inline static constexpr bool is_numeric = std::bool_constant<
            std::is_arithmetic_v<std::decay_t<T>> &&
            !std::is_same_v<bool, std::decay_t<T>>>::value;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_numeric<T>, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_numeric<T>, bool>;
};

/**
 * Tests for whether a given type is a boolean.
 */
struct if_boolean
{
    struct __
    {
        template <typename T>
        inline static constexpr bool is_boolean =
            std::bool_constant<std::is_same_v<bool, std::decay_t<T>>>::value;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_boolean<T>, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_boolean<T>, bool>;
};

/**
 * Tests for whether a given type is map-like (i.e. holds key-value pairs).
 */
struct if_map
{
    struct __
    {
        template <typename>
        struct is_map : std::false_type
        {
        };

        template <typename... Args>
        struct is_map<std::map<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_map<std::multimap<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_map<std::unordered_map<Args...>> : std::true_type
        {
        };

        template <typename... Args>
        struct is_map<std::unordered_multimap<Args...>> : std::true_type
        {
        };
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_map<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_map<T>::value, bool>;
};

/**
 * Tests for whether a given type is array-like (i.e. holds value sequences).
 */
struct if_array
{
    struct __
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

    template <typename T>
    using enabled = std::enable_if_t<__::is_array<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_array<T>::value, bool>;
};

} // namespace fly
