#include "fly/traits/traits.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace {

//==========================================================================
template <typename T>
using OstreamDeclaration =
    decltype(std::declval<std::ostream &>() << std::declval<T>());

using OstreamTraits = fly::DeclarationTraits<OstreamDeclaration>;

//==========================================================================
template <typename T>
using FooDeclaration = decltype(std::declval<T>().foo());

using FooTraits = fly::DeclarationTraits<FooDeclaration>;

//==========================================================================
class FooClass
{
public:
    FooClass() noexcept
    {
    }

    bool foo() const noexcept
    {
        return true;
    }
};

//==========================================================================
class BarClass
{
public:
    BarClass() noexcept
    {
    }

    std::string operator()() const noexcept
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

//==========================================================================
template <typename T, fly::enable_if_all<FooTraits::is_declared<T>> = 0>
bool call_foo(const T &arg) noexcept
{
    return arg.foo();
}

template <typename T, fly::enable_if_not_all<FooTraits::is_declared<T>> = 0>
bool call_foo(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::enable_if_all<OstreamTraits::is_declared<T>> = 0>
bool is_streamable(std::ostream &stream, const T &arg) noexcept
{
    stream << arg;
    return true;
}

template <typename T, fly::enable_if_not_all<OstreamTraits::is_declared<T>> = 0>
bool is_streamable(std::ostream &, const T &) noexcept
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_pointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_pointer(const T &)
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_any<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_or_pointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_none<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool is_class_or_pointer(const T &)
{
    return false;
}

} // namespace

//==============================================================================
TEST(TraitsTest, Foo)
{
    const FooClass fc;
    const BarClass bc;

    EXPECT_TRUE(call_foo(fc));
    EXPECT_TRUE(FooTraits::is_declared_v<FooClass>);

    EXPECT_FALSE(call_foo(bc));
    EXPECT_FALSE(FooTraits::is_declared_v<BarClass>);
}

//==============================================================================
TEST(TraitsTest, Stream)
{
    std::stringstream stream;

    const FooClass fc;
    const BarClass bc;

    const std::string str("a");

    EXPECT_TRUE(is_streamable(stream, bc));
    EXPECT_TRUE(OstreamTraits::is_declared_v<BarClass>);
    EXPECT_EQ(stream.str(), bc());
    stream.str(std::string());

    EXPECT_TRUE(is_streamable(stream, str));
    EXPECT_TRUE(OstreamTraits::is_declared_v<std::string>);
    EXPECT_EQ(stream.str(), str);
    stream.str(std::string());

    EXPECT_TRUE(is_streamable(stream, 1));
    EXPECT_TRUE(OstreamTraits::is_declared_v<int>);
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    EXPECT_FALSE(is_streamable(stream, fc));
    EXPECT_FALSE(OstreamTraits::is_declared_v<FooClass>);
    EXPECT_EQ(stream.str(), std::string());
    stream.str(std::string());
}

//==============================================================================
TEST(TraitsTest, EnableIfAll)
{
    const FooClass fc;
    const std::string str("a");

    int i = 0;
    bool b = false;
    float f = 3.14159f;

    EXPECT_FALSE(is_class_pointer(fc));
    EXPECT_FALSE(is_class_pointer(str));
    EXPECT_TRUE(is_class_pointer(&fc));
    EXPECT_TRUE(is_class_pointer(&str));

    EXPECT_FALSE(is_class_pointer(i));
    EXPECT_FALSE(is_class_pointer(b));
    EXPECT_FALSE(is_class_pointer(f));
    EXPECT_FALSE(is_class_pointer(&i));
    EXPECT_FALSE(is_class_pointer(&b));
    EXPECT_FALSE(is_class_pointer(&f));
}

//==============================================================================
TEST(TraitsTest, EnableIfAny)
{
    const FooClass fc;
    const std::string str("a");

    int i = 0;
    bool b = false;
    float f = 3.14159f;

    EXPECT_TRUE(is_class_or_pointer(fc));
    EXPECT_TRUE(is_class_or_pointer(str));
    EXPECT_TRUE(is_class_or_pointer(&fc));
    EXPECT_TRUE(is_class_or_pointer(&str));

    EXPECT_FALSE(is_class_or_pointer(i));
    EXPECT_FALSE(is_class_or_pointer(b));
    EXPECT_FALSE(is_class_or_pointer(f));
    EXPECT_TRUE(is_class_or_pointer(&i));
    EXPECT_TRUE(is_class_or_pointer(&b));
    EXPECT_TRUE(is_class_or_pointer(&f));
}

//==============================================================================
TEST(TraitsTest, AllSame)
{
    EXPECT_TRUE((fly::all_same_v<int, int>));
    EXPECT_TRUE((fly::all_same_v<bool, bool, bool>));
    EXPECT_TRUE((fly::all_same_v<float, float, float, float>));
    EXPECT_TRUE((fly::all_same_v<FooClass, FooClass, FooClass>));
    EXPECT_TRUE((fly::all_same_v<std::string, std::string, std::string>));

    EXPECT_FALSE((fly::all_same_v<int, char>));
    EXPECT_FALSE((fly::all_same_v<bool, bool, char>));
    EXPECT_FALSE((fly::all_same_v<FooClass, FooClass, std::string>));
}

//==============================================================================
TEST(TraitsTest, AnySame)
{
    EXPECT_TRUE((fly::any_same_v<int, int>));
    EXPECT_TRUE((fly::any_same_v<bool, bool, bool>));
    EXPECT_TRUE((fly::any_same_v<float, float, float, float>));
    EXPECT_TRUE((fly::any_same_v<FooClass, FooClass, FooClass>));
    EXPECT_TRUE((fly::any_same_v<std::string, std::string, std::string>));

    EXPECT_TRUE((fly::any_same_v<bool, bool, char>));
    EXPECT_TRUE((fly::any_same_v<FooClass, FooClass, std::string>));

    EXPECT_FALSE((fly::any_same_v<int, char>));
    EXPECT_FALSE((fly::any_same_v<bool, char>));
    EXPECT_FALSE((fly::any_same_v<FooClass, std::string>));
}
