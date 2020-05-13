#include "fly/types/json/json_traits.hpp"

#include "fly/traits/traits.hpp"
#include "fly/types/string/string_literal.hpp"

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
template <typename T>
bool is_string(const T &) noexcept
{
    return fly::JsonTraits::is_string_v<T>;
}

//==========================================================================
template <typename T>
bool is_bool(const T &) noexcept
{
    return fly::JsonTraits::is_boolean_v<T>;
}

//==========================================================================
template <typename T>
bool is_signed_integer(const T &) noexcept
{
    return fly::JsonTraits::is_signed_integer_v<T>;
}

//==========================================================================
template <typename T>
bool is_unsigned_integer(const T &) noexcept
{
    return fly::JsonTraits::is_unsigned_integer_v<T>;
}

//==========================================================================
template <typename T>
bool is_floating_point(const T &) noexcept
{
    return fly::JsonTraits::is_floating_point_v<T>;
}

//==========================================================================
template <typename T>
bool is_object(const T &) noexcept
{
    return fly::JsonTraits::is_object_v<T>;
}

//==========================================================================
template <typename T>
bool is_array(const T &) noexcept
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
TEST(JsonTraitsTest, String)
{
    using string_type = typename fly::JsonTraits::string_type;
    using char_type = typename string_type::value_type;

    const string_type str1(FLY_STR(char_type, "a"));
    string_type str2(FLY_STR(char_type, "b"));

    const char_type *cstr1 = "c";
    char_type *cstr2 = const_cast<char_type *>("d");

    const char_type chr1 = 'e';
    char_type chr2 = 'f';

    const char_type arr1[] = {'g', '\0'};
    char_type arr2[] = {'h', '\0'};

    EXPECT_TRUE(is_string(str1));
    EXPECT_TRUE(is_string(str2));
    EXPECT_TRUE(is_string(cstr1));
    EXPECT_TRUE(is_string(cstr2));
    EXPECT_TRUE(is_string(arr1));
    EXPECT_TRUE(is_string(arr2));

    EXPECT_FALSE(is_string(s_array));
    EXPECT_FALSE(is_string(s_deque));
    EXPECT_FALSE(is_string(s_forward_list));
    EXPECT_FALSE(is_string(s_list));
    EXPECT_FALSE(is_string(s_map));
    EXPECT_FALSE(is_string(s_multimap));
    EXPECT_FALSE(is_string(s_multiset));
    EXPECT_FALSE(is_string(s_set));
    EXPECT_FALSE(is_string(s_unordered_map));
    EXPECT_FALSE(is_string(s_unordered_multimap));
    EXPECT_FALSE(is_string(s_unordered_multiset));
    EXPECT_FALSE(is_string(s_unordered_set));
    EXPECT_FALSE(is_string(s_vector));

    EXPECT_FALSE(is_string(1));
    EXPECT_FALSE(is_string(true));
    EXPECT_FALSE(is_string(3.14159f));
    EXPECT_FALSE(is_string(3.14159f));
    EXPECT_FALSE(is_string(chr1));
    EXPECT_FALSE(is_string(chr2));
}

//==============================================================================
TEST(JsonTraitsTest, Bool)
{
    EXPECT_TRUE(is_bool(true));
    EXPECT_TRUE(is_bool(false));

    EXPECT_FALSE(is_bool(s_array));
    EXPECT_FALSE(is_bool(s_deque));
    EXPECT_FALSE(is_bool(s_forward_list));
    EXPECT_FALSE(is_bool(s_list));
    EXPECT_FALSE(is_bool(s_map));
    EXPECT_FALSE(is_bool(s_multimap));
    EXPECT_FALSE(is_bool(s_multiset));
    EXPECT_FALSE(is_bool(s_set));
    EXPECT_FALSE(is_bool(s_unordered_map));
    EXPECT_FALSE(is_bool(s_unordered_multimap));
    EXPECT_FALSE(is_bool(s_unordered_multiset));
    EXPECT_FALSE(is_bool(s_unordered_set));
    EXPECT_FALSE(is_bool(s_vector));

    EXPECT_FALSE(is_bool(1));
    EXPECT_FALSE(is_bool(-1));
    EXPECT_FALSE(is_bool("foo"));
    EXPECT_FALSE(is_bool(3.14));
}

//==============================================================================
TEST(JsonTraitsTest, SignedInteger)
{
    EXPECT_TRUE(is_signed_integer(1));
    EXPECT_TRUE(is_signed_integer(-1));

    EXPECT_FALSE(is_signed_integer(s_array));
    EXPECT_FALSE(is_signed_integer(s_deque));
    EXPECT_FALSE(is_signed_integer(s_forward_list));
    EXPECT_FALSE(is_signed_integer(s_list));
    EXPECT_FALSE(is_signed_integer(s_map));
    EXPECT_FALSE(is_signed_integer(s_multimap));
    EXPECT_FALSE(is_signed_integer(s_multiset));
    EXPECT_FALSE(is_signed_integer(s_set));
    EXPECT_FALSE(is_signed_integer(s_unordered_map));
    EXPECT_FALSE(is_signed_integer(s_unordered_multimap));
    EXPECT_FALSE(is_signed_integer(s_unordered_multiset));
    EXPECT_FALSE(is_signed_integer(s_unordered_set));
    EXPECT_FALSE(is_signed_integer(s_vector));

    EXPECT_FALSE(is_signed_integer("foo"));
    EXPECT_FALSE(is_signed_integer(3.14));
    EXPECT_FALSE(is_signed_integer(true));
    EXPECT_FALSE(is_signed_integer(static_cast<unsigned int>(1)));
}

//==============================================================================
TEST(JsonTraitsTest, UnsignedInteger)
{
    EXPECT_TRUE(is_unsigned_integer(static_cast<unsigned int>(1)));
    EXPECT_TRUE(is_unsigned_integer(static_cast<unsigned int>(-1)));

    EXPECT_FALSE(is_unsigned_integer(s_array));
    EXPECT_FALSE(is_unsigned_integer(s_deque));
    EXPECT_FALSE(is_unsigned_integer(s_forward_list));
    EXPECT_FALSE(is_unsigned_integer(s_list));
    EXPECT_FALSE(is_unsigned_integer(s_map));
    EXPECT_FALSE(is_unsigned_integer(s_multimap));
    EXPECT_FALSE(is_unsigned_integer(s_multiset));
    EXPECT_FALSE(is_unsigned_integer(s_set));
    EXPECT_FALSE(is_unsigned_integer(s_unordered_map));
    EXPECT_FALSE(is_unsigned_integer(s_unordered_multimap));
    EXPECT_FALSE(is_unsigned_integer(s_unordered_multiset));
    EXPECT_FALSE(is_unsigned_integer(s_unordered_set));
    EXPECT_FALSE(is_unsigned_integer(s_vector));

    EXPECT_FALSE(is_unsigned_integer(1));
    EXPECT_FALSE(is_unsigned_integer(-1));
    EXPECT_FALSE(is_unsigned_integer("foo"));
    EXPECT_FALSE(is_unsigned_integer(3.14));
    EXPECT_FALSE(is_unsigned_integer(true));
}

//==============================================================================
TEST(JsonTraitsTest, Float)
{
    EXPECT_TRUE(is_floating_point(3.14f));
    EXPECT_TRUE(is_floating_point(3.14));
    EXPECT_TRUE(is_floating_point(static_cast<long double>(3.14)));

    EXPECT_FALSE(is_floating_point(s_array));
    EXPECT_FALSE(is_floating_point(s_deque));
    EXPECT_FALSE(is_floating_point(s_forward_list));
    EXPECT_FALSE(is_floating_point(s_list));
    EXPECT_FALSE(is_floating_point(s_map));
    EXPECT_FALSE(is_floating_point(s_multimap));
    EXPECT_FALSE(is_floating_point(s_multiset));
    EXPECT_FALSE(is_floating_point(s_set));
    EXPECT_FALSE(is_floating_point(s_unordered_map));
    EXPECT_FALSE(is_floating_point(s_unordered_multimap));
    EXPECT_FALSE(is_floating_point(s_unordered_multiset));
    EXPECT_FALSE(is_floating_point(s_unordered_set));
    EXPECT_FALSE(is_floating_point(s_vector));

    EXPECT_FALSE(is_floating_point(1));
    EXPECT_FALSE(is_floating_point(-1));
    EXPECT_FALSE(is_floating_point("foo"));
    EXPECT_FALSE(is_floating_point(true));
}

//==============================================================================
TEST(JsonTraitsTest, Object)
{
    EXPECT_TRUE(is_object(s_map));
    EXPECT_TRUE(is_object(s_multimap));
    EXPECT_TRUE(is_object(s_unordered_map));
    EXPECT_TRUE(is_object(s_unordered_multimap));

    EXPECT_FALSE(is_object(s_array));
    EXPECT_FALSE(is_object(s_deque));
    EXPECT_FALSE(is_object(s_forward_list));
    EXPECT_FALSE(is_object(s_list));
    EXPECT_FALSE(is_object(s_multiset));
    EXPECT_FALSE(is_object(s_set));
    EXPECT_FALSE(is_object(s_unordered_multiset));
    EXPECT_FALSE(is_object(s_unordered_set));
    EXPECT_FALSE(is_object(s_vector));

    EXPECT_FALSE(is_object(1));
    EXPECT_FALSE(is_object(-1));
    EXPECT_FALSE(is_object("foo"));
    EXPECT_FALSE(is_object(3.14));
    EXPECT_FALSE(is_object(true));
}

//==============================================================================
TEST(JsonTraitsTest, Array)
{
    EXPECT_TRUE(is_array(s_array));
    EXPECT_TRUE(is_array(s_deque));
    EXPECT_TRUE(is_array(s_forward_list));
    EXPECT_TRUE(is_array(s_list));
    EXPECT_TRUE(is_array(s_multiset));
    EXPECT_TRUE(is_array(s_set));
    EXPECT_TRUE(is_array(s_unordered_multiset));
    EXPECT_TRUE(is_array(s_unordered_set));
    EXPECT_TRUE(is_array(s_vector));

    EXPECT_FALSE(is_array(s_map));
    EXPECT_FALSE(is_array(s_multimap));
    EXPECT_FALSE(is_array(s_unordered_map));
    EXPECT_FALSE(is_array(s_unordered_multimap));

    EXPECT_FALSE(is_array(1));
    EXPECT_FALSE(is_array(-1));
    EXPECT_FALSE(is_array("foo"));
    EXPECT_FALSE(is_array(3.14));
    EXPECT_FALSE(is_array(true));
}
