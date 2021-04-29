#pragma once

#include <type_traits>

namespace fly {

/**
 * Wrapper around std::enable_if for testing that a single conditions is true. Example:
 *
 *     template <typename T, fly::enable_if<std::is_class<T>> = 0>
 *     void func(const T &) { }
 */
template <typename Condition>
using enable_if = std::enable_if_t<Condition::value, bool>;

/**
 * Wrapper around std::enable_if for testing that a single conditions is false. Example:
 *
 *     template <typename T, fly::enable_if<std::is_class<T>> = 0>
 *     void func(const T &) { }
 */
template <typename Condition>
using disable_if = std::enable_if_t<std::negation_v<Condition>, bool>;

/**
 * Wrapper around std::enable_if for testing that all conditions in a sequence of traits are true.
 * Example:
 *
 *     template <typename T, fly::enable_if_all<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_all = enable_if<std::conjunction<Conditions...>>;

/**
 * Wrapper around std::enable_if for testing that any condition in a sequence of traits is true.
 * Example:
 *
 *     template <typename T, fly::enable_if_any<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_any = enable_if<std::disjunction<Conditions...>>;

/**
 * Wrapper around std::enable_if for testing that any condition in a sequence of traits is false.
 * Example:
 *
 *     template <typename T, fly::disable_if_all<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using disable_if_all = disable_if<std::conjunction<Conditions...>>;

/**
 * Wrapper around std::enable_if for testing that all conditions in a sequence of traits are false.
 * Example:
 *
 *     template <typename T, fly::disable_if_any<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using disable_if_any = disable_if<std::disjunction<Conditions...>>;

/**
 * Wrapper around std::is_same for testing that all types in a sequence of types are the same as a
 * specific type. Example:
 *
 *     constexpr bool same = fly::all_same_v<int, int, int>; // = true
 *     constexpr bool same = fly::all_same_v<int, int, const int &>; // = true
 *     constexpr bool same = fly::all_same_v<int, int, bool>; // = false
 *     constexpr bool same = fly::all_same_v<int, bool, bool>; // = false
 */
template <typename T, typename A, typename... As>
// NOLINTNEXTLINE(readability-identifier-naming)
struct all_same :
    std::conjunction<
        std::is_same<std::decay_t<T>, std::decay_t<A>>,
        std::is_same<std::decay_t<T>, std::decay_t<As>>...>
{
};

template <typename T, typename A, typename... As>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool all_same_v = all_same<T, A, As...>::value;

/**
 * Wrapper around std::is_same for testing that any type in a sequence of types are the same as a
 * specific type. Example:
 *
 *     constexpr bool same = fly::any_same_v<int, int, int>; // = true
 *     constexpr bool same = fly::any_same_v<int, int, const int &>; // = true
 *     constexpr bool same = fly::any_same_v<int, int, bool>; // = true
 *     constexpr bool same = fly::any_same_v<int, bool, bool>; // = false
 */
template <typename T, typename A, typename... As>
// NOLINTNEXTLINE(readability-identifier-naming)
struct any_same :
    std::disjunction<
        std::is_same<std::decay_t<T>, std::decay_t<A>>,
        std::is_same<std::decay_t<T>, std::decay_t<As>>...>
{
};

template <typename T, typename A, typename... As>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool any_same_v = any_same<T, A, As...>::value;

/**
 * Trait for testing if the size of a given type is the provided size.
 */
template <typename T, std::size_t Size>
using size_of_type_is = std::bool_constant<sizeof(T) == Size>;

template <typename T, std::size_t Size>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool size_of_type_is_v = size_of_type_is<T, Size>::value;

/**
 * Overloaded visitation pattern for std::visit. Allows providing a variadic list of lambdas for
 * overload resolution in a call to std::visit. Example:
 *
 *
 *     std::variant<int, bool, std::string> variant {std::string()};
 *
 *     std::visit(
 *         fly::visitation {
 *             [](int) { std::cout << "int\n"; },
 *             [](bool) { std::cout << "bool\n"; },
 *             [](auto) { std::cout << "auto\n"; }, // This one will be entered
 *         },
 *         variant);
 */
template <class... Ts>
// NOLINTNEXTLINE(readability-identifier-naming)
struct visitation : Ts...
{
    using Ts::operator()...;
};

template <class... Ts>
visitation(Ts...) -> visitation<Ts...>;

namespace detail {

    template <typename SFINAE, template <typename...> typename Declaration, typename... Ts>
    struct DeclarationResolves
    {
        using type = std::false_type;
    };

    template <template <typename...> typename Declaration, typename... Ts>
    struct DeclarationResolves<std::void_t<Declaration<Ts...>>, Declaration, Ts...>
    {
        using type = std::true_type;
    };

} // namespace detail

/**
 * Define SFINAE tests for whether a function is declared for a set of types.
 *
 * --------------------
 * Single type example:
 * --------------------
 *
 * To test if a class of type T declares a method called |foo|, first define a templated decltype
 * alias for the method:
 *
 *     template <typename T>
 *     using foo_type = decltype(std::declval<T>().foo());
 *
 * Then use that alias to create a fly::is_declared alias:
 *
 *     template <typename T>
 *     using has_foo = fly::is_declared<foo_type, T>;
 *
 * That alias resolves to a std::bool_constant, which will be std::true_type if the declaration can
 * be resolved with the provided type, or std::false_type otherwise.
 *
 * The alias may be used as SFINAE tests to, e.g., define a function depending on whether or not
 * |foo| is declared for a type:
 *
 *     template <typename T, fly::enable_if<has_foo<T>> = 0>
 *     void foo_wrapper(const T &obj) { obj.foo(); }
 *
 *     template <typename T, fly::disable_if<has_foo<T>> = 0>
 *     void foo_wrapper(const T &) { }
 *
 * Or in a constexpr-if expression:
 *
 *     template <typename T>
 *     void foo_wrapper(const T &obj)
 *     {
 *         if constexpr (has_foo<T>::value)
 *         {
 *             obj.foo();
 *         }
 *      }
 *
 * ----------------------
 * Multiple type example:
 * ----------------------
 *
 * To test if a class of type T declares a method called |bar| which accepts parameter types P1 and
 * P2, first define a templated decltype alias for the method:
 *
 *     template <typename T, typename P1, typename P2>
 *     using bar_type = decltype(std::declval<T>().bar(std::declval<P1>(), std::declval<P2>()));
 *
 * Then use that alias to create a fly::is_declared alias:
 *
 *     template <typename T, typename P1, typename P2>
 *     using has_bar = fly::is_declared<bar_type, T, P1, P2>;
 *
 * That alias resolves to a std::bool_constant, which will be std::true_type if the declaration can
 * be resolved with the provided types, or std::false_type otherwise.
 *
 * The alias may be used as SFINAE tests to, e.g., define a function depending on whether or not
 * |bar| is declared for a type:
 *
 *     template <typename T, typename P1, typename P2, fly::enable_if<has_bar<T, P1, P2>> = 0>
 *     void bar_wrapper(const T &obj, P1 arg2, P2 arg2) { obj.bar(arg1, arg2); }
 *
 *     template <typename T, typename P1, typename P2, fly::disable_if<has_bar<T, P1, P2>> = 0>
 *     void bar_wrapper(const T &obj, P1 arg2, P2 arg2) { }
 *
 * Or in a constexpr-if expression:
 *
 *     template <typename T>
 *     void bar_wrapper(const T &obj)
 *     {
 *         if constexpr (has_bar<T, int, std::string>::value)
 *         {
 *             obj.foo();
 *         }
 *      }
 *
 * @tparam Declaration The decltype alias to test.
 * @tparam Ts The types to be tested on the declaration.
 */
template <template <typename...> typename Declaration, typename... Ts>
using is_declared = typename detail::DeclarationResolves<void, Declaration, Ts...>::type;

} // namespace fly
