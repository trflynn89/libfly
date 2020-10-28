#include "fly/types/json/json.hpp"

#include <catch2/catch.hpp>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define CATCH_CHECK_THROWS_JSON(expression, ...)                                                   \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::JsonException,                                                                        \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("JsonException: " __VA_ARGS__)))

CATCH_TEST_CASE("JsonConstruction", "[json]")
{
    CATCH_SECTION("Construct a JSON instance from string-like types")
    {
        CATCH_SECTION("std::string-like types")
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

        CATCH_SECTION("std::wstring-like types")
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

        CATCH_SECTION("std::u8string-like types")
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

        CATCH_SECTION("std::u16string-like types")
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

        CATCH_SECTION("std::u32string-like types")
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
    }

    CATCH_SECTION("Fail to construct a JSON instance from string-like types")
    {
        CATCH_SECTION("std::string-like types")
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

        CATCH_SECTION("std::wstring-like types")
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
                "Could not convert string-like type to a JSON string");
        }

        CATCH_SECTION("std::u8string-like types")
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
                "Could not convert string-like type to a JSON string");
        }

        CATCH_SECTION("std::u16string-like types")
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
                "Could not convert string-like type to a JSON string");
        }

        CATCH_SECTION("std::u32string-like types")
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
                "Could not convert string-like type to a JSON string");
        }
    }

    CATCH_SECTION("Construct a JSON instance from object-like types")
    {
        CATCH_SECTION("std::string keys")
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

        CATCH_SECTION("std::wstring keys")
        {
            std::map<std::wstring, int> map = {{L"a", 1}, {L"b", 2}};
            CATCH_CHECK(fly::Json(map).is_object());

            std::multimap<std::wstring, int> multimap = {{L"c", 3}, {L"d", 4}};
            CATCH_CHECK(fly::Json(multimap).is_object());

            std::unordered_map<std::wstring, int> umap = {{L"e", 5}, {L"f", 6}};
            CATCH_CHECK(fly::Json(umap).is_object());

            std::unordered_multimap<std::wstring, int> umultimap = {{L"h", 7}, {L"i", 8}};
            CATCH_CHECK(fly::Json(umultimap).is_object());
        }

        CATCH_SECTION("std::u8string keys")
        {
            std::map<std::u8string, int> map = {{u8"a", 1}, {u8"b", 2}};
            CATCH_CHECK(fly::Json(map).is_object());

            std::multimap<std::u8string, int> multimap = {{u8"c", 3}, {u8"d", 4}};
            CATCH_CHECK(fly::Json(multimap).is_object());

            std::unordered_map<std::u8string, int> umap = {{u8"e", 5}, {u8"f", 6}};
            CATCH_CHECK(fly::Json(umap).is_object());

            std::unordered_multimap<std::u8string, int> umultimap = {{u8"h", 7}, {u8"i", 8}};
            CATCH_CHECK(fly::Json(umultimap).is_object());
        }

        CATCH_SECTION("std::u16string keys")
        {
            std::map<std::u16string, int> map = {{u"a", 1}, {u"b", 2}};
            CATCH_CHECK(fly::Json(map).is_object());

            std::multimap<std::u16string, int> multimap = {{u"c", 3}, {u"d", 4}};
            CATCH_CHECK(fly::Json(multimap).is_object());

            std::unordered_map<std::u16string, int> umap = {{u"e", 5}, {u"f", 6}};
            CATCH_CHECK(fly::Json(umap).is_object());

            std::unordered_multimap<std::u16string, int> umultimap = {{u"h", 7}, {u"i", 8}};
            CATCH_CHECK(fly::Json(umultimap).is_object());
        }

        CATCH_SECTION("std::u32string keys")
        {
            std::map<std::u32string, int> map = {{U"a", 1}, {U"b", 2}};
            CATCH_CHECK(fly::Json(map).is_object());

            std::multimap<std::u32string, int> multimap = {{U"c", 3}, {U"d", 4}};
            CATCH_CHECK(fly::Json(multimap).is_object());

            std::unordered_map<std::u32string, int> umap = {{U"e", 5}, {U"f", 6}};
            CATCH_CHECK(fly::Json(umap).is_object());

            std::unordered_multimap<std::u32string, int> umultimap = {{U"h", 7}, {U"i", 8}};
            CATCH_CHECK(fly::Json(umultimap).is_object());
        }
    }

    CATCH_SECTION("Fail to construct a JSON instance from object-like types")
    {
        CATCH_SECTION("std::string keys")
        {
            std::map<std::string, int> map;

            // Reverse solidus must be followed by a valid escape symbol.
            map = {{"\\", 1}};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Expected escaped character after reverse solidus");

            map = {{"\\U", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

            // Quotes must be escaped.
            map = {{"\"", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

            // Control characters must be escaped.
            for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
            {
                map = {{std::string(1, static_cast<char>(ch)), 1}};
                CATCH_CHECK_THROWS_JSON(
                    fly::Json(map),
                    "Character '%c' must be escaped",
                    static_cast<char>(ch));
            }

            // Characters must be valid Unicode.
            map = {{"\xed\xa0\x80", 1}}; // Reserved codepoint.
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Could not decode Unicode character");

            map = {{"\xf4\x90\x80\x80", 1}}; // Out-of-range codepoint.
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Could not decode Unicode character");
        }

        CATCH_SECTION("std::wstring keys")
        {
            std::map<std::wstring, int> map;

            // Reverse solidus must be followed by a valid escape symbol.
            map = {{L"\\", 1}};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Expected escaped character after reverse solidus");

            map = {{L"\\U", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

            // Quotes must be escaped.
            map = {{L"\"", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

            // Control characters must be escaped.
            for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
            {
                map = {{std::wstring(1, static_cast<wchar_t>(ch)), 1}};
                CATCH_CHECK_THROWS_JSON(
                    fly::Json(map),
                    "Character '%c' must be escaped",
                    static_cast<char>(ch));
            }

            // Characters must be valid Unicode.
            map = {{std::wstring(1, 0xd800), 1}}; // Reserved codepoint.
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Could not convert string-like type to a JSON string");
        }

        CATCH_SECTION("std::u8string keys")
        {
            std::map<std::u8string, int> map;

            // Reverse solidus must be followed by a valid escape symbol.
            map = {{u8"\\", 1}};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Expected escaped character after reverse solidus");

            map = {{u8"\\U", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

            // Quotes must be escaped.
            map = {{u8"\"", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

            // Control characters must be escaped.
            for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
            {
                map = {{std::u8string(1, static_cast<char8_t>(ch)), 1}};
                CATCH_CHECK_THROWS_JSON(
                    fly::Json(map),
                    "Character '%c' must be escaped",
                    static_cast<char>(ch));
            }

            // Characters must be valid Unicode.
            map = {{std::u8string(1, 0xff), 1}}; // Invalid leading byte.
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Could not convert string-like type to a JSON string");
        }

        CATCH_SECTION("std::u16string keys")
        {
            std::map<std::u16string, int> map;

            // Reverse solidus must be followed by a valid escape symbol.
            map = {{u"\\", 1}};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Expected escaped character after reverse solidus");

            map = {{u"\\U", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

            // Quotes must be escaped.
            map = {{u"\"", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

            // Control characters must be escaped.
            for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
            {
                map = {{std::u16string(1, static_cast<char16_t>(ch)), 1}};
                CATCH_CHECK_THROWS_JSON(
                    fly::Json(map),
                    "Character '%c' must be escaped",
                    static_cast<char>(ch));
            }

            // Characters must be valid Unicode.
            map = {{std::u16string(1, 0xd800), 1}}; // Reserved codepoint.
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Could not convert string-like type to a JSON string");
        }

        CATCH_SECTION("std::u32string keys")
        {
            std::map<std::u32string, int> map;

            // Reverse solidus must be followed by a valid escape symbol.
            map = {{U"\\", 1}};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Expected escaped character after reverse solidus");

            map = {{U"\\U", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

            // Quotes must be escaped.
            map = {{U"\"", 1}};
            CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

            // Control characters must be escaped.
            for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
            {
                map = {{std::u32string(1, static_cast<char32_t>(ch)), 1}};
                CATCH_CHECK_THROWS_JSON(
                    fly::Json(map),
                    "Character '%c' must be escaped",
                    static_cast<char>(ch));
            }

            // Characters must be valid Unicode.
            map = {{std::u32string(1, 0xd800), 1}}; // Reserved codepoint.
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Could not convert string-like type to a JSON string");
        }
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
        for (char ch = 0; ch <= 0x1f; ++ch)
        {
            vector = {std::string(1, ch)};
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
}
