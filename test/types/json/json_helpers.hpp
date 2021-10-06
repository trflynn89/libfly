#pragma once

#include "fly/traits/concepts.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/json/json_exception.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers.hpp"
#include "catch2/matchers/catch_matchers_exception.hpp"

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
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
        Catch::Matchers::ExceptionMessageMatcher(                                                  \
            fly::String::format("JsonException: " __VA_ARGS__)))

#define CATCH_CHECK_THROWS_ITERATOR(expression, ...)                                               \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::JsonIteratorException,                                                                \
        Catch::Matchers::ExceptionMessageMatcher(                                                  \
            fly::String::format("JsonIteratorException: " __VA_ARGS__)))

#define CATCH_CHECK_THROWS_BAD_COMPARISON(expression, json1, json2)                                \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::BadJsonComparisonException,                                                           \
        Catch::Matchers::ExceptionMessageMatcher(fly::String::format(                              \
            "BadJsonComparisonException: Cannot compare iterators of different JSON instances: "   \
            "({}) ({})",                                                                           \
            json1,                                                                                 \
            json2)))

#define CATCH_CHECK_THROWS_NULL(expression)                                                        \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::NullJsonException,                                                                    \
        Catch::Matchers::ExceptionMessageMatcher(                                                  \
            fly::String::format("NullJsonException: Cannot dereference an empty or past-the-end "  \
                                "iterator")))

#define CATCH_CHECK_THROWS_NULL_WITH(expression, json)                                             \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::NullJsonException,                                                                    \
        Catch::Matchers::ExceptionMessageMatcher(fly::String::format(                              \
            "NullJsonException: Cannot dereference an empty or past-the-end iterator: ({})",       \
            json)))

#define CATCH_CHECK_THROWS_OUT_OF_RANGE(expression, offset, json)                                  \
    CATCH_CHECK_THROWS_MATCHES(                                                                    \
        expression,                                                                                \
        fly::OutOfRangeJsonException,                                                              \
        Catch::Matchers::ExceptionMessageMatcher(fly::String::format(                              \
            "OutOfRangeJsonException: Offset {} is out-of-range: ({})",                            \
            offset,                                                                                \
            json)))

// These macros depend on the test defining a type named char_type for the string literals.
#define J_CHR(ch) FLY_CHR(char_type, ch)
#define J_STR(str) FLY_STR(char_type, str)
#define J_ARR(arr) FLY_ARR(char_type, arr)

#define CATCH_JSON_TEST_CASE(name)                                                                 \
    CATCH_TEMPLATE_TEST_CASE(                                                                      \
        name,                                                                                      \
        "[json]",                                                                                  \
        fly::json_null_type,                                                                       \
        fly::json_string_type,                                                                     \
        fly::json_object_type,                                                                     \
        fly::json_array_type,                                                                      \
        fly::json_boolean_type,                                                                    \
        fly::json_signed_integer_type,                                                             \
        fly::json_unsigned_integer_type,                                                           \
        fly::json_floating_point_type)

#define CATCH_JSON_STRING_TEST_CASE(name)                                                          \
    CATCH_TEMPLATE_PRODUCT_TEST_CASE(                                                              \
        name,                                                                                      \
        "[json]",                                                                                  \
        std::pair,                                                                                 \
        ((fly::json_null_type, std::string),                                                       \
         (fly::json_null_type, std::wstring),                                                      \
         (fly::json_null_type, std::u8string),                                                     \
         (fly::json_null_type, std::u16string),                                                    \
         (fly::json_null_type, std::u32string),                                                    \
         (fly::json_string_type, std::string),                                                     \
         (fly::json_string_type, std::wstring),                                                    \
         (fly::json_string_type, std::u8string),                                                   \
         (fly::json_string_type, std::u16string),                                                  \
         (fly::json_string_type, std::u32string),                                                  \
         (fly::json_object_type, std::string),                                                     \
         (fly::json_object_type, std::wstring),                                                    \
         (fly::json_object_type, std::u8string),                                                   \
         (fly::json_object_type, std::u16string),                                                  \
         (fly::json_object_type, std::u32string),                                                  \
         (fly::json_array_type, std::string),                                                      \
         (fly::json_array_type, std::wstring),                                                     \
         (fly::json_array_type, std::u8string),                                                    \
         (fly::json_array_type, std::u16string),                                                   \
         (fly::json_array_type, std::u32string),                                                   \
         (fly::json_boolean_type, std::string),                                                    \
         (fly::json_boolean_type, std::wstring),                                                   \
         (fly::json_boolean_type, std::u8string),                                                  \
         (fly::json_boolean_type, std::u16string),                                                 \
         (fly::json_boolean_type, std::u32string),                                                 \
         (fly::json_signed_integer_type, std::string),                                             \
         (fly::json_signed_integer_type, std::wstring),                                            \
         (fly::json_signed_integer_type, std::u8string),                                           \
         (fly::json_signed_integer_type, std::u16string),                                          \
         (fly::json_signed_integer_type, std::u32string),                                          \
         (fly::json_unsigned_integer_type, std::string),                                           \
         (fly::json_unsigned_integer_type, std::wstring),                                          \
         (fly::json_unsigned_integer_type, std::u8string),                                         \
         (fly::json_unsigned_integer_type, std::u16string),                                        \
         (fly::json_unsigned_integer_type, std::u32string),                                        \
         (fly::json_floating_point_type, std::string),                                             \
         (fly::json_floating_point_type, std::wstring),                                            \
         (fly::json_floating_point_type, std::u8string),                                           \
         (fly::json_floating_point_type, std::u16string),                                          \
         (fly::json_floating_point_type, std::u32string)))

namespace fly::test {

template <typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr inline bool is_object_or_array_or_string_v =
    fly::SameAsAny<T, fly::json_string_type, fly::json_object_type, fly::json_array_type>;

template <typename T, typename Other>
// NOLINTNEXTLINE(readability-identifier-naming)
constexpr inline bool is_null_or_other_type_v = fly::SameAsAny<T, fly::json_null_type, Other>;

template <typename T, typename StringType = fly::json_string_type>
fly::Json create_json()
{
    using char_type = typename StringType::value_type;

    if constexpr (std::is_same_v<T, fly::json_null_type>)
    {
        return nullptr;
    }
    else if constexpr (std::is_same_v<T, fly::json_string_type>)
    {
        return J_STR("abcdef");
    }
    else if constexpr (std::is_same_v<T, fly::json_object_type>)
    {
        return {{J_STR("a"), 1}, {J_STR("b"), 2}};
    }
    else if constexpr (std::is_same_v<T, fly::json_array_type>)
    {
        return {'7', 8, 9, 10};
    }
    else if constexpr (std::is_same_v<T, fly::json_boolean_type>)
    {
        return true;
    }
    else if constexpr (std::is_same_v<T, fly::json_signed_integer_type>)
    {
        return 1;
    }
    else if constexpr (std::is_same_v<T, fly::json_unsigned_integer_type>)
    {
        return 1U;
    }
    else if constexpr (std::is_same_v<T, fly::json_floating_point_type>)
    {
        return 1.0f;
    }
}

template <
    typename JsonType,
    typename StringType,
    typename Validator,
    typename Invalidator,
    bool AllowForType = std::is_same_v<JsonType, fly::json_object_type>>
void run_test_for_object_types(Validator validate, Invalidator invalidate)
{
    if constexpr (AllowForType)
    {
        validate.template operator()<
            std::map<StringType, int>,
            std::map<StringType, std::string>,
            std::map<StringType, fly::Json>>("map");

        validate.template operator()<
            std::multimap<StringType, int>,
            std::multimap<StringType, std::string>,
            std::multimap<StringType, fly::Json>>("multimap");

        validate.template operator()<
            std::unordered_map<StringType, int>,
            std::unordered_map<StringType, std::string>,
            std::unordered_map<StringType, fly::Json>>("unordered_map");

        validate.template operator()<
            std::unordered_multimap<StringType, int>,
            std::unordered_multimap<StringType, std::string>,
            std::unordered_multimap<StringType, fly::Json>>("unordered_multimap");
    }
    else
    {
        invalidate.template operator()<std::map<StringType, fly::Json>>("map");
        invalidate.template operator()<std::multimap<StringType, fly::Json>>("multimap");
        invalidate.template operator()<std::unordered_map<StringType, fly::Json>>("unordered_map");
        invalidate.template operator()<std::unordered_multimap<StringType, fly::Json>>(
            "unordered_multimap");
    }
}

template <
    typename JsonType,
    typename StringType,
    typename Validator,
    typename Invalidator,
    bool AllowForType = std::is_same_v<JsonType, fly::json_array_type>>
void run_test_for_array_types(Validator &&validate, Invalidator &&invalidate)
{
    if constexpr (AllowForType)
    {
        validate.template
        operator()<std::array<int, 4>, std::array<StringType, 4>, std::array<fly::Json, 4>>(
            "array");

        validate.template
        operator()<std::deque<int>, std::deque<StringType>, std::deque<fly::Json>>("deque");

        validate.template operator()<
            std::forward_list<int>,
            std::forward_list<StringType>,
            std::forward_list<fly::Json>>("forward_list");

        validate.template operator()<std::list<int>, std::list<StringType>, std::list<fly::Json>>(
            "list");

        validate.template
        operator()<std::multiset<int>, std::multiset<StringType>, std::multiset<fly::Json>>(
            "multiset");

        validate.template operator()<std::set<int>, std::set<StringType>, std::set<fly::Json>>(
            "set");

        validate.template operator()<
            std::unordered_multiset<int>,
            std::unordered_multiset<StringType>,
            std::unordered_multiset<fly::Json>>("unordered_multiset");

        validate.template operator()<
            std::unordered_set<int>,
            std::unordered_set<StringType>,
            std::unordered_set<fly::Json>>("unordered_set");

        validate.template
        operator()<std::vector<int>, std::vector<StringType>, std::vector<fly::Json>>("vector");
    }
    else
    {
        invalidate.template operator()<std::array<int, 4>>("array");
        invalidate.template operator()<std::deque<int>>("deque");
        invalidate.template operator()<std::forward_list<int>>("forward_list");
        invalidate.template operator()<std::list<int>>("list");
        invalidate.template operator()<std::multiset<int>>("multiset");
        invalidate.template operator()<std::set<int>>("set");
        invalidate.template operator()<std::unordered_multiset<int>>("unordered_multiset");
        invalidate.template operator()<std::unordered_set<int>>("unordered_set");
        invalidate.template operator()<std::vector<int>>("vector");
    }
}

} // namespace fly::test
