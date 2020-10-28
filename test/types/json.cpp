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

#define CATCH_CHECK_THROWS_JSON(expression, ...)                                                   \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::JsonException,                                                                        \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("JsonException: " __VA_ARGS__)))

CATCH_TEST_CASE("Json", "[json]")
{
    CATCH_SECTION("Construct a JSON instance from string-like types")
    {
        const std::string str1("a");
        CATCH_CHECK(fly::Json(str1).is_string());

        std::string str2("b");
        CATCH_CHECK(fly::Json(str2).is_string());

        const char *cstr1 = "c";
        CATCH_CHECK(fly::Json(cstr1).is_string());

        char *cstr2 = const_cast<char *>("d");
        CATCH_CHECK(fly::Json(cstr2).is_string());

        const char arr1[] = {'g', '\0'};
        CATCH_CHECK(fly::Json(arr1).is_string());

        char arr2[] = {'h', '\0'};
        CATCH_CHECK(fly::Json(arr2).is_string());
    }

    CATCH_SECTION("Construct a JSON instance from wstring-like types")
    {
        const std::wstring str1(L"a");
        CATCH_CHECK(fly::Json(str1).is_string());

        std::wstring str2(L"b");
        CATCH_CHECK(fly::Json(str2).is_string());

        const wchar_t *cstr1 = L"c";
        CATCH_CHECK(fly::Json(cstr1).is_string());

        wchar_t *cstr2 = const_cast<wchar_t *>(L"d");
        CATCH_CHECK(fly::Json(cstr2).is_string());

        const wchar_t arr1[] = {L'g', L'\0'};
        CATCH_CHECK(fly::Json(arr1).is_string());

        wchar_t arr2[] = {L'h', L'\0'};
        CATCH_CHECK(fly::Json(arr2).is_string());
    }

    CATCH_SECTION("Construct a JSON instance from u8string-like types")
    {
        const std::u8string str1(u8"a");
        CATCH_CHECK(fly::Json(str1).is_string());

        std::u8string str2(u8"b");
        CATCH_CHECK(fly::Json(str2).is_string());

        const char8_t *cstr1 = u8"c";
        CATCH_CHECK(fly::Json(cstr1).is_string());

        char8_t *cstr2 = const_cast<char8_t *>(u8"d");
        CATCH_CHECK(fly::Json(cstr2).is_string());

        const char8_t arr1[] = {u8'g', u8'\0'};
        CATCH_CHECK(fly::Json(arr1).is_string());

        char8_t arr2[] = {u8'h', u8'\0'};
        CATCH_CHECK(fly::Json(arr2).is_string());
    }

    CATCH_SECTION("Construct a JSON instance from u16string-like types")
    {
        const std::u16string str1(u"a");
        CATCH_CHECK(fly::Json(str1).is_string());

        std::u16string str2(u"b");
        CATCH_CHECK(fly::Json(str2).is_string());

        const char16_t *cstr1 = u"c";
        CATCH_CHECK(fly::Json(cstr1).is_string());

        char16_t *cstr2 = const_cast<char16_t *>(u"d");
        CATCH_CHECK(fly::Json(cstr2).is_string());

        const char16_t arr1[] = {u'g', u'\0'};
        CATCH_CHECK(fly::Json(arr1).is_string());

        char16_t arr2[] = {u'h', u'\0'};
        CATCH_CHECK(fly::Json(arr2).is_string());
    }

    CATCH_SECTION("Construct a JSON instance from u32string-like types")
    {
        const std::u32string str1(U"a");
        CATCH_CHECK(fly::Json(str1).is_string());

        std::u32string str2(U"b");
        CATCH_CHECK(fly::Json(str2).is_string());

        const char32_t *cstr1 = U"c";
        CATCH_CHECK(fly::Json(cstr1).is_string());

        char32_t *cstr2 = const_cast<char32_t *>(U"d");
        CATCH_CHECK(fly::Json(cstr2).is_string());

        const char32_t arr1[] = {U'g', U'\0'};
        CATCH_CHECK(fly::Json(arr1).is_string());

        char32_t arr2[] = {U'h', U'\0'};
        CATCH_CHECK(fly::Json(arr2).is_string());
    }

    CATCH_SECTION("Fail to construct a JSON instance from string-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CATCH_CHECK_THROWS_JSON(
            fly::Json("\\"),
            "Expected escaped character after reverse solidus");
        CATCH_CHECK_THROWS_JSON(fly::Json("\\U"), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CATCH_CHECK_THROWS_JSON(fly::Json("\""), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            CATCH_CHECK_THROWS_JSON(
                fly::Json(std::string(1, static_cast<char>(ch))),
                "Character '%c' must be escaped",
                static_cast<char>(ch));
        }

        // Characters must be valid Unicode.
        CATCH_CHECK_THROWS_JSON(
            fly::Json("\xed\xa0\x80"), // Reserved codepoint.
            "Could not decode Unicode character");
        CATCH_CHECK_THROWS_JSON(
            fly::Json("\xf4\x90\x80\x80"), // Out-of-range codepoint.
            "Could not decode Unicode character");
    }

    CATCH_SECTION("Fail to construct a JSON instance from wstring-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(L"\\"),
            "Expected escaped character after reverse solidus");
        CATCH_CHECK_THROWS_JSON(fly::Json(L"\\U"), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CATCH_CHECK_THROWS_JSON(fly::Json(L"\""), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            CATCH_CHECK_THROWS_JSON(
                fly::Json(std::wstring(1, static_cast<wchar_t>(ch))),
                "Character '%c' must be escaped",
                static_cast<char>(ch));
        }

        // Characters must be valid Unicode.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(std::wstring(1, 0xd800)), // Reserved codepoint.
            "Could not convert wstring-like type to a JSON string");
    }

    CATCH_SECTION("Fail to construct a JSON instance from u8string-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(u8"\\"),
            "Expected escaped character after reverse solidus");
        CATCH_CHECK_THROWS_JSON(fly::Json(u8"\\U"), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CATCH_CHECK_THROWS_JSON(fly::Json(u8"\""), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            CATCH_CHECK_THROWS_JSON(
                fly::Json(std::u8string(1, static_cast<char8_t>(ch))),
                "Character '%c' must be escaped",
                static_cast<char>(ch));
        }

        // Characters must be valid Unicode.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(std::u8string(1, 0xff)), // Invalid leading byte.
            "Could not convert u8string-like type to a JSON string");
    }

    CATCH_SECTION("Fail to construct a JSON instance from u16string-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(u"\\"),
            "Expected escaped character after reverse solidus");
        CATCH_CHECK_THROWS_JSON(fly::Json(u"\\U"), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CATCH_CHECK_THROWS_JSON(fly::Json(u"\""), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            CATCH_CHECK_THROWS_JSON(
                fly::Json(std::u16string(1, static_cast<char16_t>(ch))),
                "Character '%c' must be escaped",
                static_cast<char>(ch));
        }

        // Characters must be valid Unicode.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(std::u16string(1, 0xd800)), // Reserved codepoint.
            "Could not convert u16string-like type to a JSON string");
    }

    CATCH_SECTION("Fail to construct a JSON instance from u32string-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(U"\\"),
            "Expected escaped character after reverse solidus");
        CATCH_CHECK_THROWS_JSON(fly::Json(U"\\U"), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CATCH_CHECK_THROWS_JSON(fly::Json(U"\""), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            CATCH_CHECK_THROWS_JSON(
                fly::Json(std::u32string(1, static_cast<char32_t>(ch))),
                "Character '%c' must be escaped",
                static_cast<char>(ch));
        }

        // Characters must be valid Unicode.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(std::u32string(1, 0xd800)), // Reserved codepoint.
            "Could not convert u32string-like type to a JSON string");
    }

    CATCH_SECTION("Construct a JSON instance from object-like types")
    {
        std::map<std::string, int> map = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(fly::Json(map).is_object());

        std::multimap<std::string, int> multimap = {{"c", 3}, {"d", 4}};
        CATCH_CHECK(fly::Json(multimap).is_object());

        std::unordered_map<std::string, int> umap = {{"e", 5}, {"f", 6}};
        CATCH_CHECK(fly::Json(umap).is_object());

        std::unordered_multimap<std::string, int> umultimap = {{"h", 7}, {"i", 8}};
        CATCH_CHECK(fly::Json(umultimap).is_object());
    }

    CATCH_SECTION("Fail to construct a JSON instance from object-like types")
    {
        std::map<std::string, int> map;

        // Reverse solidus must be followed by a valid escape symbol.
        map = {{"\\", 1}};
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Expected escaped character after reverse solidus");

        map = {{"\\U", 1}};
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

        // Quotes must be escaped.
        map = {{"\"", 1}};
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
        {
            map = {{fly::JsonTraits::string_type(1, ch), 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '%c' must be escaped", ch);
        }

        // Characters must be valid Unicode.
        map = {{"\xed\xa0\x80", 1}}; // Reserved codepoint.
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Could not decode Unicode character");

        map = {{"\xf4\x90\x80\x80", 1}}; // Out-of-range codepoint.
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Could not decode Unicode character");
    }

    CATCH_SECTION("Construct a JSON instance from array-like types")
    {
        std::array<int, 4> array = {10, 20, 30, 40};
        CATCH_CHECK(fly::Json(array).is_array());
        CATCH_CHECK_FALSE(fly::Json(array).is_object_like());

        std::deque<int> deque = {50, 60, 70, 80};
        CATCH_CHECK(fly::Json(deque).is_array());
        CATCH_CHECK_FALSE(fly::Json(deque).is_object_like());

        std::forward_list<int> forward_list = {90, 100, 110, 120};
        CATCH_CHECK(fly::Json(forward_list).is_array());
        CATCH_CHECK_FALSE(fly::Json(forward_list).is_object_like());

        std::list<int> list = {130, 140, 150, 160};
        CATCH_CHECK(fly::Json(list).is_array());
        CATCH_CHECK_FALSE(fly::Json(list).is_object_like());

        std::multiset<std::string> multiset = {"a", "b", "c"};
        CATCH_CHECK(fly::Json(multiset).is_array());
        CATCH_CHECK_FALSE(fly::Json(multiset).is_object_like());

        std::set<std::string> set = {"d", "e", "f"};
        CATCH_CHECK(fly::Json(set).is_array());
        CATCH_CHECK_FALSE(fly::Json(set).is_object_like());

        std::unordered_multiset<std::string> unordered_multiset = {"g", "h", "i"};
        CATCH_CHECK(fly::Json(unordered_multiset).is_array());
        CATCH_CHECK_FALSE(fly::Json(unordered_multiset).is_object_like());

        std::unordered_set<std::string> unordered_set = {"j", "k", "l"};
        CATCH_CHECK(fly::Json(unordered_set).is_array());
        CATCH_CHECK_FALSE(fly::Json(unordered_set).is_object_like());

        std::vector<int> vector = {170, 180, 190, 200};
        CATCH_CHECK(fly::Json(vector).is_array());
        CATCH_CHECK_FALSE(fly::Json(vector).is_object_like());

        std::array<std::string, 2> object = {"nine", "ten"};
        CATCH_CHECK(fly::Json(object).is_array());
        CATCH_CHECK(fly::Json(object).is_object_like());
    }

    CATCH_SECTION("Fail to construct a JSON instance from array-like types")
    {
        std::vector<std::string> vector;

        // Reverse solidus must be followed by a valid escape symbol.
        vector = {"\\"};
        CATCH_CHECK_THROWS_JSON(
            fly::Json(vector),
            "Expected escaped character after reverse solidus");

        vector = {"\\U"};
        CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Invalid escape character 'U'");

        // Quotes must be escaped.
        vector = {"\""};
        CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (fly::JsonTraits::string_type::value_type ch = 0; ch <= 0x1f; ++ch)
        {
            vector = {fly::JsonTraits::string_type(1, ch)};
            CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Character '%c' must be escaped", ch);
        }

        // Characters must be valid Unicode.
        vector = {"\xed\xa0\x80"}; // Reserved codepoint.
        CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Could not decode Unicode character");

        vector = {"\xf4\x90\x80\x80"}; // Out-of-range codepoint.
        CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Could not decode Unicode character");
    }

    CATCH_SECTION("Construct a JSON instance from Boolean-like types")
    {
        CATCH_CHECK(fly::Json(true).is_boolean());
        CATCH_CHECK(fly::Json(false).is_boolean());
    }

    CATCH_SECTION("Construct a JSON instance from signed-integer-like types")
    {
        CATCH_CHECK(fly::Json(static_cast<char>(1)).is_signed_integer());

        CATCH_CHECK(fly::Json(static_cast<short>(1)).is_signed_integer());

        CATCH_CHECK(fly::Json(static_cast<int>(1)).is_signed_integer());
        CATCH_CHECK(fly::Json(static_cast<int>(-1)).is_signed_integer());

        CATCH_CHECK(fly::Json(static_cast<std::int32_t>(1)).is_signed_integer());
        CATCH_CHECK(fly::Json(static_cast<std::int32_t>(-1)).is_signed_integer());

        CATCH_CHECK(fly::Json(static_cast<std::int64_t>(1)).is_signed_integer());
        CATCH_CHECK(fly::Json(static_cast<std::int64_t>(-1)).is_signed_integer());
    }

    CATCH_SECTION("Construct a JSON instance from unsigned-integer-like types")
    {
        CATCH_CHECK(fly::Json(static_cast<unsigned char>(1)).is_unsigned_integer());

        CATCH_CHECK(fly::Json(static_cast<unsigned short>(1)).is_unsigned_integer());

        CATCH_CHECK(fly::Json(static_cast<unsigned int>(1)).is_unsigned_integer());
        CATCH_CHECK(fly::Json(static_cast<unsigned int>(-1)).is_unsigned_integer());

        CATCH_CHECK(fly::Json(static_cast<std::uint32_t>(1)).is_unsigned_integer());
        CATCH_CHECK(fly::Json(static_cast<std::uint32_t>(-1)).is_unsigned_integer());

        CATCH_CHECK(fly::Json(static_cast<std::uint64_t>(1)).is_unsigned_integer());
        CATCH_CHECK(fly::Json(static_cast<std::uint64_t>(-1)).is_unsigned_integer());
    }

    CATCH_SECTION("Construct a JSON instance from floating-point-like types")
    {
        CATCH_CHECK(fly::Json(static_cast<float>(1.0)).is_float());
        CATCH_CHECK(fly::Json(static_cast<double>(1.0)).is_float());
        CATCH_CHECK(fly::Json(static_cast<long double>(1.0)).is_float());
    }

    CATCH_SECTION("Construct a JSON instance from null-like types")
    {
        CATCH_CHECK(fly::Json().is_null());
        CATCH_CHECK(fly::Json(nullptr).is_null());
    }

    CATCH_SECTION("Construct a JSON instance from initializer lists")
    {
        const fly::Json empty {};
        CATCH_CHECK(fly::Json(empty).is_null());

        const fly::Json array {'7', 8, "nine", 10};
        CATCH_CHECK(fly::Json(array).is_array());

        const fly::Json object {{"a", 1}, {"b", 2}};
        CATCH_CHECK(fly::Json(object).is_object());

        const fly::Json almost {{"a", 1}, {"b", 2}, 4};
        CATCH_CHECK(fly::Json(almost).is_array());
    }

    CATCH_SECTION("Construct a JSON instance from another JSON instance via copy")
    {
        fly::Json string = "abc";
        CATCH_CHECK(fly::Json(string) == string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(fly::Json(object) == object);

        fly::Json array = {'7', 8};
        CATCH_CHECK(fly::Json(array) == array);

        fly::Json boolean = true;
        CATCH_CHECK(fly::Json(boolean) == boolean);

        fly::Json sign = 1;
        CATCH_CHECK(fly::Json(sign) == sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        CATCH_CHECK(fly::Json(unsign) == unsign);

        fly::Json floating = 1.0f;
        CATCH_CHECK(fly::Json(floating) == floating);

        fly::Json null = nullptr;
        CATCH_CHECK(fly::Json(null) == null);
    }

    CATCH_SECTION("Construct a JSON instance from another JSON instance via move semantics")
    {
        fly::Json string = "abc";
        fly::Json string_copy(string);
        fly::Json string_move(std::move(string_copy));

        CATCH_CHECK(string_copy.is_null());
        CATCH_CHECK(string_move == string);
    }

    CATCH_SECTION("Assign a JSON instance's value to another JSON instance's value")
    {
        fly::Json json;

        fly::Json string = "abc";
        json = string;
        CATCH_CHECK(json == string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        json = object;
        CATCH_CHECK(json == object);

        fly::Json array = {'7', 8};
        json = array;
        CATCH_CHECK(json == array);

        fly::Json boolean = true;
        json = boolean;
        CATCH_CHECK(json == boolean);

        fly::Json sign = 1;
        json = sign;
        CATCH_CHECK(json == sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        json = unsign;
        CATCH_CHECK(json == unsign);

        fly::Json floating = 1.0f;
        json = floating;
        CATCH_CHECK(json == floating);

        fly::Json null = nullptr;
        json = null;
        CATCH_CHECK(json == null);
    }

    CATCH_SECTION("Access a JSON object's values via the access operator")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(string1["a"], "JSON type invalid for operator[key]: (%s)", string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(string2["a"], "JSON type invalid for operator[key]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object1["a"] == 1);
        CATCH_CHECK(object1["b"] == 2);
        CATCH_CHECK_NOTHROW(object1["c"]);
        CATCH_CHECK(object1["c"] == nullptr);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object2["a"] == 1);
        CATCH_CHECK(object2["b"] == 2);
        CATCH_CHECK_THROWS_JSON(object2["c"], "Given key (c) not found: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(array1["a"], "JSON type invalid for operator[key]: (%s)", array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(array2["a"], "JSON type invalid for operator[key]: (%s)", array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(bool1["a"], "JSON type invalid for operator[key]: (%s)", bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(bool2["a"], "JSON type invalid for operator[key]: (%s)", bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(signed1["a"], "JSON type invalid for operator[key]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(signed2["a"], "JSON type invalid for operator[key]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1["a"],
            "JSON type invalid for operator[key]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2["a"],
            "JSON type invalid for operator[key]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(float1["a"], "JSON type invalid for operator[key]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(float2["a"], "JSON type invalid for operator[key]: (%s)", float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_NOTHROW(null1["a"]);
        CATCH_CHECK(null1.is_object());
        CATCH_CHECK(null1["a"] == nullptr);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(null2["a"], "JSON type invalid for operator[key]: (%s)", null2);
    }

    CATCH_SECTION("Access a JSON object's values via the accessor 'at'")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string2.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object1.at("a") == 1);
        CATCH_CHECK(object1.at("b") == 2);
        CATCH_CHECK_THROWS_JSON(object1.at("c"), "Given key (c) not found: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object2.at("a") == 1);
        CATCH_CHECK(object2.at("b") == 2);
        CATCH_CHECK_THROWS_JSON(object2.at("c"), "Given key (c) not found: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array1.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array2.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(bool1.at("a"), "JSON type invalid for operator[key]: (%s)", bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(bool2.at("a"), "JSON type invalid for operator[key]: (%s)", bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed2.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float2.at("a"),
            "JSON type invalid for operator[key]: (%s)",
            float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(null1.at("a"), "JSON type invalid for operator[key]: (%s)", null1);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(null2.at("a"), "JSON type invalid for operator[key]: (%s)", null2);
    }

    CATCH_SECTION("Access a JSON array's values via the access operator")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(string1[0], "JSON type invalid for operator[index]: (%s)", string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(string2[0], "JSON type invalid for operator[index]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(object1[0], "JSON type invalid for operator[index]: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(object2[0], "JSON type invalid for operator[index]: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK(array1[0] == '7');
        CATCH_CHECK(array1[1] == 8);
        CATCH_CHECK_NOTHROW(array1[2]);
        CATCH_CHECK(array1[2] == nullptr);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK(array2[0] == '7');
        CATCH_CHECK(array2[1] == 8);
        CATCH_CHECK_THROWS_JSON(array2[2], "Given index (2) not found: (%s)", array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(bool1[0], "JSON type invalid for operator[index]: (%s)", bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(bool2[0], "JSON type invalid for operator[index]: (%s)", bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(signed1[0], "JSON type invalid for operator[index]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(signed2[0], "JSON type invalid for operator[index]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1[0],
            "JSON type invalid for operator[index]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2[0],
            "JSON type invalid for operator[index]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(float1[0], "JSON type invalid for operator[index]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(float2[0], "JSON type invalid for operator[index]: (%s)", float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_NOTHROW(null1[0]);
        CATCH_CHECK(null1.is_array());
        CATCH_CHECK(null1[0] == nullptr);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(null2[0], "JSON type invalid for operator[index]: (%s)", null2);
    }

    CATCH_SECTION("Access a JSON array's values via the accessor 'at'")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(
            object1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(
            object2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK(array1.at(0) == '7');
        CATCH_CHECK(array1.at(1) == 8);
        CATCH_CHECK_THROWS_JSON(array1.at(2), "Given index (2) not found: (%s)", array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK(array2.at(0) == '7');
        CATCH_CHECK(array2.at(1) == 8);
        CATCH_CHECK_THROWS_JSON(array2.at(2), "Given index (2) not found: (%s)", array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(bool1.at(0), "JSON type invalid for operator[index]: (%s)", bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(bool2.at(0), "JSON type invalid for operator[index]: (%s)", bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(null1.at(0), "JSON type invalid for operator[index]: (%s)", null1);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(null2.at(0), "JSON type invalid for operator[index]: (%s)", null2);
    }

    CATCH_SECTION("Check JSON instances for emptiness")
    {
        fly::Json json;

        json = "abcdef";
        CATCH_CHECK_FALSE(json.empty());

        json = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_FALSE(json.empty());

        json = {'7', 8, 9, 10};
        CATCH_CHECK_FALSE(json.empty());

        json = true;
        CATCH_CHECK_FALSE(json.empty());

        json = 1;
        CATCH_CHECK_FALSE(json.empty());

        json = static_cast<unsigned int>(1);
        CATCH_CHECK_FALSE(json.empty());

        json = 1.0f;
        CATCH_CHECK_FALSE(json.empty());

        json = nullptr;
        CATCH_CHECK(json.empty());

        json = "";
        CATCH_CHECK(json.empty());

        json = fly::JsonTraits::object_type();
        CATCH_CHECK(json.empty());

        json = fly::JsonTraits::array_type();
        CATCH_CHECK(json.empty());
    }

    CATCH_SECTION("Check the size of JSON instances")
    {
        fly::Json json;

        json = "abcdef";
        CATCH_CHECK(json.size() == 6);

        json = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(json.size() == 2);

        json = {'7', 8, 9, 10};
        CATCH_CHECK(json.size() == 4);

        json = true;
        CATCH_CHECK(json.size() == 1);

        json = 1;
        CATCH_CHECK(json.size() == 1);

        json = static_cast<unsigned int>(1);
        CATCH_CHECK(json.size() == 1);

        json = 1.0f;
        CATCH_CHECK(json.size() == 1);

        json = nullptr;
        CATCH_CHECK(json.size() == 0);
    }

    CATCH_SECTION("Clear JSON instances and verify they are then empty")
    {
        fly::Json json;

        json = "abcdef";
        CATCH_CHECK(json.size() == 6);
        json.clear();
        CATCH_CHECK(json.empty());

        json = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(json.size() == 2);
        json.clear();
        CATCH_CHECK(json.empty());

        json = {'7', 8, 9, 10};
        CATCH_CHECK(json.size() == 4);
        json.clear();
        CATCH_CHECK(json.empty());

        json = true;
        CATCH_CHECK(json);
        json.clear();
        CATCH_CHECK_FALSE(json);

        json = 1;
        CATCH_CHECK(json == 1);
        json.clear();
        CATCH_CHECK(json == 0);

        json = static_cast<unsigned int>(1);
        CATCH_CHECK(json == 1);
        json.clear();
        CATCH_CHECK(json == 0);

        json = 1.0f;
        CATCH_CHECK(double(json) == Approx(1.0));
        json.clear();
        CATCH_CHECK(double(json) == Approx(0.0));

        json = nullptr;
        CATCH_CHECK(json == nullptr);
        json.clear();
        CATCH_CHECK(json == nullptr);
    }

    CATCH_SECTION("Swap a JSON instance with another JSON instance")
    {
        fly::Json json1 = 12389;
        fly::Json json2 = "string";
        fly::Json json3 = {1, 2, 3, 8, 9};

        json1.swap(json2);
        CATCH_CHECK(json1 == "string");
        CATCH_CHECK(json2 == 12389);

        json2.swap(json3);
        CATCH_CHECK(json2 == fly::Json({1, 2, 3, 8, 9}));
        CATCH_CHECK(json3 == 12389);

        json3.swap(json1);
        CATCH_CHECK(json1 == 12389);
        CATCH_CHECK(json3 == "string");
    }

    CATCH_SECTION("Swap a JSON instance with a string-like type")
    {
        fly::Json json;
        std::string str;

        json = "abcdef";
        str = "ghijkl";
        CATCH_CHECK_NOTHROW(json.swap(str));
        CATCH_CHECK(json == "ghijkl");
        CATCH_CHECK(str == "abcdef");

        json = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = {'7', 8, 9, 10};
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = true;
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = 1;
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = 1.0f;
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);

        json = nullptr;
        CATCH_CHECK_THROWS_JSON(json.swap(str), "JSON type invalid for swap(string): (%s)", json);
    }

    CATCH_SECTION("Swap a JSON instance with an object-like type")
    {
        auto validate = [](auto *name, auto &test1, auto &test2, auto &test3)
        {
            CATCH_CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;
            using T3 = std::decay_t<decltype(test3)>;

            test1 = T1 {{"a", 2}, {"b", 4}};
            test2 = T2 {{"a", "2"}, {"b", "4"}};
            test3 = T3 {{"a", 5}, {"b", "6"}};

            {
                fly::Json json = {{"c", 100}, {"d", 200}};
                CATCH_CHECK_NOTHROW(json.swap(test1));
                CATCH_CHECK(json == T1({{"a", 2}, {"b", 4}}));
                CATCH_CHECK(test1 == T1({{"c", 100}, {"d", 200}}));
            }
            {
                fly::Json json = {{"c", 100}, {"d", 200}};
                CATCH_CHECK_NOTHROW(json.swap(test2));
                CATCH_CHECK(json == T2({{"a", "2"}, {"b", "4"}}));
                CATCH_CHECK(test2 == T2({{"c", "100"}, {"d", "200"}}));
            }
            {
                fly::Json json = {{"c", nullptr}, {"d", true}};
                CATCH_CHECK_NOTHROW(json.swap(test3));
                CATCH_CHECK(json == T3({{"a", 5}, {"b", "6"}}));
                CATCH_CHECK(test3 == T3({{"c", nullptr}, {"d", true}}));
            }
            {
                fly::Json json = {{"c", 100}, {"d", "200"}};
                CATCH_CHECK_NOTHROW(json.swap(test1));
                CATCH_CHECK(json == T1({{"c", 100}, {"d", 200}}));
                CATCH_CHECK(test1 == T1({{"c", 100}, {"d", 200}}));
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

    CATCH_SECTION("Fail to swap a JSON instance with an object-like type")
    {
        std::map<std::string, fly::Json> map;
        std::multimap<std::string, fly::Json> multimap;
        std::unordered_map<std::string, fly::Json> unordered_map;
        std::unordered_multimap<std::string, fly::Json> unordered_multimap;

        auto invalidate = [&](fly::Json json)
        {
            CATCH_CHECK_THROWS_JSON(
                json.swap(map),
                "JSON type invalid for swap(object): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(multimap),
                "JSON type invalid for swap(object): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(unordered_map),
                "JSON type invalid for swap(object): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
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

    CATCH_SECTION("Swap a JSON instance with an array-like type")
    {
        auto validate2 = [](auto *name, auto &test1, auto &test2)
        {
            CATCH_CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;

            test1 = T1 {50, 60, 70, 80};
            test2 = T2 {"50", "60", "70", "80"};

            {
                fly::Json json = {1, 2};
                CATCH_CHECK_NOTHROW(json.swap(test1));
                CATCH_CHECK(json == T1({50, 60, 70, 80}));
                CATCH_CHECK(test1 == T1({1, 2}));
            }
            {
                fly::Json json = {1, 2};
                CATCH_CHECK_NOTHROW(json.swap(test2));
                CATCH_CHECK(json == T2({"50", "60", "70", "80"}));
                CATCH_CHECK(test2 == T2({"1", "2"}));
            }
            {
                fly::Json json = {50, "60", 70, "80"};
                CATCH_CHECK_NOTHROW(json.swap(test1));
                CATCH_CHECK(json == T1({1, 2}));
                CATCH_CHECK(test1 == T1({50, 60, 70, 80}));
            }
        };

        auto validate3 = [&](auto *name, auto &test1, auto &test2, auto &test3)
        {
            validate2(name, test1, test2);

            using T3 = std::decay_t<decltype(test3)>;
            test3 = T3 {"a", 90, "b", 100};

            fly::Json json = {nullptr, true};
            CATCH_CHECK_NOTHROW(json.swap(test3));
            CATCH_CHECK(json == T3({"a", 90, "b", 100}));
            CATCH_CHECK(test3 == T3({nullptr, true}));
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

    CATCH_SECTION("Fail to swap a JSON instance with an array-like type")
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

        auto invalidate = [&](fly::Json json)
        {
            CATCH_CHECK_THROWS_JSON(
                json.swap(array),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(deque),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(forward_list),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(list),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(multiset),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(set),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(unordered_multiset),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(unordered_set),
                "JSON type invalid for swap(array): (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.swap(vector),
                "JSON type invalid for swap(array): (%s)",
                json);
        };

        invalidate("abcdef");
        invalidate({{"a", 1}, {"b", 2}});
        invalidate(true);
        invalidate(1);
        invalidate(static_cast<unsigned int>(1));
        invalidate(1.0f);
        invalidate(nullptr);
    }

    CATCH_SECTION("Check the iterator at the beginning of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto begin1 = json1.begin();
        CATCH_CHECK(*begin1 == 1);
        CATCH_CHECK_FALSE(std::is_const_v<decltype(begin1)::value_type>);

        auto cbegin1 = json1.cbegin();
        CATCH_CHECK(*cbegin1 == 1);
        CATCH_CHECK(std::is_const_v<decltype(cbegin1)::value_type>);

        auto begin2 = json2.begin();
        CATCH_CHECK(*begin2 == 4);
        CATCH_CHECK(std::is_const_v<decltype(begin2)::value_type>);

        auto cbegin2 = json2.cbegin();
        CATCH_CHECK(*cbegin2 == 4);
        CATCH_CHECK(begin2 == cbegin2);
        CATCH_CHECK(std::is_const_v<decltype(cbegin2)::value_type>);
    }

    CATCH_SECTION("Check the iterator at the end of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto end1 = json1.end();
        CATCH_CHECK(*(end1 - 1) == 3);
        CATCH_CHECK_FALSE(std::is_const_v<decltype(end1)::value_type>);

        auto cend1 = json1.cend();
        CATCH_CHECK(*(cend1 - 1) == 3);
        CATCH_CHECK(std::is_const_v<decltype(cend1)::value_type>);

        auto end2 = json2.end();
        CATCH_CHECK(*(end2 - 1) == 6);
        CATCH_CHECK(std::is_const_v<decltype(end2)::value_type>);

        auto cend2 = json2.cend();
        CATCH_CHECK(*(cend2 - 1) == 6);
        CATCH_CHECK(end2 == cend2);
        CATCH_CHECK(std::is_const_v<decltype(cend2)::value_type>);
    }

    CATCH_SECTION("Iterate over a JSON object using plain interators")
    {
        fly::Json json {{"a", 1}, {"b", 2}};
        {
            fly::Json::size_type size = 0;

            for (auto it = json.begin(); it != json.end(); ++it, ++size)
            {
                CATCH_CHECK(*it == (size == 0 ? 1 : 2));
                CATCH_CHECK(it.key() == (size == 0 ? "a" : "b"));
                CATCH_CHECK(it.value() == (size == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
            {
                CATCH_CHECK(*it == (size == 0 ? 1 : 2));
                CATCH_CHECK(it.key() == (size == 0 ? "a" : "b"));
                CATCH_CHECK(it.value() == (size == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Iterate over a JSON object using range-based for loops")
    {
        fly::Json json {{"a", 1}, {"b", 2}};
        {
            fly::Json::size_type size = 0;

            for (fly::Json &value : json)
            {
                CATCH_CHECK(value == (size++ == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (const fly::Json &value : json)
            {
                CATCH_CHECK(value == (size++ == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Iterate over a JSON array using plain interators")
    {
        fly::Json json {1, 2, 3};
        {
            fly::Json::size_type size = 0;

            for (auto it = json.begin(); it != json.end(); ++it, ++size)
            {
                CATCH_CHECK(*it == json[size]);
                CATCH_CHECK(it.value() == json[size]);
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
            {
                CATCH_CHECK(*it == json[size]);
                CATCH_CHECK(it.value() == json[size]);
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Iterate over a JSON array using range-based for loops")
    {
        fly::Json json {1, 2, 3};
        {
            fly::Json::size_type size = 0;

            for (fly::Json &value : json)
            {
                CATCH_CHECK(value == json[size++]);
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (const fly::Json &value : json)
            {
                CATCH_CHECK(value == json[size++]);
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Compare JSON instances for equality")
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

        CATCH_CHECK(string1 == string1);
        CATCH_CHECK(string1 == string2);
        CATCH_CHECK(string1 != string3);
        CATCH_CHECK(string1 != object1);
        CATCH_CHECK(string1 != array1);
        CATCH_CHECK(string1 != bool1);
        CATCH_CHECK(string1 != signed1);
        CATCH_CHECK(string1 != unsigned1);
        CATCH_CHECK(string1 != float1);

        CATCH_CHECK(object1 == object1);
        CATCH_CHECK(object1 == object2);
        CATCH_CHECK(object1 != object3);
        CATCH_CHECK(object1 != string1);
        CATCH_CHECK(object1 != array1);
        CATCH_CHECK(object1 != bool1);
        CATCH_CHECK(object1 != signed1);
        CATCH_CHECK(object1 != unsigned1);
        CATCH_CHECK(object1 != float1);

        CATCH_CHECK(array1 == array1);
        CATCH_CHECK(array1 == array2);
        CATCH_CHECK(array1 != array3);
        CATCH_CHECK(array1 != string1);
        CATCH_CHECK(array1 != object1);
        CATCH_CHECK(array1 != bool1);
        CATCH_CHECK(array1 != signed1);
        CATCH_CHECK(array1 != unsigned1);
        CATCH_CHECK(array1 != float1);

        CATCH_CHECK(bool1 == bool1);
        CATCH_CHECK(bool1 == bool2);
        CATCH_CHECK(bool1 != bool3);
        CATCH_CHECK(bool1 != string1);
        CATCH_CHECK(bool1 != object1);
        CATCH_CHECK(bool1 != array1);
        CATCH_CHECK(bool1 != signed1);
        CATCH_CHECK(bool1 != unsigned1);
        CATCH_CHECK(bool1 != float1);

        CATCH_CHECK(signed1 == signed1);
        CATCH_CHECK(signed1 == signed2);
        CATCH_CHECK(signed1 != signed3);
        CATCH_CHECK(signed1 != string1);
        CATCH_CHECK(signed1 != object1);
        CATCH_CHECK(signed1 != array1);
        CATCH_CHECK(signed1 != bool1);
        CATCH_CHECK(signed1 == unsigned1);
        CATCH_CHECK(signed1 != unsigned3);
        CATCH_CHECK(signed1 == float1);
        CATCH_CHECK(signed1 != float3);

        CATCH_CHECK(unsigned1 == unsigned1);
        CATCH_CHECK(unsigned1 == unsigned2);
        CATCH_CHECK(unsigned1 != unsigned3);
        CATCH_CHECK(unsigned1 != string1);
        CATCH_CHECK(unsigned1 != object1);
        CATCH_CHECK(unsigned1 != array1);
        CATCH_CHECK(unsigned1 != bool1);
        CATCH_CHECK(unsigned1 == signed1);
        CATCH_CHECK(unsigned1 != signed3);
        CATCH_CHECK(unsigned1 == float1);
        CATCH_CHECK(unsigned1 != float3);

        CATCH_CHECK(float1 == float1);
        CATCH_CHECK(float1 == float2);
        CATCH_CHECK(float1 != float3);
        CATCH_CHECK(float1 != string1);
        CATCH_CHECK(float1 != object1);
        CATCH_CHECK(float1 != array1);
        CATCH_CHECK(float1 != bool1);
        CATCH_CHECK(float1 == signed1);
        CATCH_CHECK(float1 != signed3);
        CATCH_CHECK(float1 == unsigned1);
        CATCH_CHECK(float1 != unsigned3);
    }

    CATCH_SECTION("Stream a JSON instance")
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
        CATCH_CHECK(stream.str() == "\"abc\"");
        stream.str(std::string());

        stream << object;
        CATCH_CHECK(stream.str() == "{\"a\":1,\"b\":2}");
        stream.str(std::string());

        stream << array;
        CATCH_CHECK(stream.str() == "[55,8]");
        stream.str(std::string());

        stream << boolean;
        CATCH_CHECK(stream.str() == "true");
        stream.str(std::string());

        stream << sign;
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << unsign;
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << floating;
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << null;
        CATCH_CHECK(stream.str() == "null");
        stream.str(std::string());
    }

    CATCH_SECTION("Stream a JSON instance, expecting special symbols to be escaped")
    {
        {
            fly::Json json = "abc\\\"def";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\\"def\"");
        }
        {
            fly::Json json = "abc\\\\def";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\\\def\"");
        }
        {
            fly::Json json = "abc\\bdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\bdef\"");
        }
        {
            fly::Json json = "abc\\fdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\fdef\"");
        }
        {
            fly::Json json = "abc\\ndef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\ndef\"");
        }
        {
            fly::Json json = "abc\\rdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\rdef\"");
        }
        {
            fly::Json json = "abc\\tdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\tdef\"");
        }
        {
            fly::Json json = "abc\xce\xa9zef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\u03a9zef\"");
        }
        {
            fly::Json json = "abc\xf0\x9f\x8d\x95zef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\ud83c\\udf55zef\"");
        }
    }
}
