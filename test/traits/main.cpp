#include <gtest/gtest.h>

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
    template <typename T, fly::enable_if_all<fly::if_foo::enabled<T>>...>
    bool callFoo(const T &arg)
    {
        return arg.Foo();
    }

    template <typename T, fly::enable_if_all<fly::if_foo::disabled<T>>...>
    bool callFoo(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::enable_if_all<fly::if_string::enabled<T>>...>
    bool isString(const T &)
    {
        return true;
    }

    template <typename T, fly::enable_if_all<fly::if_string::disabled<T>>...>
    bool isString(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::enable_if_all<fly::if_ostream::enabled<T>>...>
    bool isStreamable(const T &)
    {
        return true;
    }

    template <typename T, fly::enable_if_all<fly::if_ostream::disabled<T>>...>
    bool isStreamable(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T, fly::enable_if_all<fly::if_hash::enabled<T>>...>
    bool isHashable(const T &)
    {
        return true;
    }

    template <typename T, fly::enable_if_all<fly::if_hash::disabled<T>>...>
    bool isHashable(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T,
        fly::enable_if_all<std::is_pointer<T>, std::is_pod<typename std::remove_pointer<T>::type>>...>
    bool isPodPointer(const T &)
    {
        return true;
    }

    template <typename T,
        fly::enable_if_not_all<std::is_pointer<T>, std::is_pod<typename std::remove_pointer<T>::type>>...>
    bool isPodPointer(const T &)
    {
        return false;
    }

    //==========================================================================
    template <typename T,
        fly::enable_if_any<std::is_pointer<T>, std::is_pod<typename std::remove_pointer<T>::type>>...>
    bool isPodOrPointer(const T &)
    {
        return true;
    }

    template <typename T,
        fly::enable_if_none<std::is_pointer<T>, std::is_pod<typename std::remove_pointer<T>::type>>...>
    bool isPodOrPointer(const T &)
    {
        return false;
    }
}

//==============================================================================
namespace std
{
    template <>
    struct hash<FooClass>
    {
        size_t operator()(const FooClass &) const
        {
            return 0;
        }
    };
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

//==============================================================================
TEST(TraitsTest, HashTest)
{
    const FooClass fc;
    const BarClass bc;

    const std::string str("a");

    ASSERT_TRUE(isHashable(fc));
    ASSERT_TRUE(isHashable(str));
    ASSERT_TRUE(isHashable(1));

    ASSERT_FALSE(isHashable(bc));
}

//==============================================================================
TEST(TraitsTest, EnableIfAllTest)
{
    const FooClass fc;
    const std::string str("a");

    int i = 0;
    bool b = false;
    float f = 3.14159;

    ASSERT_TRUE(isPodPointer(&i));
    ASSERT_TRUE(isPodPointer(&b));
    ASSERT_TRUE(isPodPointer(&f));

    ASSERT_FALSE(isPodPointer(i));
    ASSERT_FALSE(isPodPointer(b));
    ASSERT_FALSE(isPodPointer(f));
    ASSERT_FALSE(isPodPointer(fc));
    ASSERT_FALSE(isPodPointer(&fc));
    ASSERT_FALSE(isPodPointer(str));
    ASSERT_FALSE(isPodPointer(&str));
}

//==============================================================================
TEST(TraitsTest, EnableIfAnyTest)
{
    const FooClass fc;
    const std::string str("a");

    int i = 0;
    bool b = false;
    float f = 3.14159;

    ASSERT_TRUE(isPodOrPointer(i));
    ASSERT_TRUE(isPodOrPointer(&i));
    ASSERT_TRUE(isPodOrPointer(b));
    ASSERT_TRUE(isPodOrPointer(&b));
    ASSERT_TRUE(isPodOrPointer(f));
    ASSERT_TRUE(isPodOrPointer(&f));
    ASSERT_TRUE(isPodOrPointer(&fc));
    ASSERT_TRUE(isPodOrPointer(&str));

    ASSERT_FALSE(isPodOrPointer(fc));
    ASSERT_FALSE(isPodOrPointer(str));
}
