#pragma once

#include <functional>
#include <iostream>
#include <type_traits>

/**
 * Custom type traits not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 28, 2017
*/
namespace fly {

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
 * This will define these wrappers around std::enable_if:
 *
 *      fly::if_foo::enabled<T>
 *      fly::if_foo::disabled<T>
 *
 * And may be used as SFINAE tests to, e.g., define a function depending on
 * whether or not Foo() was declared:
 *
 *      template <typename T, fly::if_foo::enabled<T> = 0>
 *      void foo_wrapper(const T &arg) { arg.Foo(); }
 *
 *      template <typename T, fly::if_foo::disabled<T> = 0>
 *      void foo_wrapper(const T &) { }
 *
 * @param label The name to give the test.
 * @param Type The type to be tested.
 * @param functor The function to test for.
 */
#define DECL_TESTS(label, Type, functor) \
struct if_##label \
{ \
    struct __##label \
    { \
        template <typename Type> \
        static constexpr auto test_##label(Type *) -> decltype(functor); \
        \
        template <typename Type> \
        static constexpr auto test_##label(...) -> std::false_type; \
        \
        template <typename Type> \
        using is_undefined = std::is_same<decltype(test_##label<Type>(0)), std::false_type>; \
    }; \
    \
    template <typename Type> \
    using enabled = std::enable_if_t<!__##label::is_undefined<Type>::value, bool>; \
    \
    template <typename Type> \
    using disabled = std::enable_if_t<__##label::is_undefined<Type>::value, bool>; \
    \
    template <typename Type> \
    using value = bool_constant<!__##label::is_undefined<Type>::value>; \
};
 
/**
 * Tests for whether a given type is string-like.
 */
struct if_string
{
    struct __string
    {
        template <typename T>
        using is_string = bool_constant<
            std::is_same<char,         std::decay_t<T>>::value ||
            std::is_same<char *,       std::decay_t<T>>::value ||
            std::is_same<char const *, std::decay_t<T>>::value ||
            std::is_same<std::string,  std::decay_t<T>>::value>;
    };

    template <typename T>
    using enabled = std::enable_if_t<__string::is_string<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__string::is_string<T>::value, bool>;
};

/**
 * Tests for whether a type defines operator<<.
 */
DECL_TESTS(ostream, T, std::declval<std::ostream &>() << std::declval<const T &>());

/**
 * Tests for whether a type is hashable with std::hash.
 */
DECL_TESTS(hash, T, std::declval<const std::hash<T> &>() (std::declval<const T &>()));

}
