#pragma once

#include "fly/types/json/json_exception.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

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
            "NullJsonException: Cannot dereference an empty or past-the-end "                      \
            "iterator: (%s)",                                                                      \
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
