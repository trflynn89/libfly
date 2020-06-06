#include "fly/fly.hpp"
#include "fly/types/json/json.hpp"

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

template <typename T>
void bad_conversion(const fly::Json &json)
{
    SCOPED_TRACE(json);

    EXPECT_THROW(FLY_UNUSED(T(json)), fly::JsonException);
}

} // namespace

//==================================================================================================
TEST(JsonTest, StringConversion)
{
    fly::Json json;

    json = "abc";
    EXPECT_EQ(std::string(json), "abc");

    json = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(std::string(json), "{\"a\":1,\"b\":2}");

    json = {'7', 8};
    EXPECT_EQ(std::string(json), "[55,8]");

    json = true;
    EXPECT_EQ(std::string(json), "true");

    json = 1;
    EXPECT_EQ(std::string(json), "1");

    json = static_cast<unsigned int>(1);
    EXPECT_EQ(std::string(json), "1");

    json = 1.0f;
    EXPECT_EQ(std::string(json), "1");

    json = nullptr;
    EXPECT_EQ(std::string(json), "null");
}

//==================================================================================================
TEST(JsonTest, ObjectConversionValid)
{
    auto validate = [](auto *name, auto &test1, auto &test2, auto &test3) {
        SCOPED_TRACE(name);

        using T1 = std::decay_t<decltype(test1)>;
        using T2 = std::decay_t<decltype(test2)>;
        using T3 = std::decay_t<decltype(test3)>;

        test1 = T1 {{"a", 2}, {"b", 4}};
        test2 = T2 {{"a", "2"}, {"b", "4"}};
        test3 = T3 {{"a", 2}, {"b", "4"}};

        {
            fly::Json json = test1;
            EXPECT_EQ((T1(json)), test1);
            EXPECT_EQ((T2(json)), test2);
        }
        {
            fly::Json json = test2;
            EXPECT_EQ((T1(json)), test1);
            EXPECT_EQ((T2(json)), test2);
        }
        {
            fly::Json json = test3;
            EXPECT_EQ((T1(json)), test1);
            EXPECT_EQ((T2(json)), test2);
            EXPECT_EQ((T3(json)), test3);
        }
        {
            fly::Json json = {{"a", true}};
            bad_conversion<std::map<std::string, int>>(json);
        }
        {
            fly::Json json = {{"a", "string"}};
            bad_conversion<std::map<std::string, int>>(json);
        }
    };

    std::map<std::string, int> map1;
    std::map<std::string, std::string> map2;
    std::map<std::string, fly::Json> map3;
    validate("map", map1, map2, map3);

    std::multimap<std::string, int> multimap1;
    std::multimap<std::string, std::string> multimap2;
    std::multimap<std::string, fly::Json> multimap3;
    validate("multimap", multimap1, multimap2, multimap3);

    std::unordered_map<std::string, int> unordered_map1;
    std::unordered_map<std::string, std::string> unordered_map2;
    std::unordered_map<std::string, fly::Json> unordered_map3;
    validate("unordered_map", unordered_map1, unordered_map2, unordered_map3);

    std::unordered_multimap<std::string, int> unordered_multimap1;
    std::unordered_multimap<std::string, std::string> unordered_multimap2;
    std::unordered_multimap<std::string, fly::Json> unordered_multimap3;
    validate("unordered_multimap", unordered_multimap1, unordered_multimap2, unordered_multimap3);
}

//==================================================================================================
TEST(JsonTest, ObjectConversionInvalid)
{
    auto invalidate = [&](fly::Json json) {
        bad_conversion<std::map<std::string, fly::Json>>(json);
        bad_conversion<std::multimap<std::string, fly::Json>>(json);
        bad_conversion<std::unordered_map<std::string, fly::Json>>(json);
        bad_conversion<std::unordered_multimap<std::string, fly::Json>>(json);
    };

    invalidate("abc");
    invalidate({'7', 8});
    invalidate(true);
    invalidate(1);
    invalidate(static_cast<unsigned int>(1));
    invalidate(1.0f);
    invalidate(nullptr);
}

//==================================================================================================
TEST(JsonTest, ArrayConversionValid)
{
    auto validate2 = [](auto *name, auto &test1, auto &test2) {
        SCOPED_TRACE(name);

        using T1 = std::decay_t<decltype(test1)>;
        using T2 = std::decay_t<decltype(test2)>;

        test1 = T1 {50, 60, 70, 80};
        test2 = T2 {"50", "60", "70", "80"};

        {
            fly::Json json = test1;
            EXPECT_EQ((T1(json)), test1);
            EXPECT_EQ((T2(json)), test2);
        }
        {
            fly::Json json = test2;
            EXPECT_EQ((T1(json)), test1);
            EXPECT_EQ((T2(json)), test2);
        }
        {
            fly::Json json = {true};
            bad_conversion<std::array<int, 1>>(json);
        }
        {
            fly::Json json = {"string"};
            bad_conversion<std::array<int, 1>>(json);
        }
    };

    auto validate3 = [&](auto *name, auto &test1, auto &test2, auto &test3) {
        validate2(name, test1, test2);

        using T1 = std::decay_t<decltype(test1)>;
        using T2 = std::decay_t<decltype(test2)>;
        using T3 = std::decay_t<decltype(test3)>;

        test3 = T3 {50, "60", 70, "80"};

        fly::Json json = test3;
        EXPECT_EQ((T1(json)), test1);
        EXPECT_EQ((T2(json)), test2);
        EXPECT_EQ((T3(json)), test3);
    };

    std::array<int, 4> array1;
    std::array<std::string, 4> array2;
    std::array<fly::Json, 4> array3;
    validate3("array", array1, array2, array3);

    std::array<int, 1> array4 = {7};
    std::array<int, 2> array5 = {7, 8};
    std::array<int, 3> array6 = {7, 8, 0};
    fly::Json json = array5;
    EXPECT_EQ((decltype(array4)(json)), array4);
    EXPECT_EQ((decltype(array5)(json)), array5);
    EXPECT_EQ((decltype(array6)(json)), array6);

    std::deque<int> deque1;
    std::deque<std::string> deque2;
    std::deque<fly::Json> deque3;
    validate3("deque", deque1, deque2, deque3);

    std::forward_list<int> forward_list1;
    std::forward_list<std::string> forward_list2;
    std::forward_list<fly::Json> forward_list3;
    validate3("forward_list", forward_list1, forward_list2, forward_list3);

    std::list<int> list1;
    std::list<std::string> list2;
    std::list<fly::Json> list3;
    validate3("list", list1, list2, list3);

    std::multiset<int> multiset1;
    std::multiset<std::string> multiset2;
    // std::multiset<fly::Json> multiset3;
    validate2("multiset", multiset1, multiset2);

    std::set<int> set1;
    std::set<std::string> set2;
    // std::set<fly::Json> set3;
    validate2("set", set1, set2);

    std::unordered_multiset<int> unordered_multiset1;
    std::unordered_multiset<std::string> unordered_multiset2;
    // std::unordered_multiset<fly::Json> unordered_multiset3;
    validate2("unordered_multiset", unordered_multiset1, unordered_multiset2);

    std::unordered_set<int> unordered_set1;
    std::unordered_set<std::string> unordered_set2;
    // std::unordered_set<fly::Json> unordered_set3;
    validate2("unordered_set", unordered_set1, unordered_set2);

    std::vector<int> vector1;
    std::vector<std::string> vector2;
    std::vector<fly::Json> vector3;
    validate3("vector", vector1, vector2, vector3);
}

//==================================================================================================
TEST(JsonTest, ArrayConversionInvalid)
{
    auto invalidate = [&](fly::Json json) {
        bad_conversion<std::array<int, 1>>(json);
        bad_conversion<std::deque<int>>(json);
        bad_conversion<std::forward_list<int>>(json);
        bad_conversion<std::list<int>>(json);
        bad_conversion<std::multiset<int>>(json);
        bad_conversion<std::set<int>>(json);
        bad_conversion<std::unordered_multiset<int>>(json);
        bad_conversion<std::unordered_set<int>>(json);
        bad_conversion<std::vector<int>>(json);
    };

    invalidate("abc");
    invalidate({{"a", 1}, {"b", 2}});
    invalidate(true);
    invalidate(1);
    invalidate(static_cast<unsigned int>(1));
    invalidate(1.0f);
    invalidate(nullptr);
}

//==================================================================================================
TEST(JsonTest, BooleanConversion)
{
    fly::Json json;

    json = "";
    EXPECT_FALSE(bool(json));
    json = "abc";
    EXPECT_TRUE(bool(json));

    json = std::map<std::string, int>();
    EXPECT_FALSE(bool(json));
    json = {{"a", 1}, {"b", 2}};
    EXPECT_TRUE(bool(json));

    json = std::vector<int>();
    EXPECT_FALSE(bool(json));
    json = {7, 8};
    EXPECT_TRUE(bool(json));

    json = true;
    EXPECT_TRUE(bool(json));
    json = false;
    EXPECT_FALSE(bool(json));

    json = 1;
    EXPECT_TRUE(bool(json));
    json = 0;
    EXPECT_FALSE(bool(json));

    json = static_cast<unsigned int>(1);
    EXPECT_TRUE(bool(json));
    json = static_cast<unsigned int>(0);
    EXPECT_FALSE(bool(json));

    json = 1.0f;
    EXPECT_TRUE(bool(json));
    json = 0.0f;
    EXPECT_FALSE(bool(json));

    json = nullptr;
    EXPECT_FALSE(bool(json));
}

//==================================================================================================
TEST(JsonTest, SignedIntegerConversion)
{
    fly::Json json;

    json = "abc";
    bad_conversion<int>(json);

    json = "123";
    EXPECT_EQ(int(json), 123);

    json = {{"a", 1}, {"b", 2}};
    bad_conversion<int>(json);

    json = {7, 8};
    bad_conversion<int>(json);

    json = true;
    bad_conversion<int>(json);

    char ch = 'a';
    json = ch;
    EXPECT_EQ(char(json), ch);

    int sign = 12;
    json = sign;
    EXPECT_EQ(int(json), sign);

    unsigned int unsign = static_cast<unsigned int>(12);
    json = unsign;
    EXPECT_EQ(int(json), int(unsign));

    float floating = 3.14f;
    json = floating;
    EXPECT_EQ(int(json), int(floating));

    json = nullptr;
    bad_conversion<int>(json);
}

//==================================================================================================
TEST(JsonTest, UnsignedIntegerConversion)
{
    fly::Json json;

    json = "abc";
    bad_conversion<unsigned>(json);

    json = "123";
    EXPECT_EQ(unsigned(json), unsigned(123));

    json = {{"a", 1}, {"b", 2}};
    bad_conversion<unsigned>(json);

    json = {7, 8};
    bad_conversion<unsigned>(json);

    json = true;
    bad_conversion<unsigned>(json);

    char ch = 'a';
    json = ch;
    EXPECT_EQ(static_cast<unsigned char>(json), static_cast<unsigned char>(ch));

    int sign = 12;
    json = sign;
    EXPECT_EQ(unsigned(json), unsigned(sign));

    unsigned int unsign = static_cast<unsigned int>(12);
    json = unsign;
    EXPECT_EQ(unsigned(json), unsign);

    float floating = 3.14f;
    json = floating;
    EXPECT_EQ(unsigned(json), unsigned(floating));

    json = nullptr;
    bad_conversion<unsigned>(json);
}

//==================================================================================================
TEST(JsonTest, FloatConversion)
{
    fly::Json json;

    json = "abc";
    bad_conversion<float>(json);

    json = "123.5";
    EXPECT_EQ(float(json), 123.5f);

    json = {{"a", 1}, {"b", 2}};
    bad_conversion<float>(json);

    json = {7, 8};
    bad_conversion<float>(json);

    json = true;
    bad_conversion<float>(json);

    char ch = 'a';
    json = ch;
    EXPECT_EQ(float(json), float(ch));

    int sign = 12;
    json = sign;
    EXPECT_EQ(float(json), float(sign));

    unsigned int unsign = static_cast<unsigned int>(12);
    json = unsign;
    EXPECT_EQ(float(json), float(unsign));

    float floating = 3.14f;
    json = floating;
    EXPECT_EQ(float(json), floating);

    json = nullptr;
    bad_conversion<float>(json);
}

//==================================================================================================
TEST(JsonTest, NullConversion)
{
    fly::Json json;

    json = "abc";
    bad_conversion<std::nullptr_t>(json);

    json = {{"a", 1}, {"b", 2}};
    bad_conversion<std::nullptr_t>(json);

    json = {7, 8};
    bad_conversion<std::nullptr_t>(json);

    json = true;
    bad_conversion<std::nullptr_t>(json);

    json = 'a';
    bad_conversion<std::nullptr_t>(json);

    json = 12;
    bad_conversion<std::nullptr_t>(json);

    json = static_cast<unsigned int>(12);
    bad_conversion<std::nullptr_t>(json);

    json = 3.14f;
    bad_conversion<std::nullptr_t>(json);

    json = nullptr;
    EXPECT_EQ((std::nullptr_t(json)), nullptr);
}
