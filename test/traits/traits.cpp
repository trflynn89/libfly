#include "fly/traits/traits.hpp"

#include "catch2/catch.hpp"

#include <sstream>
#include <variant>

namespace {

//==================================================================================================
template <typename T>
using OstreamDeclaration = decltype(std::declval<std::ostream &>() << std::declval<T>());

using OstreamTraits = fly::DeclarationTraits<OstreamDeclaration>;

//==================================================================================================
template <typename T>
using FooDeclaration = decltype(std::declval<T>().foo());

using FooTraits = fly::DeclarationTraits<FooDeclaration>;

//==================================================================================================
class FooClass
{
public:
    FooClass() noexcept
    {
    }

    bool foo() const
    {
        return true;
    }
};

//==================================================================================================
class BarClass
{
public:
    BarClass() noexcept
    {
    }

    std::string operator()() const
    {
        return "BarClass";
    }

private:
    friend std::ostream &operator<<(std::ostream &, const BarClass &);
};

std::ostream &operator<<(std::ostream &stream, const BarClass &bar)
{
    return (stream << bar());
}

//==================================================================================================
template <typename T, fly::enable_if_all<FooTraits::is_declared<T>> = 0>
bool call_foo(const T &arg)
{
    return arg.foo();
}

template <typename T, fly::enable_if_not_all<FooTraits::is_declared<T>> = 0>
bool call_foo(const T &)
{
    return false;
}

//==================================================================================================
template <typename T, fly::enable_if_all<OstreamTraits::is_declared<T>> = 0>
bool is_streamable(std::ostream &stream, const T &arg)
{
    stream << arg;
    return true;
}

template <typename T, fly::enable_if_not_all<OstreamTraits::is_declared<T>> = 0>
bool is_streamable(std::ostream &, const T &)
{
    return false;
}

//==================================================================================================
template <typename T, fly::enable_if<std::is_class<std::remove_pointer_t<T>>> = 0>
bool is_class(const T &)
{
    return true;
}

template <typename T, fly::disable_if<std::is_class<std::remove_pointer_t<T>>> = 0>
bool is_class(const T &)
{
    return false;
}

//==================================================================================================
template <
    typename T,
    fly::enable_if_all<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>> = 0>
bool is_class_pointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>> = 0>
bool is_class_pointer(const T &)
{
    return false;
}

//==================================================================================================
template <
    typename T,
    fly::enable_if_any<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>> = 0>
bool is_class_or_pointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_none<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>> = 0>
bool is_class_or_pointer(const T &)
{
    return false;
}

} // namespace

CATCH_TEST_CASE("Traits", "[traits]")
{
    CATCH_SECTION("DeclarationTraits for whether a class defines a method foo()")
    {
        const FooClass fc;
        const BarClass bc;

        CATCH_CHECK(call_foo(fc));
        CATCH_CHECK(FooTraits::is_declared_v<FooClass>);

        CATCH_CHECK_FALSE(call_foo(bc));
        CATCH_CHECK_FALSE(FooTraits::is_declared_v<BarClass>);
    }

    CATCH_SECTION("DeclarationTraits for whether a class defines operator<<")
    {
        std::stringstream stream;

        const FooClass fc;
        const BarClass bc;

        const std::string str("a");

        CATCH_CHECK(is_streamable(stream, bc));
        CATCH_CHECK(OstreamTraits::is_declared_v<BarClass>);
        CATCH_CHECK(stream.str() == bc());
        stream.str(std::string());

        CATCH_CHECK(is_streamable(stream, str));
        CATCH_CHECK(OstreamTraits::is_declared_v<std::string>);
        CATCH_CHECK(stream.str() == str);
        stream.str(std::string());

        CATCH_CHECK(is_streamable(stream, 1));
        CATCH_CHECK(OstreamTraits::is_declared_v<int>);
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        CATCH_CHECK_FALSE(is_streamable(stream, fc));
        CATCH_CHECK_FALSE(OstreamTraits::is_declared_v<FooClass>);
        CATCH_CHECK(stream.str() == std::string());
        stream.str(std::string());
    }

    CATCH_SECTION("Combination of traits for enable_if and disable_if")
    {
        const FooClass fc;
        const std::string str("a");

        int i = 0;
        bool b = false;
        float f = 3.14159f;

        CATCH_CHECK(is_class(fc));
        CATCH_CHECK(is_class(str));
        CATCH_CHECK(is_class(&fc));
        CATCH_CHECK(is_class(&str));

        CATCH_CHECK_FALSE(is_class(i));
        CATCH_CHECK_FALSE(is_class(b));
        CATCH_CHECK_FALSE(is_class(f));
        CATCH_CHECK_FALSE(is_class(&i));
        CATCH_CHECK_FALSE(is_class(&b));
        CATCH_CHECK_FALSE(is_class(&f));
    }

    CATCH_SECTION("Combination of traits for enable_if_all and enable_if_not_all")
    {
        const FooClass fc;
        const std::string str("a");

        int i = 0;
        bool b = false;
        float f = 3.14159f;

        CATCH_CHECK_FALSE(is_class_pointer(fc));
        CATCH_CHECK_FALSE(is_class_pointer(str));
        CATCH_CHECK(is_class_pointer(&fc));
        CATCH_CHECK(is_class_pointer(&str));

        CATCH_CHECK_FALSE(is_class_pointer(i));
        CATCH_CHECK_FALSE(is_class_pointer(b));
        CATCH_CHECK_FALSE(is_class_pointer(f));
        CATCH_CHECK_FALSE(is_class_pointer(&i));
        CATCH_CHECK_FALSE(is_class_pointer(&b));
        CATCH_CHECK_FALSE(is_class_pointer(&f));
    }

    CATCH_SECTION("Combination of traits for enable_if_any and enable_if_none")
    {
        const FooClass fc;
        const std::string str("a");

        int i = 0;
        bool b = false;
        float f = 3.14159f;

        CATCH_CHECK(is_class_or_pointer(fc));
        CATCH_CHECK(is_class_or_pointer(str));
        CATCH_CHECK(is_class_or_pointer(&fc));
        CATCH_CHECK(is_class_or_pointer(&str));

        CATCH_CHECK_FALSE(is_class_or_pointer(i));
        CATCH_CHECK_FALSE(is_class_or_pointer(b));
        CATCH_CHECK_FALSE(is_class_or_pointer(f));
        CATCH_CHECK(is_class_or_pointer(&i));
        CATCH_CHECK(is_class_or_pointer(&b));
        CATCH_CHECK(is_class_or_pointer(&f));
    }

    CATCH_SECTION("Variadic all_same trait")
    {
        CATCH_CHECK(fly::all_same_v<int, int>);
        CATCH_CHECK(fly::all_same_v<int, const int>);
        CATCH_CHECK(fly::all_same_v<const int, int>);
        CATCH_CHECK(fly::all_same_v<const int, const int>);

        CATCH_CHECK(fly::all_same_v<int, int>);
        CATCH_CHECK(fly::all_same_v<int, int &>);
        CATCH_CHECK(fly::all_same_v<int &, int>);
        CATCH_CHECK(fly::all_same_v<int &, int &>);

        CATCH_CHECK(fly::all_same_v<int, int>);
        CATCH_CHECK(fly::all_same_v<int, const int &>);
        CATCH_CHECK(fly::all_same_v<const int &, int>);
        CATCH_CHECK(fly::all_same_v<const int &, const int &>);

        CATCH_CHECK(fly::all_same_v<int, int, int>);
        CATCH_CHECK(fly::all_same_v<int, int, const int>);
        CATCH_CHECK(fly::all_same_v<int, const int, int>);
        CATCH_CHECK(fly::all_same_v<int, const int, const int>);

        CATCH_CHECK(fly::all_same_v<const int, int, int>);
        CATCH_CHECK(fly::all_same_v<const int, int, const int>);
        CATCH_CHECK(fly::all_same_v<const int, const int, int>);
        CATCH_CHECK(fly::all_same_v<const int, const int, const int>);

        CATCH_CHECK(fly::all_same_v<bool, bool, bool>);
        CATCH_CHECK(fly::all_same_v<float, float, float, float>);
        CATCH_CHECK(fly::all_same_v<FooClass, FooClass, FooClass>);
        CATCH_CHECK(fly::all_same_v<std::string, std::string, std::string>);

        CATCH_CHECK_FALSE(fly::all_same_v<int, char>);
        CATCH_CHECK_FALSE(fly::all_same_v<int *, int>);
        CATCH_CHECK_FALSE(fly::all_same_v<bool, bool, char>);
        CATCH_CHECK_FALSE(fly::all_same_v<FooClass, FooClass, std::string>);
    }

    CATCH_SECTION("Variadic any_same trait")
    {
        CATCH_CHECK(fly::any_same_v<int, int>);
        CATCH_CHECK(fly::any_same_v<int, const int>);
        CATCH_CHECK(fly::any_same_v<const int, int>);
        CATCH_CHECK(fly::any_same_v<const int, const int>);

        CATCH_CHECK(fly::any_same_v<int, int>);
        CATCH_CHECK(fly::any_same_v<int, int &>);
        CATCH_CHECK(fly::any_same_v<int &, int>);
        CATCH_CHECK(fly::any_same_v<int &, int &>);

        CATCH_CHECK(fly::any_same_v<int, int>);
        CATCH_CHECK(fly::any_same_v<int, const int &>);
        CATCH_CHECK(fly::any_same_v<const int &, int>);
        CATCH_CHECK(fly::any_same_v<const int &, const int &>);

        CATCH_CHECK(fly::any_same_v<int, int, int>);
        CATCH_CHECK(fly::any_same_v<int, int, const int>);
        CATCH_CHECK(fly::any_same_v<int, const int, int>);
        CATCH_CHECK(fly::any_same_v<int, const int, const int>);

        CATCH_CHECK(fly::any_same_v<const int, int, int>);
        CATCH_CHECK(fly::any_same_v<const int, int, const int>);
        CATCH_CHECK(fly::any_same_v<const int, const int, int>);
        CATCH_CHECK(fly::any_same_v<const int, const int, const int>);

        CATCH_CHECK(fly::any_same_v<bool, bool, bool>);
        CATCH_CHECK(fly::any_same_v<float, float, float, float>);
        CATCH_CHECK(fly::any_same_v<FooClass, FooClass, FooClass>);
        CATCH_CHECK(fly::any_same_v<std::string, std::string, std::string>);

        CATCH_CHECK(fly::any_same_v<bool, bool, char>);
        CATCH_CHECK(fly::any_same_v<FooClass, FooClass, std::string>);

        CATCH_CHECK_FALSE(fly::any_same_v<int, char>);
        CATCH_CHECK_FALSE(fly::any_same_v<int *, int>);
        CATCH_CHECK_FALSE(fly::any_same_v<bool, char>);
        CATCH_CHECK_FALSE(fly::any_same_v<FooClass, std::string>);
    }

    CATCH_SECTION("Overloaded visitation pattern")
    {
        using TestVariant = std::variant<int, bool, std::string>;
        int result;

        result = std::visit(
            fly::visitation {
                [](int) -> int
                {
                    return 1;
                },
                [](bool) -> int
                {
                    return 2;
                },
                [](std::string) -> int
                {
                    return 3;
                },
            },
            TestVariant {int()});
        CATCH_CHECK(1 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int
                {
                    return 1;
                },
                [](bool) -> int
                {
                    return 2;
                },
                [](std::string) -> int
                {
                    return 3;
                },
            },
            TestVariant {bool()});
        CATCH_CHECK(2 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int
                {
                    return 1;
                },
                [](bool) -> int
                {
                    return 2;
                },
                [](std::string) -> int
                {
                    return 3;
                },
            },
            TestVariant {std::string()});
        CATCH_CHECK(3 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int
                {
                    return 1;
                },
                [](auto) -> int
                {
                    return 2;
                },
            },
            TestVariant {int()});
        CATCH_CHECK(1 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int
                {
                    return 1;
                },
                [](auto) -> int
                {
                    return 2;
                },
            },
            TestVariant {std::string()});
        CATCH_CHECK(2 == result);
    }
}
