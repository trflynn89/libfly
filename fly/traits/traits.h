#pragma once

#include <type_traits>

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

} // namespace fly
