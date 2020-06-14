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

//==================================================================================================
TEST(JsonTest, StringConstructor)
{
    const std::string str1("a");
    EXPECT_TRUE(fly::Json(str1).is_string());

    std::string str2("b");
    EXPECT_TRUE(fly::Json(str2).is_string());

    const char *cstr1 = "c";
    EXPECT_TRUE(fly::Json(cstr1).is_string());

    char *cstr2 = const_cast<char *>("d");
    EXPECT_TRUE(fly::Json(cstr2).is_string());

    const char arr1[] = {'g', '\0'};
    EXPECT_TRUE(fly::Json(arr1).is_string());

    char arr2[] = {'h', '\0'};
    EXPECT_TRUE(fly::Json(arr2).is_string());
}

//==================================================================================================
TEST(JsonTest, InvalidString)
{
    // Reverse solidus must be followed by a valid escape symbol.
    EXPECT_THROW(fly::Json("\\"), fly::JsonException);
    EXPECT_THROW(fly::Json("\\U"), fly::JsonException);

    // Quotes must be escaped.
    EXPECT_THROW(fly::Json("\""), fly::JsonException);

    // Control characters must be escaped.
    for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
    {
        EXPECT_THROW(fly::Json(fly::JsonTraits::string_type(1, ch)), fly::JsonException);
    }

    // Characters must be valid Unicode.
    EXPECT_THROW(fly::Json("\xed\xa0\x80"), fly::JsonException); // Reserved codepoint.
    EXPECT_THROW(fly::Json("\xf4\x90\x80\x80"), fly::JsonException); // Out-of-range codepoint.
}

//==================================================================================================
TEST(JsonTest, ObjectConstructor)
{
    std::map<std::string, int> map = {{"a", 1}, {"b", 2}};
    EXPECT_TRUE(fly::Json(map).is_object());

    std::multimap<std::string, int> multimap = {{"c", 3}, {"d", 4}};
    EXPECT_TRUE(fly::Json(multimap).is_object());

    std::unordered_map<std::string, int> umap = {{"e", 5}, {"f", 6}};
    EXPECT_TRUE(fly::Json(umap).is_object());

    std::unordered_multimap<std::string, int> umultimap = {{"h", 7}, {"i", 8}};
    EXPECT_TRUE(fly::Json(umultimap).is_object());
}

//==================================================================================================
TEST(JsonTest, InvalidObjectKey)
{
    std::map<std::string, int> map;

    // Reverse solidus must be followed by a valid escape symbol.
    map = {{"\\", 1}};
    EXPECT_THROW((fly::Json(map)), fly::JsonException);
    map = {{"\\U", 1}};
    EXPECT_THROW((fly::Json(map)), fly::JsonException);

    // Quotes must be escaped.
    map = {{"\"", 1}};
    EXPECT_THROW((fly::Json(map)), fly::JsonException);

    // Control characters must be escaped.
    for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
    {
        map = {{fly::JsonTraits::string_type(1, ch), 1}};
        EXPECT_THROW((fly::Json(map)), fly::JsonException);
    }

    // Characters must be valid Unicode.
    map = {{"\xed\xa0\x80", 1}}; // Reserved codepoint.
    EXPECT_THROW((fly::Json(map)), fly::JsonException);
    map = {{"\xf4\x90\x80\x80", 1}}; // Out-of-range codepoint.
    EXPECT_THROW((fly::Json(map)), fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, ArrayConstructor)
{
    std::array<int, 4> array = {10, 20, 30, 40};
    EXPECT_TRUE(fly::Json(array).is_array());
    EXPECT_FALSE(fly::Json(array).is_object_like());

    std::deque<int> deque = {50, 60, 70, 80};
    EXPECT_TRUE(fly::Json(deque).is_array());
    EXPECT_FALSE(fly::Json(deque).is_object_like());

    std::forward_list<int> forward_list = {90, 100, 110, 120};
    EXPECT_TRUE(fly::Json(forward_list).is_array());
    EXPECT_FALSE(fly::Json(forward_list).is_object_like());

    std::list<int> list = {130, 140, 150, 160};
    EXPECT_TRUE(fly::Json(list).is_array());
    EXPECT_FALSE(fly::Json(list).is_object_like());

    std::multiset<std::string> multiset = {"a", "b", "c"};
    EXPECT_TRUE(fly::Json(multiset).is_array());
    EXPECT_FALSE(fly::Json(multiset).is_object_like());

    std::set<std::string> set = {"d", "e", "f"};
    EXPECT_TRUE(fly::Json(set).is_array());
    EXPECT_FALSE(fly::Json(set).is_object_like());

    std::unordered_multiset<std::string> unordered_multiset = {"g", "h", "i"};
    EXPECT_TRUE(fly::Json(unordered_multiset).is_array());
    EXPECT_FALSE(fly::Json(unordered_multiset).is_object_like());

    std::unordered_set<std::string> unordered_set = {"j", "k", "l"};
    EXPECT_TRUE(fly::Json(unordered_set).is_array());
    EXPECT_FALSE(fly::Json(unordered_set).is_object_like());

    std::vector<int> vector = {170, 180, 190, 200};
    EXPECT_TRUE(fly::Json(vector).is_array());
    EXPECT_FALSE(fly::Json(vector).is_object_like());

    std::array<std::string, 2> object = {"nine", "ten"};
    EXPECT_TRUE(fly::Json(object).is_array());
    EXPECT_TRUE(fly::Json(object).is_object_like());
}

//==================================================================================================
TEST(JsonTest, InvalidArrayString)
{
    std::vector<std::string> vector;

    // Reverse solidus must be followed by a valid escape symbol.
    vector = {"\\"};
    EXPECT_THROW((fly::Json(vector)), fly::JsonException);
    vector = {"\\U"};
    EXPECT_THROW((fly::Json(vector)), fly::JsonException);

    // Quotes must be escaped.
    vector = {"\""};
    EXPECT_THROW((fly::Json(vector)), fly::JsonException);

    // Control characters must be escaped.
    for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
    {
        vector = {fly::JsonTraits::string_type(1, ch)};
        EXPECT_THROW((fly::Json(vector)), fly::JsonException);
    }

    // Characters must be valid Unicode.
    vector = {"\xed\xa0\x80"}; // Reserved codepoint.
    EXPECT_THROW((fly::Json(vector)), fly::JsonException);
    vector = {"\xf4\x90\x80\x80"}; // Out-of-range codepoint.
    EXPECT_THROW((fly::Json(vector)), fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, BooleanConstructor)
{
    EXPECT_TRUE(fly::Json(true).is_boolean());
    EXPECT_TRUE(fly::Json(false).is_boolean());
}

//==================================================================================================
TEST(JsonTest, SignedIntegerConstructor)
{
    EXPECT_TRUE(fly::Json(static_cast<char>(1)).is_signed_integer());

    EXPECT_TRUE(fly::Json(static_cast<short>(1)).is_signed_integer());

    EXPECT_TRUE(fly::Json(static_cast<int>(1)).is_signed_integer());
    EXPECT_TRUE(fly::Json(static_cast<int>(-1)).is_signed_integer());

    EXPECT_TRUE(fly::Json(static_cast<std::int32_t>(1)).is_signed_integer());
    EXPECT_TRUE(fly::Json(static_cast<std::int32_t>(-1)).is_signed_integer());

    EXPECT_TRUE(fly::Json(static_cast<std::int64_t>(1)).is_signed_integer());
    EXPECT_TRUE(fly::Json(static_cast<std::int64_t>(-1)).is_signed_integer());
}

//==================================================================================================
TEST(JsonTest, UnsignedIntegerConstructor)
{
    EXPECT_TRUE(fly::Json(static_cast<unsigned char>(1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<unsigned short>(1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(1)).is_unsigned_integer());
    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(-1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(1)).is_unsigned_integer());
    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(-1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(1)).is_unsigned_integer());
    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(-1)).is_unsigned_integer());
}

//==================================================================================================
TEST(JsonTest, FloatConstructor)
{
    EXPECT_TRUE(fly::Json(static_cast<float>(1.0)).is_float());
    EXPECT_TRUE(fly::Json(static_cast<double>(1.0)).is_float());
    EXPECT_TRUE(fly::Json(static_cast<long double>(1.0)).is_float());
}

//==================================================================================================
TEST(JsonTest, NullConstructor)
{
    EXPECT_TRUE(fly::Json().is_null());
    EXPECT_TRUE(fly::Json(nullptr).is_null());
}

//==================================================================================================
TEST(JsonTest, InitializerListConstructor)
{
    const fly::Json empty = {};
    EXPECT_TRUE(fly::Json(empty).is_null());

    const fly::Json array = {'7', 8, "nine", 10};
    EXPECT_TRUE(fly::Json(array).is_array());

    const fly::Json object = {{"a", 1}, {"b", 2}};
    EXPECT_TRUE(fly::Json(object).is_object());

    const fly::Json almost = {{"a", 1}, {"b", 2}, 4};
    EXPECT_TRUE(fly::Json(almost).is_array());
}

//==================================================================================================
TEST(JsonTest, CopyConstructor)
{
    fly::Json string = "abc";
    EXPECT_EQ(fly::Json(string), string);

    fly::Json object = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(fly::Json(object), object);

    fly::Json array = {'7', 8};
    EXPECT_EQ(fly::Json(array), array);

    fly::Json boolean = true;
    EXPECT_EQ(fly::Json(boolean), boolean);

    fly::Json sign = 1;
    EXPECT_EQ(fly::Json(sign), sign);

    fly::Json unsign = static_cast<unsigned int>(1);
    EXPECT_EQ(fly::Json(unsign), unsign);

    fly::Json floating = 1.0f;
    EXPECT_EQ(fly::Json(floating), floating);

    fly::Json null = nullptr;
    EXPECT_EQ(fly::Json(null), null);
}

//==================================================================================================
TEST(JsonTest, MoveConstructor)
{
    fly::Json string = "abc";
    fly::Json string_copy(string);
    fly::Json string_move(std::move(string_copy));

    EXPECT_TRUE(string_copy.is_null());
    EXPECT_EQ(string_move, string);
}

//==================================================================================================
TEST(JsonTest, Assignment)
{
    fly::Json json;

    fly::Json string = "abc";
    json = string;
    EXPECT_EQ(json, string);

    fly::Json object = {{"a", 1}, {"b", 2}};
    json = object;
    EXPECT_EQ(json, object);

    fly::Json array = {'7', 8};
    json = array;
    EXPECT_EQ(json, array);

    fly::Json boolean = true;
    json = boolean;
    EXPECT_EQ(json, boolean);

    fly::Json sign = 1;
    json = sign;
    EXPECT_EQ(json, sign);

    fly::Json unsign = static_cast<unsigned int>(1);
    json = unsign;
    EXPECT_EQ(json, unsign);

    fly::Json floating = 1.0f;
    json = floating;
    EXPECT_EQ(json, floating);

    fly::Json null = nullptr;
    json = null;
    EXPECT_EQ(json, null);
}

//==================================================================================================
TEST(JsonTest, ObjectAccess)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1["a"], fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2["a"], fly::JsonException);

    fly::Json object1 = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(object1["a"], 1);
    EXPECT_EQ(object1["b"], 2);
    EXPECT_NO_THROW(object1["c"]);
    EXPECT_EQ(object1["c"], nullptr);

    const fly::Json object2 = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(object2["a"], 1);
    EXPECT_EQ(object2["b"], 2);
    EXPECT_THROW(object2["c"], fly::JsonException);

    fly::Json array1 = {'7', 8};
    EXPECT_THROW(array1["a"], fly::JsonException);

    const fly::Json array2 = {'7', 8};
    EXPECT_THROW(array2["a"], fly::JsonException);

    fly::Json bool1 = true;
    EXPECT_THROW(bool1["a"], fly::JsonException);

    const fly::Json bool2 = true;
    EXPECT_THROW(bool2["a"], fly::JsonException);

    fly::Json signed1 = 1;
    EXPECT_THROW(signed1["a"], fly::JsonException);

    const fly::Json signed2 = 1;
    EXPECT_THROW(signed2["a"], fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned1["a"], fly::JsonException);

    const fly::Json unsigned2 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned2["a"], fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1["a"], fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2["a"], fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_NO_THROW(null1["a"]);
    EXPECT_TRUE(null1.is_object());
    EXPECT_EQ(null1["a"], nullptr);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2["a"], fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, ObjectAt)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1.at("a"), fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2.at("a"), fly::JsonException);

    fly::Json object1 = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(object1.at("a"), 1);
    EXPECT_EQ(object1.at("b"), 2);
    EXPECT_THROW(object1.at("c"), fly::JsonException);

    const fly::Json object2 = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(object2.at("a"), 1);
    EXPECT_EQ(object2.at("b"), 2);
    EXPECT_THROW(object2.at("c"), fly::JsonException);

    fly::Json array1 = {'7', 8};
    EXPECT_THROW(array1.at("a"), fly::JsonException);

    const fly::Json array2 = {'7', 8};
    EXPECT_THROW(array2.at("a"), fly::JsonException);

    fly::Json bool1 = true;
    EXPECT_THROW(bool1.at("a"), fly::JsonException);

    const fly::Json bool2 = true;
    EXPECT_THROW(bool2.at("a"), fly::JsonException);

    fly::Json signed1 = 1;
    EXPECT_THROW(signed1.at("a"), fly::JsonException);

    const fly::Json signed2 = 1;
    EXPECT_THROW(signed2.at("a"), fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned1.at("a"), fly::JsonException);

    const fly::Json unsigned2 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned2.at("a"), fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1.at("a"), fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2.at("a"), fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_THROW(null1.at("a"), fly::JsonException);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2.at("a"), fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, ArrayAccess)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1[0], fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2[0], fly::JsonException);

    fly::Json object1 = {{"a", 1}, {"b", 2}};
    EXPECT_THROW(object1[0], fly::JsonException);

    const fly::Json object2 = {{"a", 1}, {"b", 2}};
    EXPECT_THROW(object2[0], fly::JsonException);

    fly::Json array1 = {'7', 8};
    EXPECT_EQ(array1[0], '7');
    EXPECT_EQ(array1[1], 8);
    EXPECT_NO_THROW(array1[2]);
    EXPECT_EQ(array1[2], nullptr);

    const fly::Json array2 = {'7', 8};
    EXPECT_EQ(array2[0], '7');
    EXPECT_EQ(array2[1], 8);
    EXPECT_THROW(array2[2], fly::JsonException);

    fly::Json bool1 = true;
    EXPECT_THROW(bool1[0], fly::JsonException);

    const fly::Json bool2 = true;
    EXPECT_THROW(bool2[0], fly::JsonException);

    fly::Json signed1 = 1;
    EXPECT_THROW(signed1[0], fly::JsonException);

    const fly::Json signed2 = 1;
    EXPECT_THROW(signed2[0], fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned1[0], fly::JsonException);

    const fly::Json unsigned2 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned2[0], fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1[0], fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2[0], fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_NO_THROW(null1[0]);
    EXPECT_TRUE(null1.is_array());
    EXPECT_EQ(null1[0], nullptr);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2[0], fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, ArrayAt)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1.at(0), fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2.at(0), fly::JsonException);

    fly::Json object1 = {{"a", 1}, {"b", 2}};
    EXPECT_THROW(object1.at(0), fly::JsonException);

    const fly::Json object2 = {{"a", 1}, {"b", 2}};
    EXPECT_THROW(object2.at(0), fly::JsonException);

    fly::Json array1 = {'7', 8};
    EXPECT_EQ(array1.at(0), '7');
    EXPECT_EQ(array1.at(1), 8);
    EXPECT_THROW(array1.at(2), fly::JsonException);

    const fly::Json array2 = {'7', 8};
    EXPECT_EQ(array2.at(0), '7');
    EXPECT_EQ(array2.at(1), 8);
    EXPECT_THROW(array2.at(2), fly::JsonException);

    fly::Json bool1 = true;
    EXPECT_THROW(bool1.at(0), fly::JsonException);

    const fly::Json bool2 = true;
    EXPECT_THROW(bool2.at(0), fly::JsonException);

    fly::Json signed1 = 1;
    EXPECT_THROW(signed1.at(0), fly::JsonException);

    const fly::Json signed2 = 1;
    EXPECT_THROW(signed2.at(0), fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned1.at(0), fly::JsonException);

    const fly::Json unsigned2 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned2.at(0), fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1.at(0), fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2.at(0), fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_THROW(null1.at(0), fly::JsonException);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2.at(0), fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, Empty)
{
    fly::Json json;

    json = "abcdef";
    EXPECT_FALSE(json.empty());

    json = {{"a", 1}, {"b", 2}};
    EXPECT_FALSE(json.empty());

    json = {'7', 8, 9, 10};
    EXPECT_FALSE(json.empty());

    json = true;
    EXPECT_FALSE(json.empty());

    json = 1;
    EXPECT_FALSE(json.empty());

    json = static_cast<unsigned int>(1);
    EXPECT_FALSE(json.empty());

    json = 1.0f;
    EXPECT_FALSE(json.empty());

    json = nullptr;
    EXPECT_TRUE(json.empty());

    json = "";
    EXPECT_TRUE(json.empty());

    json = fly::JsonTraits::object_type();
    EXPECT_TRUE(json.empty());

    json = fly::JsonTraits::array_type();
    EXPECT_TRUE(json.empty());
}

//==================================================================================================
TEST(JsonTest, Size)
{
    fly::Json json;

    json = "abcdef";
    EXPECT_EQ(json.size(), 6);

    json = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(json.size(), 2);

    json = {'7', 8, 9, 10};
    EXPECT_EQ(json.size(), 4);

    json = true;
    EXPECT_EQ(json.size(), 1);

    json = 1;
    EXPECT_EQ(json.size(), 1);

    json = static_cast<unsigned int>(1);
    EXPECT_EQ(json.size(), 1);

    json = 1.0f;
    EXPECT_EQ(json.size(), 1);

    json = nullptr;
    EXPECT_EQ(json.size(), 0);
}

//==================================================================================================
TEST(JsonTest, Clear)
{
    fly::Json json;

    json = "abcdef";
    EXPECT_EQ(json.size(), 6);
    json.clear();
    EXPECT_TRUE(json.empty());

    json = {{"a", 1}, {"b", 2}};
    EXPECT_EQ(json.size(), 2);
    json.clear();
    EXPECT_TRUE(json.empty());

    json = {'7', 8, 9, 10};
    EXPECT_EQ(json.size(), 4);
    json.clear();
    EXPECT_TRUE(json.empty());

    json = true;
    EXPECT_TRUE(json);
    json.clear();
    EXPECT_FALSE(json);

    json = 1;
    EXPECT_EQ(json, 1);
    json.clear();
    EXPECT_EQ(json, 0);

    json = static_cast<unsigned int>(1);
    EXPECT_EQ(json, 1);
    json.clear();
    EXPECT_EQ(json, 0);

    json = 1.0f;
    EXPECT_DOUBLE_EQ(double(json), 1.0);
    json.clear();
    EXPECT_DOUBLE_EQ(double(json), 0.0);

    json = nullptr;
    EXPECT_EQ(json, nullptr);
    json.clear();
    EXPECT_EQ(json, nullptr);
}

//==================================================================================================
TEST(JsonTest, JsonSwap)
{
    fly::Json json1 = 12389;
    fly::Json json2 = "string";
    fly::Json json3 = {1, 2, 3, 8, 9};

    json1.swap(json2);
    EXPECT_EQ(json1, "string");
    EXPECT_EQ(json2, 12389);

    json2.swap(json3);
    EXPECT_EQ(json2, fly::Json({1, 2, 3, 8, 9}));
    EXPECT_EQ(json3, 12389);

    json3.swap(json1);
    EXPECT_EQ(json1, 12389);
    EXPECT_EQ(json3, "string");
}

//==================================================================================================
TEST(JsonTest, StringSwap)
{
    fly::Json json;
    std::string str;

    json = "abcdef";
    str = "ghijkl";
    EXPECT_NO_THROW(json.swap(str));
    EXPECT_EQ(json, "ghijkl");
    EXPECT_EQ(str, "abcdef");

    json = {{"a", 1}, {"b", 2}};
    EXPECT_THROW(json.swap(str), fly::JsonException);

    json = {'7', 8, 9, 10};
    EXPECT_THROW(json.swap(str), fly::JsonException);

    json = true;
    EXPECT_THROW(json.swap(str), fly::JsonException);

    json = 1;
    EXPECT_THROW(json.swap(str), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW(json.swap(str), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW(json.swap(str), fly::JsonException);

    json = nullptr;
    EXPECT_THROW(json.swap(str), fly::JsonException);
}

//==================================================================================================
TEST(JsonTest, ObjectSwapValid)
{
    auto validate = [](auto *name, auto &test1, auto &test2, auto &test3) {
        SCOPED_TRACE(name);

        using T1 = std::decay_t<decltype(test1)>;
        using T2 = std::decay_t<decltype(test2)>;
        using T3 = std::decay_t<decltype(test3)>;

        test1 = T1 {{"a", 2}, {"b", 4}};
        test2 = T2 {{"a", "2"}, {"b", "4"}};
        test3 = T3 {{"a", 5}, {"b", "6"}};

        {
            fly::Json json = {{"c", 100}, {"d", 200}};
            EXPECT_NO_THROW(json.swap(test1));
            EXPECT_EQ(json, (T1({{"a", 2}, {"b", 4}})));
            EXPECT_EQ(test1, (T1({{"c", 100}, {"d", 200}})));
        }
        {
            fly::Json json = {{"c", 100}, {"d", 200}};
            EXPECT_NO_THROW(json.swap(test2));
            EXPECT_EQ(json, (T2({{"a", "2"}, {"b", "4"}})));
            EXPECT_EQ(test2, (T2({{"c", "100"}, {"d", "200"}})));
        }
        {
            fly::Json json = {{"c", nullptr}, {"d", true}};
            EXPECT_NO_THROW(json.swap(test3));
            EXPECT_EQ(json, (T3({{"a", 5}, {"b", "6"}})));
            EXPECT_EQ(test3, (T3({{"c", nullptr}, {"d", true}})));
        }
        {
            fly::Json json = {{"c", 100}, {"d", "200"}};
            EXPECT_NO_THROW(json.swap(test1));
            EXPECT_EQ(json, (T1({{"c", 100}, {"d", 200}})));
            EXPECT_EQ(test1, (T1({{"c", 100}, {"d", 200}})));
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
TEST(JsonTest, ObjectSwapInvalid)
{
    std::map<std::string, fly::Json> map;
    std::multimap<std::string, fly::Json> multimap;
    std::unordered_map<std::string, fly::Json> unordered_map;
    std::unordered_multimap<std::string, fly::Json> unordered_multimap;

    auto invalidate = [&](fly::Json json) {
        EXPECT_THROW(json.swap(map), fly::JsonException);
        EXPECT_THROW(json.swap(multimap), fly::JsonException);
        EXPECT_THROW(json.swap(unordered_map), fly::JsonException);
        EXPECT_THROW(json.swap(unordered_multimap), fly::JsonException);
    };

    invalidate("abcdef");
    invalidate({'7', 8, 9, 10});
    invalidate(true);
    invalidate(1);
    invalidate(static_cast<unsigned int>(1));
    invalidate(1.0f);
    invalidate(nullptr);
}

//==================================================================================================
TEST(JsonTest, ArraySwapValid)
{
    auto validate2 = [](auto *name, auto &test1, auto &test2) {
        SCOPED_TRACE(name);

        using T1 = std::decay_t<decltype(test1)>;
        using T2 = std::decay_t<decltype(test2)>;

        test1 = T1 {50, 60, 70, 80};
        test2 = T2 {"50", "60", "70", "80"};

        {
            fly::Json json = {1, 2};
            EXPECT_NO_THROW(json.swap(test1));
            EXPECT_EQ(json, (T1({50, 60, 70, 80})));
            EXPECT_EQ(test1, (T1({1, 2})));
        }
        {
            fly::Json json = {1, 2};
            EXPECT_NO_THROW(json.swap(test2));
            EXPECT_EQ(json, (T2({"50", "60", "70", "80"})));
            EXPECT_EQ(test2, (T2({"1", "2"})));
        }
        {
            fly::Json json = {50, "60", 70, "80"};
            EXPECT_NO_THROW(json.swap(test1));
            EXPECT_EQ(json, (T1({1, 2})));
            EXPECT_EQ(test1, (T1({50, 60, 70, 80})));
        }
    };

    auto validate3 = [&](auto *name, auto &test1, auto &test2, auto &test3) {
        validate2(name, test1, test2);

        using T3 = std::decay_t<decltype(test3)>;
        test3 = T3 {"a", 90, "b", 100};

        fly::Json json = {nullptr, true};
        EXPECT_NO_THROW(json.swap(test3));
        EXPECT_EQ(json, (T3({"a", 90, "b", 100})));
        EXPECT_EQ(test3, (T3({nullptr, true})));
    };

    std::array<int, 4> array1;
    std::array<std::string, 4> array2;
    std::array<fly::Json, 4> array3;
    validate3("array", array1, array2, array3);

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
TEST(JsonTest, ArraySwapInvalid)
{
    std::array<int, 4> array;
    std::deque<int> deque;
    std::forward_list<int> forward_list;
    std::list<int> list;
    std::multiset<int> multiset;
    std::set<int> set;
    std::unordered_multiset<int> unordered_multiset;
    std::unordered_set<int> unordered_set;
    std::vector<int> vector;

    auto invalidate = [&](fly::Json json) {
        EXPECT_THROW(json.swap(array), fly::JsonException);
        EXPECT_THROW(json.swap(deque), fly::JsonException);
        EXPECT_THROW(json.swap(forward_list), fly::JsonException);
        EXPECT_THROW(json.swap(list), fly::JsonException);
        EXPECT_THROW(json.swap(multiset), fly::JsonException);
        EXPECT_THROW(json.swap(set), fly::JsonException);
        EXPECT_THROW(json.swap(unordered_multiset), fly::JsonException);
        EXPECT_THROW(json.swap(unordered_set), fly::JsonException);
        EXPECT_THROW(json.swap(vector), fly::JsonException);
    };

    invalidate("abcdef");
    invalidate({{"a", 1}, {"b", 2}});
    invalidate(true);
    invalidate(1);
    invalidate(static_cast<unsigned int>(1));
    invalidate(1.0f);
    invalidate(nullptr);
}

//==================================================================================================
TEST(JsonTest, IteratorBegin)
{
    fly::Json json1 {1, 2, 3};
    const fly::Json json2 {4, 5, 6};

    auto begin1 = json1.begin();
    EXPECT_EQ(*begin1, 1);
    EXPECT_FALSE(std::is_const_v<decltype(begin1)::value_type>);

    auto cbegin1 = json1.cbegin();
    EXPECT_EQ(*cbegin1, 1);
    EXPECT_TRUE(std::is_const_v<decltype(cbegin1)::value_type>);

    auto begin2 = json2.begin();
    EXPECT_EQ(*begin2, 4);
    EXPECT_TRUE(std::is_const_v<decltype(begin2)::value_type>);

    auto cbegin2 = json2.cbegin();
    EXPECT_EQ(*cbegin2, 4);
    EXPECT_EQ(begin2, cbegin2);
    EXPECT_TRUE(std::is_const_v<decltype(cbegin2)::value_type>);
}

//==================================================================================================
TEST(JsonTest, IteratorEnd)
{
    fly::Json json1 {1, 2, 3};
    const fly::Json json2 {4, 5, 6};

    auto end1 = json1.end();
    EXPECT_EQ(*(end1 - 1), 3);
    EXPECT_FALSE(std::is_const_v<decltype(end1)::value_type>);

    auto cend1 = json1.cend();
    EXPECT_EQ(*(cend1 - 1), 3);
    EXPECT_TRUE(std::is_const_v<decltype(cend1)::value_type>);

    auto end2 = json2.end();
    EXPECT_EQ(*(end2 - 1), 6);
    EXPECT_TRUE(std::is_const_v<decltype(end2)::value_type>);

    auto cend2 = json2.cend();
    EXPECT_EQ(*(cend2 - 1), 6);
    EXPECT_EQ(end2, cend2);
    EXPECT_TRUE(std::is_const_v<decltype(cend2)::value_type>);
}

//==================================================================================================
TEST(JsonTest, ObjectIteratorIterate)
{
    fly::Json json {{"a", 1}, {"b", 2}};
    {
        fly::Json::size_type size = 0;

        for (auto it = json.begin(); it != json.end(); ++it, ++size)
        {
            EXPECT_EQ(*it, size == 0 ? 1 : 2);
            EXPECT_EQ(it.key(), size == 0 ? "a" : "b");
            EXPECT_EQ(it.value(), size == 0 ? 1 : 2);
        }

        EXPECT_EQ(size, json.size());
    }
    {
        fly::Json::size_type size = 0;

        for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
        {
            EXPECT_EQ(*it, size == 0 ? 1 : 2);
            EXPECT_EQ(it.key(), size == 0 ? "a" : "b");
            EXPECT_EQ(it.value(), size == 0 ? 1 : 2);
        }

        EXPECT_EQ(size, json.size());
    }
}

//==================================================================================================
TEST(JsonTest, ObjectIteratorRangeBasedFor)
{
    fly::Json json {{"a", 1}, {"b", 2}};
    {
        fly::Json::size_type size = 0;

        for (fly::Json &value : json)
        {
            EXPECT_EQ(value, size++ == 0 ? 1 : 2);
        }

        EXPECT_EQ(size, json.size());
    }
    {
        fly::Json::size_type size = 0;

        for (const fly::Json &value : json)
        {
            EXPECT_EQ(value, size++ == 0 ? 1 : 2);
        }

        EXPECT_EQ(size, json.size());
    }
}

//==================================================================================================
TEST(JsonTest, ArrayIteratorIterate)
{
    fly::Json json {1, 2, 3};
    {
        fly::Json::size_type size = 0;

        for (auto it = json.begin(); it != json.end(); ++it, ++size)
        {
            EXPECT_EQ(*it, json[size]);
            EXPECT_EQ(it.value(), json[size]);
        }

        EXPECT_EQ(size, json.size());
    }
    {
        fly::Json::size_type size = 0;

        for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
        {
            EXPECT_EQ(*it, json[size]);
            EXPECT_EQ(it.value(), json[size]);
        }

        EXPECT_EQ(size, json.size());
    }
}

//==================================================================================================
TEST(JsonTest, ArrayIteratorRangeBasedFor)
{
    fly::Json json {1, 2, 3};
    {
        fly::Json::size_type size = 0;

        for (fly::Json &value : json)
        {
            EXPECT_EQ(value, json[size++]);
        }

        EXPECT_EQ(size, json.size());
    }
    {
        fly::Json::size_type size = 0;

        for (const fly::Json &value : json)
        {
            EXPECT_EQ(value, json[size++]);
        }

        EXPECT_EQ(size, json.size());
    }
}

//==================================================================================================
TEST(JsonTest, Equality)
{
    fly::Json string1 = "abc";
    fly::Json string2 = "abc";
    fly::Json string3 = "def";

    fly::Json object1 = {{"a", 1}, {"b", 2}};
    fly::Json object2 = {{"a", 1}, {"b", 2}};
    fly::Json object3 = {{"a", 1}, {"b", 3}};

    fly::Json array1 = {'7', 8};
    fly::Json array2 = {'7', 8};
    fly::Json array3 = {'7', 9};

    fly::Json bool1 = true;
    fly::Json bool2 = true;
    fly::Json bool3 = false;

    fly::Json signed1 = 1;
    fly::Json signed2 = 1;
    fly::Json signed3 = 0;

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    fly::Json unsigned2 = static_cast<unsigned int>(1);
    fly::Json unsigned3 = static_cast<unsigned int>(0);

    fly::Json float1 = 1.0f;
    fly::Json float2 = 1.0f;
    fly::Json float3 = 2.5f;

    EXPECT_EQ(string1, string1);
    EXPECT_EQ(string1, string2);
    EXPECT_NE(string1, string3);
    EXPECT_NE(string1, object1);
    EXPECT_NE(string1, array1);
    EXPECT_NE(string1, bool1);
    EXPECT_NE(string1, signed1);
    EXPECT_NE(string1, unsigned1);
    EXPECT_NE(string1, float1);

    EXPECT_EQ(object1, object1);
    EXPECT_EQ(object1, object2);
    EXPECT_NE(object1, object3);
    EXPECT_NE(object1, string1);
    EXPECT_NE(object1, array1);
    EXPECT_NE(object1, bool1);
    EXPECT_NE(object1, signed1);
    EXPECT_NE(object1, unsigned1);
    EXPECT_NE(object1, float1);

    EXPECT_EQ(array1, array1);
    EXPECT_EQ(array1, array2);
    EXPECT_NE(array1, array3);
    EXPECT_NE(array1, string1);
    EXPECT_NE(array1, object1);
    EXPECT_NE(array1, bool1);
    EXPECT_NE(array1, signed1);
    EXPECT_NE(array1, unsigned1);
    EXPECT_NE(array1, float1);

    EXPECT_EQ(bool1, bool1);
    EXPECT_EQ(bool1, bool2);
    EXPECT_NE(bool1, bool3);
    EXPECT_NE(bool1, string1);
    EXPECT_NE(bool1, object1);
    EXPECT_NE(bool1, array1);
    EXPECT_NE(bool1, signed1);
    EXPECT_NE(bool1, unsigned1);
    EXPECT_NE(bool1, float1);

    EXPECT_EQ(signed1, signed1);
    EXPECT_EQ(signed1, signed2);
    EXPECT_NE(signed1, signed3);
    EXPECT_NE(signed1, string1);
    EXPECT_NE(signed1, object1);
    EXPECT_NE(signed1, array1);
    EXPECT_NE(signed1, bool1);
    EXPECT_EQ(signed1, unsigned1);
    EXPECT_NE(signed1, unsigned3);
    EXPECT_EQ(signed1, float1);
    EXPECT_NE(signed1, float3);

    EXPECT_EQ(unsigned1, unsigned1);
    EXPECT_EQ(unsigned1, unsigned2);
    EXPECT_NE(unsigned1, unsigned3);
    EXPECT_NE(unsigned1, string1);
    EXPECT_NE(unsigned1, object1);
    EXPECT_NE(unsigned1, array1);
    EXPECT_NE(unsigned1, bool1);
    EXPECT_EQ(unsigned1, signed1);
    EXPECT_NE(unsigned1, signed3);
    EXPECT_EQ(unsigned1, float1);
    EXPECT_NE(unsigned1, float3);

    EXPECT_EQ(float1, float1);
    EXPECT_EQ(float1, float2);
    EXPECT_NE(float1, float3);
    EXPECT_NE(float1, string1);
    EXPECT_NE(float1, object1);
    EXPECT_NE(float1, array1);
    EXPECT_NE(float1, bool1);
    EXPECT_EQ(float1, signed1);
    EXPECT_NE(float1, signed3);
    EXPECT_EQ(float1, unsigned1);
    EXPECT_NE(float1, unsigned3);
}

//==================================================================================================
TEST(JsonTest, Stream)
{
    std::stringstream stream;

    fly::Json string = "abc";
    fly::Json object = {{"a", 1}, {"b", 2}};
    fly::Json array = {'7', 8};
    fly::Json boolean = true;
    fly::Json sign = 1;
    fly::Json unsign = static_cast<unsigned int>(1);
    fly::Json floating = 1.0f;
    fly::Json null = nullptr;

    stream << string;
    EXPECT_EQ(stream.str(), "\"abc\"");
    stream.str(std::string());

    stream << object;
    EXPECT_EQ(stream.str(), "{\"a\":1,\"b\":2}");
    stream.str(std::string());

    stream << array;
    EXPECT_EQ(stream.str(), "[55,8]");
    stream.str(std::string());

    stream << boolean;
    EXPECT_EQ(stream.str(), "true");
    stream.str(std::string());

    stream << sign;
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    stream << unsign;
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    stream << floating;
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    stream << null;
    EXPECT_EQ(stream.str(), "null");
    stream.str(std::string());
}

//==================================================================================================
TEST(JsonTest, StreamWithEscapedSymbols)
{
    {
        fly::Json json = "abc\\\"def";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\\"def\"");
    }
    {
        fly::Json json = "abc\\\\def";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\\\def\"");
    }
    {
        fly::Json json = "abc\\bdef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\bdef\"");
    }
    {
        fly::Json json = "abc\\fdef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\fdef\"");
    }
    {
        fly::Json json = "abc\\ndef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\ndef\"");
    }
    {
        fly::Json json = "abc\\rdef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\rdef\"");
    }
    {
        fly::Json json = "abc\\tdef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\tdef\"");
    }
    {
        fly::Json json = "abc\xce\xa9zef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\u03a9zef\"");
    }
    {
        fly::Json json = "abc\xf0\x9f\x8d\x95zef";

        std::stringstream stream;
        stream << json;

        const std::string str = stream.str();
        EXPECT_EQ(str, "\"abc\\ud83c\\udf55zef\"");
    }
}
