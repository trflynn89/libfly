#pragma once

#include <type_traits>

/**
 * Define SFINAE tests for whether a function is declared for a type.
 *
 * For example, to test if a class of type T declares a function called Foo:
 *
 *     FLY_DECLARATION_TESTS(Foo, T, std::declval<const T &>().Foo());
 *
 * The macro invocation will define the following std::bool_constant<> (and
 * convenience constexpr bool):
 *
 *     template <typename T>
 *     struct FooTraits::is_declared<T>;
 *
 *     template <typename T>
 *     inline constexpr bool FooTraits::is_declared_v = is_declared<T>::value;
 *
 * And may be used as SFINAE tests to, e.g., define a function depending on
 * whether or not Foo() was declared:
 *
 *     template <
 *         typename T,
 *         fly::enable_if_all<FooTraits::is_declared<T>> = 0>
 *     void foo_wrapper(const T &arg) { arg.Foo(); }
 *
 *     template <
 *         typename T,
 *         fly::enable_if_not_all<FooTraits::is_declared<T>> = 0>
 *     void foo_wrapper(const T &) { }
 *
 * @param Label The name to give the test.
 * @param Type The type to be tested.
 * @param functor The function to test for.
 */
#define FLY_DECLARATION_TESTS(Label, Type, functor)                            \
    struct Label##Traits                                                       \
    {                                                                          \
        struct __##Label##__                                                   \
        {                                                                      \
            template <typename Type>                                           \
            static constexpr auto test_##Label(Type *) -> decltype(functor);   \
                                                                               \
            template <typename Type>                                           \
            static constexpr auto test_##Label(...) -> std::false_type;        \
        };                                                                     \
                                                                               \
        template <typename Type>                                               \
        using is_declared = std::negation<std::is_same<                        \
            decltype(__##Label##__::test_##Label<Type>(0)),                    \
            std::false_type>>;                                                 \
                                                                               \
        template <typename Type>                                               \
        inline static constexpr bool is_declared_v = is_declared<Type>::value; \
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
