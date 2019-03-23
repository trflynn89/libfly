#include "fly/traits/traits.h"

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

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
template <typename T, fly::if_signed_integer::enabled<T> = 0>
bool isSignedInteger(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_signed_integer::disabled<T> = 0>
bool isSignedInteger(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::if_unsigned_integer::enabled<T> = 0>
bool isUnsignedInteger(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_unsigned_integer::disabled<T> = 0>
bool isUnsignedInteger(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::if_floating_point::enabled<T> = 0>
bool isFloat(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_floating_point::disabled<T> = 0>
bool isFloat(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::if_numeric::enabled<T> = 0>
bool isNumeric(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_numeric::disabled<T> = 0>
bool isNumeric(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::if_boolean::enabled<T> = 0>
bool isBool(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_boolean::disabled<T> = 0>
bool isBool(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::if_map::enabled<T> = 0>
bool isMap(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_map::disabled<T> = 0>
bool isMap(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::if_array::enabled<T> = 0>
bool isArray(const T &) noexcept
{
    return true;
}

template <typename T, fly::if_array::disabled<T> = 0>
bool isArray(const T &) noexcept
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
TEST(TraitsTest, SignedIntegerTest)
{
    EXPECT_TRUE(isSignedInteger(1));
    EXPECT_TRUE(isSignedInteger(-1));

    EXPECT_FALSE(isSignedInteger(std::array<int, 4>()));
    EXPECT_FALSE(isSignedInteger(std::deque<int>()));
    EXPECT_FALSE(isSignedInteger(std::forward_list<int>()));
    EXPECT_FALSE(isSignedInteger(std::list<int>()));
    EXPECT_FALSE(isSignedInteger(std::map<std::string, int>()));
    EXPECT_FALSE(isSignedInteger(std::multimap<std::string, int>()));
    EXPECT_FALSE(isSignedInteger(std::multiset<int>()));
    EXPECT_FALSE(isSignedInteger(std::set<int>()));
    EXPECT_FALSE(isSignedInteger(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(isSignedInteger(std::unordered_multimap<std::string, int>()));
    EXPECT_FALSE(isSignedInteger(std::unordered_multiset<int>()));
    EXPECT_FALSE(isSignedInteger(std::unordered_set<int>()));
    EXPECT_FALSE(isSignedInteger(std::vector<int>()));

    EXPECT_FALSE(isSignedInteger("foo"));
    EXPECT_FALSE(isSignedInteger(3.14));
    EXPECT_FALSE(isSignedInteger(true));
    EXPECT_FALSE(isSignedInteger(static_cast<unsigned int>(1)));
}

//==============================================================================
TEST(TraitsTest, UnsignedIntegerTest)
{
    EXPECT_TRUE(isUnsignedInteger(static_cast<unsigned int>(1)));
    EXPECT_TRUE(isUnsignedInteger(static_cast<unsigned int>(-1)));

    EXPECT_FALSE(isUnsignedInteger(std::array<int, 4>()));
    EXPECT_FALSE(isUnsignedInteger(std::deque<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::forward_list<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::list<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::map<std::string, int>()));
    EXPECT_FALSE(isUnsignedInteger(std::multimap<std::string, int>()));
    EXPECT_FALSE(isUnsignedInteger(std::multiset<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::set<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(
        isUnsignedInteger(std::unordered_multimap<std::string, int>()));
    EXPECT_FALSE(isUnsignedInteger(std::unordered_multiset<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::unordered_set<int>()));
    EXPECT_FALSE(isUnsignedInteger(std::vector<int>()));

    EXPECT_FALSE(isUnsignedInteger(1));
    EXPECT_FALSE(isUnsignedInteger(-1));
    EXPECT_FALSE(isUnsignedInteger("foo"));
    EXPECT_FALSE(isUnsignedInteger(3.14));
    EXPECT_FALSE(isUnsignedInteger(true));
}

//==============================================================================
TEST(TraitsTest, FloatTest)
{
    EXPECT_TRUE(isFloat(3.14f));
    EXPECT_TRUE(isFloat(3.14));
    EXPECT_TRUE(isFloat(static_cast<long double>(3.14)));

    EXPECT_FALSE(isFloat(std::array<int, 4>()));
    EXPECT_FALSE(isFloat(std::deque<int>()));
    EXPECT_FALSE(isFloat(std::forward_list<int>()));
    EXPECT_FALSE(isFloat(std::list<int>()));
    EXPECT_FALSE(isFloat(std::map<std::string, int>()));
    EXPECT_FALSE(isFloat(std::multimap<std::string, int>()));
    EXPECT_FALSE(isFloat(std::multiset<int>()));
    EXPECT_FALSE(isFloat(std::set<int>()));
    EXPECT_FALSE(isFloat(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(isFloat(std::unordered_multimap<std::string, int>()));
    EXPECT_FALSE(isFloat(std::unordered_multiset<int>()));
    EXPECT_FALSE(isFloat(std::unordered_set<int>()));
    EXPECT_FALSE(isFloat(std::vector<int>()));

    EXPECT_FALSE(isFloat(1));
    EXPECT_FALSE(isFloat(-1));
    EXPECT_FALSE(isFloat("foo"));
    EXPECT_FALSE(isFloat(true));
}

//==============================================================================
TEST(TraitsTest, NumericTest)
{
    EXPECT_TRUE(isNumeric(1));
    EXPECT_TRUE(isNumeric(-1));
    EXPECT_TRUE(isNumeric(static_cast<unsigned int>(1)));
    EXPECT_TRUE(isNumeric(3.14f));
    EXPECT_TRUE(isNumeric(3.14));
    EXPECT_TRUE(isNumeric(static_cast<long double>(3.14)));

    EXPECT_FALSE(isNumeric(std::array<int, 4>()));
    EXPECT_FALSE(isNumeric(std::deque<int>()));
    EXPECT_FALSE(isNumeric(std::forward_list<int>()));
    EXPECT_FALSE(isNumeric(std::list<int>()));
    EXPECT_FALSE(isNumeric(std::map<std::string, int>()));
    EXPECT_FALSE(isNumeric(std::multimap<std::string, int>()));
    EXPECT_FALSE(isNumeric(std::multiset<int>()));
    EXPECT_FALSE(isNumeric(std::set<int>()));
    EXPECT_FALSE(isNumeric(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(isNumeric(std::unordered_multimap<std::string, int>()));
    EXPECT_FALSE(isNumeric(std::unordered_multiset<int>()));
    EXPECT_FALSE(isNumeric(std::unordered_set<int>()));
    EXPECT_FALSE(isNumeric(std::vector<int>()));

    EXPECT_FALSE(isNumeric("foo"));
    EXPECT_FALSE(isNumeric(true));
}

//==============================================================================
TEST(TraitsTest, BoolTest)
{
    EXPECT_TRUE(isBool(true));
    EXPECT_TRUE(isBool(false));

    EXPECT_FALSE(isBool(std::array<int, 4>()));
    EXPECT_FALSE(isBool(std::deque<int>()));
    EXPECT_FALSE(isBool(std::forward_list<int>()));
    EXPECT_FALSE(isBool(std::list<int>()));
    EXPECT_FALSE(isBool(std::map<std::string, int>()));
    EXPECT_FALSE(isBool(std::multimap<std::string, int>()));
    EXPECT_FALSE(isBool(std::multiset<int>()));
    EXPECT_FALSE(isBool(std::set<int>()));
    EXPECT_FALSE(isBool(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(isBool(std::unordered_multimap<std::string, int>()));
    EXPECT_FALSE(isBool(std::unordered_multiset<int>()));
    EXPECT_FALSE(isBool(std::unordered_set<int>()));
    EXPECT_FALSE(isBool(std::vector<int>()));

    EXPECT_FALSE(isBool(1));
    EXPECT_FALSE(isBool(-1));
    EXPECT_FALSE(isBool("foo"));
    EXPECT_FALSE(isBool(3.14));
}

//==============================================================================
TEST(TraitsTest, MapTest)
{
    EXPECT_TRUE(isMap(std::map<std::string, int>()));
    EXPECT_TRUE(isMap(std::multimap<std::string, int>()));
    EXPECT_TRUE(isMap(std::unordered_map<std::string, int>()));
    EXPECT_TRUE(isMap(std::unordered_multimap<std::string, int>()));

    EXPECT_FALSE(isMap(std::array<int, 4>()));
    EXPECT_FALSE(isMap(std::deque<int>()));
    EXPECT_FALSE(isMap(std::forward_list<int>()));
    EXPECT_FALSE(isMap(std::list<int>()));
    EXPECT_FALSE(isMap(std::multiset<int>()));
    EXPECT_FALSE(isMap(std::set<int>()));
    EXPECT_FALSE(isMap(std::unordered_multiset<int>()));
    EXPECT_FALSE(isMap(std::unordered_set<int>()));
    EXPECT_FALSE(isMap(std::vector<int>()));

    EXPECT_FALSE(isMap(1));
    EXPECT_FALSE(isMap(-1));
    EXPECT_FALSE(isMap("foo"));
    EXPECT_FALSE(isMap(3.14));
    EXPECT_FALSE(isMap(true));
}

//==============================================================================
TEST(TraitsTest, ArrayTest)
{
    EXPECT_TRUE(isArray(std::array<int, 4>()));
    EXPECT_TRUE(isArray(std::deque<int>()));
    EXPECT_TRUE(isArray(std::forward_list<int>()));
    EXPECT_TRUE(isArray(std::list<int>()));
    EXPECT_TRUE(isArray(std::multiset<int>()));
    EXPECT_TRUE(isArray(std::set<int>()));
    EXPECT_TRUE(isArray(std::unordered_multiset<int>()));
    EXPECT_TRUE(isArray(std::unordered_set<int>()));
    EXPECT_TRUE(isArray(std::vector<int>()));

    EXPECT_FALSE(isArray(std::map<std::string, int>()));
    EXPECT_FALSE(isArray(std::multimap<std::string, int>()));
    EXPECT_FALSE(isArray(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(isArray(std::unordered_multimap<std::string, int>()));

    EXPECT_FALSE(isArray(1));
    EXPECT_FALSE(isArray(-1));
    EXPECT_FALSE(isArray("foo"));
    EXPECT_FALSE(isArray(3.14));
    EXPECT_FALSE(isArray(true));
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
