#include "fly/traits/traits.hpp"

#include "catch2/catch_test_macros.hpp"

#include <sstream>
#include <variant>

namespace {

//==================================================================================================
template <typename T>
using ostream_type = decltype(std::declval<std::ostream &>() << std::declval<T>());

template <typename T>
using has_ostream = fly::is_declared<ostream_type, T>;

//==================================================================================================
template <typename T>
using foo_type = decltype(std::declval<T>().foo());

template <typename T>
using has_foo = fly::is_declared<foo_type, T>;

//==================================================================================================
template <typename T, typename P1, typename P2>
using bar_type = decltype(std::declval<T>().bar(std::declval<P1>(), std::declval<P2>()));

template <typename T, typename P1, typename P2>
using has_bar = fly::is_declared<bar_type, T, P1, P2>;

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

    bool bar(int, std::string) const
    {
        return true;
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
template <typename T, fly::enable_if<has_foo<T>> = 0>
bool call_foo(const T &obj)
{
    return obj.foo();
}

template <typename T, fly::disable_if<has_foo<T>> = 0>
bool call_foo(const T &)
{
    return false;
}

//==================================================================================================
template <typename T, typename P1, typename P2, fly::enable_if<has_bar<T, P1, P2>> = 0>
bool call_bar(const T &obj, P1 arg1, P2 arg2)
{
    return obj.bar(arg1, arg2);
}

template <typename T, typename P1, typename P2, fly::disable_if<has_bar<T, P1, P2>> = 0>
bool call_bar(const T &, P1, P2)
{
    return false;
}

//==================================================================================================
template <typename T, fly::enable_if<has_ostream<T>> = 0>
bool is_streamable(std::ostream &stream, const T &obj)
{
    stream << obj;
    return true;
}

template <typename T, fly::disable_if<has_ostream<T>> = 0>
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
    fly::disable_if_all<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>> = 0>
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
    fly::disable_if_any<std::is_pointer<T>, std::is_class<std::remove_pointer_t<T>>> = 0>
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
        CATCH_CHECK(has_foo<FooClass>::value);

        CATCH_CHECK_FALSE(call_foo(bc));
        CATCH_CHECK_FALSE(has_foo<BarClass>::value);
    }

    CATCH_SECTION("DeclarationTraits for whether a class defines a method bar(type1, type2)")
    {
        const FooClass fc;
        const BarClass bc;

        CATCH_CHECK(call_bar(bc, 1, "str"));
        CATCH_CHECK(has_bar<BarClass, int, std::string>::value);
        CATCH_CHECK_FALSE(has_bar<BarClass, std::string, int>::value);
        CATCH_CHECK_FALSE(has_bar<BarClass, FooClass, std::string>::value);

        CATCH_CHECK_FALSE(call_bar(fc, 1, "str"));
        CATCH_CHECK_FALSE(has_bar<FooClass, int, std::string>::value);
    }

    CATCH_SECTION("DeclarationTraits for whether a class defines operator<<")
    {
        std::stringstream stream;

        const FooClass fc;
        const BarClass bc;

        const std::string str("a");

        CATCH_CHECK(is_streamable(stream, bc));
        CATCH_CHECK(has_ostream<BarClass>::value);
        CATCH_CHECK(stream.str() == bc());
        stream.str(std::string());

        CATCH_CHECK(is_streamable(stream, str));
        CATCH_CHECK(has_ostream<std::string>::value);
        CATCH_CHECK(stream.str() == str);
        stream.str(std::string());

        CATCH_CHECK(is_streamable(stream, 1));
        CATCH_CHECK(has_ostream<int>::value);
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        CATCH_CHECK_FALSE(is_streamable(stream, fc));
        CATCH_CHECK_FALSE(has_ostream<FooClass>::value);
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

    CATCH_SECTION("Combination of traits for enable_if_all and disable_if_all")
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

    CATCH_SECTION("Combination of traits for enable_if_any and disable_if_any")
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

    CATCH_SECTION("Trait: all_same")
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

    CATCH_SECTION("Trait: any_same")
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

    CATCH_SECTION("Trait: size_of_type_is")
    {
        CATCH_CHECK(fly::size_of_type_is_v<int, sizeof(int)>);
        CATCH_CHECK(fly::size_of_type_is_v<bool, sizeof(bool)>);
        CATCH_CHECK(fly::size_of_type_is_v<FooClass, sizeof(FooClass)>);

        CATCH_CHECK_FALSE(fly::size_of_type_is_v<int, sizeof(int) - 1>);
        CATCH_CHECK_FALSE(fly::size_of_type_is_v<bool, sizeof(bool) - 1>);
        CATCH_CHECK_FALSE(fly::size_of_type_is_v<FooClass, sizeof(FooClass) - 1>);

        CATCH_CHECK_FALSE(fly::size_of_type_is_v<int, sizeof(int) + 1>);
        CATCH_CHECK_FALSE(fly::size_of_type_is_v<bool, sizeof(bool) + 1>);
        CATCH_CHECK_FALSE(fly::size_of_type_is_v<FooClass, sizeof(FooClass) + 1>);
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
