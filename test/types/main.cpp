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

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

#if defined(FLY_WINDOWS)
    #include <Windows.h>

    #define utf8(str) ConvertToUTF(CP_UTF8, L##str)
    #define utf16(str) ConvertToUTF(CP_UTF16, L##str)
#else
    #define utf8(str) str
    #define utf16(str) str
#endif

//==============================================================================
namespace
{
#if defined(FLY_WINDOWS)

    const char *ConvertToUTF(UINT code, const wchar_t *str)
    {
        static char buff[1024];

        ::WideCharToMultiByte(code, 0, str, -1, buff, sizeof(buff), NULL, NULL);
        return buff;
    }

#endif

    void ValidateFail(const std::string &test)
    {
        SCOPED_TRACE(test);

        EXPECT_THROW({
            fly::Json actual = test;
        }, fly::JsonException);
    }

    void ValidatePass(const std::string &test, const std::string &expected)
    {
        SCOPED_TRACE(test);

        fly::Json actual;

        EXPECT_NO_THROW({
            actual = test;
        });

        std::stringstream ss;
        ss << actual;

        EXPECT_EQ(actual, expected);

        fly::Json repeat = actual;
        EXPECT_EQ(actual, repeat);
    }

    void ValidatePass(const std::string &test)
    {
        ValidatePass(test, test);
    }
}

//==============================================================================
TEST(JsonExceptionTest, ExceptionTest)
{
    std::stringstream stream;
    fly::Json string = "abc";
    stream << string;

    bool thrown = false;

    try
    {
        throw fly::JsonException(string, "some message");
    }
    catch (const fly::JsonException &e)
    {
        std::string what(e.what());

        std::string expect("*some message*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::WildcardMatch(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==============================================================================
TEST(JsonTest, StringConstructorTest)
{
    const std::string str1("a");
    EXPECT_TRUE(fly::Json(str1).IsString());

    std::string str2("b");
    EXPECT_TRUE(fly::Json(str2).IsString());

    const char *cstr1 = "c";
    EXPECT_TRUE(fly::Json(cstr1).IsString());

    char *cstr2 = (char *)"d";
    EXPECT_TRUE(fly::Json(cstr2).IsString());

    const char arr1[] = { 'g', '\0' };
    EXPECT_TRUE(fly::Json(arr1).IsString());

    char arr2[] = { 'h', '\0' };
    EXPECT_TRUE(fly::Json(arr2).IsString());
}

//==============================================================================
TEST(JsonTest, ObjectConstructorTest)
{
    std::map<std::string, int> map = { { "a", 1 }, { "b", 2 } };
    EXPECT_TRUE(fly::Json(map).IsObject());

    std::multimap<std::string, int> multimap = { { "c", 3 }, { "d", 4 } };
    EXPECT_TRUE(fly::Json(multimap).IsObject());

    std::unordered_map<std::string, int> unordered_map = { { "e", 5 }, { "f", 6 } };
    EXPECT_TRUE(fly::Json(unordered_map).IsObject());

    std::unordered_multimap<std::string, int> unordered_multimap = { { "h", 7 }, { "i", 8 } };
    EXPECT_TRUE(fly::Json(unordered_multimap).IsObject());
}

//==============================================================================
TEST(JsonTest, ArrayConstructorTest)
{
    std::array<int, 4> array = { 10, 20, 30, 40 };
    EXPECT_TRUE(fly::Json(array).IsArray());
    EXPECT_FALSE(fly::Json(array).IsObjectLike());

    std::deque<int> deque = { 50, 60, 70, 80 };
    EXPECT_TRUE(fly::Json(deque).IsArray());
    EXPECT_FALSE(fly::Json(deque).IsObjectLike());

    std::forward_list<int> forward_list = { 90, 100, 110, 120 };
    EXPECT_TRUE(fly::Json(forward_list).IsArray());
    EXPECT_FALSE(fly::Json(forward_list).IsObjectLike());

    std::list<int> list = { 130, 140, 150, 160 };
    EXPECT_TRUE(fly::Json(list).IsArray());
    EXPECT_FALSE(fly::Json(list).IsObjectLike());

    std::multiset<std::string> multiset = { "a", "b", "c" };
    EXPECT_TRUE(fly::Json(multiset).IsArray());
    EXPECT_FALSE(fly::Json(multiset).IsObjectLike());

    std::set<std::string> set = { "d", "e", "f" };
    EXPECT_TRUE(fly::Json(set).IsArray());
    EXPECT_FALSE(fly::Json(set).IsObjectLike());

    std::unordered_multiset<std::string> unordered_multiset = { "g", "h", "i" };
    EXPECT_TRUE(fly::Json(unordered_multiset).IsArray());
    EXPECT_FALSE(fly::Json(unordered_multiset).IsObjectLike());

    std::unordered_set<std::string> unordered_set = { "j", "k", "l" };
    EXPECT_TRUE(fly::Json(unordered_set).IsArray());
    EXPECT_FALSE(fly::Json(unordered_set).IsObjectLike());

    std::vector<int> vector = { 170, 180, 190, 200 };
    EXPECT_TRUE(fly::Json(vector).IsArray());
    EXPECT_FALSE(fly::Json(vector).IsObjectLike());

    std::array<std::string, 2> object = { "nine", "ten" };
    EXPECT_TRUE(fly::Json(object).IsArray());
    EXPECT_TRUE(fly::Json(object).IsObjectLike());
}

//==============================================================================
TEST(JsonTest, BooleanConstructorTest)
{
    EXPECT_TRUE(fly::Json(true).IsBoolean());
    EXPECT_TRUE(fly::Json(false).IsBoolean());
}

//==============================================================================
TEST(JsonTest, SignedIntegerConstructorTest)
{
    EXPECT_TRUE(fly::Json(static_cast<char>(1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<short>(1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<int>(1)).IsSignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<int>(-1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::int32_t>(1)).IsSignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::int32_t>(-1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::int64_t>(1)).IsSignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::int64_t>(-1)).IsSignedInteger());
}

//==============================================================================
TEST(JsonTest, UnsignedIntegerConstructorTest)
{
    EXPECT_TRUE(fly::Json(static_cast<unsigned char>(1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<unsigned short>(1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(1)).IsUnsignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(-1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(1)).IsUnsignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(-1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(1)).IsUnsignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(-1)).IsUnsignedInteger());
}

//==============================================================================
TEST(JsonTest, FloatConstructorTest)
{
    EXPECT_TRUE(fly::Json(static_cast<float>(1.0)).IsFloat());
    EXPECT_TRUE(fly::Json(static_cast<double>(1.0)).IsFloat());
    EXPECT_TRUE(fly::Json(static_cast<long double>(1.0)).IsFloat());
}

//==============================================================================
TEST(JsonTest, NullConstructorTest)
{
    EXPECT_TRUE(fly::Json().IsNull());
    EXPECT_TRUE(fly::Json(nullptr).IsNull());
}

//==============================================================================
TEST(JsonTest, InitializerListConstructorTest)
{
    const fly::Json empty = { };
    EXPECT_TRUE(fly::Json(empty).IsNull());

    const fly::Json array = { '7', 8, "nine", 10 };
    EXPECT_TRUE(fly::Json(array).IsArray());

    const fly::Json object = { { "a", 1 }, { "b", 2 } };
    EXPECT_TRUE(fly::Json(object).IsObject());

    const fly::Json almost = { { "a", 1 }, { "b", 2 }, 4 };
    EXPECT_TRUE(fly::Json(almost).IsArray());
}

//==============================================================================
TEST(JsonTest, CopyConstructorTest)
{
    fly::Json string = "abc";
    EXPECT_EQ(fly::Json(string), string);

    fly::Json object = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(fly::Json(object), object);

    fly::Json array = { '7', 8 };
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

//==============================================================================
TEST(JsonTest, AssignmentTest)
{
    fly::Json json;

    fly::Json string = "abc";
    json = string;
    EXPECT_EQ(json, string);

    fly::Json object = { { "a", 1 }, { "b", 2 } };
    json = object;
    EXPECT_EQ(json, object);

    fly::Json array = { '7', 8 };
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

//==============================================================================
TEST(JsonTest, StringConversionTest)
{
    fly::Json json;

    std::string string = "abc";
    json = string;
    EXPECT_EQ(std::string(json), string);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(std::string(json), "{ \"a\" : 1, \"b\" : 2 }");

    json = { '7', 8 };
    EXPECT_EQ(std::string(json), "[ 55, 8 ]");

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

//==============================================================================
TEST(JsonTest, ObjectConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    std::map<std::string, int> map = { { "a", 1 }, { "b", 2 } };
    std::multimap<std::string, int> multimap(map.begin(), map.end());
    json = map;
    EXPECT_EQ((std::map<std::string, int>(json)), map);
    EXPECT_EQ((std::multimap<std::string, int>(json)), multimap);

    std::map<std::string, int> empty = { };
    std::multimap<std::string, int> multiempty(empty.begin(), empty.end());
    json = empty;
    EXPECT_EQ((std::map<std::string, int>(json)), empty);
    EXPECT_EQ((std::multimap<std::string, int>(json)), multiempty);

    json = { '7', 8 };
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = 1;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = nullptr;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, ArrayConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    std::vector<int> vector = { 7, 8 };
    std::array<int, 1> array1 = { 7 };
    std::array<int, 2> array2 = { 7, 8 };
    std::array<int, 3> array3 = { 7, 8, 0 };
    json = vector;
    EXPECT_EQ((std::vector<int>(json)), vector);
    EXPECT_EQ((std::array<int, 1>(json)), array1);
    EXPECT_EQ((std::array<int, 2>(json)), array2);
    EXPECT_EQ((std::array<int, 3>(json)), array3);

    std::vector<int> empty = { };
    std::array<int, 1> empty1 = { };
    std::array<int, 2> empty2 = { };
    std::array<int, 3> empty3 = { };
    json = empty;
    EXPECT_EQ((std::vector<int>(json)), empty);
    EXPECT_EQ((std::array<int, 1>(json)), empty1);
    EXPECT_EQ((std::array<int, 2>(json)), empty2);
    EXPECT_EQ((std::array<int, 3>(json)), empty3);

    json = true;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = 1;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = nullptr;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, BooleanConversionTest)
{
    fly::Json json;

    json = "";
    EXPECT_FALSE(bool(json));
    json = "abc";
    EXPECT_TRUE(bool(json));

    json = std::map<std::string, int>();
    EXPECT_FALSE(bool(json));
    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_TRUE(bool(json));

    json = std::vector<int>();
    EXPECT_FALSE(bool(json));
    json = { 7, 8 };
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

//==============================================================================
TEST(JsonTest, SignedIntegerConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((int(json)), fly::JsonException);

    json = "123";
    EXPECT_EQ(int(json), 123);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((int(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((int(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((int(json)), fly::JsonException);

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
    EXPECT_THROW((int(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, UnsignedIntegerConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    json = "123";
    EXPECT_EQ(unsigned(json), unsigned(123));

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((unsigned(json)), fly::JsonException);

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
    EXPECT_THROW((unsigned(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, FloatConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((float(json)), fly::JsonException);

    json = "123.5";
    EXPECT_EQ(float(json), 123.5f);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((float(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((float(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((float(json)), fly::JsonException);

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
    EXPECT_THROW((float(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, NullConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = 'a';
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = 12;
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = static_cast<unsigned int>(12);
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = 3.14f;
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = nullptr;
    EXPECT_EQ((std::nullptr_t(json)), nullptr);
}

//==============================================================================
TEST(JsonTest, ObjectAccessTest)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1["a"], fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2["a"], fly::JsonException);

    fly::Json object1 = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(object1["a"], 1);
    EXPECT_EQ(object1["b"], 2);
    EXPECT_NO_THROW(object1["c"]);
    EXPECT_EQ(object1["c"], nullptr);

    const fly::Json object2 = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(object2["a"], 1);
    EXPECT_EQ(object2["b"], 2);
    EXPECT_THROW(object2["c"], fly::JsonException);

    fly::Json array1 = { '7', 8 };
    EXPECT_THROW(array1["a"], fly::JsonException);

    const fly::Json array2 = { '7', 8 };
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

    const fly::Json unsigned2 = static_cast<unsigned int>(1);;
    EXPECT_THROW(unsigned2["a"], fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1["a"], fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2["a"], fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_NO_THROW(null1["a"]);
    EXPECT_TRUE(null1.IsObject());
    EXPECT_EQ(null1["a"], nullptr);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2["a"], fly::JsonException);
}

//==============================================================================
TEST(JsonTest, ArrayAccessTest)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1[0], fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2[0], fly::JsonException);

    fly::Json object1 = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW(object1[0], fly::JsonException);

    const fly::Json object2 = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW(object2[0], fly::JsonException);

    fly::Json array1 = { '7', 8 };
    EXPECT_EQ(array1[0], '7');
    EXPECT_EQ(array1[1], 8);
    EXPECT_NO_THROW(array1[2]);
    EXPECT_EQ(array1[2], nullptr);

    const fly::Json array2 = { '7', 8 };
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

    const fly::Json unsigned2 = static_cast<unsigned int>(1);;
    EXPECT_THROW(unsigned2[0], fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1[0], fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2[0], fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_NO_THROW(null1[0]);
    EXPECT_TRUE(null1.IsArray());
    EXPECT_EQ(null1[0], nullptr);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2[0], fly::JsonException);
}

//==============================================================================
TEST(JsonTest, SizeTest)
{
    fly::Json json;

    json = "abcdef";
    EXPECT_EQ(json.Size(), 6);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(json.Size(), 2);

    json = { '7', 8, 9, 10 };
    EXPECT_EQ(json.Size(), 4);

    json = true;
    EXPECT_EQ(json.Size(), 1);

    json = 1;
    EXPECT_EQ(json.Size(), 1);

    json = static_cast<unsigned int>(1);
    EXPECT_EQ(json.Size(), 1);

    json = 1.0f;
    EXPECT_EQ(json.Size(), 1);

    json = nullptr;
    EXPECT_EQ(json.Size(), 0);
}

//==============================================================================
TEST(JsonTest, EqualityTest)
{
    fly::Json string1 = "abc";
    fly::Json string2 = "abc";
    fly::Json string3 = "def";

    fly::Json object1 = { { "a", 1 }, { "b", 2 } };
    fly::Json object2 = { { "a", 1 }, { "b", 2 } };
    fly::Json object3 = { { "a", 1 }, { "b", 3 } };

    fly::Json array1 = { '7', 8 };
    fly::Json array2 = { '7', 8 };
    fly::Json array3 = { '7', 9 };

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

//==============================================================================
TEST(JsonTest, StreamTest)
{
    std::stringstream stream;

    fly::Json string = "abc";
    fly::Json object = { { "a", 1 }, { "b", 2 } };
    fly::Json array = { '7', 8 };
    fly::Json boolean = true;
    fly::Json sign = 1;
    fly::Json unsign = static_cast<unsigned int>(1);
    fly::Json floating = 1.0f;
    fly::Json null = nullptr;

    stream << string;
    EXPECT_EQ(stream.str(), "\"abc\"");
    stream.str(std::string());

    stream << object;
    EXPECT_EQ(stream.str(), "{ \"a\" : 1, \"b\" : 2 }");
    stream.str(std::string());

    stream << array;
    EXPECT_EQ(stream.str(), "[ 55, 8 ]");
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

//==============================================================================
TEST(JsonTest, UnicodeConversionTest)
{
    ValidateFail("\\u");
    ValidateFail("\\u0");
    ValidateFail("\\u00");
    ValidateFail("\\u000");
    ValidateFail("\\u000z");

    ValidatePass("\\u0040", utf8("\u0040"));
    ValidatePass("\\u007A", utf8("\u007A"));
    ValidatePass("\\u007a", utf8("\u007a"));
    ValidatePass("\\u00c4", utf8("\u00c4"));
    ValidatePass("\\u00e4", utf8("\u00e4"));
    ValidatePass("\\u0298", utf8("\u0298"));
    ValidatePass("\\u0800", utf8("\u0800"));
    ValidatePass("\\uffff", utf8("\uffff"));

    ValidateFail("\\uDC00");
    ValidateFail("\\uDFFF");
    ValidateFail("\\uD800");
    ValidateFail("\\uDBFF");
    ValidateFail("\\uD800\\u");
    ValidateFail("\\uD800\\z");
    ValidateFail("\\uD800\\u0");
    ValidateFail("\\uD800\\u00");
    ValidateFail("\\uD800\\u000");
    ValidateFail("\\uD800\\u0000");
    ValidateFail("\\uD800\\u000z");
    ValidateFail("\\uD800\\uDBFF");
    ValidateFail("\\uD800\\uE000");
    ValidateFail("\\uD800\\uFFFF");

    ValidatePass("\\uD800\\uDC00", utf16("\U00010000"));
    ValidatePass("\\uD803\\uDE6D", utf16("\U00010E6D"));
    ValidatePass("\\uD834\\uDD1E", utf16("\U0001D11E"));
    ValidatePass("\\uDBFF\\uDFFF", utf16("\U0010FFFF"));
}

//==============================================================================
TEST(JsonTest, MarkusKuhnStressTest)
{
    // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

    // 1  Some correct UTF-8 text
    {
        ValidatePass("κόσμε");
    }

    // 2  Boundary condition test cases
    {
        // 2.1  First possible sequence of a certain length
        {
            // 2.1.1  1 byte  (U-00000001)
            ValidateFail("\x01");

            // 2.1.2  2 bytes (U-00000080)
            ValidatePass("\xc2\x80");

            // 2.1.3  3 bytes (U-00000800)
            ValidatePass("\xe0\xa0\x80");

            // 2.1.4  4 bytes (U-00010000)
            ValidatePass("\xf0\x90\x80\x80");

            // 2.1.5  5 bytes (U-00200000)
            ValidateFail("\xf8\x88\x80\x80\x80");

            // 2.1.6  6 bytes (U-04000000)
            ValidateFail("\xfc\x84\x80\x80\x80\x80");
        }

        // 2.2  Last possible sequence of a certain length
        {
            // 2.2.1  1 byte  (U-0000007F)
            ValidatePass("\x7f");

            // 2.2.2  2 bytes (U-000007FF)
            ValidatePass("\xdf\xbf");

            // 2.2.3  3 bytes (U-0000FFFF)
            ValidatePass("\xef\xbf\xbf");

            // 2.1.4  4 bytes (U-00200000)
            ValidateFail("\xf7\xbf\xbf\xbf");

            // 2.1.5  5 bytes (U-03FFFFFF)
            ValidateFail("\xfb\xbf\xbf\xbf\xbf");

            // 2.1.6  6 bytes (U-7FFFFFFF)
            ValidateFail("\xfd\xbf\xbf\xbf\xbf\xbf");
        }

        // 2.3  Other boundary conditions
        {
            // 2.3.1  U-0000D7FF
            ValidatePass("\xed\x9f\xbf");

            // 2.3.2  U-0000E000
            ValidatePass("\xee\x80\x80");

            // 2.3.3  U-0000FFFD
            ValidatePass("\xef\xbf\xbd");

            // 2.3.4  U-0010FFFF
            ValidatePass("\xf4\x8f\xbf\xbf");

            // 2.3.5  U-00110000
            ValidateFail("\xf4\x90\x80\x80");
        }
    }

    // 3  Malformed sequences
    {
        // 3.1  Unexpected continuation bytes
        {
            // 3.1.1  First continuation byte 0x80
            ValidateFail("\x80");

            // 3.1.2 Last  continuation byte 0xbf
            ValidateFail("\xbf");

            // 3.1.3  2 continuation bytes
            ValidateFail("\x80\xbf");

            // 3.1.4  3 continuation bytes
            ValidateFail("\x80\xbf\x80");

            // 3.1.5  4 continuation bytes
            ValidateFail("\x80\xbf\x80\xbf");

            // 3.1.6  5 continuation bytes
            ValidateFail("\x80\xbf\x80\xbf\x80");

            // 3.1.7  6 continuation bytes
            ValidateFail("\x80\xbf\x80\xbf\x80\xbf");

            // 3.1.8  7 continuation bytes
            ValidateFail("\x80\xbf\x80\xbf\x80\xbf\x80");

            // 3.1.9  Sequence of all 64 possible continuation bytes (0x80-0xbf)
            ValidateFail("\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf");
        }

        // 3.2  Lonely start characters
        {
            // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf),
            //        each followed by a space character
            ValidateFail("\xc0 \xc1 \xc2 \xc3 \xc4 \xc5 \xc6 \xc7 \xc8 \xc9 \xca \xcb \xcc \xcd \xce \xcf \xd0 \xd1 \xd2 \xd3 \xd4 \xd5 \xd6 \xd7 \xd8 \xd9 \xda \xdb \xdc \xdd \xde \xdf");
            ValidateFail("\xc0 ");
            ValidateFail("\xc1 ");
            ValidateFail("\xc2 ");
            ValidateFail("\xc3 ");
            ValidateFail("\xc4 ");
            ValidateFail("\xc5 ");
            ValidateFail("\xc6 ");
            ValidateFail("\xc7 ");
            ValidateFail("\xc8 ");
            ValidateFail("\xc9 ");
            ValidateFail("\xca ");
            ValidateFail("\xcb ");
            ValidateFail("\xcc ");
            ValidateFail("\xcd ");
            ValidateFail("\xce ");
            ValidateFail("\xcf ");
            ValidateFail("\xd0 ");
            ValidateFail("\xd1 ");
            ValidateFail("\xd2 ");
            ValidateFail("\xd3 ");
            ValidateFail("\xd4 ");
            ValidateFail("\xd5 ");
            ValidateFail("\xd6 ");
            ValidateFail("\xd7 ");
            ValidateFail("\xd8 ");
            ValidateFail("\xd9 ");
            ValidateFail("\xda ");
            ValidateFail("\xdb ");
            ValidateFail("\xdc ");
            ValidateFail("\xdd ");
            ValidateFail("\xde ");
            ValidateFail("\xdf ");

            // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef)
            //        each followed by a space character
            ValidateFail("\xe0 \xe1 \xe2 \xe3 \xe4 \xe5 \xe6 \xe7 \xe8 \xe9 \xea \xeb \xec \xed \xee \xef");
            ValidateFail("\xe0 ");
            ValidateFail("\xe1 ");
            ValidateFail("\xe2 ");
            ValidateFail("\xe3 ");
            ValidateFail("\xe4 ");
            ValidateFail("\xe5 ");
            ValidateFail("\xe6 ");
            ValidateFail("\xe7 ");
            ValidateFail("\xe8 ");
            ValidateFail("\xe9 ");
            ValidateFail("\xea ");
            ValidateFail("\xeb ");
            ValidateFail("\xec ");
            ValidateFail("\xed ");
            ValidateFail("\xee ");
            ValidateFail("\xef ");

            // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7),
            //        each followed by a space character
            ValidateFail("\xf0 \xf1 \xf2 \xf3 \xf4 \xf5 \xf6 \xf7");
            ValidateFail("\xf0 ");
            ValidateFail("\xf1 ");
            ValidateFail("\xf2 ");
            ValidateFail("\xf3 ");
            ValidateFail("\xf4 ");
            ValidateFail("\xf5 ");
            ValidateFail("\xf6 ");
            ValidateFail("\xf7 ");

            // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb),
            //        each followed by a space character
            ValidateFail("\xf8 \xf9 \xfa \xfb");
            ValidateFail("\xf8 ");
            ValidateFail("\xf9 ");
            ValidateFail("\xfa ");
            ValidateFail("\xfb ");

            // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd),
            //        each followed by a space character
            ValidateFail("\xfc \xfd");
            ValidateFail("\xfc ");
            ValidateFail("\xfc ");
        }

        // 3.3  Sequences with last continuation byte missing
        {
            // 3.3.1  2-byte sequence with last byte missing (U+0000)
            ValidateFail("\xc0");

            // 3.3.2  3-byte sequence with last byte missing (U+0000)
            ValidateFail("\xe0\x80");

            // 3.3.3  4-byte sequence with last byte missing (U+0000)
            ValidateFail("\xf0\x80\x80");

            // 3.3.4  5-byte sequence with last byte missing (U+0000)
            ValidateFail("\xf8\x80\x80\x80");

            // 3.3.5  6-byte sequence with last byte missing (U+0000)
            ValidateFail("\xfc\x80\x80\x80\x80");

            // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
            ValidateFail("\xdf");

            // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
            ValidateFail("\xef\xbf");

            // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
            ValidateFail("\xf7\xbf\xbf");

            // 3.3.9  5-byte sequence with last byte missing (U-03FFFFFF)
            ValidateFail("\xfb\xbf\xbf\xbf");

            // 3.3.10 6-byte sequence with last byte missing (U-7FFFFFFF)
            ValidateFail("\xfd\xbf\xbf\xbf\xbf");
        }

        // 3.4  Concatenation of incomplete sequences
        {
            // All the 10 sequences of 3.3 concatenated
            ValidateFail("\xc0\xe0\x80\xf0\x80\x80\xf8\x80\x80\x80\xfc\x80\x80\x80\x80\xdf\xef\xbf\xf7\xbf\xbf\xfb\xbf\xbf\xbf\xfd\xbf\xbf\xbf\xbf");
        }

        // 3.5  Impossible bytes
        {
            // 3.5.1  fe
            ValidateFail("\xfe");

            // 3.5.2  ff
            ValidateFail("\xff");

            // 3.5.3  fe fe ff ff
            ValidateFail("\xfe\xfe\xff\xff");
        }
    }

    // 4  Overlong sequences
    {
        // 4.1  Examples of an overlong ASCII character
        {
            // 4.1.1 U+002F = c0 af
            ValidateFail("\xc0\xaf");

            // 4.1.2 U+002F = e0 80 af
            ValidateFail("\xe0\x80\xaf");

            // 4.1.3 U+002F = f0 80 80 af
            ValidateFail("\xf0\x80\x80\xaf");

            // 4.1.4 U+002F = f8 80 80 80 af
            ValidateFail("\xf8\x80\x80\x80\xaf");

            // 4.1.5 U+002F = fc 80 80 80 80 af
            ValidateFail("\xfc\x80\x80\x80\x80\xaf");
        }

        // 4.2  Maximum overlong sequences
        {
            // 4.2.1  U-0000007F = c1 bf
            ValidateFail("\xc1\xbf");

            // 4.2.2  U-000007FF = e0 9f bf
            ValidateFail("\xe0\x9f\xbf");

            // 4.2.3  U-0000FFFF = f0 8f bf bf
            ValidateFail("\xf0\x8f\xbf\xbf");

            // 4.2.4  U-001FFFFF = f8 87 bf bf bf
            ValidateFail("\xf8\x87\xbf\xbf\xbf");

            // 4.2.5  U-03FFFFFF = fc 83 bf bf bf bf
            ValidateFail("\xfc\x83\xbf\xbf\xbf\xbf");
        }

        // 4.3  Overlong representation of the NUL character
        {
            // 4.3.1  U+0000 = c0 80
            ValidateFail("\xc0\x80");

            // 4.3.2  U+0000 = e0 80 80
            ValidateFail("\xe0\x80\x80");

            // 4.3.3  U+0000 = f0 80 80 80
            ValidateFail("\xf0\x80\x80\x80");

            // 4.3.4  U+0000 = f8 80 80 80 80
            ValidateFail("\xf8\x80\x80\x80\x80");

            // 4.3.5  U+0000 = fc 80 80 80 80 80
            ValidateFail("\xfc\x80\x80\x80\x80\x80");
        }
    }

    // 5  Illegal code positions
    {
        // 5.1 Single UTF-16 surrogates
        {
            // 5.1.1  U+D800 = ed a0 80
            ValidateFail("\xed\xa0\x80");

            // 5.1.2  U+DB7F = ed ad bf
            ValidateFail("\xed\xad\xbf");

            // 5.1.3  U+DB80 = ed ae 80
            ValidateFail("\xed\xae\x80");

            // 5.1.4  U+DBFF = ed af bf
            ValidateFail("\xed\xaf\xbf");

            // 5.1.5  U+DC00 = ed b0 80
            ValidateFail("\xed\xb0\x80");

            // 5.1.6  U+DF80 = ed be 80
            ValidateFail("\xed\xbe\x80");

            // 5.1.7  U+DFFF = ed bf bf
            ValidateFail("\xed\xbf\xbf");
        }

        // 5.2 Paired UTF-16 surrogates
        {
            // 5.2.1  U+D800 U+DC00 = ed a0 80 ed b0 80
            ValidateFail("\xed\xa0\x80\xed\xb0\x80");

            // 5.2.2  U+D800 U+DFFF = ed a0 80 ed bf bf
            ValidateFail("\xed\xa0\x80\xed\xbf\xbf");

            // 5.2.3  U+DB7F U+DC00 = ed ad bf ed b0 80
            ValidateFail("\xed\xad\xbf\xed\xb0\x80");

            // 5.2.4  U+DB7F U+DFFF = ed ad bf ed bf bf
            ValidateFail("\xed\xad\xbf\xed\xbf\xbf");

            // 5.2.5  U+DB80 U+DC00 = ed ae 80 ed b0 80
            ValidateFail("\xed\xae\x80\xed\xb0\x80");

            // 5.2.6  U+DB80 U+DFFF = ed ae 80 ed bf bf
            ValidateFail("\xed\xae\x80\xed\xbf\xbf");

            // 5.2.7  U+DBFF U+DC00 = ed af bf ed b0 80
            ValidateFail("\xed\xaf\xbf\xed\xb0\x80");

            // 5.2.8  U+DBFF U+DFFF = ed af bf ed bf bf
            ValidateFail("\xed\xaf\xbf\xed\xbf\xbf");
        }

        // 5.3 Noncharacter code positions
        {
            // 5.3.1  U+FFFE = ef bf be
            ValidatePass("\xef\xbf\xbe");

            // 5.3.2  U+FFFF = ef bf bf
            ValidatePass("\xef\xbf\xbf");

            // 5.3.3  U+FDD0 .. U+FDEF
            ValidatePass("\xef\xb7\x90");
            ValidatePass("\xef\xb7\x91");
            ValidatePass("\xef\xb7\x92");
            ValidatePass("\xef\xb7\x93");
            ValidatePass("\xef\xb7\x94");
            ValidatePass("\xef\xb7\x95");
            ValidatePass("\xef\xb7\x96");
            ValidatePass("\xef\xb7\x97");
            ValidatePass("\xef\xb7\x98");
            ValidatePass("\xef\xb7\x99");
            ValidatePass("\xef\xb7\x9a");
            ValidatePass("\xef\xb7\x9b");
            ValidatePass("\xef\xb7\x9c");
            ValidatePass("\xef\xb7\x9d");
            ValidatePass("\xef\xb7\x9e");
            ValidatePass("\xef\xb7\x9f");
            ValidatePass("\xef\xb7\xa0");
            ValidatePass("\xef\xb7\xa1");
            ValidatePass("\xef\xb7\xa2");
            ValidatePass("\xef\xb7\xa3");
            ValidatePass("\xef\xb7\xa4");
            ValidatePass("\xef\xb7\xa5");
            ValidatePass("\xef\xb7\xa6");
            ValidatePass("\xef\xb7\xa7");
            ValidatePass("\xef\xb7\xa8");
            ValidatePass("\xef\xb7\xa9");
            ValidatePass("\xef\xb7\xaa");
            ValidatePass("\xef\xb7\xab");
            ValidatePass("\xef\xb7\xac");
            ValidatePass("\xef\xb7\xad");
            ValidatePass("\xef\xb7\xae");
            ValidatePass("\xef\xb7\xaf");

            // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
            ValidatePass("\xf0\x9f\xbf\xbf");
            ValidatePass("\xf0\xaf\xbf\xbf");
            ValidatePass("\xf0\xbf\xbf\xbf");
            ValidatePass("\xf1\x8f\xbf\xbf");
            ValidatePass("\xf1\x9f\xbf\xbf");
            ValidatePass("\xf1\xaf\xbf\xbf");
            ValidatePass("\xf1\xbf\xbf\xbf");
            ValidatePass("\xf2\x8f\xbf\xbf");
            ValidatePass("\xf2\x9f\xbf\xbf");
            ValidatePass("\xf2\xaf\xbf\xbf");
        }
    }
}

//==============================================================================
TEST(JsonTest, MarkusKuhnExtendedTest)
{
    // Exceptions not caught by Markus Kuhn's stress test
    ValidateFail("\x22");

    ValidateFail("\xe0\xa0\x79");
    ValidateFail("\xe0\xa0\xff");

    ValidateFail("\xed\x80\x79");
    ValidateFail("\xed\x80\xff");

    ValidateFail("\xf0\x90\x79");
    ValidateFail("\xf0\x90\xff");
    ValidateFail("\xf0\x90\x80\x79");
    ValidateFail("\xf0\x90\x80\xff");

    ValidateFail("\xf1\x80\x79");
    ValidateFail("\xf1\x80\xff");
    ValidateFail("\xf1\x80\x80\x79");
    ValidateFail("\xf1\x80\x80\xff");

    ValidateFail("\xf4\x80\x79");
    ValidateFail("\xf4\x80\xff");
    ValidateFail("\xf4\x80\x80\x79");
    ValidateFail("\xf4\x80\x80\xff");
}
