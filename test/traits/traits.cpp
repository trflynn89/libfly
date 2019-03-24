#include "fly/traits/traits.h"

#include <gtest/gtest.h>

#include <sstream>

namespace {

//==========================================================================
FLY_DECLARATION_TESTS(foo, T, std::declval<const T &>().Foo());

//==========================================================================
FLY_DECLARATION_TESTS(
    ostream,
    T,
    std::declval<std::ostream &>() << std::declval<const T &>());

//==========================================================================
class FooClass
{
public:
    FooClass() noexcept
    {
    }

    bool Foo() const noexcept
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
template <typename T, if_foo::enabled<T> = 0>
bool callFoo(const T &arg) noexcept
{
    return arg.Foo();
}

template <typename T, if_foo::disabled<T> = 0>
bool callFoo(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, if_ostream::enabled<T> = 0>
bool isStreamable(std::ostream &stream, const T &arg) noexcept
{
    stream << arg;
    return true;
}

template <typename T, if_ostream::disabled<T> = 0>
bool isStreamable(std::ostream &, const T &) noexcept
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool isClassPointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool isClassPointer(const T &)
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_any<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool isClassOrPointer(const T &)
{
    return true;
}

template <
    typename T,
    fly::enable_if_none<
        std::is_pointer<T>,
        std::is_class<std::remove_pointer_t<T>>>...>
bool isClassOrPointer(const T &)
{
    return false;
}

} // namespace

//==============================================================================
TEST(TraitsTest, FooTest)
{
    const FooClass fc;
    const BarClass bc;

    EXPECT_TRUE(callFoo(fc));
    EXPECT_FALSE(callFoo(bc));
}

//==============================================================================
TEST(TraitsTest, StreamTest)
{
    std::stringstream stream;

    const FooClass fc;
    const BarClass bc;

    const std::string str("a");

    EXPECT_TRUE(isStreamable(stream, bc));
    EXPECT_EQ(stream.str(), bc());
    stream.str(std::string());

    EXPECT_TRUE(isStreamable(stream, str));
    EXPECT_EQ(stream.str(), str);
    stream.str(std::string());

    EXPECT_TRUE(isStreamable(stream, 1));
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    EXPECT_FALSE(isStreamable(stream, fc));
    EXPECT_EQ(stream.str(), std::string());
    stream.str(std::string());
}

//==============================================================================
TEST(TraitsTest, EnableIfAllTest)
{
    const FooClass fc;
    const std::string str("a");

    int i = 0;
    bool b = false;
    float f = 3.14159f;

    EXPECT_FALSE(isClassPointer(fc));
    EXPECT_FALSE(isClassPointer(str));
    EXPECT_TRUE(isClassPointer(&fc));
    EXPECT_TRUE(isClassPointer(&str));

    EXPECT_FALSE(isClassPointer(i));
    EXPECT_FALSE(isClassPointer(b));
    EXPECT_FALSE(isClassPointer(f));
    EXPECT_FALSE(isClassPointer(&i));
    EXPECT_FALSE(isClassPointer(&b));
    EXPECT_FALSE(isClassPointer(&f));
}

//==============================================================================
TEST(TraitsTest, EnableIfAnyTest)
{
    const FooClass fc;
    const std::string str("a");

    int i = 0;
    bool b = false;
    float f = 3.14159f;

    EXPECT_TRUE(isClassOrPointer(fc));
    EXPECT_TRUE(isClassOrPointer(str));
    EXPECT_TRUE(isClassOrPointer(&fc));
    EXPECT_TRUE(isClassOrPointer(&str));

    EXPECT_FALSE(isClassOrPointer(i));
    EXPECT_FALSE(isClassOrPointer(b));
    EXPECT_FALSE(isClassOrPointer(f));
    EXPECT_TRUE(isClassOrPointer(&i));
    EXPECT_TRUE(isClassOrPointer(&b));
    EXPECT_TRUE(isClassOrPointer(&f));
}
