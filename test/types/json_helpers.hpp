#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/json/json_exception.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#define CATCH_CHECK_THROWS_JSON(expression, ...)                                                   \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::JsonException,                                                                        \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("JsonException: " __VA_ARGS__)))

#define CATCH_CHECK_THROWS_ITERATOR(expression, ...)                                               \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::JsonIteratorException,                                                                \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("JsonIteratorException: " __VA_ARGS__)))

#define CATCH_CHECK_THROWS_BAD_COMPARISON(expression, json1, json2)                                \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::BadJsonComparisonException,                                                           \
        Catch::Matchers::Exception::ExceptionMessageMatcher(fly::String::format(                   \
            "BadJsonComparisonException: Cannot compare iterators of different JSON instances: "   \
            "(%s) (%s)",                                                                           \
            json1,                                                                                 \
            json2)))

#define CATCH_CHECK_THROWS_NULL(expression)                                                        \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::NullJsonException,                                                                    \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("NullJsonException: Cannot dereference an empty or past-the-end "  \
                                "iterator")))

#define CATCH_CHECK_THROWS_NULL_WITH(expression, json)                                             \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::NullJsonException,                                                                    \
        Catch::Matchers::Exception::ExceptionMessageMatcher(fly::String::format(                   \
            "NullJsonException: Cannot dereference an empty or past-the-end iterator: (%s)",       \
            json)))

#define CATCH_CHECK_THROWS_OUT_OF_RANGE(expression, offset, json)                                  \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::OutOfRangeJsonException,                                                              \
        Catch::Matchers::Exception::ExceptionMessageMatcher(fly::String::format(                   \
            "OutOfRangeJsonException: Offset %d is out-of-range: (%s)",                            \
            offset,                                                                                \
            json)))

// These macros depend on the test defining a type named char_type for the string literals.
#define J_CHR(ch) FLY_CHR(char_type, ch)
#define J_STR(str) FLY_STR(char_type, str)

#define CATCH_JSON_TEST_CASE(name)                                                                 \
    CATCH_TEMPLATE_TEST_CASE(                                                                      \
        name,                                                                                      \
        "[json]",                                                                                  \
        fly::JsonTraits::null_type,                                                                \
        fly::JsonTraits::string_type,                                                              \
        fly::JsonTraits::object_type,                                                              \
        fly::JsonTraits::array_type,                                                               \
        fly::JsonTraits::boolean_type,                                                             \
        fly::JsonTraits::signed_type,                                                              \
        fly::JsonTraits::unsigned_type,                                                            \
        fly::JsonTraits::float_type)

#define CATCH_JSON_STRING_TEST_CASE(name)                                                          \
    CATCH_TEMPLATE_PRODUCT_TEST_CASE(                                                              \
        name,                                                                                      \
        "[json]",                                                                                  \
        std::pair,                                                                                 \
        ((fly::JsonTraits::null_type, std::string),                                                \
         (fly::JsonTraits::null_type, std::wstring),                                               \
         (fly::JsonTraits::null_type, std::u8string),                                              \
         (fly::JsonTraits::null_type, std::u16string),                                             \
         (fly::JsonTraits::null_type, std::u32string),                                             \
         (fly::JsonTraits::string_type, std::string),                                              \
         (fly::JsonTraits::string_type, std::wstring),                                             \
         (fly::JsonTraits::string_type, std::u8string),                                            \
         (fly::JsonTraits::string_type, std::u16string),                                           \
         (fly::JsonTraits::string_type, std::u32string),                                           \
         (fly::JsonTraits::object_type, std::string),                                              \
         (fly::JsonTraits::object_type, std::wstring),                                             \
         (fly::JsonTraits::object_type, std::u8string),                                            \
         (fly::JsonTraits::object_type, std::u16string),                                           \
         (fly::JsonTraits::object_type, std::u32string),                                           \
         (fly::JsonTraits::array_type, std::string),                                               \
         (fly::JsonTraits::array_type, std::wstring),                                              \
         (fly::JsonTraits::array_type, std::u8string),                                             \
         (fly::JsonTraits::array_type, std::u16string),                                            \
         (fly::JsonTraits::array_type, std::u32string),                                            \
         (fly::JsonTraits::boolean_type, std::string),                                             \
         (fly::JsonTraits::boolean_type, std::wstring),                                            \
         (fly::JsonTraits::boolean_type, std::u8string),                                           \
         (fly::JsonTraits::boolean_type, std::u16string),                                          \
         (fly::JsonTraits::boolean_type, std::u32string),                                          \
         (fly::JsonTraits::signed_type, std::string),                                              \
         (fly::JsonTraits::signed_type, std::wstring),                                             \
         (fly::JsonTraits::signed_type, std::u8string),                                            \
         (fly::JsonTraits::signed_type, std::u16string),                                           \
         (fly::JsonTraits::signed_type, std::u32string),                                           \
         (fly::JsonTraits::unsigned_type, std::string),                                            \
         (fly::JsonTraits::unsigned_type, std::wstring),                                           \
         (fly::JsonTraits::unsigned_type, std::u8string),                                          \
         (fly::JsonTraits::unsigned_type, std::u16string),                                         \
         (fly::JsonTraits::unsigned_type, std::u32string),                                         \
         (fly::JsonTraits::float_type, std::string),                                               \
         (fly::JsonTraits::float_type, std::wstring),                                              \
         (fly::JsonTraits::float_type, std::u8string),                                             \
         (fly::JsonTraits::float_type, std::u16string),                                            \
         (fly::JsonTraits::float_type, std::u32string)))

namespace fly::test {

template <typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool is_object_or_array_or_string_v = fly::any_same_v<
    T,
    fly::JsonTraits::string_type,
    fly::JsonTraits::object_type,
    fly::JsonTraits::array_type>;

template <typename T, typename Other>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool is_null_or_other_type_v =
    fly::any_same_v<T, fly::JsonTraits::null_type, Other>;

template <typename T, typename StringType = fly::JsonTraits::string_type>
fly::Json create_json()
{
    using char_type = typename StringType::value_type;

    if constexpr (std::is_same_v<T, fly::JsonTraits::null_type>)
    {
        return nullptr;
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::string_type>)
    {
        return J_STR("abcdef");
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::object_type>)
    {
        return {{J_STR("a"), 1}, {J_STR("b"), 2}};
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::array_type>)
    {
        return {'7', 8, 9, 10};
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::boolean_type>)
    {
        return true;
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::signed_type>)
    {
        return 1;
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::unsigned_type>)
    {
        return 1U;
    }
    else if constexpr (std::is_same_v<T, fly::JsonTraits::float_type>)
    {
        return 1.0f;
    }
}

template <typename JsonType, typename StringType, typename Validator, typename Invalidator>
void run_test_for_object_types(Validator validate, Invalidator invalidate)
{
    if constexpr (std::is_same_v<JsonType, fly::JsonTraits::object_type>)
    {
        std::map<StringType, int> map1;
        std::map<StringType, std::string> map2;
        std::map<StringType, fly::Json> map3;
        validate("map", map1, map2, map3);

        std::multimap<StringType, int> multimap1;
        std::multimap<StringType, std::string> multimap2;
        std::multimap<StringType, fly::Json> multimap3;
        validate("multimap", multimap1, multimap2, multimap3);

        std::unordered_map<StringType, int> unordered_map1;
        std::unordered_map<StringType, std::string> unordered_map2;
        std::unordered_map<StringType, fly::Json> unordered_map3;
        validate("unordered_map", unordered_map1, unordered_map2, unordered_map3);

        std::unordered_multimap<StringType, int> unordered_multimap1;
        std::unordered_multimap<StringType, std::string> unordered_multimap2;
        std::unordered_multimap<StringType, fly::Json> unordered_multimap3;
        validate(
            "unordered_multimap",
            unordered_multimap1,
            unordered_multimap2,
            unordered_multimap3);
    }
    else
    {
        std::map<StringType, fly::Json> map;
        invalidate("map", map);

        std::multimap<StringType, fly::Json> multimap;
        invalidate("multimap", multimap);

        std::unordered_map<StringType, fly::Json> unordered_map;
        invalidate("unordered_map", unordered_map);

        std::unordered_multimap<StringType, fly::Json> unordered_multimap;
        invalidate("unordered_multimap", unordered_multimap);
    }
}

template <
    typename JsonType,
    typename StringType,
    typename Validator2,
    typename Validator3,
    typename Invalidator>
void run_test_for_array_types(Validator2 validate2, Validator3 validate3, Invalidator invalidate)
{
    if constexpr (std::is_same_v<JsonType, fly::JsonTraits::array_type>)
    {
        std::array<int, 4> array1;
        std::array<StringType, 4> array2;
        std::array<fly::Json, 4> array3;
        validate3("array", array1, array2, array3);

        std::deque<int> deque1;
        std::deque<StringType> deque2;
        std::deque<fly::Json> deque3;
        validate3("deque", deque1, deque2, deque3);

        std::forward_list<int> forward_list1;
        std::forward_list<StringType> forward_list2;
        std::forward_list<fly::Json> forward_list3;
        validate3("forward_list", forward_list1, forward_list2, forward_list3);

        std::list<int> list1;
        std::list<StringType> list2;
        std::list<fly::Json> list3;
        validate3("list", list1, list2, list3);

        std::multiset<int> multiset1;
        std::multiset<StringType> multiset2;
        // std::multiset<fly::Json> multiset3;
        validate2("multiset", multiset1, multiset2);

        std::set<int> set1;
        std::set<StringType> set2;
        // std::set<fly::Json> set3;
        validate2("set", set1, set2);

        std::unordered_multiset<int> unordered_multiset1;
        std::unordered_multiset<StringType> unordered_multiset2;
        // std::unordered_multiset<fly::Json> unordered_multiset3;
        validate2("unordered_multiset", unordered_multiset1, unordered_multiset2);

        std::unordered_set<int> unordered_set1;
        std::unordered_set<StringType> unordered_set2;
        // std::unordered_set<fly::Json> unordered_set3;
        validate2("unordered_set", unordered_set1, unordered_set2);

        std::vector<int> vector1;
        std::vector<StringType> vector2;
        std::vector<fly::Json> vector3;
        validate3("vector", vector1, vector2, vector3);
    }
    else
    {
        std::array<int, 4> array;
        invalidate("array", array);

        std::deque<int> deque;
        invalidate("deque", deque);

        std::forward_list<int> forward_list;
        invalidate("forward_list", forward_list);

        std::list<int> list;
        invalidate("list", list);

        std::multiset<int> multiset;
        invalidate("multiset", multiset);

        std::set<int> set;
        invalidate("set", set);

        std::unordered_multiset<int> unordered_multiset;
        invalidate("unordered_multiset", unordered_multiset);

        std::unordered_set<int> unordered_set;
        invalidate("unordered_set", unordered_set);

        std::vector<int> vector;
        invalidate("vector", vector);
    }
}

} // namespace fly::test
