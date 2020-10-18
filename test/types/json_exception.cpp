#include "fly/types/json/json_exception.hpp"

#include "fly/types/json/json.hpp"

#include <catch2/catch.hpp>

#include <string>

CATCH_TEST_CASE("JsonException", "[json]")
{
    std::string string = "abc";

    CATCH_SECTION("Throw a base JsonException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::JsonException(string, "some message"),
            Catch::StartsWith("JsonException") && Catch::Contains("some message") &&
                Catch::Contains(string));
    }

    CATCH_SECTION("Throw a JsonIteratorException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::JsonIteratorException(string, "some message"),
            Catch::StartsWith("JsonIteratorException") && Catch::Contains("some message") &&
                Catch::Contains(string));
    }

    CATCH_SECTION("Throw a BadJsonComparisonException")
    {
        fly::Json number = 12389;

        CATCH_CHECK_THROWS_WITH(
            throw fly::BadJsonComparisonException(string, number),
            Catch::StartsWith("BadJsonComparisonException") && Catch::Contains(string) &&
                Catch::Contains(std::string(number)));
    }

    CATCH_SECTION("Throw a NullJsonException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::NullJsonException(string),
            Catch::StartsWith("NullJsonException") && Catch::Contains(string));
    }

    CATCH_SECTION("Throw an OutOfRangeJsonException")
    {
        CATCH_CHECK_THROWS_WITH(
            throw fly::OutOfRangeJsonException(string, 12389),
            Catch::StartsWith("OutOfRangeJsonException") && Catch::Contains(string) &&
                Catch::Contains("12389"));
    }
}
