#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/traits/type_traits.h"

namespace fly
{
    DECL_TESTS(foo, T, std::declval<const T &>().Foo());
}

namespace
{
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
        friend std::ostream &operator << (std::ostream &, const BarClass &);
    };

    std::ostream &operator << (std::ostream &stream, const BarClass &)
    {
        return stream;
    }

    //==========================================================================
    template <typename T, fly::if_foo::enabled<T> = 0>
    bool callFoo(const T &arg)
    {
        return arg.Foo();
    }

    template <typename T, fly::if_foo::disabled<T> = 0>
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
    bool isStreamable(const T &)
    {
        return true;
    }

    template <typename T, fly::if_ostream::disabled<T> = 0>
    bool isStreamable(const T &)
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
    ASSERT_TRUE(isString(chr1));
    ASSERT_TRUE(isString(chr2));
    ASSERT_TRUE(isString(arr1));
    ASSERT_TRUE(isString(arr2));

    ASSERT_FALSE(isString(1));
    ASSERT_FALSE(isString(true));
    ASSERT_FALSE(isString(3.14159f));
    ASSERT_FALSE(isString(3.14159f));
    ASSERT_FALSE(isString(fc));
}

//==============================================================================
TEST(TraitsTest, StreamTest)
{
    const FooClass fc;
    const BarClass bc;

    const std::string str("a");

    ASSERT_TRUE(isStreamable(bc));
    ASSERT_TRUE(isStreamable(str));
    ASSERT_TRUE(isStreamable(1));

    ASSERT_FALSE(isStreamable(fc));
}
