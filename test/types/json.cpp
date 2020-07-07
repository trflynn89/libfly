#include "fly/types/json/json.hpp"

#include <catch2/catch.hpp>

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

#define CHECK_THROWS_JSON(expression, ...)                                                         \
    CHECK_THROWS_MATCHES(                                                                          \
        expression,                                                                                \
        fly::JsonException,                                                                        \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("JsonException: " __VA_ARGS__)))

TEST_CASE("Json", "[json]")
{
    SECTION("Construct a JSON instance from string-like types")
    {
        const std::string str1("a");
        CHECK(fly::Json(str1).is_string());

        std::string str2("b");
        CHECK(fly::Json(str2).is_string());

        const char *cstr1 = "c";
        CHECK(fly::Json(cstr1).is_string());

        char *cstr2 = const_cast<char *>("d");
        CHECK(fly::Json(cstr2).is_string());

        const char arr1[] = {'g', '\0'};
        CHECK(fly::Json(arr1).is_string());

        char arr2[] = {'h', '\0'};
        CHECK(fly::Json(arr2).is_string());
    }

    SECTION("Fail to construct a JSON instance from string-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CHECK_THROWS_JSON(fly::Json("\\"), "Expected escaped character after reverse solidus");
        CHECK_THROWS_JSON(fly::Json("\\U"), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CHECK_THROWS_JSON(fly::Json("\""), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
        {
            CHECK_THROWS_JSON(
                fly::Json(fly::JsonTraits::string_type(1, ch)),
                "Character '%c' must be escaped",
                ch);
        }

        // Characters must be valid Unicode.
        CHECK_THROWS_JSON(
            fly::Json("\xed\xa0\x80"), // Reserved codepoint.
            "Could not decode Unicode character");
        CHECK_THROWS_JSON(
            fly::Json("\xf4\x90\x80\x80"), // Out-of-range codepoint.
            "Could not decode Unicode character");
    }

    SECTION("Construct a JSON instance from object-like types")
    {
        std::map<std::string, int> map = {{"a", 1}, {"b", 2}};
        CHECK(fly::Json(map).is_object());

        std::multimap<std::string, int> multimap = {{"c", 3}, {"d", 4}};
        CHECK(fly::Json(multimap).is_object());

        std::unordered_map<std::string, int> umap = {{"e", 5}, {"f", 6}};
        CHECK(fly::Json(umap).is_object());

        std::unordered_multimap<std::string, int> umultimap = {{"h", 7}, {"i", 8}};
        CHECK(fly::Json(umultimap).is_object());
    }

    SECTION("Fail to construct a JSON instance from object-like types")
    {
        std::map<std::string, int> map;

        // Reverse solidus must be followed by a valid escape symbol.
        map = {{"\\", 1}};
        CHECK_THROWS_JSON(fly::Json(map), "Expected escaped character after reverse solidus");

        map = {{"\\U", 1}};
        CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

        // Quotes must be escaped.
        map = {{"\"", 1}};
        CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
        {
            map = {{fly::JsonTraits::string_type(1, ch), 1}};
            CHECK_THROWS_JSON(fly::Json(map), "Character '%c' must be escaped", ch);
        }

        // Characters must be valid Unicode.
        map = {{"\xed\xa0\x80", 1}}; // Reserved codepoint.
        CHECK_THROWS_JSON(fly::Json(map), "Could not decode Unicode character");

        map = {{"\xf4\x90\x80\x80", 1}}; // Out-of-range codepoint.
        CHECK_THROWS_JSON(fly::Json(map), "Could not decode Unicode character");
    }

    SECTION("Construct a JSON instance from array-like types")
    {
        std::array<int, 4> array = {10, 20, 30, 40};
        CHECK(fly::Json(array).is_array());
        CHECK_FALSE(fly::Json(array).is_object_like());

        std::deque<int> deque = {50, 60, 70, 80};
        CHECK(fly::Json(deque).is_array());
        CHECK_FALSE(fly::Json(deque).is_object_like());

        std::forward_list<int> forward_list = {90, 100, 110, 120};
        CHECK(fly::Json(forward_list).is_array());
        CHECK_FALSE(fly::Json(forward_list).is_object_like());

        std::list<int> list = {130, 140, 150, 160};
        CHECK(fly::Json(list).is_array());
        CHECK_FALSE(fly::Json(list).is_object_like());

        std::multiset<std::string> multiset = {"a", "b", "c"};
        CHECK(fly::Json(multiset).is_array());
        CHECK_FALSE(fly::Json(multiset).is_object_like());

        std::set<std::string> set = {"d", "e", "f"};
        CHECK(fly::Json(set).is_array());
        CHECK_FALSE(fly::Json(set).is_object_like());

        std::unordered_multiset<std::string> unordered_multiset = {"g", "h", "i"};
        CHECK(fly::Json(unordered_multiset).is_array());
        CHECK_FALSE(fly::Json(unordered_multiset).is_object_like());

        std::unordered_set<std::string> unordered_set = {"j", "k", "l"};
        CHECK(fly::Json(unordered_set).is_array());
        CHECK_FALSE(fly::Json(unordered_set).is_object_like());

        std::vector<int> vector = {170, 180, 190, 200};
        CHECK(fly::Json(vector).is_array());
        CHECK_FALSE(fly::Json(vector).is_object_like());

        std::array<std::string, 2> object = {"nine", "ten"};
        CHECK(fly::Json(object).is_array());
        CHECK(fly::Json(object).is_object_like());
    }

    SECTION("Fail to construct a JSON instance from array-like types")
    {
        std::vector<std::string> vector;

        // Reverse solidus must be followed by a valid escape symbol.
        vector = {"\\"};
        CHECK_THROWS_JSON(fly::Json(vector), "Expected escaped character after reverse solidus");

        vector = {"\\U"};
        CHECK_THROWS_JSON(fly::Json(vector), "Invalid escape character 'U'");

        // Quotes must be escaped.
        vector = {"\""};
        CHECK_THROWS_JSON(fly::Json(vector), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
        {
            vector = {fly::JsonTraits::string_type(1, ch)};
            CHECK_THROWS_JSON(fly::Json(vector), "Character '%c' must be escaped", ch);
        }

        // Characters must be valid Unicode.
        vector = {"\xed\xa0\x80"}; // Reserved codepoint.
        CHECK_THROWS_JSON(fly::Json(vector), "Could not decode Unicode character");

        vector = {"\xf4\x90\x80\x80"}; // Out-of-range codepoint.
        CHECK_THROWS_JSON(fly::Json(vector), "Could not decode Unicode character");
    }

    SECTION("Construct a JSON instance from Boolean-like types")
    {
        CHECK(fly::Json(true).is_boolean());
        CHECK(fly::Json(false).is_boolean());
    }

    SECTION("Construct a JSON instance from signed-integer-like types")
    {
        CHECK(fly::Json(static_cast<char>(1)).is_signed_integer());

        CHECK(fly::Json(static_cast<short>(1)).is_signed_integer());

        CHECK(fly::Json(static_cast<int>(1)).is_signed_integer());
        CHECK(fly::Json(static_cast<int>(-1)).is_signed_integer());

        CHECK(fly::Json(static_cast<std::int32_t>(1)).is_signed_integer());
        CHECK(fly::Json(static_cast<std::int32_t>(-1)).is_signed_integer());

        CHECK(fly::Json(static_cast<std::int64_t>(1)).is_signed_integer());
        CHECK(fly::Json(static_cast<std::int64_t>(-1)).is_signed_integer());
    }

    SECTION("Construct a JSON instance from unsigned-integer-like types")
    {
        CHECK(fly::Json(static_cast<unsigned char>(1)).is_unsigned_integer());

        CHECK(fly::Json(static_cast<unsigned short>(1)).is_unsigned_integer());

        CHECK(fly::Json(static_cast<unsigned int>(1)).is_unsigned_integer());
        CHECK(fly::Json(static_cast<unsigned int>(-1)).is_unsigned_integer());

        CHECK(fly::Json(static_cast<std::uint32_t>(1)).is_unsigned_integer());
        CHECK(fly::Json(static_cast<std::uint32_t>(-1)).is_unsigned_integer());

        CHECK(fly::Json(static_cast<std::uint64_t>(1)).is_unsigned_integer());
        CHECK(fly::Json(static_cast<std::uint64_t>(-1)).is_unsigned_integer());
    }

    SECTION("Construct a JSON instance from floating-point-like types")
    {
        CHECK(fly::Json(static_cast<float>(1.0)).is_float());
        CHECK(fly::Json(static_cast<double>(1.0)).is_float());
        CHECK(fly::Json(static_cast<long double>(1.0)).is_float());
    }

    SECTION("Construct a JSON instance from null-like types")
    {
        CHECK(fly::Json().is_null());
        CHECK(fly::Json(nullptr).is_null());
    }

    SECTION("Construct a JSON instance from initializer lists")
    {
        const fly::Json empty {};
        CHECK(fly::Json(empty).is_null());

        const fly::Json array {'7', 8, "nine", 10};
        CHECK(fly::Json(array).is_array());

        const fly::Json object {{"a", 1}, {"b", 2}};
        CHECK(fly::Json(object).is_object());

        const fly::Json almost {{"a", 1}, {"b", 2}, 4};
        CHECK(fly::Json(almost).is_array());
    }

    SECTION("Construct a JSON instance from another JSON instance via copy")
    {
        fly::Json string = "abc";
        CHECK(fly::Json(string) == string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        CHECK(fly::Json(object) == object);

        fly::Json array = {'7', 8};
        CHECK(fly::Json(array) == array);

        fly::Json boolean = true;
        CHECK(fly::Json(boolean) == boolean);

        fly::Json sign = 1;
        CHECK(fly::Json(sign) == sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        CHECK(fly::Json(unsign) == unsign);

        fly::Json floating = 1.0f;
        CHECK(fly::Json(floating) == floating);

        fly::Json null = nullptr;
        CHECK(fly::Json(null) == null);
    }

    SECTION("Construct a JSON instance from another JSON instance via move semantics")
    {
        fly::Json string = "abc";
        fly::Json string_copy(string);
        fly::Json string_move(std::move(string_copy));

        CHECK(string_copy.is_null());
        CHECK(string_move == string);
    }

    SECTION("Assign a JSON instance's value to another JSON instance's value")
    {
        fly::Json json;

        fly::Json string = "abc";
        json = string;
        CHECK(json == string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        json = object;
        CHECK(json == object);

        fly::Json array = {'7', 8};
        json = array;
        CHECK(json == array);

        fly::Json boolean = true;
        json = boolean;
        CHECK(json == boolean);

        fly::Json sign = 1;
        json = sign;
        CHECK(json == sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        json = unsign;
        CHECK(json == unsign);

        fly::Json floating = 1.0f;
        json = floating;
        CHECK(json == floating);

        fly::Json null = nullptr;
        json = null;
        CHECK(json == null);
    }

    SECTION("Access a JSON object's values via the access operator")
    {
        fly::Json string1 = "abc";
        CHECK_THROWS_JSON(string1["a"], "JSON type invalid for operator[key]: (%s)", string1);

        const fly::Json string2 = "abc";
        CHECK_THROWS_JSON(string2["a"], "JSON type invalid for operator[key]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CHECK(object1["a"] == 1);
        CHECK(object1["b"] == 2);
        CHECK_NOTHROW(object1["c"]);
        CHECK(object1["c"] == nullptr);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CHECK(object2["a"] == 1);
        CHECK(object2["b"] == 2);
        CHECK_THROWS_JSON(object2["c"], "Given key (c) not found: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CHECK_THROWS_JSON(array1["a"], "JSON type invalid for operator[key]: (%s)", array1);

        const fly::Json array2 = {'7', 8};
        CHECK_THROWS_JSON(array2["a"], "JSON type invalid for operator[key]: (%s)", array2);

        fly::Json bool1 = true;
        CHECK_THROWS_JSON(bool1["a"], "JSON type invalid for operator[key]: (%s)", bool1);

        const fly::Json bool2 = true;
        CHECK_THROWS_JSON(bool2["a"], "JSON type invalid for operator[key]: (%s)", bool2);

        fly::Json signed1 = 1;
        CHECK_THROWS_JSON(signed1["a"], "JSON type invalid for operator[key]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CHECK_THROWS_JSON(signed2["a"], "JSON type invalid for operator[key]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(unsigned1["a"], "JSON type invalid for operator[key]: (%s)", unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(unsigned2["a"], "JSON type invalid for operator[key]: (%s)", unsigned2);

        fly::Json float1 = 1.0f;
        CHECK_THROWS_JSON(float1["a"], "JSON type invalid for operator[key]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CHECK_THROWS_JSON(float2["a"], "JSON type invalid for operator[key]: (%s)", float2);

        fly::Json null1 = nullptr;
        CHECK_NOTHROW(null1["a"]);
        CHECK(null1.is_object());
        CHECK(null1["a"] == nullptr);

        const fly::Json null2 = nullptr;
        CHECK_THROWS_JSON(null2["a"], "JSON type invalid for operator[key]: (%s)", null2);
    }

    SECTION("Access a JSON object's values via the accessor 'at'")
    {
        fly::Json string1 = "abc";
        CHECK_THROWS_JSON(string1.at("a"), "JSON type invalid for operator[key]: (%s)", string1);

        const fly::Json string2 = "abc";
        CHECK_THROWS_JSON(string2.at("a"), "JSON type invalid for operator[key]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CHECK(object1.at("a") == 1);
        CHECK(object1.at("b") == 2);
        CHECK_THROWS_JSON(object1.at("c"), "Given key (c) not found: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CHECK(object2.at("a") == 1);
        CHECK(object2.at("b") == 2);
        CHECK_THROWS_JSON(object2.at("c"), "Given key (c) not found: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CHECK_THROWS_JSON(array1.at("a"), "JSON type invalid for operator[key]: (%s)", array1);

        const fly::Json array2 = {'7', 8};
        CHECK_THROWS_JSON(array2.at("a"), "JSON type invalid for operator[key]: (%s)", array2);

        fly::Json bool1 = true;
        CHECK_THROWS_JSON(bool1.at("a"), "JSON type invalid for operator[key]: (%s)", bool1);

        const fly::Json bool2 = true;
        CHECK_THROWS_JSON(bool2.at("a"), "JSON type invalid for operator[key]: (%s)", bool2);

        fly::Json signed1 = 1;
        CHECK_THROWS_JSON(signed1.at("a"), "JSON type invalid for operator[key]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CHECK_THROWS_JSON(signed2.at("a"), "JSON type invalid for operator[key]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(
            unsigned1.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(
            unsigned2.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CHECK_THROWS_JSON(float1.at("a"), "JSON type invalid for operator[key]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CHECK_THROWS_JSON(float2.at("a"), "JSON type invalid for operator[key]: (%s)", float2);

        fly::Json null1 = nullptr;
        CHECK_THROWS_JSON(null1.at("a"), "JSON type invalid for operator[key]: (%s)", null1);

        const fly::Json null2 = nullptr;
        CHECK_THROWS_JSON(null2.at("a"), "JSON type invalid for operator[key]: (%s)", null2);
    }

    SECTION("Access a JSON array's values via the access operator")
    {
        fly::Json string1 = "abc";
        CHECK_THROWS_JSON(string1[0], "JSON type invalid for operator[index]: (%s)", string1);

        const fly::Json string2 = "abc";
        CHECK_THROWS_JSON(string2[0], "JSON type invalid for operator[index]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CHECK_THROWS_JSON(object1[0], "JSON type invalid for operator[index]: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CHECK_THROWS_JSON(object2[0], "JSON type invalid for operator[index]: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CHECK(array1[0] == '7');
        CHECK(array1[1] == 8);
        CHECK_NOTHROW(array1[2]);
        CHECK(array1[2] == nullptr);

        const fly::Json array2 = {'7', 8};
        CHECK(array2[0] == '7');
        CHECK(array2[1] == 8);
        CHECK_THROWS_JSON(array2[2], "Given index (2) not found: (%s)", array2);

        fly::Json bool1 = true;
        CHECK_THROWS_JSON(bool1[0], "JSON type invalid for operator[index]: (%s)", bool1);

        const fly::Json bool2 = true;
        CHECK_THROWS_JSON(bool2[0], "JSON type invalid for operator[index]: (%s)", bool2);

        fly::Json signed1 = 1;
        CHECK_THROWS_JSON(signed1[0], "JSON type invalid for operator[index]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CHECK_THROWS_JSON(signed2[0], "JSON type invalid for operator[index]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(unsigned1[0], "JSON type invalid for operator[index]: (%s)", unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(unsigned2[0], "JSON type invalid for operator[index]: (%s)", unsigned2);

        fly::Json float1 = 1.0f;
        CHECK_THROWS_JSON(float1[0], "JSON type invalid for operator[index]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CHECK_THROWS_JSON(float2[0], "JSON type invalid for operator[index]: (%s)", float2);

        fly::Json null1 = nullptr;
        CHECK_NOTHROW(null1[0]);
        CHECK(null1.is_array());
        CHECK(null1[0] == nullptr);

        const fly::Json null2 = nullptr;
        CHECK_THROWS_JSON(null2[0], "JSON type invalid for operator[index]: (%s)", null2);
    }

    SECTION("Access a JSON array's values via the accessor 'at'")
    {
        fly::Json string1 = "abc";
        CHECK_THROWS_JSON(string1.at(0), "JSON type invalid for operator[index]: (%s)", string1);

        const fly::Json string2 = "abc";
        CHECK_THROWS_JSON(string2.at(0), "JSON type invalid for operator[index]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CHECK_THROWS_JSON(object1.at(0), "JSON type invalid for operator[index]: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CHECK_THROWS_JSON(object2.at(0), "JSON type invalid for operator[index]: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CHECK(array1.at(0) == '7');
        CHECK(array1.at(1) == 8);
        CHECK_THROWS_JSON(array1.at(2), "Given index (2) not found: (%s)", array1);

        const fly::Json array2 = {'7', 8};
        CHECK(array2.at(0) == '7');
        CHECK(array2.at(1) == 8);
        CHECK_THROWS_JSON(array2.at(2), "Given index (2) not found: (%s)", array2);

        fly::Json bool1 = true;
        CHECK_THROWS_JSON(bool1.at(0), "JSON type invalid for operator[index]: (%s)", bool1);

        const fly::Json bool2 = true;
        CHECK_THROWS_JSON(bool2.at(0), "JSON type invalid for operator[index]: (%s)", bool2);

        fly::Json signed1 = 1;
        CHECK_THROWS_JSON(signed1.at(0), "JSON type invalid for operator[index]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CHECK_THROWS_JSON(signed2.at(0), "JSON type invalid for operator[index]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(
            unsigned1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(
            unsigned2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CHECK_THROWS_JSON(float1.at(0), "JSON type invalid for operator[index]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CHECK_THROWS_JSON(float2.at(0), "JSON type invalid for operator[index]: (%s)", float2);

        fly::Json null1 = nullptr;
        CHECK_THROWS_JSON(null1.at(0), "JSON type invalid for operator[index]: (%s)", null1);

        const fly::Json null2 = nullptr;
        CHECK_THROWS_JSON(null2.at(0), "JSON type invalid for operator[index]: (%s)", null2);
    }

    SECTION("Check JSON instances for emptiness")
    {
        fly::Json json;

        json = "abcdef";
        CHECK_FALSE(json.empty());

        json = {{"a", 1}, {"b", 2}};
        CHECK_FALSE(json.empty());

        json = {'7', 8, 9, 10};
        CHECK_FALSE(json.empty());

        json = true;
        CHECK_FALSE(json.empty());

        json = 1;
        CHECK_FALSE(json.empty());

        json = static_cast<unsigned int>(1);
        CHECK_FALSE(json.empty());

        json = 1.0f;
        CHECK_FALSE(json.empty());

        json = nullptr;
        CHECK(json.empty());

        json = "";
        CHECK(json.empty());

        json = fly::JsonTraits::object_type();
        CHECK(json.empty());

        json = fly::JsonTraits::array_type();
        CHECK(json.empty());
    }

    SECTION("Check the size of JSON instances")
    {
        fly::Json json;

        json = "abcdef";
        CHECK(json.size() == 6);

        json = {{"a", 1}, {"b", 2}};
        CHECK(json.size() == 2);

        json = {'7', 8, 9, 10};
        CHECK(json.size() == 4);

        json = true;
        CHECK(json.size() == 1);

        json = 1;
        CHECK(json.size() == 1);

        json = static_cast<unsigned int>(1);
        CHECK(json.size() == 1);

        json = 1.0f;
        CHECK(json.size() == 1);

        json = nullptr;
        CHECK(json.size() == 0);
    }

    SECTION("Clear JSON instances and verify they are then empty")
    {
        fly::Json json;

        json = "abcdef";
        CHECK(json.size() == 6);
        json.clear();
        CHECK(json.empty());

        json = {{"a", 1}, {"b", 2}};
        CHECK(json.size() == 2);
        json.clear();
        CHECK(json.empty());

        json = {'7', 8, 9, 10};
        CHECK(json.size() == 4);
        json.clear();
        CHECK(json.empty());

        json = true;
        CHECK(json);
        json.clear();
        CHECK_FALSE(json);

        json = 1;
        CHECK(json == 1);
        json.clear();
        CHECK(json == 0);

        json = static_cast<unsigned int>(1);
        CHECK(json == 1);
        json.clear();
        CHECK(json == 0);

        json = 1.0f;
        CHECK(double(json) == Approx(1.0));
        json.clear();
        CHECK(double(json) == Approx(0.0));

        json = nullptr;
        CHECK(json == nullptr);
        json.clear();
        CHECK(json == nullptr);
    }

    SECTION("Swap a JSON instance with another JSON instance")
    {
        fly::Json json1 = 12389;
        fly::Json json2 = "string";
        fly::Json json3 = {1, 2, 3, 8, 9};

        json1.swap(json2);
        CHECK(json1 == "string");
        CHECK(json2 == 12389);

        json2.swap(json3);
        CHECK(json2 == fly::Json({1, 2, 3, 8, 9}));
        CHECK(json3 == 12389);

        json3.swap(json1);
        CHECK(json1 == 12389);
        CHECK(json3 == "string");
    }

    SECTION("Swap a JSON instance with a string-like type")
    {
        fly::Json json;
        std::string str;

        json = "abcdef";
        str = "ghijkl";
        CHECK_NOTHROW(json.swap(str));
        CHECK(json == "ghijkl");
        CHECK(str == "abcdef");

        json = {{"a", 1}, {"b", 2}};
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = {'7', 8, 9, 10};
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = true;
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = 1;
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = 1.0f;
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = nullptr;
        CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);
    }

    SECTION("Swap a JSON instance with an object-like type")
    {
        auto validate = [](auto *name, auto &test1, auto &test2, auto &test3) {
            CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;
            using T3 = std::decay_t<decltype(test3)>;

            test1 = T1 {{"a", 2}, {"b", 4}};
            test2 = T2 {{"a", "2"}, {"b", "4"}};
            test3 = T3 {{"a", 5}, {"b", "6"}};

            {
                fly::Json json = {{"c", 100}, {"d", 200}};
                CHECK_NOTHROW(json.swap(test1));
                CHECK(json == T1({{"a", 2}, {"b", 4}}));
                CHECK(test1 == T1({{"c", 100}, {"d", 200}}));
            }
            {
                fly::Json json = {{"c", 100}, {"d", 200}};
                CHECK_NOTHROW(json.swap(test2));
                CHECK(json == T2({{"a", "2"}, {"b", "4"}}));
                CHECK(test2 == T2({{"c", "100"}, {"d", "200"}}));
            }
            {
                fly::Json json = {{"c", nullptr}, {"d", true}};
                CHECK_NOTHROW(json.swap(test3));
                CHECK(json == T3({{"a", 5}, {"b", "6"}}));
                CHECK(test3 == T3({{"c", nullptr}, {"d", true}}));
            }
            {
                fly::Json json = {{"c", 100}, {"d", "200"}};
                CHECK_NOTHROW(json.swap(test1));
                CHECK(json == T1({{"c", 100}, {"d", 200}}));
                CHECK(test1 == T1({{"c", 100}, {"d", 200}}));
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
        validate(
            "unordered_multimap",
            unordered_multimap1,
            unordered_multimap2,
            unordered_multimap3);
    }

    SECTION("Fail to swap a JSON instance with an object-like type")
    {
        std::map<std::string, fly::Json> map;
        std::multimap<std::string, fly::Json> multimap;
        std::unordered_map<std::string, fly::Json> unordered_map;
        std::unordered_multimap<std::string, fly::Json> unordered_multimap;

        auto invalidate = [&](fly::Json json) {
            CHECK_THROWS_JSON(json.swap(map), "JSON type invalid for swap(object): (%s)", json);
            CHECK_THROWS_JSON(
                json.swap(multimap),
                "JSON type invalid for swap(object): (%s)",
                json);
            CHECK_THROWS_JSON(
                json.swap(unordered_map),
                "JSON type invalid for swap(object): (%s)",
                json);
            CHECK_THROWS_JSON(
                json.swap(unordered_multimap),
                "JSON type invalid for swap(object): (%s)",
                json);
        };

        invalidate("abcdef");
        invalidate({'7', 8, 9, 10});
        invalidate(true);
        invalidate(1);
        invalidate(static_cast<unsigned int>(1));
        invalidate(1.0f);
        invalidate(nullptr);
    }

    SECTION("Swap a JSON instance with an array-like type")
    {
        auto validate2 = [](auto *name, auto &test1, auto &test2) {
            CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;

            test1 = T1 {50, 60, 70, 80};
            test2 = T2 {"50", "60", "70", "80"};

            {
                fly::Json json = {1, 2};
                CHECK_NOTHROW(json.swap(test1));
                CHECK(json == T1({50, 60, 70, 80}));
                CHECK(test1 == T1({1, 2}));
            }
            {
                fly::Json json = {1, 2};
                CHECK_NOTHROW(json.swap(test2));
                CHECK(json == T2({"50", "60", "70", "80"}));
                CHECK(test2 == T2({"1", "2"}));
            }
            {
                fly::Json json = {50, "60", 70, "80"};
                CHECK_NOTHROW(json.swap(test1));
                CHECK(json == T1({1, 2}));
                CHECK(test1 == T1({50, 60, 70, 80}));
            }
        };

        auto validate3 = [&](auto *name, auto &test1, auto &test2, auto &test3) {
            validate2(name, test1, test2);

            using T3 = std::decay_t<decltype(test3)>;
            test3 = T3 {"a", 90, "b", 100};

            fly::Json json = {nullptr, true};
            CHECK_NOTHROW(json.swap(test3));
            CHECK(json == T3({"a", 90, "b", 100}));
            CHECK(test3 == T3({nullptr, true}));
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

    SECTION("Fail to swap a JSON instance with an array-like type")
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
            CHECK_THROWS_JSON(json.swap(array), "JSON type invalid for swap(array): (%s)", json);
            CHECK_THROWS_JSON(json.swap(deque), "JSON type invalid for swap(array): (%s)", json);
            CHECK_THROWS_JSON(
                json.swap(forward_list),
                "JSON type invalid for swap(array): (%s)",
                json);
            CHECK_THROWS_JSON(json.swap(list), "JSON type invalid for swap(array): (%s)", json);
            CHECK_THROWS_JSON(json.swap(multiset), "JSON type invalid for swap(array): (%s)", json);
            CHECK_THROWS_JSON(json.swap(set), "JSON type invalid for swap(array): (%s)", json);
            CHECK_THROWS_JSON(
                json.swap(unordered_multiset),
                "JSON type invalid for swap(array): (%s)",
                json);
            CHECK_THROWS_JSON(
                json.swap(unordered_set),
                "JSON type invalid for swap(array): (%s)",
                json);
            CHECK_THROWS_JSON(json.swap(vector), "JSON type invalid for swap(array): (%s)", json);
        };

        invalidate("abcdef");
        invalidate({{"a", 1}, {"b", 2}});
        invalidate(true);
        invalidate(1);
        invalidate(static_cast<unsigned int>(1));
        invalidate(1.0f);
        invalidate(nullptr);
    }

    SECTION("Check the iterator at the beginning of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto begin1 = json1.begin();
        CHECK(*begin1 == 1);
        CHECK_FALSE(std::is_const_v<decltype(begin1)::value_type>);

        auto cbegin1 = json1.cbegin();
        CHECK(*cbegin1 == 1);
        CHECK(std::is_const_v<decltype(cbegin1)::value_type>);

        auto begin2 = json2.begin();
        CHECK(*begin2 == 4);
        CHECK(std::is_const_v<decltype(begin2)::value_type>);

        auto cbegin2 = json2.cbegin();
        CHECK(*cbegin2 == 4);
        CHECK(begin2 == cbegin2);
        CHECK(std::is_const_v<decltype(cbegin2)::value_type>);
    }

    SECTION("Check the iterator at the end of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto end1 = json1.end();
        CHECK(*(end1 - 1) == 3);
        CHECK_FALSE(std::is_const_v<decltype(end1)::value_type>);

        auto cend1 = json1.cend();
        CHECK(*(cend1 - 1) == 3);
        CHECK(std::is_const_v<decltype(cend1)::value_type>);

        auto end2 = json2.end();
        CHECK(*(end2 - 1) == 6);
        CHECK(std::is_const_v<decltype(end2)::value_type>);

        auto cend2 = json2.cend();
        CHECK(*(cend2 - 1) == 6);
        CHECK(end2 == cend2);
        CHECK(std::is_const_v<decltype(cend2)::value_type>);
    }

    SECTION("Iterate over a JSON object using plain interators")
    {
        fly::Json json {{"a", 1}, {"b", 2}};
        {
            fly::Json::size_type size = 0;

            for (auto it = json.begin(); it != json.end(); ++it, ++size)
            {
                CHECK(*it == (size == 0 ? 1 : 2));
                CHECK(it.key() == (size == 0 ? "a" : "b"));
                CHECK(it.value() == (size == 0 ? 1 : 2));
            }

            CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
            {
                CHECK(*it == (size == 0 ? 1 : 2));
                CHECK(it.key() == (size == 0 ? "a" : "b"));
                CHECK(it.value() == (size == 0 ? 1 : 2));
            }

            CHECK(size == json.size());
        }
    }

    SECTION("Iterate over a JSON object using range-based for loops")
    {
        fly::Json json {{"a", 1}, {"b", 2}};
        {
            fly::Json::size_type size = 0;

            for (fly::Json &value : json)
            {
                CHECK(value == (size++ == 0 ? 1 : 2));
            }

            CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (const fly::Json &value : json)
            {
                CHECK(value == (size++ == 0 ? 1 : 2));
            }

            CHECK(size == json.size());
        }
    }

    SECTION("Iterate over a JSON array using plain interators")
    {
        fly::Json json {1, 2, 3};
        {
            fly::Json::size_type size = 0;

            for (auto it = json.begin(); it != json.end(); ++it, ++size)
            {
                CHECK(*it == json[size]);
                CHECK(it.value() == json[size]);
            }

            CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
            {
                CHECK(*it == json[size]);
                CHECK(it.value() == json[size]);
            }

            CHECK(size == json.size());
        }
    }

    SECTION("Iterate over a JSON array using range-based for loops")
    {
        fly::Json json {1, 2, 3};
        {
            fly::Json::size_type size = 0;

            for (fly::Json &value : json)
            {
                CHECK(value == json[size++]);
            }

            CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (const fly::Json &value : json)
            {
                CHECK(value == json[size++]);
            }

            CHECK(size == json.size());
        }
    }

    SECTION("Compare JSON instances for equality")
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

        CHECK(string1 == string1);
        CHECK(string1 == string2);
        CHECK(string1 != string3);
        CHECK(string1 != object1);
        CHECK(string1 != array1);
        CHECK(string1 != bool1);
        CHECK(string1 != signed1);
        CHECK(string1 != unsigned1);
        CHECK(string1 != float1);

        CHECK(object1 == object1);
        CHECK(object1 == object2);
        CHECK(object1 != object3);
        CHECK(object1 != string1);
        CHECK(object1 != array1);
        CHECK(object1 != bool1);
        CHECK(object1 != signed1);
        CHECK(object1 != unsigned1);
        CHECK(object1 != float1);

        CHECK(array1 == array1);
        CHECK(array1 == array2);
        CHECK(array1 != array3);
        CHECK(array1 != string1);
        CHECK(array1 != object1);
        CHECK(array1 != bool1);
        CHECK(array1 != signed1);
        CHECK(array1 != unsigned1);
        CHECK(array1 != float1);

        CHECK(bool1 == bool1);
        CHECK(bool1 == bool2);
        CHECK(bool1 != bool3);
        CHECK(bool1 != string1);
        CHECK(bool1 != object1);
        CHECK(bool1 != array1);
        CHECK(bool1 != signed1);
        CHECK(bool1 != unsigned1);
        CHECK(bool1 != float1);

        CHECK(signed1 == signed1);
        CHECK(signed1 == signed2);
        CHECK(signed1 != signed3);
        CHECK(signed1 != string1);
        CHECK(signed1 != object1);
        CHECK(signed1 != array1);
        CHECK(signed1 != bool1);
        CHECK(signed1 == unsigned1);
        CHECK(signed1 != unsigned3);
        CHECK(signed1 == float1);
        CHECK(signed1 != float3);

        CHECK(unsigned1 == unsigned1);
        CHECK(unsigned1 == unsigned2);
        CHECK(unsigned1 != unsigned3);
        CHECK(unsigned1 != string1);
        CHECK(unsigned1 != object1);
        CHECK(unsigned1 != array1);
        CHECK(unsigned1 != bool1);
        CHECK(unsigned1 == signed1);
        CHECK(unsigned1 != signed3);
        CHECK(unsigned1 == float1);
        CHECK(unsigned1 != float3);

        CHECK(float1 == float1);
        CHECK(float1 == float2);
        CHECK(float1 != float3);
        CHECK(float1 != string1);
        CHECK(float1 != object1);
        CHECK(float1 != array1);
        CHECK(float1 != bool1);
        CHECK(float1 == signed1);
        CHECK(float1 != signed3);
        CHECK(float1 == unsigned1);
        CHECK(float1 != unsigned3);
    }

    SECTION("Stream a JSON instance")
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
        CHECK(stream.str() == "\"abc\"");
        stream.str(std::string());

        stream << object;
        CHECK(stream.str() == "{\"a\":1,\"b\":2}");
        stream.str(std::string());

        stream << array;
        CHECK(stream.str() == "[55,8]");
        stream.str(std::string());

        stream << boolean;
        CHECK(stream.str() == "true");
        stream.str(std::string());

        stream << sign;
        CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << unsign;
        CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << floating;
        CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << null;
        CHECK(stream.str() == "null");
        stream.str(std::string());
    }

    SECTION("Stream a JSON instance, expecting special symbols to be escaped")
    {
        {
            fly::Json json = "abc\\\"def";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\\"def\"");
        }
        {
            fly::Json json = "abc\\\\def";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\\\def\"");
        }
        {
            fly::Json json = "abc\\bdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\bdef\"");
        }
        {
            fly::Json json = "abc\\fdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\fdef\"");
        }
        {
            fly::Json json = "abc\\ndef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\ndef\"");
        }
        {
            fly::Json json = "abc\\rdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\rdef\"");
        }
        {
            fly::Json json = "abc\\tdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\tdef\"");
        }
        {
            fly::Json json = "abc\xce\xa9zef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\u03a9zef\"");
        }
        {
            fly::Json json = "abc\xf0\x9f\x8d\x95zef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CHECK(str == "\"abc\\ud83c\\udf55zef\"");
        }
    }
}
