#pragma once

#include <type_traits>

namespace fly {

/**
 * Wrapper around std::enable_if for testing that all conditions in a sequence of traits are true.
 * Example:
 *
 *     template <
 *         typename T,
 *         fly::enable_if_all<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_all = std::enable_if_t<std::conjunction_v<Conditions...>, bool>;

/**
 * Wrapper around std::enable_if for testing that any condition in a sequence of traits is true.
 * Example:
 *
 *     template <
 *         typename T,
 *         fly::enable_if_any<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_any = std::enable_if_t<std::disjunction_v<Conditions...>, bool>;

/**
 * Wrapper around std::enable_if for testing that all conditions in a sequence of traits are false.
 * Example:
 *
 *     template <
 *         typename T,
 *         fly::enable_if_none<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_none = std::enable_if_t<std::negation_v<std::disjunction<Conditions...>>, bool>;

/**
 * Wrapper around std::enable_if for testing that any condition in a sequence of traits is false.
 * Example:
 *
 *     template <
 *         typename T,
 *         fly::enable_if_not_all<std::is_class<T>, std::is_empty<T>> = 0>
 *     void func(const T &) { }
 */
template <typename... Conditions>
using enable_if_not_all = std::enable_if_t<std::negation_v<std::conjunction<Conditions...>>, bool>;

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

/**
 * Define SFINAE tests for whether a function is declared for a type.
 *
 * For example, to test if a class of type T declares a method called Foo, first define a templated
 * decltype alias for the method:
 *
 *     template <typename T>
 *     using FooDeclaration = decltype(std::declval<T>().Foo());
 *
 * Then use that alias to create a fly::DeclarationTraits alias:
 *
 *     using FooTraits = fly::DeclarationTraits<FooDeclaration>;
 *
 * This provides the following std::bool_constant(and constexpr bool):
 *
 *     template <typename T>
 *     struct FooTraits::is_declared<T>;
 *
 *     template <typename T>
 *     inline constexpr bool FooTraits::is_declared_v = is_declared<T>::value;
 *
 * Which may be used as SFINAE tests to, e.g., define a function depending on whether or not Foo()
 * is declared for a type:
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
 * Or in a constexpr-if expression:
 *
 *     template <typename T>
 *     void foo_wrapper(const T &)
 *     {
 *         if constexpr (FooTraits::is_declared_v<T>)
 *         {
 *             arg.Foo();
 *         }
 *      }
 *
 * @tparam Declaration The decltype alias to test.
 * @tparam T The type to be tested.
 */
template <template <typename> class Declaration>
struct DeclarationTraits
{
private:
    struct Test
    {
        template <typename T>
        static constexpr auto test(T *) -> Declaration<T>;

        template <typename T>
        static constexpr auto test(...) -> std::false_type;
    };

public:
    template <typename T>
    using is_declared =
        std::negation<std::is_same<decltype(Test::template test<T>(nullptr)), std::false_type>>;

    template <typename T>
    inline static constexpr bool is_declared_v = is_declared<T>::value;
};

} // namespace fly
