#include "fly/traits/traits.hpp"

#include <catch2/catch.hpp>

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
template <
    typename T,
    fly::enable_if_all<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_pointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_pointer(const T &)
{
    return false;
}

//==================================================================================================
template <
    typename T,
    fly::enable_if_any<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_or_pointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_none<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_or_pointer(const T &)
{
    return false;
}

} // namespace

TEST_CASE("Traits", "[traits]")
{
    SECTION("DeclarationTraits for whether a class defines a method foo()")
    {
        const FooClass fc;
        const BarClass bc;

        CHECK(call_foo(fc));
        CHECK(FooTraits::is_declared_v<FooClass>);

        CHECK_FALSE(call_foo(bc));
        CHECK_FALSE(FooTraits::is_declared_v<BarClass>);
    }

    SECTION("DeclarationTraits for whether a class defines operator<<")
    {
        std::stringstream stream;

        const FooClass fc;
        const BarClass bc;

        const std::string str("a");

        CHECK(is_streamable(stream, bc));
        CHECK(OstreamTraits::is_declared_v<BarClass>);
        CHECK(stream.str() == bc());
        stream.str(std::string());

        CHECK(is_streamable(stream, str));
        CHECK(OstreamTraits::is_declared_v<std::string>);
        CHECK(stream.str() == str);
        stream.str(std::string());

        CHECK(is_streamable(stream, 1));
        CHECK(OstreamTraits::is_declared_v<int>);
        CHECK(stream.str() == "1");
        stream.str(std::string());

        CHECK_FALSE(is_streamable(stream, fc));
        CHECK_FALSE(OstreamTraits::is_declared_v<FooClass>);
        CHECK(stream.str() == std::string());
        stream.str(std::string());
    }

    SECTION("Combination of traits for enable_if_all and enable_if_not_all")
    {
        const FooClass fc;
        const std::string str("a");

        int i = 0;
        bool b = false;
        float f = 3.14159f;

        CHECK_FALSE(is_class_pointer(fc));
        CHECK_FALSE(is_class_pointer(str));
        CHECK(is_class_pointer(&fc));
        CHECK(is_class_pointer(&str));

        CHECK_FALSE(is_class_pointer(i));
        CHECK_FALSE(is_class_pointer(b));
        CHECK_FALSE(is_class_pointer(f));
        CHECK_FALSE(is_class_pointer(&i));
        CHECK_FALSE(is_class_pointer(&b));
        CHECK_FALSE(is_class_pointer(&f));
    }

    SECTION("Combination of traits for enable_if_any and enable_if_none")
    {
        const FooClass fc;
        const std::string str("a");

        int i = 0;
        bool b = false;
        float f = 3.14159f;

        CHECK(is_class_or_pointer(fc));
        CHECK(is_class_or_pointer(str));
        CHECK(is_class_or_pointer(&fc));
        CHECK(is_class_or_pointer(&str));

        CHECK_FALSE(is_class_or_pointer(i));
        CHECK_FALSE(is_class_or_pointer(b));
        CHECK_FALSE(is_class_or_pointer(f));
        CHECK(is_class_or_pointer(&i));
        CHECK(is_class_or_pointer(&b));
        CHECK(is_class_or_pointer(&f));
    }

    SECTION("Variadic all_same trait")
    {
        CHECK(fly::all_same_v<int, int>);
        CHECK(fly::all_same_v<int, const int>);
        CHECK(fly::all_same_v<const int, int>);
        CHECK(fly::all_same_v<const int, const int>);

        CHECK(fly::all_same_v<int, int>);
        CHECK(fly::all_same_v<int, int &>);
        CHECK(fly::all_same_v<int &, int>);
        CHECK(fly::all_same_v<int &, int &>);

        CHECK(fly::all_same_v<int, int>);
        CHECK(fly::all_same_v<int, const int &>);
        CHECK(fly::all_same_v<const int &, int>);
        CHECK(fly::all_same_v<const int &, const int &>);

        CHECK(fly::all_same_v<int, int, int>);
        CHECK(fly::all_same_v<int, int, const int>);
        CHECK(fly::all_same_v<int, const int, int>);
        CHECK(fly::all_same_v<int, const int, const int>);

        CHECK(fly::all_same_v<const int, int, int>);
        CHECK(fly::all_same_v<const int, int, const int>);
        CHECK(fly::all_same_v<const int, const int, int>);
        CHECK(fly::all_same_v<const int, const int, const int>);

        CHECK(fly::all_same_v<bool, bool, bool>);
        CHECK(fly::all_same_v<float, float, float, float>);
        CHECK(fly::all_same_v<FooClass, FooClass, FooClass>);
        CHECK(fly::all_same_v<std::string, std::string, std::string>);

        CHECK_FALSE(fly::all_same_v<int, char>);
        CHECK_FALSE(fly::all_same_v<int *, int>);
        CHECK_FALSE(fly::all_same_v<bool, bool, char>);
        CHECK_FALSE(fly::all_same_v<FooClass, FooClass, std::string>);
    }

    SECTION("Variadic any_same trait")
    {
        CHECK(fly::any_same_v<int, int>);
        CHECK(fly::any_same_v<int, const int>);
        CHECK(fly::any_same_v<const int, int>);
        CHECK(fly::any_same_v<const int, const int>);

        CHECK(fly::any_same_v<int, int>);
        CHECK(fly::any_same_v<int, int &>);
        CHECK(fly::any_same_v<int &, int>);
        CHECK(fly::any_same_v<int &, int &>);

        CHECK(fly::any_same_v<int, int>);
        CHECK(fly::any_same_v<int, const int &>);
        CHECK(fly::any_same_v<const int &, int>);
        CHECK(fly::any_same_v<const int &, const int &>);

        CHECK(fly::any_same_v<int, int, int>);
        CHECK(fly::any_same_v<int, int, const int>);
        CHECK(fly::any_same_v<int, const int, int>);
        CHECK(fly::any_same_v<int, const int, const int>);

        CHECK(fly::any_same_v<const int, int, int>);
        CHECK(fly::any_same_v<const int, int, const int>);
        CHECK(fly::any_same_v<const int, const int, int>);
        CHECK(fly::any_same_v<const int, const int, const int>);

        CHECK(fly::any_same_v<bool, bool, bool>);
        CHECK(fly::any_same_v<float, float, float, float>);
        CHECK(fly::any_same_v<FooClass, FooClass, FooClass>);
        CHECK(fly::any_same_v<std::string, std::string, std::string>);

        CHECK(fly::any_same_v<bool, bool, char>);
        CHECK(fly::any_same_v<FooClass, FooClass, std::string>);

        CHECK_FALSE(fly::any_same_v<int, char>);
        CHECK_FALSE(fly::any_same_v<int *, int>);
        CHECK_FALSE(fly::any_same_v<bool, char>);
        CHECK_FALSE(fly::any_same_v<FooClass, std::string>);
    }

    SECTION("Overloaded visitation pattern")
    {
        using TestVariant = std::variant<int, bool, std::string>;
        int result;

        result = std::visit(
            fly::visitation {
                [](int) -> int { return 1; },
                [](bool) -> int { return 2; },
                [](std::string) -> int { return 3; },
            },
            TestVariant {int()});
        CHECK(1 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int { return 1; },
                [](bool) -> int { return 2; },
                [](std::string) -> int { return 3; },
            },
            TestVariant {bool()});
        CHECK(2 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int { return 1; },
                [](bool) -> int { return 2; },
                [](std::string) -> int { return 3; },
            },
            TestVariant {std::string()});
        CHECK(3 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int { return 1; },
                [](auto) -> int { return 2; },
            },
            TestVariant {int()});
        CHECK(1 == result);

        result = std::visit(
            fly::visitation {
                [](int) -> int { return 1; },
                [](auto) -> int { return 2; },
            },
            TestVariant {std::string()});
        CHECK(2 == result);
    }
}
