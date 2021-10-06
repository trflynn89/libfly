#include "fly/types/json/concepts.hpp"

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
    "JsonConcepts",
    "[json]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using string_type = TestType;
    using char_type = typename TestType::value_type;
    using view_type = std::basic_string_view<char_type>;

    using array_type = std::array<int, 4>;
    using deque_type = std::deque<int>;
    using forward_list_type = std::forward_list<int>;
    using list_type = std::list<int>;
    using multiset_type = std::multiset<int>;
    using set_type = std::set<int>;
    using unordered_multiset_type = std::unordered_multiset<int>;
    using unordered_set_type = std::unordered_set<int>;
    using vector_type = std::vector<int>;

    using map_type = std::map<string_type, int>;
    using multimap_type = std::multimap<string_type, int>;
    using unordered_map_type = std::unordered_map<string_type, int>;
    using unordered_multimap_type = std::unordered_multimap<string_type, int>;

    using null_type = std::nullptr_t;
    using boolean_type = bool;
    using signed_integer_type = int;
    using unsigned_integer_type = unsigned int;
    using float_type = float;
    using double_type = double;
    using long_double_type = long double;

    CATCH_SECTION("Concepts for null-like JSON types")
    {
        CATCH_CHECK(fly::JsonNull<null_type>);

        CATCH_CHECK_FALSE(fly::JsonNull<array_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<list_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<map_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<set_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonNull<boolean_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<string_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonNull<double_type>);
    }

    CATCH_SECTION("Concepts for string-like JSON types")
    {
        CATCH_CHECK(fly::JsonString<const string_type>);
        CATCH_CHECK(fly::JsonString<string_type>);
        CATCH_CHECK_FALSE(fly::JsonString<const char_type *>);
        CATCH_CHECK_FALSE(fly::JsonString<char_type *>);
        CATCH_CHECK_FALSE(fly::JsonString<const char_type[]>);
        CATCH_CHECK_FALSE(fly::JsonString<char_type[]>);
        CATCH_CHECK_FALSE(fly::JsonString<const view_type>);
        CATCH_CHECK_FALSE(fly::JsonString<view_type>);

        CATCH_CHECK(fly::JsonStringLike<const string_type>);
        CATCH_CHECK(fly::JsonStringLike<string_type>);
        CATCH_CHECK(fly::JsonStringLike<const char_type *>);
        CATCH_CHECK(fly::JsonStringLike<char_type *>);
        CATCH_CHECK(fly::JsonStringLike<const char_type[]>);
        CATCH_CHECK(fly::JsonStringLike<char_type[]>);
        CATCH_CHECK(fly::JsonStringLike<const view_type>);
        CATCH_CHECK(fly::JsonStringLike<view_type>);

        CATCH_CHECK_FALSE(fly::JsonString<array_type>);
        CATCH_CHECK_FALSE(fly::JsonString<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonString<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonString<list_type>);
        CATCH_CHECK_FALSE(fly::JsonString<map_type>);
        CATCH_CHECK_FALSE(fly::JsonString<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonString<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonString<set_type>);
        CATCH_CHECK_FALSE(fly::JsonString<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonString<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonString<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonString<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonString<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonStringLike<array_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<list_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<map_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<set_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonString<null_type>);
        CATCH_CHECK_FALSE(fly::JsonString<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonString<boolean_type>);
        CATCH_CHECK_FALSE(fly::JsonString<float_type>);
        CATCH_CHECK_FALSE(fly::JsonString<double_type>);
        CATCH_CHECK_FALSE(fly::JsonString<const char_type>);
        CATCH_CHECK_FALSE(fly::JsonString<char_type>);

        CATCH_CHECK_FALSE(fly::JsonStringLike<null_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<boolean_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<float_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<double_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<const char_type>);
        CATCH_CHECK_FALSE(fly::JsonStringLike<char_type>);
    }

    CATCH_SECTION("Concepts for object-like JSON types")
    {
        CATCH_CHECK(fly::JsonObject<map_type>);
        CATCH_CHECK(fly::JsonObject<multimap_type>);
        CATCH_CHECK(fly::JsonObject<unordered_map_type>);
        CATCH_CHECK(fly::JsonObject<unordered_multimap_type>);

        CATCH_CHECK_FALSE(fly::JsonObject<std::map<int, int>>);
        CATCH_CHECK_FALSE(fly::JsonObject<std::multimap<int, int>>);
        CATCH_CHECK_FALSE(fly::JsonObject<std::unordered_map<int, int>>);
        CATCH_CHECK_FALSE(fly::JsonObject<std::unordered_multimap<int, int>>);

        CATCH_CHECK_FALSE(fly::JsonObject<array_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<list_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<set_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonObject<null_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<string_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<double_type>);
        CATCH_CHECK_FALSE(fly::JsonObject<boolean_type>);
    }

    CATCH_SECTION("Concepts for array-like JSON types")
    {
        CATCH_CHECK(fly::JsonArray<array_type>);
        CATCH_CHECK(fly::JsonArray<deque_type>);
        CATCH_CHECK(fly::JsonArray<forward_list_type>);
        CATCH_CHECK(fly::JsonArray<list_type>);
        CATCH_CHECK(fly::JsonArray<multiset_type>);
        CATCH_CHECK(fly::JsonArray<set_type>);
        CATCH_CHECK(fly::JsonArray<unordered_multiset_type>);
        CATCH_CHECK(fly::JsonArray<unordered_set_type>);
        CATCH_CHECK(fly::JsonArray<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonArray<map_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<unordered_multimap_type>);

        CATCH_CHECK_FALSE(fly::JsonArray<null_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<string_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<double_type>);
        CATCH_CHECK_FALSE(fly::JsonArray<boolean_type>);
    }

    CATCH_SECTION("Concepts for container JSON types")
    {
        CATCH_CHECK(fly::JsonContainer<string_type>);

        CATCH_CHECK(fly::JsonContainer<array_type>);
        CATCH_CHECK(fly::JsonContainer<deque_type>);
        CATCH_CHECK(fly::JsonContainer<forward_list_type>);
        CATCH_CHECK(fly::JsonContainer<list_type>);
        CATCH_CHECK(fly::JsonContainer<multiset_type>);
        CATCH_CHECK(fly::JsonContainer<set_type>);
        CATCH_CHECK(fly::JsonContainer<unordered_multiset_type>);
        CATCH_CHECK(fly::JsonContainer<unordered_set_type>);
        CATCH_CHECK(fly::JsonContainer<vector_type>);

        CATCH_CHECK(fly::JsonContainer<map_type>);
        CATCH_CHECK(fly::JsonContainer<multimap_type>);
        CATCH_CHECK(fly::JsonContainer<unordered_map_type>);
        CATCH_CHECK(fly::JsonContainer<unordered_multimap_type>);

        CATCH_CHECK_FALSE(fly::JsonContainer<null_type>);
        CATCH_CHECK_FALSE(fly::JsonContainer<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonContainer<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonContainer<double_type>);
        CATCH_CHECK_FALSE(fly::JsonContainer<boolean_type>);
    }

    CATCH_SECTION("Concepts for boolean-like JSON types")
    {
        CATCH_CHECK(fly::JsonBoolean<boolean_type>);

        CATCH_CHECK_FALSE(fly::JsonBoolean<array_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<list_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<map_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<set_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonBoolean<null_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<string_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonBoolean<double_type>);
    }

    CATCH_SECTION("Concepts for signed-integer-like JSON types")
    {
        CATCH_CHECK(fly::JsonSignedInteger<signed_integer_type>);

        CATCH_CHECK_FALSE(fly::JsonSignedInteger<array_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<list_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<map_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<set_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonSignedInteger<null_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<string_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<double_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<boolean_type>);
        CATCH_CHECK_FALSE(fly::JsonSignedInteger<unsigned_integer_type>);
    }

    CATCH_SECTION("Concepts for unsigned-integer-like JSON types")
    {
        CATCH_CHECK(fly::JsonUnsignedInteger<unsigned_integer_type>);

        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<array_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<list_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<map_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<set_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<null_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<string_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<double_type>);
        CATCH_CHECK_FALSE(fly::JsonUnsignedInteger<boolean_type>);
    }

    CATCH_SECTION("Concepts for floating-point-like JSON types")
    {
        CATCH_CHECK(fly::JsonFloatingPoint<float_type>);
        CATCH_CHECK(fly::JsonFloatingPoint<double_type>);
        CATCH_CHECK(fly::JsonFloatingPoint<long_double_type>);

        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<array_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<deque_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<forward_list_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<list_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<map_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<set_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<unordered_map_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<unordered_multimap_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<unordered_multiset_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<unordered_set_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<vector_type>);

        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<null_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<string_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonFloatingPoint<boolean_type>);
    }

    CATCH_SECTION("Concepts for iterable JSON types")
    {
        CATCH_CHECK(fly::JsonIterable<array_type>);
        CATCH_CHECK(fly::JsonIterable<deque_type>);
        CATCH_CHECK(fly::JsonIterable<forward_list_type>);
        CATCH_CHECK(fly::JsonIterable<list_type>);
        CATCH_CHECK(fly::JsonIterable<multiset_type>);
        CATCH_CHECK(fly::JsonIterable<set_type>);
        CATCH_CHECK(fly::JsonIterable<unordered_multiset_type>);
        CATCH_CHECK(fly::JsonIterable<unordered_set_type>);
        CATCH_CHECK(fly::JsonIterable<vector_type>);

        CATCH_CHECK(fly::JsonIterable<map_type>);
        CATCH_CHECK(fly::JsonIterable<multimap_type>);
        CATCH_CHECK(fly::JsonIterable<unordered_map_type>);
        CATCH_CHECK(fly::JsonIterable<unordered_multimap_type>);

        CATCH_CHECK_FALSE(fly::JsonIterable<null_type>);
        CATCH_CHECK_FALSE(fly::JsonIterable<string_type>);
        CATCH_CHECK_FALSE(fly::JsonIterable<signed_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonIterable<unsigned_integer_type>);
        CATCH_CHECK_FALSE(fly::JsonIterable<double_type>);
        CATCH_CHECK_FALSE(fly::JsonIterable<boolean_type>);
    }
}
