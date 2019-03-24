#pragma once

#include <type_traits>

/**
 * Wrappers around std::enable_if for testing variadic sequences of traits.
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
 * Define SFINAE tests for whether a function is declared for a type.
 *
 * For example, to test if a class of type T declares a method called Foo, first
 * define a templated decltype alias for the method:
 *
 *     template <typename T>
 *     using FooDeclaration = decltype(std::declval<T>().Foo());
 *
 * Then use that alias to create a fly::Declaration alias:
 *
 *     using FooTraits = fly::DeclarationTraits<FooDeclaration>;
 *
 * This provides the following std::bool_constant<> (and constexpr bool):
 *
 *     template <typename T>
 *     struct FooTraits::is_declared<T>;
 *
 *     template <typename T>
 *     inline constexpr bool FooTraits::is_declared_v = is_declared<T>::value;
 *
 * Which may be used as SFINAE tests to, e.g., define a function depending on
 * whether or not Foo() is declared for a type:
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
 * @tparam Declaration The decltype alias to test.
 * @tparam T The type to be tested.
 */
template <template <typename> class Declaration>
struct DeclarationTraits
{
private:
    struct __test__
    {
        template <typename T>
        static constexpr auto test(T *) -> Declaration<T>;

        template <typename T>
        static constexpr auto test(...) -> std::false_type;
    };

public:
    template <typename T>
    using is_declared = std::negation<
        std::is_same<decltype(__test__::template test<T>(0)), std::false_type>>;

    template <typename T>
    inline static constexpr bool is_declared_v = is_declared<T>::value;
};

} // namespace fly
