#include "fly/types/json/json_exception.hpp"

#include "fly/types/json/json.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/matchers/catch_matchers_contains.hpp"
#include "catch2/matchers/catch_matchers_string.hpp"

#include <string>

CATCH_TEST_CASE("JsonException", "[json]")
{
    std::string string = "abc";

    CATCH_SECTION("Throw a base JsonException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::JsonException(string, "some message"),
            Catch::Matchers::StartsWith("JsonException") &&
                Catch::Matchers::ContainsSubstring("some message") &&
                Catch::Matchers::ContainsSubstring(string));
    }

    CATCH_SECTION("Throw a JsonIteratorException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::JsonIteratorException(string, "some message"),
            Catch::Matchers::StartsWith("JsonIteratorException") &&
                Catch::Matchers::ContainsSubstring("some message") &&
                Catch::Matchers::ContainsSubstring(string));
    }

    CATCH_SECTION("Throw a BadJsonComparisonException")
    {
        fly::Json number = 12389;

        CATCH_CHECK_THROWS_WITH(
            throw fly::BadJsonComparisonException(string, number),
            Catch::Matchers::StartsWith("BadJsonComparisonException") &&
                Catch::Matchers::ContainsSubstring(string) &&
                Catch::Matchers::ContainsSubstring(std::string(number)));
    }

    CATCH_SECTION("Throw a NullJsonException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::NullJsonException(string),
            Catch::Matchers::StartsWith("NullJsonException") &&
                Catch::Matchers::ContainsSubstring(string));
    }

    CATCH_SECTION("Throw an OutOfRangeJsonException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::OutOfRangeJsonException(string, 12389),
            Catch::Matchers::StartsWith("OutOfRangeJsonException") &&
                Catch::Matchers::ContainsSubstring(string) &&
                Catch::Matchers::ContainsSubstring("12389"));

        std::ptrdiff_t offset;

        try
        {
            throw fly::OutOfRangeJsonException(string, 12389);
        }
        catch (fly::OutOfRangeJsonException const &ex)
        {
            offset = ex.offset();
        }

        CATCH_CHECK(offset == 12389);
    }
}
