#include "fly/types/json/json_traits.h"

#include "fly/traits/traits.h"
#include "fly/types/string/string_literal.h"

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
template <typename T, fly::enable_if_all<fly::JsonTraits::is_string<T>> = 0>
bool isString(const T &) noexcept
{
    return fly::JsonTraits::is_string_v<T>;
}

template <typename T, fly::enable_if_not_all<fly::JsonTraits::is_string<T>> = 0>
bool isString(const T &) noexcept
{
    return fly::JsonTraits::is_string_v<T>;
}

//==========================================================================
template <typename T, fly::enable_if_all<fly::JsonTraits::is_boolean<T>> = 0>
bool isBool(const T &) noexcept
{
    return fly::JsonTraits::is_boolean_v<T>;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_boolean<T>> = 0>
bool isBool(const T &) noexcept
{
    return fly::JsonTraits::is_boolean_v<T>;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_signed_integer<T>> = 0>
bool isSignedInteger(const T &) noexcept
{
    return fly::JsonTraits::is_signed_integer_v<T>;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_signed_integer<T>> = 0>
bool isSignedInteger(const T &) noexcept
{
    return fly::JsonTraits::is_signed_integer_v<T>;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_unsigned_integer<T>> = 0>
bool isUnsignedInteger(const T &) noexcept
{
    return fly::JsonTraits::is_unsigned_integer_v<T>;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_unsigned_integer<T>> = 0>
bool isUnsignedInteger(const T &) noexcept
{
    return fly::JsonTraits::is_unsigned_integer_v<T>;
}

//==========================================================================
template <
    typename T,
    fly::enable_if_all<fly::JsonTraits::is_floating_point<T>> = 0>
bool isFloat(const T &) noexcept
{
    return fly::JsonTraits::is_floating_point_v<T>;
}

template <
    typename T,
    fly::enable_if_not_all<fly::JsonTraits::is_floating_point<T>> = 0>
bool isFloat(const T &) noexcept
{
    return fly::JsonTraits::is_floating_point_v<T>;
}

//==========================================================================
template <typename T, fly::enable_if_all<fly::JsonTraits::is_object<T>> = 0>
bool isObject(const T &) noexcept
{
    return fly::JsonTraits::is_object_v<T>;
}

template <typename T, fly::enable_if_not_all<fly::JsonTraits::is_object<T>> = 0>
bool isObject(const T &) noexcept
{
    return fly::JsonTraits::is_object_v<T>;
}

//==========================================================================
template <typename T, fly::enable_if_all<fly::JsonTraits::is_array<T>> = 0>
bool isArray(const T &) noexcept
{
    return fly::JsonTraits::is_array_v<T>;
}

template <typename T, fly::enable_if_not_all<fly::JsonTraits::is_array<T>> = 0>
bool isArray(const T &) noexcept
{
    return fly::JsonTraits::is_array_v<T>;
}

const auto s_array = std::array<int, 4>();
const auto s_deque = std::deque<int>();
const auto s_forward_list = std::forward_list<int>();
const auto s_list = std::list<int>();
const auto s_map = std::map<std::string, int>();
const auto s_multimap = std::multimap<std::string, int>();
const auto s_multiset = std::multiset<int>();
const auto s_set = std::set<int>();
const auto s_unordered_map = std::unordered_map<std::string, int>();
const auto s_unordered_multimap = std::unordered_multimap<std::string, int>();
const auto s_unordered_multiset = std::unordered_multiset<int>();
const auto s_unordered_set = std::unordered_set<int>();
const auto s_vector = std::vector<int>();

} // namespace

//==============================================================================
TEST(JsonTraitsTest, StringTest)
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
    EXPECT_TRUE(isString(str2));
    EXPECT_TRUE(isString(cstr1));
    EXPECT_TRUE(isString(cstr2));
    EXPECT_TRUE(isString(arr1));
    EXPECT_TRUE(isString(arr2));

    EXPECT_FALSE(isString(s_array));
    EXPECT_FALSE(isString(s_deque));
    EXPECT_FALSE(isString(s_forward_list));
    EXPECT_FALSE(isString(s_list));
    EXPECT_FALSE(isString(s_map));
    EXPECT_FALSE(isString(s_multimap));
    EXPECT_FALSE(isString(s_multiset));
    EXPECT_FALSE(isString(s_set));
    EXPECT_FALSE(isString(s_unordered_map));
    EXPECT_FALSE(isString(s_unordered_multimap));
    EXPECT_FALSE(isString(s_unordered_multiset));
    EXPECT_FALSE(isString(s_unordered_set));
    EXPECT_FALSE(isString(s_vector));

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

    EXPECT_FALSE(isBool(s_array));
    EXPECT_FALSE(isBool(s_deque));
    EXPECT_FALSE(isBool(s_forward_list));
    EXPECT_FALSE(isBool(s_list));
    EXPECT_FALSE(isBool(s_map));
    EXPECT_FALSE(isBool(s_multimap));
    EXPECT_FALSE(isBool(s_multiset));
    EXPECT_FALSE(isBool(s_set));
    EXPECT_FALSE(isBool(s_unordered_map));
    EXPECT_FALSE(isBool(s_unordered_multimap));
    EXPECT_FALSE(isBool(s_unordered_multiset));
    EXPECT_FALSE(isBool(s_unordered_set));
    EXPECT_FALSE(isBool(s_vector));

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

    EXPECT_FALSE(isSignedInteger(s_array));
    EXPECT_FALSE(isSignedInteger(s_deque));
    EXPECT_FALSE(isSignedInteger(s_forward_list));
    EXPECT_FALSE(isSignedInteger(s_list));
    EXPECT_FALSE(isSignedInteger(s_map));
    EXPECT_FALSE(isSignedInteger(s_multimap));
    EXPECT_FALSE(isSignedInteger(s_multiset));
    EXPECT_FALSE(isSignedInteger(s_set));
    EXPECT_FALSE(isSignedInteger(s_unordered_map));
    EXPECT_FALSE(isSignedInteger(s_unordered_multimap));
    EXPECT_FALSE(isSignedInteger(s_unordered_multiset));
    EXPECT_FALSE(isSignedInteger(s_unordered_set));
    EXPECT_FALSE(isSignedInteger(s_vector));

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

    EXPECT_FALSE(isUnsignedInteger(s_array));
    EXPECT_FALSE(isUnsignedInteger(s_deque));
    EXPECT_FALSE(isUnsignedInteger(s_forward_list));
    EXPECT_FALSE(isUnsignedInteger(s_list));
    EXPECT_FALSE(isUnsignedInteger(s_map));
    EXPECT_FALSE(isUnsignedInteger(s_multimap));
    EXPECT_FALSE(isUnsignedInteger(s_multiset));
    EXPECT_FALSE(isUnsignedInteger(s_set));
    EXPECT_FALSE(isUnsignedInteger(s_unordered_map));
    EXPECT_FALSE(isUnsignedInteger(s_unordered_multimap));
    EXPECT_FALSE(isUnsignedInteger(s_unordered_multiset));
    EXPECT_FALSE(isUnsignedInteger(s_unordered_set));
    EXPECT_FALSE(isUnsignedInteger(s_vector));

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

    EXPECT_FALSE(isFloat(s_array));
    EXPECT_FALSE(isFloat(s_deque));
    EXPECT_FALSE(isFloat(s_forward_list));
    EXPECT_FALSE(isFloat(s_list));
    EXPECT_FALSE(isFloat(s_map));
    EXPECT_FALSE(isFloat(s_multimap));
    EXPECT_FALSE(isFloat(s_multiset));
    EXPECT_FALSE(isFloat(s_set));
    EXPECT_FALSE(isFloat(s_unordered_map));
    EXPECT_FALSE(isFloat(s_unordered_multimap));
    EXPECT_FALSE(isFloat(s_unordered_multiset));
    EXPECT_FALSE(isFloat(s_unordered_set));
    EXPECT_FALSE(isFloat(s_vector));

    EXPECT_FALSE(isFloat(1));
    EXPECT_FALSE(isFloat(-1));
    EXPECT_FALSE(isFloat("foo"));
    EXPECT_FALSE(isFloat(true));
}

//==============================================================================
TEST(JsonTraitsTest, ObjectTest)
{
    EXPECT_TRUE(isObject(s_map));
    EXPECT_TRUE(isObject(s_multimap));
    EXPECT_TRUE(isObject(s_unordered_map));
    EXPECT_TRUE(isObject(s_unordered_multimap));

    EXPECT_FALSE(isObject(s_array));
    EXPECT_FALSE(isObject(s_deque));
    EXPECT_FALSE(isObject(s_forward_list));
    EXPECT_FALSE(isObject(s_list));
    EXPECT_FALSE(isObject(s_multiset));
    EXPECT_FALSE(isObject(s_set));
    EXPECT_FALSE(isObject(s_unordered_multiset));
    EXPECT_FALSE(isObject(s_unordered_set));
    EXPECT_FALSE(isObject(s_vector));

    EXPECT_FALSE(isObject(1));
    EXPECT_FALSE(isObject(-1));
    EXPECT_FALSE(isObject("foo"));
    EXPECT_FALSE(isObject(3.14));
    EXPECT_FALSE(isObject(true));
}

//==============================================================================
TEST(JsonTraitsTest, ArrayTest)
{
    EXPECT_TRUE(isArray(s_array));
    EXPECT_TRUE(isArray(s_deque));
    EXPECT_TRUE(isArray(s_forward_list));
    EXPECT_TRUE(isArray(s_list));
    EXPECT_TRUE(isArray(s_multiset));
    EXPECT_TRUE(isArray(s_set));
    EXPECT_TRUE(isArray(s_unordered_multiset));
    EXPECT_TRUE(isArray(s_unordered_set));
    EXPECT_TRUE(isArray(s_vector));

    EXPECT_FALSE(isArray(s_map));
    EXPECT_FALSE(isArray(s_multimap));
    EXPECT_FALSE(isArray(s_unordered_map));
    EXPECT_FALSE(isArray(s_unordered_multimap));

    EXPECT_FALSE(isArray(1));
    EXPECT_FALSE(isArray(-1));
    EXPECT_FALSE(isArray("foo"));
    EXPECT_FALSE(isArray(3.14));
    EXPECT_FALSE(isArray(true));
}
