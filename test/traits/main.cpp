#include <sstream>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/traits/traits.h"

namespace
{
    //==========================================================================
    DECLARATION_TESTS(foo, T, std::declval<const T &>().Foo());

    //==========================================================================
    class FooClass
    {
    public:
        FooClass() { }
        bool Foo() const { return true; }
    };

    //==========================================================================
    class BarClass
    {
    public:
        BarClass() { }

        std::string operator()() const
        {
            return "BarClass";
        }

    private:
        friend std::ostream &operator << (std::ostream &, const BarClass &);
    };

    std::ostream &operator << (std::ostream &stream, const BarClass &bar)
    {
        return (stream << bar());
    }

    //==========================================================================
    template <typename T, if_foo::enabled<T> = 0>
    bool callFoo(const T &arg)
    {
        return arg.Foo();
    }

    template <typename T, if_foo::disabled<T> = 0>
    bool callFoo(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::if_string::enabled<T> = 0>
    bool isString(const T &)
    {
        return true;
    }

    template <typename T, fly::if_string::disabled<T> = 0>
    bool isString(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::if_ostream::enabled<T> = 0>
    bool isStreamable(std::ostream &stream, const T &arg)
    {
        stream << arg;
        return true;
    }

    template <typename T, fly::if_ostream::disabled<T> = 0>
    bool isStreamable(std::ostream &, const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::if_signed_integer::enabled<T> = 0>
    bool isSignedInteger(const T &)
    {
        return true;
    }

    template <typename T, fly::if_signed_integer::disabled<T> = 0>
    bool isSignedInteger(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::if_unsigned_integer::enabled<T> = 0>
    bool isUnsignedInteger(const T &)
    {
        return true;
    }

    template <typename T, fly::if_unsigned_integer::disabled<T> = 0>
    bool isUnsignedInteger(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::if_floating_point::enabled<T> = 0>
    bool isFloat(const T &)
    {
        return true;
    }

    template <typename T, fly::if_floating_point::disabled<T> = 0>
    bool isFloat(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::if_boolean::enabled<T> = 0>
    bool isBool(const T &)
    {
        return true;
    }

    template <typename T, fly::if_boolean::disabled<T> = 0>
    bool isBool(const T &)
    {
        return false;
    }
}

//==============================================================================
TEST(TraitsTest, FooTest)
{
    const FooClass fc;
    const BarClass bc;

    ASSERT_TRUE(callFoo(fc));
    ASSERT_FALSE(callFoo(bc));
}

//==============================================================================
TEST(TraitsTest, StringTest)
{
    const FooClass fc;

    const std::string str1("a");
    std::string str2("b");

    const char *cstr1 = "c";
    char *cstr2 = (char *)"d";

    const char chr1 = 'e';
    char chr2 = 'f';

    const char arr1[] = { 'g', '\0' };
    char arr2[] = { 'h', '\0' };

    ASSERT_TRUE(isString(str1));
    ASSERT_TRUE(isString(str1));
    ASSERT_TRUE(isString(cstr1));
    ASSERT_TRUE(isString(cstr2));
    ASSERT_TRUE(isString(arr1));
    ASSERT_TRUE(isString(arr2));

    ASSERT_FALSE(isString(1));
    ASSERT_FALSE(isString(true));
    ASSERT_FALSE(isString(3.14159f));
    ASSERT_FALSE(isString(3.14159f));
    ASSERT_FALSE(isString(fc));
    ASSERT_FALSE(isString(chr1));
    ASSERT_FALSE(isString(chr2));
}

//==============================================================================
TEST(TraitsTest, StreamTest)
{
    std::stringstream stream;

    const FooClass fc;
    const BarClass bc;

    const std::string str("a");

    ASSERT_TRUE(isStreamable(stream, bc));
    ASSERT_EQ(stream.str(), bc());
    stream.str(std::string());

    ASSERT_TRUE(isStreamable(stream, str));
    ASSERT_EQ(stream.str(), str);
    stream.str(std::string());

    ASSERT_TRUE(isStreamable(stream, 1));
    ASSERT_EQ(stream.str(), "1");
    stream.str(std::string());

    ASSERT_FALSE(isStreamable(stream, fc));
    ASSERT_EQ(stream.str(), std::string());
    stream.str(std::string());
}

//==============================================================================
TEST(TraitsTest, SignedIntegerTest)
{
    ASSERT_TRUE(isSignedInteger(1));
    ASSERT_TRUE(isSignedInteger(-1));

    ASSERT_FALSE(isSignedInteger("foo"));
    ASSERT_FALSE(isSignedInteger(3.14));
    ASSERT_FALSE(isSignedInteger(true));
    ASSERT_FALSE(isSignedInteger(static_cast<unsigned int>(1)));
}

//==============================================================================
TEST(TraitsTest, UnsignedIntegerTest)
{
    ASSERT_TRUE(isUnsignedInteger(static_cast<unsigned int>(1)));
    ASSERT_TRUE(isUnsignedInteger(static_cast<unsigned int>(-1)));

    ASSERT_FALSE(isUnsignedInteger(1));
    ASSERT_FALSE(isUnsignedInteger(-1));
    ASSERT_FALSE(isUnsignedInteger("foo"));
    ASSERT_FALSE(isUnsignedInteger(3.14));
    ASSERT_FALSE(isUnsignedInteger(true));
}

//==============================================================================
TEST(TraitsTest, FloatTest)
{
    ASSERT_TRUE(isFloat(3.14f));
    ASSERT_TRUE(isFloat(3.14));
    ASSERT_TRUE(isFloat(static_cast<long double>(3.14)));

    ASSERT_FALSE(isFloat(1));
    ASSERT_FALSE(isFloat(-1));
    ASSERT_FALSE(isFloat("foo"));
    ASSERT_FALSE(isFloat(true));
}

//==============================================================================
TEST(TraitsTest, BoolTest)
{
    ASSERT_TRUE(isBool(true));
    ASSERT_TRUE(isBool(false));

    ASSERT_FALSE(isFloat(1));
    ASSERT_FALSE(isFloat(-1));
    ASSERT_FALSE(isFloat("foo"));
    ASSERT_FALSE(isUnsignedInteger(3.14));
}
