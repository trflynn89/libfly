#include "fly/types/json_traits.h"

#include "fly/traits/traits.h"
#include "fly/types/string_literal.h"

#include <gtest/gtest.h>

#include <array>
#include <deque>
#include <forward_list>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_string<T>> = 0>
bool isString(const T &) noexcept
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_string<T>> = 0>
bool isString(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::enable_if_all<fly::JsonTraits::is_boolean<T>> = 0>
bool isBool(const T &) noexcept
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_boolean<T>> = 0>
bool isBool(const T &) noexcept
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_signed_integer<T>> = 0>
bool isSignedInteger(const T &) noexcept
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_signed_integer<T>> = 0>
bool isSignedInteger(const T &) noexcept
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_unsigned_integer<T>> = 0>
bool isUnsignedInteger(const T &) noexcept
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_unsigned_integer<T>> = 0>
bool isUnsignedInteger(const T &) noexcept
{
    return false;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_floating_point<T>> = 0>
bool isFloat(const T &) noexcept
{
    return true;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_floating_point<T>> = 0>
bool isFloat(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::enable_if_all<fly::JsonTraits::is_object<T>> = 0>
bool isObject(const T &) noexcept
{
    return true;
}

template <typename T, fly::enable_if_not_all<fly::JsonTraits::is_object<T>> = 0>
bool isObject(const T &) noexcept
{
    return false;
}

//==========================================================================
template <typename T, fly::enable_if_all<fly::JsonTraits::is_array<T>> = 0>
bool isArray(const T &) noexcept
{
    return true;
}

template <typename T, fly::enable_if_not_all<fly::JsonTraits::is_array<T>> = 0>
bool isArray(const T &) noexcept
{
    return false;
}

} // namespace

//==============================================================================
TEST(TraitsTest, StringTest)
{
    using string_type = typename fly::JsonTraits::string_type;
    using char_type = typename string_type::value_type;

    const string_type str1(FLY_STR(char_type, "a"));
    string_type str2(FLY_STR(char_type, "b"));

    const char_type *cstr1 = "c";
    char_type *cstr2 = (char_type *)"d";

    const char_type chr1 = 'e';
    char_type chr2 = 'f';

    const char_type arr1[] = {'g', '\0'};
    char_type arr2[] = {'h', '\0'};

    EXPECT_TRUE(isString(str1));
    EXPECT_TRUE(isString(str1));
    EXPECT_TRUE(isString(cstr1));
    EXPECT_TRUE(isString(cstr2));
    EXPECT_TRUE(isString(arr1));
    EXPECT_TRUE(isString(arr2));

    EXPECT_FALSE(isString(std::array<int, 4>()));
    EXPECT_FALSE(isString(std::deque<int>()));
    EXPECT_FALSE(isString(std::forward_list<int>()));
    EXPECT_FALSE(isString(std::list<int>()));
    EXPECT_FALSE(isString(std::map<std::string, int>()));
    EXPECT_FALSE(isString(std::multimap<std::string, int>()));
    EXPECT_FALSE(isString(std::multiset<int>()));
    EXPECT_FALSE(isString(std::set<int>()));
    EXPECT_FALSE(isString(std::unordered_map<std::string, int>()));
    EXPECT_FALSE(isString(std::unordered_multimap<std::string, int>()));
    EXPECT_FALSE(isString(std::unordered_multiset<int>()));
    EXPECT_FALSE(isString(std::unordered_set<int>()));
    EXPECT_FALSE(isString(std::vector<int>()));

    EXPECT_FALSE(isString(1));
    EXPECT_FALSE(isString(true));
    EXPECT_FALSE(isString(3.14159f));
    EXPECT_FALSE(isString(3.14159f));
    EXPECT_FALSE(isString(chr1));
    EXPECT_FALSE(isString(chr2));
}

//==============================================================================
TEST(JsonTraitsTest, BoolTest)
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
TEST(JsonTraitsTest, SignedIntegerTest)
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
TEST(JsonTraitsTest, UnsignedIntegerTest)
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
TEST(JsonTraitsTest, FloatTest)
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
TEST(JsonTraitsTest, ObjectTest)
{
    EXPECT_TRUE(isObject(std::map<std::string, int>()));
    EXPECT_TRUE(isObject(std::multimap<std::string, int>()));
    EXPECT_TRUE(isObject(std::unordered_map<std::string, int>()));
    EXPECT_TRUE(isObject(std::unordered_multimap<std::string, int>()));

    EXPECT_FALSE(isObject(std::array<int, 4>()));
    EXPECT_FALSE(isObject(std::deque<int>()));
    EXPECT_FALSE(isObject(std::forward_list<int>()));
    EXPECT_FALSE(isObject(std::list<int>()));
    EXPECT_FALSE(isObject(std::multiset<int>()));
    EXPECT_FALSE(isObject(std::set<int>()));
    EXPECT_FALSE(isObject(std::unordered_multiset<int>()));
    EXPECT_FALSE(isObject(std::unordered_set<int>()));
    EXPECT_FALSE(isObject(std::vector<int>()));

    EXPECT_FALSE(isObject(1));
    EXPECT_FALSE(isObject(-1));
    EXPECT_FALSE(isObject("foo"));
    EXPECT_FALSE(isObject(3.14));
    EXPECT_FALSE(isObject(true));
}

//==============================================================================
TEST(JsonTraitsTest, ArrayTest)
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
