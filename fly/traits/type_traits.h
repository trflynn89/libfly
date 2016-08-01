#pragma once

#include <functional>
#include <iostream>
#include <type_traits>

/**
 * Custom type traits not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version May 30, 2016
 */
namespace fly {

namespace detail
{
    /**
     * Utility to get the type member for type T.
     */
    template <typename T>
    using invoke = typename T::type;

    /**
     * Wrapper to invoke a std::conditional.
     */
    template <typename If, typename Then, typename Else>
    using conditional = invoke<std::conditional<If::value, Then, Else>>;

    /**
     * Wrapper around a set of conditionals to check if all conditions are true.
     */
    template <typename ...> struct all : std::true_type { };

    template <typename Head, typename ... Tail>
    struct all<Head, Tail...> : conditional<Head, all<Tail...>, std::false_type> { };

    /**
     * Wrapper around a set of conditionals to check if any conditions are true.
     */
    template <typename ...> struct any : std::false_type { };

    template <typename Head, typename ... Tail>
    struct any<Head, Tail...> : conditional<Head, std::true_type, any<Tail...>> { };

    /**
     * Wrapper around a set of conditionals to check if all conditions are false.
     */
    template <typename ...> struct none : std::true_type { };

    template <typename Head, typename ... Tail>
    struct none<Head, Tail...> : conditional<Head, std::false_type, none<Tail...>> { };

    /**
     * Wrapper around a set of conditionals to check if any conditions are false.
     */
    template <typename ...> struct not_all : std::false_type { };

    template <typename Head, typename ... Tail>
    struct not_all<Head, Tail...> : conditional<Head, not_all<Tail...>, std::true_type> { };
}

/**
 * Wrapper around std::enable_if for testing that all coditions are true.
 *
 * For example:
 *
 *      template <typename T, enable_if_all<std::is_class<T>, std::is_empty<T>>...>
 *      void func(const T &) { }
 */
template <typename ... Conditions>
using enable_if_all = detail::invoke<std::enable_if<detail::all<Conditions...>::value>>;

/**
 * Wrapper around std::enable_if for testing that any coditions are true.
 *
 * For example:
 *
 *      template <typename T, enable_if_any<std::is_class<T>, std::is_pod<T>>...>
 *      void func(const T &) { }
 */
template <typename ... Conditions>
using enable_if_any = detail::invoke<std::enable_if<detail::any<Conditions...>::value>>;

/**
 * Wrapper around std::enable_if for testing that all coditions are false.
 *
 * For example:
 *
 *      template <typename T, enable_if_none<std::is_class<T>, std::is_pod<T>>...>
 *      void func(const T &) { }
 */
template <typename ... Conditions>
using enable_if_none = detail::invoke<std::enable_if<detail::none<Conditions...>::value>>;

/**
 * Wrapper around std::enable_if for testing that any coditions are false.
 *
 * For example:
 *
 *      template <typename T, enable_if_not_all<std::is_class<T>, std::is_pod<T>>...>
 *      void func(const T &) { }
 */
template <typename ... Conditions>
using enable_if_not_all = detail::invoke<std::enable_if<detail::not_all<Conditions...>::value>>;

/**
 * Wrapper around std::integral_constant for boolean constants.
 */
template <bool B>
using bool_constant = std::integral_constant<bool, B>;

/**
 * Define SFINAE tests for whether a function is declared for a type.
 *
 * For example, to test if a class of type T declares a function called Foo:
 *
 *      DECL_TESTS(foo, T, std::declval<const T &>().Foo());
 *
 * This will define these bool_constant wrappers around std::enable_if:
 *
 *      if_foo::enabled<T>
 *      if_foo::disabled<T>
 *
 * And may be used as SFINAE tests to, e.g., define a function depending on
 * whether or not Foo() was declared:
 *
 *      template <typename T, enable_if_all<if_foo::enabled<T>>...>
 *      void foo_wrapper(const T &arg) { arg.Foo(); }
 *
 *      template <typename T, enable_if_all<if_foo::disabled<T>>...>
 *      void foo_wrapper(const T &) { }
 *
 * @param label The name to give the test.
 * @param Type The type to be tested.
 * @param functor The function to test for.
 */
#define DECL_TESTS(label, Type, functor) \
namespace if_##label \
{ \
    namespace detail_##label \
    { \
        template <typename Type> auto test_##label(Type *) -> decltype(functor); \
        template <typename Type> auto test_##label(...) -> std::false_type; \
        \
        template <typename Type> \
        using is_undefined = std::is_same<decltype(test_##label<Type>(0)), std::false_type>; \
    } \
    \
    template <typename Type, typename S = bool> \
    using enabled = bool_constant<!detail_##label::is_undefined<Type>::value>; \
    \
    template <typename Type, typename S = bool> \
    using disabled = bool_constant<detail_##label::is_undefined<Type>::value>; \
}

/**
 * Tests for whether a given type is string-like.
 */
namespace if_string
{
    namespace detail_string
    {
        template <typename T>
        using is_string = bool_constant<
            std::is_same<char,         detail::invoke<std::decay<T>>>::value ||
            std::is_same<char *,       detail::invoke<std::decay<T>>>::value ||
            std::is_same<char const *, detail::invoke<std::decay<T>>>::value ||
            std::is_same<std::string,  detail::invoke<std::decay<T>>>::value>;
    }

    template <typename T, typename S = bool>
    using enabled = bool_constant<detail_string::is_string<T>::value>;

    template <typename T, typename S = bool>
    using disabled = bool_constant<!detail_string::is_string<T>::value>;
}

/**
 * Tests for whether a type defines operator<<.
 */
DECL_TESTS(ostream, T, std::declval<std::ostream &>() << std::declval<const T &>());

/**
 * Tests for whether a type is hashable with std::hash.
 */
DECL_TESTS(hash, T, std::declval<const std::hash<T> &>() (std::declval<const T &>()));

}
