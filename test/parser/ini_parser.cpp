#include "fly/parser/ini_parser.hpp"

#include "catch2/catch_test_macros.hpp"

#include <filesystem>

CATCH_TEST_CASE("IniParser", "[parser]")
{
    fly::IniParser parser;

    CATCH_SECTION("Non-existing directory cannot be parsed")
    {
        auto parsed = parser.parse_file(std::filesystem::path("foo_abc") / "a.json");
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("Non-existing file cannot be parsed")
    {
        auto parsed = parser.parse_file(std::filesystem::temp_directory_path() / "a.json");
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("Empty file cannot be parsed")
    {
        const std::string contents;

        auto parsed = parser.parse_string(contents);
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("Empty section can be parsed")
    {
        const std::string contents("[section]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1);
        CATCH_CHECK(values["section"].size() == 0);
    }

    CATCH_SECTION("Single section with name-value pairs")
    {
        const std::string contents(
            "[section]\n"
            "name=John Doe\n"
            "address=USA");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1);
        CATCH_CHECK(values["section"].size() == 2);
        CATCH_CHECK(values["section"]["name"] == "John Doe");
        CATCH_CHECK(values["section"]["address"] == "USA");
    }

    CATCH_SECTION("Multiple section with name-value pairs")
    {
        const std::string contents(
            "[section1]\n"
            "name=John Doe\n"
            "age=26\n"
            "[section2]\n"
            "name=Jane Doe\n"
            "age=30.12\n"
            "[section3]\n"
            "name=Joe Doe\n"
            "noage=1\n");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 3);

        CATCH_CHECK(values["section1"].size() == 2);
        CATCH_CHECK(values["section1"]["name"] == "John Doe");
        CATCH_CHECK(values["section1"]["age"] == "26");

        CATCH_CHECK(values["section2"].size() == 2);
        CATCH_CHECK(values["section2"]["name"] == "Jane Doe");
        CATCH_CHECK(values["section2"]["age"] == "30.12");

        CATCH_CHECK(values["section3"].size() == 2);
        CATCH_CHECK(values["section3"]["name"] == "Joe Doe");
        CATCH_CHECK(values["section3"]["noage"] == "1");
    }

    CATCH_SECTION("Only existing section names are parsed")
    {
        const std::string contents(
            "[section]\n"
            "name=John Doe\n"
            "address=USA");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values["section"].size() == 2);
        CATCH_CHECK_THROWS_AS(values["bad-section"], fly::JsonException);
        CATCH_CHECK_THROWS_AS(values["section-bad"], fly::JsonException);
    }

    CATCH_SECTION("Commented out sections are not parsed")
    {
        const std::string contents(
            "[section]\n"
            "name=John Doe\n"
            "; [other-section]\n"
            "; name=Jane Doe\n");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1);
        CATCH_CHECK(values["section"].size() == 1);
        CATCH_CHECK_THROWS_AS(values["other-section"], fly::JsonException);
    }

    CATCH_SECTION("Extra whitespace is ignored")
    {
        const std::string contents(
            "   [section   ]  \n"
            "\t\t\n   name=John Doe\t  \n"
            "\taddress  = USA\t \r \n");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1);
        CATCH_CHECK(values["section"].size() == 2);
        CATCH_CHECK(values["section"]["name"] == "John Doe");
        CATCH_CHECK(values["section"]["address"] == "USA");
    }

    CATCH_SECTION("Extra whitespace between quotes is not ignored")
    {
        const std::string contents(
            "[section]\n"
            "name=\"  John Doe  \"\n"
            "address= \t '\\tUSA'");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1);
        CATCH_CHECK(values["section"].size() == 2);
        CATCH_CHECK(values["section"]["name"] == "  John Doe  ");
        CATCH_CHECK(values["section"]["address"] == "\\tUSA");
    }

    CATCH_SECTION("Duplicate sections override existing sections")
    {
        const std::string contents1(
            "[section]\n"
            "name=John Doe\n"
            "[section]\n"
            "name=Jane Doe\n");

        auto parsed1 = parser.parse_string(contents1);
        CATCH_REQUIRE(parsed1.has_value());

        const fly::Json values1 = *std::move(parsed1);
        CATCH_CHECK(values1.size() == 1);
        CATCH_CHECK(values1["section"].size() == 1);
        CATCH_CHECK(values1["section"]["name"] == "Jane Doe");

        const std::string contents2(
            "[  \tsection]\n"
            "name=John Doe\n"
            "[section  ]\n"
            "name=Jane Doe\n");

        auto parsed2 = parser.parse_string(contents1);
        CATCH_REQUIRE(parsed2.has_value());

        const fly::Json values2 = *std::move(parsed2);
        CATCH_CHECK(values2.size() == 1);
        CATCH_CHECK(values2["section"].size() == 1);
        CATCH_CHECK(values2["section"]["name"] == "Jane Doe");
    }

    CATCH_SECTION("Duplicate values override existing values")
    {
        const std::string contents(
            "[section]\n"
            "name=John Doe\n"
            "name=Jane Doe\n");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        const fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1);
        CATCH_CHECK(values["section"].size() == 1);
        CATCH_CHECK(values["section"]["name"] == "Jane Doe");
    }

    CATCH_SECTION("Imbalanced braces on section names cannot be parsed")
    {
        const std::string contents1(
            "[section\n"
            "name=John Doe\n");

        const std::string contents2(
            "section]\n"
            "name=John Doe\n");

        CATCH_CHECK_FALSE(parser.parse_string(contents1).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents2).has_value());
    }

    CATCH_SECTION("Imbalanced quotes on values cannot be parsed")
    {
        const std::string contents1(
            "[section]\n"
            "name=\"John Doe\n");

        const std::string contents2(
            "[section]\n"
            "name=John Doe\"\n");
        const std::string contents3(
            "[section]\n"
            "name='John Doe\n");

        const std::string contents4(
            "[section]\n"
            "name=John Doe'\n");

        const std::string contents5(
            "[section]\n"
            "name=\"John Doe'\n");

        const std::string contents6(
            "[section]\n"
            "name='John Doe\"\n");

        CATCH_CHECK_FALSE(parser.parse_string(contents1).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents2).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents3).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents4).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents5).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents6).has_value());
    }

    CATCH_SECTION("Section names and values names cannot be quoted")
    {
        const std::string contents1(
            "[section]\n"
            "\"name\"=John Doe\n");

        const std::string contents2(
            "[section]\n"
            "\'name\'=John Doe\n");

        const std::string contents3(
            "[\"section\"]\n"
            "name=John Doe\n");

        const std::string contents4(
            "[\'section\']\n"
            "name=John Doe\n");

        const std::string contents5(
            "\"[section]\"\n"
            "name=John Doe\n");

        const std::string contents6(
            "\'[section]\'\n"
            "name=John Doe\n");

        CATCH_CHECK_FALSE(parser.parse_string(contents1).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents2).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents3).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents4).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents5).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents6).has_value());
    }

    CATCH_SECTION("Secondary assignment operators are captured as part of the value")
    {
        const std::string contents1(
            "[section]\n"
            "name=John=Doe\n");
        const std::string contents2(
            "[section]\n"
            "name=\"John=Doe\"\n");

        auto parsed1 = parser.parse_string(contents1);
        CATCH_REQUIRE(parsed1.has_value());

        const fly::Json values1 = *std::move(parsed1);
        CATCH_CHECK(values1.size() == 1);
        CATCH_CHECK(values1["section"].size() == 1);
        CATCH_CHECK(values1["section"]["name"] == "John=Doe");

        auto parsed2 = parser.parse_string(contents2);
        CATCH_REQUIRE(parsed2.has_value());

        const fly::Json values2 = *std::move(parsed2);
        CATCH_CHECK(values2.size() == 1);
        CATCH_CHECK(values2["section"].size() == 1);
        CATCH_CHECK(values2["section"]["name"] == "John=Doe");
    }

    CATCH_SECTION("Missing an assignment operator cannot be parsed")
    {
        const std::string contents(
            "[section]\n"
            "name\n");

        CATCH_CHECK_FALSE(parser.parse_string(contents).has_value());
    }

    CATCH_SECTION("Empty values cannot be parsed")
    {
        const std::string contents(
            "[section]\n"
            "name=\n");

        CATCH_CHECK_FALSE(parser.parse_string(contents).has_value());
    }

    CATCH_SECTION("Assignments before a section name cannot be parsed")
    {
        const std::string contents1(
            "name=John Doe\n"
            "[section]\n");

        const std::string contents2(
            "name=\n"
            "[section]\n");

        const std::string contents3(
            "name\n"
            "[section]\n");

        CATCH_CHECK_FALSE(parser.parse_string(contents1).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents2).has_value());
        CATCH_CHECK_FALSE(parser.parse_string(contents3).has_value());
    }

    CATCH_SECTION("Section names with invalid JSON string contents cannot be parsed")
    {
        const std::string contents(
            "[\xff]\n"
            "name=John Doe\n"
            "address=USA");

        CATCH_CHECK_FALSE(parser.parse_string(contents).has_value());
    }

    CATCH_SECTION("Value names with invalid JSON string contents cannot be parsed")
    {
        const std::string contents(
            "[section]\n"
            "\xff=John Doe\n"
            "address=USA");

        CATCH_CHECK_FALSE(parser.parse_string(contents).has_value());
    }

    CATCH_SECTION("Values with invalid JSON string contents cannot be parsed")
    {
        const std::string contents(
            "[section]\n"
            "name=John Doe\n"
            "address=\xff");

        CATCH_CHECK_FALSE(parser.parse_string(contents).has_value());
    }

    CATCH_SECTION("The parser is re-entrant")
    {
        const std::string contents(
            "[section]\n"
            "name=John Doe\n"
            "address=USA");

        for (int i = 0; i < 5; ++i)
        {
            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            const fly::Json values = *std::move(parsed);
            CATCH_CHECK(values.size() == 1);
            CATCH_CHECK(values["section"].size() == 2);
            CATCH_CHECK(values["section"]["name"] == "John Doe");
            CATCH_CHECK(values["section"]["address"] == "USA");
        }
    }
}
