#include "test/types/json/json_helpers.hpp"

#include "fly/types/json/json.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

CATCH_TEMPLATE_TEST_CASE(
    "JsonConstruction",
    "[json]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using string_type = TestType;
    using char_type = typename string_type::value_type;

    auto reserved_codepoint = []() -> string_type {
        static constexpr const std::uint32_t s_reserved = 0xd800;
        string_type result;

        if constexpr (sizeof(char_type) == 1)
        {
            result += static_cast<char_type>(0xe0 | (s_reserved >> 12));
            result += static_cast<char_type>(0x80 | ((s_reserved >> 6) & 0x3f));
            result += static_cast<char_type>(0x80 | (s_reserved & 0x3f));
        }
        else
        {
            result = string_type(1, static_cast<char_type>(s_reserved));
        }

        return result;
    };

    CATCH_SECTION("Construct a JSON instance from string-like types")
    {
        const string_type str1 = J_STR("a");
        CATCH_CHECK(fly::Json(str1).is_string());

        string_type str2 = J_STR("b");
        CATCH_CHECK(fly::Json(str2).is_string());

        const char_type *cstr1 = J_STR("c");
        CATCH_CHECK(fly::Json(cstr1).is_string());

        char_type *cstr2 = const_cast<char_type *>(J_STR("d"));
        CATCH_CHECK(fly::Json(cstr2).is_string());

        const char_type arr1[] = {J_CHR('g'), '\0'};
        CATCH_CHECK(fly::Json(arr1).is_string());

        char_type arr2[] = {J_CHR('h'), '\0'};
        CATCH_CHECK(fly::Json(arr2).is_string());
    }

    CATCH_SECTION("Fail to construct a JSON instance from string-like types")
    {
        // Reverse solidus must be followed by a valid escape symbol.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(J_STR("\\")),
            "Expected escaped character after reverse solidus");
        CATCH_CHECK_THROWS_JSON(fly::Json(J_STR("\\U")), "Invalid escape character 'U'");

        // Quotes must be escaped.
        CATCH_CHECK_THROWS_JSON(fly::Json(J_STR("\"")), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            CATCH_CHECK_THROWS_JSON(
                fly::Json(string_type(1, static_cast<char_type>(ch))),
                "Character {:#04x} must be escaped",
                static_cast<char_type>(ch));
        }

        // Characters must be valid Unicode.
        CATCH_CHECK_THROWS_JSON(
            fly::Json(reserved_codepoint()),
            "Could not convert string-like type to a JSON string");
    }

    CATCH_SECTION("Construct a JSON instance from object-like types")
    {
        std::map<string_type, int> map = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK(fly::Json(map).is_object());

        std::multimap<string_type, int> multimap = {{J_STR("c"), 3}, {J_STR("d"), 4}};
        CATCH_CHECK(fly::Json(multimap).is_object());

        std::unordered_map<string_type, int> umap = {{J_STR("e"), 5}, {J_STR("f"), 6}};
        CATCH_CHECK(fly::Json(umap).is_object());

        std::unordered_multimap<string_type, int> umultimap = {{J_STR("h"), 7}, {J_STR("i"), 8}};
        CATCH_CHECK(fly::Json(umultimap).is_object());
    }

    CATCH_SECTION("Fail to construct a JSON instance from object-like types")
    {
        std::map<string_type, int> map;

        // Reverse solidus must be followed by a valid escape symbol.
        map = {{J_STR("\\"), 1}};
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Expected escaped character after reverse solidus");

        map = {{J_STR("\\U"), 1}};
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Invalid escape character 'U'");

        // Quotes must be escaped.
        map = {{J_STR("\""), 1}};
        CATCH_CHECK_THROWS_JSON(fly::Json(map), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            map = {{string_type(1, static_cast<char_type>(ch)), 1}};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(map),
                "Character {:#04x} must be escaped",
                static_cast<char_type>(ch));
        }

        // Characters must be valid Unicode.
        map = {{reserved_codepoint(), 1}};
        CATCH_CHECK_THROWS_JSON(
            fly::Json(map),
            "Could not convert string-like type to a JSON string");
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

        std::multiset<string_type> multiset = {J_STR("a"), J_STR("b"), J_STR("c")};
        CATCH_CHECK(fly::Json(multiset).is_array());
        CATCH_CHECK_FALSE(fly::Json(multiset).is_object_like());

        std::multiset<string_type> set = {J_STR("d"), J_STR("e"), J_STR("f")};
        CATCH_CHECK(fly::Json(set).is_array());
        CATCH_CHECK_FALSE(fly::Json(set).is_object_like());

        std::multiset<string_type> unordered_multiset = {J_STR("g"), J_STR("h"), J_STR("i")};
        CATCH_CHECK(fly::Json(unordered_multiset).is_array());
        CATCH_CHECK_FALSE(fly::Json(unordered_multiset).is_object_like());

        std::multiset<string_type> unordered_set = {J_STR("j"), J_STR("k"), J_STR("l")};
        CATCH_CHECK(fly::Json(unordered_set).is_array());
        CATCH_CHECK_FALSE(fly::Json(unordered_set).is_object_like());

        std::vector<int> vector = {170, 180, 190, 200};
        CATCH_CHECK(fly::Json(vector).is_array());
        CATCH_CHECK_FALSE(fly::Json(vector).is_object_like());

        std::array<string_type, 2> object = {J_STR("nine"), J_STR("ten")};
        CATCH_CHECK(fly::Json(object).is_array());
        CATCH_CHECK(fly::Json(object).is_object_like());
    }

    CATCH_SECTION("Fail to construct a JSON instance from array-like types")
    {
        std::vector<string_type> vector;

        // Reverse solidus must be followed by a valid escape symbol.
        vector = {J_STR("\\")};
        CATCH_CHECK_THROWS_JSON(
            fly::Json(vector),
            "Expected escaped character after reverse solidus");

        vector = {J_STR("\\U")};
        CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Invalid escape character 'U'");

        // Quotes must be escaped.
        vector = {J_STR("\"")};
        CATCH_CHECK_THROWS_JSON(fly::Json(vector), "Character '\"' must be escaped");

        // Control characters must be escaped.
        for (std::uint32_t ch = 0; ch <= 0x1f; ++ch)
        {
            vector = {string_type(1, static_cast<char_type>(ch))};
            CATCH_CHECK_THROWS_JSON(
                fly::Json(vector),
                "Character {:#04x} must be escaped",
                static_cast<char_type>(ch));
        }

        // Characters must be valid Unicode.
        vector = {reserved_codepoint()};
        CATCH_CHECK_THROWS_JSON(
            fly::Json(vector),
            "Could not convert string-like type to a JSON string");
    }

    CATCH_SECTION("Construct a JSON instance from initializer lists")
    {
        const fly::Json empty {};
        CATCH_CHECK(fly::Json(empty).is_null());

        const fly::Json array {J_CHR('7'), 8, J_STR("nine"), 10};
        CATCH_CHECK(fly::Json(array).is_array());

        const fly::Json object {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK(fly::Json(object).is_object());

        const fly::Json almost {{J_STR("a"), 1}, {J_STR("b"), 2}, 4};
        CATCH_CHECK(fly::Json(almost).is_array());
    }

    CATCH_SECTION("Construct a JSON instance from another JSON instance via copy")
    {
        fly::Json string = J_STR("abc");
        CATCH_CHECK(fly::Json(string) == string);

        fly::Json object = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK(fly::Json(object) == object);

        fly::Json array = {J_CHR('7'), 8};
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
        fly::Json string = J_STR("abc");
        fly::Json string_copy(string);
        fly::Json string_move(std::move(string_copy));

        CATCH_CHECK(string_copy.is_null());
        CATCH_CHECK(string_move == string);
    }
}

CATCH_TEST_CASE("JsonPlainTypeConstruction", "[json]")
{
    CATCH_SECTION("Construct a JSON instance from null-like types")
    {
        CATCH_CHECK(fly::Json().is_null());
        CATCH_CHECK(fly::Json(nullptr).is_null());
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
}
