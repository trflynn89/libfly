#include "fly/traits/traits.h"

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

//==========================================================================
DECLARATION_TESTS(foo, T, std::declval<const T &>().Foo());

//==========================================================================
class FooClass
{
public:
    FooClass()
    {
    }
    bool Foo() const
    {
        return true;
    }
};

//==========================================================================
class BarClass
{
public:
    BarClass()
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
template <typename T, fly::if_numeric::enabled<T> = 0>
bool isNumeric(const T &)
{
    return true;
}

template <typename T, fly::if_numeric::disabled<T> = 0>
bool isNumeric(const T &)
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

//==========================================================================
template <typename T, fly::if_map::enabled<T> = 0>
bool isMap(const T &)
{
    return true;
}

template <typename T, fly::if_map::disabled<T> = 0>
bool isMap(const T &)
{
    return false;
}

//==========================================================================
template <typename T, fly::if_array::enabled<T> = 0>
bool isArray(const T &)
{
    return true;
}

template <typename T, fly::if_array::disabled<T> = 0>
bool isArray(const T &)
{
    return false;
}

} // namespace

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

    const char arr1[] = {'g', '\0'};
    char arr2[] = {'h', '\0'};

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

    ASSERT_FALSE(isSignedInteger(std::array<int, 4>()));
    ASSERT_FALSE(isSignedInteger(std::deque<int>()));
    ASSERT_FALSE(isSignedInteger(std::forward_list<int>()));
    ASSERT_FALSE(isSignedInteger(std::list<int>()));
    ASSERT_FALSE(isSignedInteger(std::map<std::string, int>()));
    ASSERT_FALSE(isSignedInteger(std::multimap<std::string, int>()));
    ASSERT_FALSE(isSignedInteger(std::multiset<int>()));
    ASSERT_FALSE(isSignedInteger(std::set<int>()));
    ASSERT_FALSE(isSignedInteger(std::unordered_map<std::string, int>()));
    ASSERT_FALSE(isSignedInteger(std::unordered_multimap<std::string, int>()));
    ASSERT_FALSE(isSignedInteger(std::unordered_multiset<int>()));
    ASSERT_FALSE(isSignedInteger(std::unordered_set<int>()));
    ASSERT_FALSE(isSignedInteger(std::vector<int>()));

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

    ASSERT_FALSE(isUnsignedInteger(std::array<int, 4>()));
    ASSERT_FALSE(isUnsignedInteger(std::deque<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::forward_list<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::list<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::map<std::string, int>()));
    ASSERT_FALSE(isUnsignedInteger(std::multimap<std::string, int>()));
    ASSERT_FALSE(isUnsignedInteger(std::multiset<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::set<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::unordered_map<std::string, int>()));
    ASSERT_FALSE(
        isUnsignedInteger(std::unordered_multimap<std::string, int>()));
    ASSERT_FALSE(isUnsignedInteger(std::unordered_multiset<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::unordered_set<int>()));
    ASSERT_FALSE(isUnsignedInteger(std::vector<int>()));

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

    ASSERT_FALSE(isFloat(std::array<int, 4>()));
    ASSERT_FALSE(isFloat(std::deque<int>()));
    ASSERT_FALSE(isFloat(std::forward_list<int>()));
    ASSERT_FALSE(isFloat(std::list<int>()));
    ASSERT_FALSE(isFloat(std::map<std::string, int>()));
    ASSERT_FALSE(isFloat(std::multimap<std::string, int>()));
    ASSERT_FALSE(isFloat(std::multiset<int>()));
    ASSERT_FALSE(isFloat(std::set<int>()));
    ASSERT_FALSE(isFloat(std::unordered_map<std::string, int>()));
    ASSERT_FALSE(isFloat(std::unordered_multimap<std::string, int>()));
    ASSERT_FALSE(isFloat(std::unordered_multiset<int>()));
    ASSERT_FALSE(isFloat(std::unordered_set<int>()));
    ASSERT_FALSE(isFloat(std::vector<int>()));

    ASSERT_FALSE(isFloat(1));
    ASSERT_FALSE(isFloat(-1));
    ASSERT_FALSE(isFloat("foo"));
    ASSERT_FALSE(isFloat(true));
}

//==============================================================================
TEST(TraitsTest, NumericTest)
{
    ASSERT_TRUE(isNumeric(1));
    ASSERT_TRUE(isNumeric(-1));
    ASSERT_TRUE(isNumeric(static_cast<unsigned int>(1)));
    ASSERT_TRUE(isNumeric(3.14f));
    ASSERT_TRUE(isNumeric(3.14));
    ASSERT_TRUE(isNumeric(static_cast<long double>(3.14)));

    ASSERT_FALSE(isNumeric(std::array<int, 4>()));
    ASSERT_FALSE(isNumeric(std::deque<int>()));
    ASSERT_FALSE(isNumeric(std::forward_list<int>()));
    ASSERT_FALSE(isNumeric(std::list<int>()));
    ASSERT_FALSE(isNumeric(std::map<std::string, int>()));
    ASSERT_FALSE(isNumeric(std::multimap<std::string, int>()));
    ASSERT_FALSE(isNumeric(std::multiset<int>()));
    ASSERT_FALSE(isNumeric(std::set<int>()));
    ASSERT_FALSE(isNumeric(std::unordered_map<std::string, int>()));
    ASSERT_FALSE(isNumeric(std::unordered_multimap<std::string, int>()));
    ASSERT_FALSE(isNumeric(std::unordered_multiset<int>()));
    ASSERT_FALSE(isNumeric(std::unordered_set<int>()));
    ASSERT_FALSE(isNumeric(std::vector<int>()));

    ASSERT_FALSE(isNumeric("foo"));
    ASSERT_FALSE(isNumeric(true));
}

//==============================================================================
TEST(TraitsTest, BoolTest)
{
    ASSERT_TRUE(isBool(true));
    ASSERT_TRUE(isBool(false));

    ASSERT_FALSE(isBool(std::array<int, 4>()));
    ASSERT_FALSE(isBool(std::deque<int>()));
    ASSERT_FALSE(isBool(std::forward_list<int>()));
    ASSERT_FALSE(isBool(std::list<int>()));
    ASSERT_FALSE(isBool(std::map<std::string, int>()));
    ASSERT_FALSE(isBool(std::multimap<std::string, int>()));
    ASSERT_FALSE(isBool(std::multiset<int>()));
    ASSERT_FALSE(isBool(std::set<int>()));
    ASSERT_FALSE(isBool(std::unordered_map<std::string, int>()));
    ASSERT_FALSE(isBool(std::unordered_multimap<std::string, int>()));
    ASSERT_FALSE(isBool(std::unordered_multiset<int>()));
    ASSERT_FALSE(isBool(std::unordered_set<int>()));
    ASSERT_FALSE(isBool(std::vector<int>()));

    ASSERT_FALSE(isBool(1));
    ASSERT_FALSE(isBool(-1));
    ASSERT_FALSE(isBool("foo"));
    ASSERT_FALSE(isBool(3.14));
}

//==============================================================================
TEST(TraitsTest, MapTest)
{
    ASSERT_TRUE(isMap(std::map<std::string, int>()));
    ASSERT_TRUE(isMap(std::multimap<std::string, int>()));
    ASSERT_TRUE(isMap(std::unordered_map<std::string, int>()));
    ASSERT_TRUE(isMap(std::unordered_multimap<std::string, int>()));

    ASSERT_FALSE(isMap(std::array<int, 4>()));
    ASSERT_FALSE(isMap(std::deque<int>()));
    ASSERT_FALSE(isMap(std::forward_list<int>()));
    ASSERT_FALSE(isMap(std::list<int>()));
    ASSERT_FALSE(isMap(std::multiset<int>()));
    ASSERT_FALSE(isMap(std::set<int>()));
    ASSERT_FALSE(isMap(std::unordered_multiset<int>()));
    ASSERT_FALSE(isMap(std::unordered_set<int>()));
    ASSERT_FALSE(isMap(std::vector<int>()));

    ASSERT_FALSE(isMap(1));
    ASSERT_FALSE(isMap(-1));
    ASSERT_FALSE(isMap("foo"));
    ASSERT_FALSE(isMap(3.14));
    ASSERT_FALSE(isMap(true));
}

//==============================================================================
TEST(TraitsTest, ArrayTest)
{
    ASSERT_TRUE(isArray(std::array<int, 4>()));
    ASSERT_TRUE(isArray(std::deque<int>()));
    ASSERT_TRUE(isArray(std::forward_list<int>()));
    ASSERT_TRUE(isArray(std::list<int>()));
    ASSERT_TRUE(isArray(std::multiset<int>()));
    ASSERT_TRUE(isArray(std::set<int>()));
    ASSERT_TRUE(isArray(std::unordered_multiset<int>()));
    ASSERT_TRUE(isArray(std::unordered_set<int>()));
    ASSERT_TRUE(isArray(std::vector<int>()));

    ASSERT_FALSE(isArray(std::map<std::string, int>()));
    ASSERT_FALSE(isArray(std::multimap<std::string, int>()));
    ASSERT_FALSE(isArray(std::unordered_map<std::string, int>()));
    ASSERT_FALSE(isArray(std::unordered_multimap<std::string, int>()));

    ASSERT_FALSE(isArray(1));
    ASSERT_FALSE(isArray(-1));
    ASSERT_FALSE(isArray("foo"));
    ASSERT_FALSE(isArray(3.14));
    ASSERT_FALSE(isArray(true));
}
