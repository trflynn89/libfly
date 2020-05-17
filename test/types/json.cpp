#include "fly/types/json/json.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"

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

#if defined(FLY_WINDOWS)
#    include <Windows.h>
#    define utf8(str) convert_to_utf8(L##str)
#else
#    define utf8(str) str
#endif

namespace {

#if defined(FLY_WINDOWS)
const char *convert_to_utf8(const wchar_t *str)
{
    static char buff[1024];

    ::WideCharToMultiByte(CP_UTF8, 0, str, -1, buff, sizeof(buff), NULL, NULL);

    return buff;
}
#endif

} // namespace

//==============================================================================
class JsonTest : public ::testing::Test
{
protected:
    template <typename T>
    void validate_throw(const fly::Json &json) noexcept(false)
    {
        SCOPED_TRACE(json);

        EXPECT_THROW((void)(T(json)), fly::JsonException);
    }

    void validate_fail(const std::string &test) noexcept(false)
    {
        SCOPED_TRACE(test);

        fly::Json actual;

        EXPECT_THROW({ actual = test; }, fly::JsonException);
    }

    void validate_pass(const std::string &test) noexcept(false)
    {
        validate_pass(test, test);
    }

    void validate_pass(
        const std::string &test,
        const std::string &expected) noexcept(false)
    {
        SCOPED_TRACE(test);

        fly::Json actual;

        EXPECT_NO_THROW({ actual = test; });

        std::stringstream ss;
        ss << actual;

        EXPECT_EQ(actual, expected);

        fly::Json repeat = actual;
        EXPECT_EQ(actual, repeat);
    }
};

//==============================================================================
TEST(JsonExceptionTest, Exception)
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
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==============================================================================
TEST_F(JsonTest, StringConstructor)
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

//==============================================================================
TEST_F(JsonTest, ObjectConstructor)
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

//==============================================================================
TEST_F(JsonTest, ArrayConstructor)
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

//==============================================================================
TEST_F(JsonTest, BooleanConstructor)
{
    EXPECT_TRUE(fly::Json(true).is_boolean());
    EXPECT_TRUE(fly::Json(false).is_boolean());
}

//==============================================================================
TEST_F(JsonTest, SignedIntegerConstructor)
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

//==============================================================================
TEST_F(JsonTest, UnsignedIntegerConstructor)
{
    EXPECT_TRUE(fly::Json(static_cast<unsigned char>(1)).is_unsigned_integer());

    EXPECT_TRUE(
        fly::Json(static_cast<unsigned short>(1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(1)).is_unsigned_integer());
    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(-1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(1)).is_unsigned_integer());
    EXPECT_TRUE(
        fly::Json(static_cast<std::uint32_t>(-1)).is_unsigned_integer());

    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(1)).is_unsigned_integer());
    EXPECT_TRUE(
        fly::Json(static_cast<std::uint64_t>(-1)).is_unsigned_integer());
}

//==============================================================================
TEST_F(JsonTest, FloatConstructor)
{
    EXPECT_TRUE(fly::Json(static_cast<float>(1.0)).is_float());
    EXPECT_TRUE(fly::Json(static_cast<double>(1.0)).is_float());
    EXPECT_TRUE(fly::Json(static_cast<long double>(1.0)).is_float());
}

//==============================================================================
TEST_F(JsonTest, NullConstructor)
{
    EXPECT_TRUE(fly::Json().is_null());
    EXPECT_TRUE(fly::Json(nullptr).is_null());
}

//==============================================================================
TEST_F(JsonTest, InitializerListConstructor)
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

//==============================================================================
TEST_F(JsonTest, CopyConstructor)
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

//==============================================================================
TEST_F(JsonTest, MoveConstructor)
{
    fly::Json string = "abc";
    fly::Json string_copy(string);
    fly::Json string_move(std::move(string_copy));

    EXPECT_TRUE(string_copy.is_null());
    EXPECT_EQ(string_move, string);
}

//==============================================================================
TEST_F(JsonTest, Assignment)
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

//==============================================================================
TEST_F(JsonTest, StringConversion)
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

//==============================================================================
TEST_F(JsonTest, ObjectConversion)
{
    fly::Json json;

    json = "abc";
    validate_throw<std::map<std::string, fly::Json>>(json);

    std::map<std::string, int> map = {};
    std::multimap<std::string, int> multimap(map.begin(), map.end());
    json = map;
    EXPECT_EQ(decltype(map)(json), map);
    EXPECT_EQ(decltype(multimap)(json), multimap);

    map = {{"a", 1}, {"b", 2}};
    multimap = decltype(multimap)(map.begin(), map.end());
    json = map;
    EXPECT_EQ(decltype(map)(json), map);
    EXPECT_EQ(decltype(multimap)(json), multimap);

    std::unordered_map<std::string, int> umap(map.begin(), map.end());
    json = umap;
    EXPECT_EQ(decltype(map)(json), map);
    EXPECT_EQ(decltype(umap)(json), umap);

    std::unordered_multimap<std::string, int> umultimap(map.begin(), map.end());
    json = umap;
    EXPECT_EQ(decltype(map)(json), map);
    EXPECT_EQ(decltype(umultimap)(json), umultimap);

    json = {'7', 8};
    validate_throw<std::map<std::string, fly::Json>>(json);

    json = true;
    validate_throw<std::map<std::string, fly::Json>>(json);

    json = 1;
    validate_throw<std::map<std::string, fly::Json>>(json);

    json = static_cast<unsigned int>(1);
    validate_throw<std::map<std::string, fly::Json>>(json);

    json = 1.0f;
    validate_throw<std::map<std::string, fly::Json>>(json);

    json = nullptr;
    validate_throw<std::map<std::string, fly::Json>>(json);
}

//==============================================================================
TEST_F(JsonTest, ArrayConversion)
{
    fly::Json json;

    json = "abc";
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);

    json = {{"a", 1}, {"b", 2}};
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);

    std::array<int, 4> array1 = {50, 60, 70, 80};
    std::array<std::string, 4> array2 = {"50", "60", "70", "80"};
    json = array1;
    EXPECT_EQ((std::array<int, 4>(json)), array1);
    EXPECT_EQ((std::array<std::string, 4>(json)), array2);
    json = array2;
    EXPECT_EQ((std::array<int, 4>(json)), array1);
    EXPECT_EQ((std::array<std::string, 4>(json)), array2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::array<int, 4>(json)), array1);
    EXPECT_EQ((std::array<std::string, 4>(json)), array2);
    json = {true};
    validate_throw<std::array<int, 1>>(json);
    json = {"string"};
    validate_throw<std::array<int, 1>>(json);

    std::array<int, 1> array3 = {7};
    std::array<int, 2> array4 = {7, 8};
    std::array<int, 3> array5 = {7, 8, 0};
    json = array4;
    EXPECT_EQ((std::array<int, 1>(json)), array3);
    EXPECT_EQ((std::array<int, 2>(json)), array4);
    EXPECT_EQ((std::array<int, 3>(json)), array5);

    std::deque<int> deque1 = {50, 60, 70, 80};
    std::deque<std::string> deque2 = {"50", "60", "70", "80"};
    json = deque1;
    EXPECT_EQ((std::deque<int>(json)), deque1);
    EXPECT_EQ((std::deque<std::string>(json)), deque2);
    json = deque2;
    EXPECT_EQ((std::deque<int>(json)), deque1);
    EXPECT_EQ((std::deque<std::string>(json)), deque2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::deque<int>(json)), deque1);
    EXPECT_EQ((std::deque<std::string>(json)), deque2);
    json = {true};
    validate_throw<std::deque<int>>(json);
    json = {"string"};
    validate_throw<std::deque<int>>(json);

    std::forward_list<int> forward_list1 = {50, 60, 70, 80};
    std::forward_list<std::string> forward_list2 = {"50", "60", "70", "80"};
    json = forward_list1;
    EXPECT_EQ((std::forward_list<int>(json)), forward_list1);
    EXPECT_EQ((std::forward_list<std::string>(json)), forward_list2);
    json = forward_list2;
    EXPECT_EQ((std::forward_list<int>(json)), forward_list1);
    EXPECT_EQ((std::forward_list<std::string>(json)), forward_list2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::forward_list<int>(json)), forward_list1);
    EXPECT_EQ((std::forward_list<std::string>(json)), forward_list2);
    json = {true};
    validate_throw<std::forward_list<int>>(json);
    json = {"string"};
    validate_throw<std::forward_list<int>>(json);

    std::list<int> list1 = {50, 60, 70, 80};
    std::list<std::string> list2 = {"50", "60", "70", "80"};
    json = list1;
    EXPECT_EQ((std::list<int>(json)), list1);
    EXPECT_EQ((std::list<std::string>(json)), list2);
    json = list2;
    EXPECT_EQ((std::list<int>(json)), list1);
    EXPECT_EQ((std::list<std::string>(json)), list2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::list<int>(json)), list1);
    EXPECT_EQ((std::list<std::string>(json)), list2);
    json = {true};
    validate_throw<std::list<int>>(json);
    json = {"string"};
    validate_throw<std::list<int>>(json);

    std::multiset<int> multiset1 = {50, 60, 70, 80};
    std::multiset<std::string> multiset2 = {"50", "60", "70", "80"};
    json = multiset1;
    EXPECT_EQ((std::multiset<int>(json)), multiset1);
    EXPECT_EQ((std::multiset<std::string>(json)), multiset2);
    json = multiset2;
    EXPECT_EQ((std::multiset<int>(json)), multiset1);
    EXPECT_EQ((std::multiset<std::string>(json)), multiset2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::multiset<int>(json)), multiset1);
    EXPECT_EQ((std::multiset<std::string>(json)), multiset2);
    json = {true};
    validate_throw<std::multiset<int>>(json);
    json = {"string"};
    validate_throw<std::multiset<int>>(json);

    std::set<int> set1 = {50, 60, 70, 80};
    std::set<std::string> set2 = {"50", "60", "70", "80"};
    json = set1;
    EXPECT_EQ((std::set<int>(json)), set1);
    EXPECT_EQ((std::set<std::string>(json)), set2);
    json = set2;
    EXPECT_EQ((std::set<int>(json)), set1);
    EXPECT_EQ((std::set<std::string>(json)), set2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::set<int>(json)), set1);
    EXPECT_EQ((std::set<std::string>(json)), set2);
    json = {true};
    validate_throw<std::set<int>>(json);
    json = {"string"};
    validate_throw<std::set<int>>(json);

    std::unordered_multiset<int> u_multiset1 = {50, 60, 70, 80};
    std::unordered_multiset<std::string> u_multiset2 = {"50", "60", "70", "80"};
    json = u_multiset1;
    EXPECT_EQ((std::unordered_multiset<int>(json)), u_multiset1);
    EXPECT_EQ((std::unordered_multiset<std::string>(json)), u_multiset2);
    json = u_multiset2;
    EXPECT_EQ((std::unordered_multiset<int>(json)), u_multiset1);
    EXPECT_EQ((std::unordered_multiset<std::string>(json)), u_multiset2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::unordered_multiset<int>(json)), u_multiset1);
    EXPECT_EQ((std::unordered_multiset<std::string>(json)), u_multiset2);
    json = {true};
    validate_throw<std::unordered_multiset<int>>(json);
    json = {"string"};
    validate_throw<std::unordered_multiset<int>>(json);

    std::unordered_set<int> unordered_set1 = {50, 60, 70, 80};
    std::unordered_set<std::string> unordered_set2 = {"50", "60", "70", "80"};
    json = unordered_set1;
    EXPECT_EQ((std::unordered_set<int>(json)), unordered_set1);
    EXPECT_EQ((std::unordered_set<std::string>(json)), unordered_set2);
    json = unordered_set2;
    EXPECT_EQ((std::unordered_set<int>(json)), unordered_set1);
    EXPECT_EQ((std::unordered_set<std::string>(json)), unordered_set2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::unordered_set<int>(json)), unordered_set1);
    EXPECT_EQ((std::unordered_set<std::string>(json)), unordered_set2);
    json = {true};
    validate_throw<std::unordered_set<int>>(json);
    json = {"string"};
    validate_throw<std::unordered_set<int>>(json);

    std::vector<int> vector1 = {50, 60, 70, 80};
    std::vector<std::string> vector2 = {"50", "60", "70", "80"};
    json = vector1;
    EXPECT_EQ((std::vector<int>(json)), vector1);
    EXPECT_EQ((std::vector<std::string>(json)), vector2);
    json = vector2;
    EXPECT_EQ((std::vector<int>(json)), vector1);
    EXPECT_EQ((std::vector<std::string>(json)), vector2);
    json = {50, "60", 70, "80"};
    EXPECT_EQ((std::vector<int>(json)), vector1);
    EXPECT_EQ((std::vector<std::string>(json)), vector2);
    json = {true};
    validate_throw<std::vector<int>>(json);
    json = {"string"};
    validate_throw<std::vector<int>>(json);

    json = true;
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);

    json = 1;
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);

    json = static_cast<unsigned int>(1);
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);

    json = 1.0f;
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);

    json = nullptr;
    validate_throw<std::array<int, 1>>(json);
    validate_throw<std::deque<int>>(json);
    validate_throw<std::forward_list<int>>(json);
    validate_throw<std::list<int>>(json);
    validate_throw<std::multiset<int>>(json);
    validate_throw<std::set<int>>(json);
    validate_throw<std::unordered_multiset<int>>(json);
    validate_throw<std::unordered_set<int>>(json);
    validate_throw<std::vector<int>>(json);
}

//==============================================================================
TEST_F(JsonTest, BooleanConversion)
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

//==============================================================================
TEST_F(JsonTest, SignedIntegerConversion)
{
    fly::Json json;

    json = "abc";
    validate_throw<int>(json);

    json = "123";
    EXPECT_EQ(int(json), 123);

    json = {{"a", 1}, {"b", 2}};
    validate_throw<int>(json);

    json = {7, 8};
    validate_throw<int>(json);

    json = true;
    validate_throw<int>(json);

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
    validate_throw<int>(json);
}

//==============================================================================
TEST_F(JsonTest, UnsignedIntegerConversion)
{
    fly::Json json;

    json = "abc";
    validate_throw<unsigned>(json);

    json = "123";
    EXPECT_EQ(unsigned(json), unsigned(123));

    json = {{"a", 1}, {"b", 2}};
    validate_throw<unsigned>(json);

    json = {7, 8};
    validate_throw<unsigned>(json);

    json = true;
    validate_throw<unsigned>(json);

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
    validate_throw<unsigned>(json);
}

//==============================================================================
TEST_F(JsonTest, FloatConversion)
{
    fly::Json json;

    json = "abc";
    validate_throw<float>(json);

    json = "123.5";
    EXPECT_EQ(float(json), 123.5f);

    json = {{"a", 1}, {"b", 2}};
    validate_throw<float>(json);

    json = {7, 8};
    validate_throw<float>(json);

    json = true;
    validate_throw<float>(json);

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
    validate_throw<float>(json);
}

//==============================================================================
TEST_F(JsonTest, NullConversion)
{
    fly::Json json;

    json = "abc";
    validate_throw<std::nullptr_t>(json);

    json = {{"a", 1}, {"b", 2}};
    validate_throw<std::nullptr_t>(json);

    json = {7, 8};
    validate_throw<std::nullptr_t>(json);

    json = true;
    validate_throw<std::nullptr_t>(json);

    json = 'a';
    validate_throw<std::nullptr_t>(json);

    json = 12;
    validate_throw<std::nullptr_t>(json);

    json = static_cast<unsigned int>(12);
    validate_throw<std::nullptr_t>(json);

    json = 3.14f;
    validate_throw<std::nullptr_t>(json);

    json = nullptr;
    EXPECT_EQ((std::nullptr_t(json)), nullptr);
}

//==============================================================================
TEST_F(JsonTest, ObjectAccess)
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

//==============================================================================
TEST_F(JsonTest, ObjectAt)
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

//==============================================================================
TEST_F(JsonTest, ArrayAccess)
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

//==============================================================================
TEST_F(JsonTest, ArrayAt)
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

//==============================================================================
TEST_F(JsonTest, Empty)
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

//==============================================================================
TEST_F(JsonTest, Size)
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

//==============================================================================
TEST_F(JsonTest, Clear)
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

//==============================================================================
TEST_F(JsonTest, JsonSwap)
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

//==============================================================================
TEST_F(JsonTest, StringSwap)
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

//==============================================================================
TEST_F(JsonTest, ObjectSwap)
{
    fly::Json json;
    std::map<fly::JsonTraits::string_type, fly::Json> map;

    json = "abcdef";
    EXPECT_THROW(json.swap(map), fly::JsonException);

    json = {{"a", 1}, {"b", 2}};
    map = {{"c", 3}, {"d", 4}};
    EXPECT_NO_THROW(json.swap(map));
    EXPECT_EQ(json, fly::Json({{"c", 3}, {"d", 4}}));
    EXPECT_EQ(map, fly::Json({{"a", 1}, {"b", 2}}));

    json = {'7', 8, 9, 10};
    EXPECT_THROW(json.swap(map), fly::JsonException);

    json = true;
    EXPECT_THROW(json.swap(map), fly::JsonException);

    json = 1;
    EXPECT_THROW(json.swap(map), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW(json.swap(map), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW(json.swap(map), fly::JsonException);

    json = nullptr;
    EXPECT_THROW(json.swap(map), fly::JsonException);
}

//==============================================================================
TEST_F(JsonTest, ArraySwap)
{
    fly::Json json;
    std::vector<fly::Json> array;

    json = "abcdef";
    EXPECT_THROW(json.swap(array), fly::JsonException);

    json = {{"a", 1}, {"b", 2}};
    EXPECT_THROW(json.swap(array), fly::JsonException);

    json = {'7', 8, 9, 10};
    array = {1, true, nullptr};
    EXPECT_NO_THROW(json.swap(array));
    EXPECT_EQ(json, fly::Json({1, true, nullptr}));
    EXPECT_EQ(array, fly::Json({'7', 8, 9, 10}));

    json = true;
    EXPECT_THROW(json.swap(array), fly::JsonException);

    json = 1;
    EXPECT_THROW(json.swap(array), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW(json.swap(array), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW(json.swap(array), fly::JsonException);

    json = nullptr;
    EXPECT_THROW(json.swap(array), fly::JsonException);
}

//==============================================================================
TEST_F(JsonTest, Equality)
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

//==============================================================================
TEST_F(JsonTest, Stream)
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

//==============================================================================
TEST_F(JsonTest, StreamWithEscapedSymbols)
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
}

//==============================================================================
TEST_F(JsonTest, UnicodeConversion)
{
    validate_fail("\\u");
    validate_fail("\\u0");
    validate_fail("\\u00");
    validate_fail("\\u000");
    validate_fail("\\u000z");

    validate_pass("\\u0040", utf8("\u0040"));
    validate_pass("\\u007A", utf8("\u007A"));
    validate_pass("\\u007a", utf8("\u007a"));
    validate_pass("\\u00c4", utf8("\u00c4"));
    validate_pass("\\u00e4", utf8("\u00e4"));
    validate_pass("\\u0298", utf8("\u0298"));
    validate_pass("\\u0800", utf8("\u0800"));
    validate_pass("\\uffff", utf8("\uffff"));

    validate_fail("\\uDC00");
    validate_fail("\\uDFFF");
    validate_fail("\\uD800");
    validate_fail("\\uDBFF");
    validate_fail("\\uD800\\u");
    validate_fail("\\uD800\\z");
    validate_fail("\\uD800\\u0");
    validate_fail("\\uD800\\u00");
    validate_fail("\\uD800\\u000");
    validate_fail("\\uD800\\u0000");
    validate_fail("\\uD800\\u000z");
    validate_fail("\\uD800\\uDBFF");
    validate_fail("\\uD800\\uE000");
    validate_fail("\\uD800\\uFFFF");

    validate_pass("\\uD800\\uDC00", utf8("\U00010000"));
    validate_pass("\\uD803\\uDE6D", utf8("\U00010E6D"));
    validate_pass("\\uD834\\uDD1E", utf8("\U0001D11E"));
    validate_pass("\\uDBFF\\uDFFF", utf8("\U0010FFFF"));
}

//==============================================================================
TEST_F(JsonTest, MarkusKuhnStress)
{
    // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

    // 1  Some correct UTF-8 text
    validate_pass("κόσμε");

    // 2  Boundary condition test cases

    // 2.1  First possible sequence of a certain length

    // 2.1.1  1 byte  (U-00000001)
    validate_fail("\x01");

    // 2.1.2  2 bytes (U-00000080)
    validate_pass("\xc2\x80");

    // 2.1.3  3 bytes (U-00000800)
    validate_pass("\xe0\xa0\x80");

    // 2.1.4  4 bytes (U-00010000)
    validate_pass("\xf0\x90\x80\x80");

    // 2.1.5  5 bytes (U-00200000)
    validate_fail("\xf8\x88\x80\x80\x80");

    // 2.1.6  6 bytes (U-04000000)
    validate_fail("\xfc\x84\x80\x80\x80\x80");

    // 2.2  Last possible sequence of a certain length

    // 2.2.1  1 byte  (U-0000007F)
    validate_pass("\x7f");

    // 2.2.2  2 bytes (U-000007FF)
    validate_pass("\xdf\xbf");

    // 2.2.3  3 bytes (U-0000FFFF)
    validate_pass("\xef\xbf\xbf");

    // 2.1.4  4 bytes (U-00200000)
    validate_fail("\xf7\xbf\xbf\xbf");

    // 2.1.5  5 bytes (U-03FFFFFF)
    validate_fail("\xfb\xbf\xbf\xbf\xbf");

    // 2.1.6  6 bytes (U-7FFFFFFF)
    validate_fail("\xfd\xbf\xbf\xbf\xbf\xbf");

    // 2.3  Other boundary conditions

    // 2.3.1  U-0000D7FF
    validate_pass("\xed\x9f\xbf");

    // 2.3.2  U-0000E000
    validate_pass("\xee\x80\x80");

    // 2.3.3  U-0000FFFD
    validate_pass("\xef\xbf\xbd");

    // 2.3.4  U-0010FFFF
    validate_pass("\xf4\x8f\xbf\xbf");

    // 2.3.5  U-00110000
    validate_fail("\xf4\x90\x80\x80");

    // 3  Malformed sequences

    // 3.1  Unexpected continuation bytes

    // 3.1.1  First continuation byte 0x80
    validate_fail("\x80");

    // 3.1.2 Last  continuation byte 0xbf
    validate_fail("\xbf");

    // 3.1.3  2 continuation bytes
    validate_fail("\x80\xbf");

    // 3.1.4  3 continuation bytes
    validate_fail("\x80\xbf\x80");

    // 3.1.5  4 continuation bytes
    validate_fail("\x80\xbf\x80\xbf");

    // 3.1.6  5 continuation bytes
    validate_fail("\x80\xbf\x80\xbf\x80");

    // 3.1.7  6 continuation bytes
    validate_fail("\x80\xbf\x80\xbf\x80\xbf");

    // 3.1.8  7 continuation bytes
    validate_fail("\x80\xbf\x80\xbf\x80\xbf\x80");

    // 3.1.9  Sequence of all 64 possible continuation bytes (0x80-0xbf)
    validate_fail(
        "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90"
        "\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1"
        "\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2"
        "\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf");

    // 3.2  Lonely start characters

    // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf), each followed
    // by a space character
    validate_fail(
        "\xc0 \xc1 \xc2 \xc3 \xc4 \xc5 \xc6 \xc7 \xc8 \xc9 \xca \xcb \xcc \xcd "
        "\xce \xcf \xd0 \xd1 \xd2 \xd3 \xd4 \xd5 \xd6 \xd7 \xd8 \xd9 \xda \xdb "
        "\xdc \xdd \xde \xdf");
    validate_fail("\xc0 ");
    validate_fail("\xc1 ");
    validate_fail("\xc2 ");
    validate_fail("\xc3 ");
    validate_fail("\xc4 ");
    validate_fail("\xc5 ");
    validate_fail("\xc6 ");
    validate_fail("\xc7 ");
    validate_fail("\xc8 ");
    validate_fail("\xc9 ");
    validate_fail("\xca ");
    validate_fail("\xcb ");
    validate_fail("\xcc ");
    validate_fail("\xcd ");
    validate_fail("\xce ");
    validate_fail("\xcf ");
    validate_fail("\xd0 ");
    validate_fail("\xd1 ");
    validate_fail("\xd2 ");
    validate_fail("\xd3 ");
    validate_fail("\xd4 ");
    validate_fail("\xd5 ");
    validate_fail("\xd6 ");
    validate_fail("\xd7 ");
    validate_fail("\xd8 ");
    validate_fail("\xd9 ");
    validate_fail("\xda ");
    validate_fail("\xdb ");
    validate_fail("\xdc ");
    validate_fail("\xdd ");
    validate_fail("\xde ");
    validate_fail("\xdf ");

    // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef) each followed
    // by a space character
    validate_fail(
        "\xe0 \xe1 \xe2 \xe3 \xe4 \xe5 \xe6 \xe7 \xe8 \xe9 \xea \xeb \xec \xed "
        "\xee \xef");
    validate_fail("\xe0 ");
    validate_fail("\xe1 ");
    validate_fail("\xe2 ");
    validate_fail("\xe3 ");
    validate_fail("\xe4 ");
    validate_fail("\xe5 ");
    validate_fail("\xe6 ");
    validate_fail("\xe7 ");
    validate_fail("\xe8 ");
    validate_fail("\xe9 ");
    validate_fail("\xea ");
    validate_fail("\xeb ");
    validate_fail("\xec ");
    validate_fail("\xed ");
    validate_fail("\xee ");
    validate_fail("\xef ");

    // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7), each followed
    // by a space character
    validate_fail("\xf0 \xf1 \xf2 \xf3 \xf4 \xf5 \xf6 \xf7");
    validate_fail("\xf0 ");
    validate_fail("\xf1 ");
    validate_fail("\xf2 ");
    validate_fail("\xf3 ");
    validate_fail("\xf4 ");
    validate_fail("\xf5 ");
    validate_fail("\xf6 ");
    validate_fail("\xf7 ");

    // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb), each followed
    // by a space character
    validate_fail("\xf8 \xf9 \xfa \xfb");
    validate_fail("\xf8 ");
    validate_fail("\xf9 ");
    validate_fail("\xfa ");
    validate_fail("\xfb ");

    // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd), each followed
    // by a space character
    validate_fail("\xfc \xfd");
    validate_fail("\xfc ");
    validate_fail("\xfc ");

    // 3.3  Sequences with last continuation byte missing

    // 3.3.1  2-byte sequence with last byte missing (U+0000)
    validate_fail("\xc0");

    // 3.3.2  3-byte sequence with last byte missing (U+0000)
    validate_fail("\xe0\x80");

    // 3.3.3  4-byte sequence with last byte missing (U+0000)
    validate_fail("\xf0\x80\x80");

    // 3.3.4  5-byte sequence with last byte missing (U+0000)
    validate_fail("\xf8\x80\x80\x80");

    // 3.3.5  6-byte sequence with last byte missing (U+0000)
    validate_fail("\xfc\x80\x80\x80\x80");

    // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
    validate_fail("\xdf");

    // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
    validate_fail("\xef\xbf");

    // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
    validate_fail("\xf7\xbf\xbf");

    // 3.3.9  5-byte sequence with last byte missing (U-03FFFFFF)
    validate_fail("\xfb\xbf\xbf\xbf");

    // 3.3.10 6-byte sequence with last byte missing (U-7FFFFFFF)
    validate_fail("\xfd\xbf\xbf\xbf\xbf");

    // 3.4  Concatenation of incomplete sequences

    // All the 10 sequences of 3.3 concatenated
    validate_fail(
        "\xc0\xe0\x80\xf0\x80\x80\xf8\x80\x80\x80\xfc\x80\x80\x80\x80\xdf\xef"
        "\xbf\xf7\xbf\xbf\xfb\xbf\xbf\xbf\xfd\xbf\xbf\xbf\xbf");

    // 3.5  Impossible bytes

    // 3.5.1  fe
    validate_fail("\xfe");

    // 3.5.2  ff
    validate_fail("\xff");

    // 3.5.3  fe fe ff ff
    validate_fail("\xfe\xfe\xff\xff");

    // 4  Overlong sequences

    // 4.1  Examples of an overlong ASCII character

    // 4.1.1 U+002F = c0 af
    validate_fail("\xc0\xaf");

    // 4.1.2 U+002F = e0 80 af
    validate_fail("\xe0\x80\xaf");

    // 4.1.3 U+002F = f0 80 80 af
    validate_fail("\xf0\x80\x80\xaf");

    // 4.1.4 U+002F = f8 80 80 80 af
    validate_fail("\xf8\x80\x80\x80\xaf");

    // 4.1.5 U+002F = fc 80 80 80 80 af
    validate_fail("\xfc\x80\x80\x80\x80\xaf");

    // 4.2  Maximum overlong sequences

    // 4.2.1  U-0000007F = c1 bf
    validate_fail("\xc1\xbf");

    // 4.2.2  U-000007FF = e0 9f bf
    validate_fail("\xe0\x9f\xbf");

    // 4.2.3  U-0000FFFF = f0 8f bf bf
    validate_fail("\xf0\x8f\xbf\xbf");

    // 4.2.4  U-001FFFFF = f8 87 bf bf bf
    validate_fail("\xf8\x87\xbf\xbf\xbf");

    // 4.2.5  U-03FFFFFF = fc 83 bf bf bf bf
    validate_fail("\xfc\x83\xbf\xbf\xbf\xbf");

    // 4.3  Overlong representation of the NUL character

    // 4.3.1  U+0000 = c0 80
    validate_fail("\xc0\x80");

    // 4.3.2  U+0000 = e0 80 80
    validate_fail("\xe0\x80\x80");

    // 4.3.3  U+0000 = f0 80 80 80
    validate_fail("\xf0\x80\x80\x80");

    // 4.3.4  U+0000 = f8 80 80 80 80
    validate_fail("\xf8\x80\x80\x80\x80");

    // 4.3.5  U+0000 = fc 80 80 80 80 80
    validate_fail("\xfc\x80\x80\x80\x80\x80");

    // 5  Illegal code positions

    // 5.1 Single UTF-16 surrogates

    // 5.1.1  U+D800 = ed a0 80
    validate_fail("\xed\xa0\x80");

    // 5.1.2  U+DB7F = ed ad bf
    validate_fail("\xed\xad\xbf");

    // 5.1.3  U+DB80 = ed ae 80
    validate_fail("\xed\xae\x80");

    // 5.1.4  U+DBFF = ed af bf
    validate_fail("\xed\xaf\xbf");

    // 5.1.5  U+DC00 = ed b0 80
    validate_fail("\xed\xb0\x80");

    // 5.1.6  U+DF80 = ed be 80
    validate_fail("\xed\xbe\x80");

    // 5.1.7  U+DFFF = ed bf bf
    validate_fail("\xed\xbf\xbf");

    // 5.2 Paired UTF-16 surrogates

    // 5.2.1  U+D800 U+DC00 = ed a0 80 ed b0 80
    validate_fail("\xed\xa0\x80\xed\xb0\x80");

    // 5.2.2  U+D800 U+DFFF = ed a0 80 ed bf bf
    validate_fail("\xed\xa0\x80\xed\xbf\xbf");

    // 5.2.3  U+DB7F U+DC00 = ed ad bf ed b0 80
    validate_fail("\xed\xad\xbf\xed\xb0\x80");

    // 5.2.4  U+DB7F U+DFFF = ed ad bf ed bf bf
    validate_fail("\xed\xad\xbf\xed\xbf\xbf");

    // 5.2.5  U+DB80 U+DC00 = ed ae 80 ed b0 80
    validate_fail("\xed\xae\x80\xed\xb0\x80");

    // 5.2.6  U+DB80 U+DFFF = ed ae 80 ed bf bf
    validate_fail("\xed\xae\x80\xed\xbf\xbf");

    // 5.2.7  U+DBFF U+DC00 = ed af bf ed b0 80
    validate_fail("\xed\xaf\xbf\xed\xb0\x80");

    // 5.2.8  U+DBFF U+DFFF = ed af bf ed bf bf
    validate_fail("\xed\xaf\xbf\xed\xbf\xbf");

    // 5.3 Noncharacter code positions

    // 5.3.1  U+FFFE = ef bf be
    validate_pass("\xef\xbf\xbe");

    // 5.3.2  U+FFFF = ef bf bf
    validate_pass("\xef\xbf\xbf");

    // 5.3.3  U+FDD0 .. U+FDEF
    validate_pass("\xef\xb7\x90");
    validate_pass("\xef\xb7\x91");
    validate_pass("\xef\xb7\x92");
    validate_pass("\xef\xb7\x93");
    validate_pass("\xef\xb7\x94");
    validate_pass("\xef\xb7\x95");
    validate_pass("\xef\xb7\x96");
    validate_pass("\xef\xb7\x97");
    validate_pass("\xef\xb7\x98");
    validate_pass("\xef\xb7\x99");
    validate_pass("\xef\xb7\x9a");
    validate_pass("\xef\xb7\x9b");
    validate_pass("\xef\xb7\x9c");
    validate_pass("\xef\xb7\x9d");
    validate_pass("\xef\xb7\x9e");
    validate_pass("\xef\xb7\x9f");
    validate_pass("\xef\xb7\xa0");
    validate_pass("\xef\xb7\xa1");
    validate_pass("\xef\xb7\xa2");
    validate_pass("\xef\xb7\xa3");
    validate_pass("\xef\xb7\xa4");
    validate_pass("\xef\xb7\xa5");
    validate_pass("\xef\xb7\xa6");
    validate_pass("\xef\xb7\xa7");
    validate_pass("\xef\xb7\xa8");
    validate_pass("\xef\xb7\xa9");
    validate_pass("\xef\xb7\xaa");
    validate_pass("\xef\xb7\xab");
    validate_pass("\xef\xb7\xac");
    validate_pass("\xef\xb7\xad");
    validate_pass("\xef\xb7\xae");
    validate_pass("\xef\xb7\xaf");

    // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
    validate_pass("\xf0\x9f\xbf\xbf");
    validate_pass("\xf0\xaf\xbf\xbf");
    validate_pass("\xf0\xbf\xbf\xbf");
    validate_pass("\xf1\x8f\xbf\xbf");
    validate_pass("\xf1\x9f\xbf\xbf");
    validate_pass("\xf1\xaf\xbf\xbf");
    validate_pass("\xf1\xbf\xbf\xbf");
    validate_pass("\xf2\x8f\xbf\xbf");
    validate_pass("\xf2\x9f\xbf\xbf");
    validate_pass("\xf2\xaf\xbf\xbf");
}

//==============================================================================
TEST_F(JsonTest, MarkusKuhnExtended)
{
    // Exceptions not caught by Markus Kuhn's stress test
    validate_fail("\x22");
    validate_fail("\x5c");

    validate_fail("\xe0\xa0\x79");
    validate_fail("\xe0\xa0\xff");

    validate_fail("\xed\x80\x79");
    validate_fail("\xed\x80\xff");

    validate_fail("\xf0\x90\x79");
    validate_fail("\xf0\x90\xff");
    validate_fail("\xf0\x90\x80\x79");
    validate_fail("\xf0\x90\x80\xff");

    validate_fail("\xf1\x80\x79");
    validate_fail("\xf1\x80\xff");
    validate_fail("\xf1\x80\x80\x79");
    validate_fail("\xf1\x80\x80\xff");

    validate_fail("\xf4\x80\x79");
    validate_fail("\xf4\x80\xff");
    validate_fail("\xf4\x80\x80\x79");
    validate_fail("\xf4\x80\x80\xff");
}
