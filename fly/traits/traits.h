#pragma once

#include <iostream>
#include <type_traits>
#include <utility>

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
#define DECLARATION_TESTS(label, Type, functor) \
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
};

/**
 * Custom type traits not provided by the STL.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 28, 2017
*/
namespace fly {

/**
 * Tests for whether a given type is string-like.
 */
struct if_string
{
    struct __
    {
        template <typename T>
        using is_string = std::integral_constant<bool,
            std::is_same<char *,       std::decay_t<T>>::value ||
            std::is_same<char const *, std::decay_t<T>>::value ||
            std::is_same<std::string,  std::decay_t<T>>::value
        >;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_string<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_string<T>::value, bool>;
};

/**
 * Tests for whether a given type is a signed integer.
 */
struct if_signed_integer
{
    struct __
    {
        template <typename T>
        using is_signed_integer = std::integral_constant<bool,
            std::is_integral<std::decay_t<T>>::value &&
            std::is_signed<std::decay_t<T>>::value &&
            !std::is_same<bool, std::decay_t<T>>::value
        >;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_signed_integer<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_signed_integer<T>::value, bool>;
};

/**
 * Tests for whether a given type is an unsigned integer.
 */
struct if_unsigned_integer
{
    struct __
    {
        template <typename T>
        using is_unsigned_integer = std::integral_constant<bool,
            std::is_integral<std::decay_t<T>>::value &&
            std::is_unsigned<std::decay_t<T>>::value &&
            !std::is_same<bool, std::decay_t<T>>::value
        >;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_unsigned_integer<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_unsigned_integer<T>::value, bool>;
};

/**
 * Tests for whether a given type is a float.
 */
struct if_floating_point
{
    struct __
    {
        template <typename T>
        using is_floating_point = std::integral_constant<bool,
            std::is_floating_point<std::decay_t<T>>::value
        >;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_floating_point<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_floating_point<T>::value, bool>;
};

/**
 * Tests for whether a given type is a boolean.
 */
struct if_boolean
{
    struct __
    {
        template <typename T>
        using is_boolean = std::integral_constant<bool,
            std::is_same<bool, std::decay_t<T>>::value
        >;
    };

    template <typename T>
    using enabled = std::enable_if_t<__::is_boolean<T>::value, bool>;

    template <typename T>
    using disabled = std::enable_if_t<!__::is_boolean<T>::value, bool>;
};

/**
 * Tests for whether a type defines operator<<.
 */
DECLARATION_TESTS(ostream, T, std::declval<std::ostream &>() << std::declval<const T &>());

}
