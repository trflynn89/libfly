#include "fly/parser/json_parser.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

TEST_CASE("JsonParser", "[parser]")
{
    fly::JsonParser parser;

    auto validate_fail_raw = [&](const std::string &test) {
        CAPTURE(test);
        CHECK_FALSE(parser.parse_string(test).has_value());
    };

    auto validate_fail = [&](const std::string &test) {
        validate_fail_raw(fly::String::format("{ \"a\" : %s }", test));
    };

    auto validate_pass_raw =
        [&](const std::string &test, const std::string &key, const fly::Json &expected) {
            CAPTURE(test);

            std::optional<fly::Json> actual = parser.parse_string(test);
            REQUIRE(actual.has_value());

            if (expected.is_float())
            {
                CHECK(double(actual->at(key)) == Approx(double(expected)));
            }
            else
            {
                CHECK(actual->at(key) == expected);
            }

            std::stringstream ss;
            ss << actual.value();

            std::optional<fly::Json> repeat = parser.parse_string(ss.str());
            REQUIRE(repeat.has_value());

            CHECK(actual.value() == repeat.value());
        };

    auto validate_pass = [&](const std::string &test, const fly::Json &expected) {
        validate_pass_raw(fly::String::format("{ \"a\" : %s }", test), "a", expected);
    };

    SECTION("JSON_checker test suite: https://www.json.org/JSON_checker/")
    {
        // The following files are excluded from this test:
        // - fail18.json: The parser has no max-depth

        // Get the path to the JSON checker directory.
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "json_checker";

        // Validate each JSON file in the JSON checker directory.
        for (const auto &it : std::filesystem::directory_iterator(path))
        {
            const auto file = it.path().filename();
            CAPTURE(file);

            if (fly::String::starts_with(file.string(), "pass"))
            {
                CHECK(parser.parse_file(it.path()).has_value());
            }
            else if (fly::String::starts_with(file.string(), "fail"))
            {
                CHECK_FALSE(parser.parse_file(it.path()).has_value());
            }
            else
            {
                FAIL("Unrecognized JSON file: " << file);
            }
        }
    }

    SECTION("Google's json-test-suite: https://code.google.com/archive/p/json-test-suite/")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "google_json_test_suite";

        CHECK(parser.parse_file(path / "sample.json").has_value());
    }

    SECTION("JSON Parsing Test Suite for RFC-8259 compliance: https://github.com/nst/JSONTestSuite")
    {
        // The following files are excluded from this test:
        // - n_structure_100000_opening_arrays.json: Causes stack overflow
        // - n_structure_open_array_object.json: Causes stack overflow
        // - i_number_double_huge_neg_exp.json: Platform dependent (fails Windows)

        // Indeterminate files expected to pass
        std::vector<std::string> i_pass = {
            "i_structure_500_nested_arrays.json", // No enforced depth limit
            "i_structure_UTF-8_BOM_empty_object.json", // Byte order mark is handled
            "i_string_UTF-16LE_with_BOM.json", // Byte order mark is handled
        };

        // JSONTestSuite contains test files that aren't only objects or arrays.
        fly::JsonParser type_parser(fly::JsonParser::Features::AllowAnyType);

        // Get the path to the JSONTestSuite directory.
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "nst_json_test_suite";

        // Validate each JSON file in the JSONTestSuite directory.
        for (const auto &it : std::filesystem::directory_iterator(path))
        {
            const auto file = it.path().filename();
            CAPTURE(file);

            if (fly::String::starts_with(file.string(), 'y'))
            {
                CHECK(type_parser.parse_file(it.path()).has_value());
            }
            else if (fly::String::starts_with(file.string(), 'n'))
            {
                CHECK_FALSE(type_parser.parse_file(it.path()).has_value());
            }
            else if (fly::String::starts_with(file.string(), 'i'))
            {
                if (std::find(i_pass.begin(), i_pass.end(), file) != i_pass.end())
                {
                    CHECK(type_parser.parse_file(it.path()).has_value());
                }
                else
                {
                    CHECK_FALSE(type_parser.parse_file(it.path()).has_value());
                }
            }
            else
            {
                FAIL("Unrecognized JSON file: " << file);
            }
        }
    }

    SECTION("Big List of Naughty Strings: https://github.com/minimaxir/big-list-of-naughty-strings")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "big_list_of_naughty_strings";

        auto parsed = parser.parse_file(path / "blns.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CHECK(values.size() == 507);

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            CHECK(values[i].is_string());
        }
    }

    SECTION("All Unicode characters")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "all_unicode.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CHECK(values.size() == 1112064);
    }

    SECTION("String with UTF-8 encoding is parsed as-is")
    {
        const std::string contents(u8"{\"encoding\": \"UTF-8\"}");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-8");
    }

    SECTION("File with UTF-8 byte order mark is parsed as-is")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_8.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-8");
    }

    SECTION("String with UTF-16 encoding is converted to UTF-8")
    {
        const std::u16string contents(u"{\"encoding\": \"UTF-16\"}");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-16");

        parsed = parser.parse_string(std::u16string(1, 0xd800));
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("File with UTF-16 big endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_16_big_endian.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-16 BE");

        parsed = parser.parse_file(path / "utf_16_big_endian_invalid.json");
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("File with UTF-16 little endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_16_little_endian.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-16 LE");

        parsed = parser.parse_file(path / "utf_16_little_endian_invalid.json");
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("String with UTF-32 encoding is converted to UTF-8")
    {
        const std::u32string contents(U"{\"encoding\": \"UTF-32\"}");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-32");

        parsed = parser.parse_string(std::u32string(1, 0xd800));
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("File with UTF-32 big endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_32_big_endian.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-32 BE");

        parsed = parser.parse_file(path / "utf_32_big_endian_invalid.json");
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("File with UTF-32 little endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_32_little_endian.json");
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CHECK(encoded_encoding == "UTF-32 LE");

        parsed = parser.parse_file(path / "utf_32_big_endian_invalid.json");
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("Non-existing directory cannot be parsed")
    {
        auto parsed = parser.parse_file(std::filesystem::path("foo_abc") / "a.json");
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("Non-existing file cannot be parsed")
    {
        auto parsed = parser.parse_file(std::filesystem::temp_directory_path() / "a.json");
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("Empty file cannot be parsed")
    {
        const std::string contents;

        auto parsed = parser.parse_string(contents);
        CHECK_FALSE(parsed.has_value());
    }

    SECTION("Empty JSON object can be parsed")
    {
        const std::string contents("{}");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CHECK(values.is_object());
        CHECK(values.size() == 0);
    }

    SECTION("Empty JSON array can be parsed")
    {
        const std::string contents("[]");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CHECK(values.is_array());
        CHECK(values.size() == 0);
    }

    SECTION("Nested empty JSON object can be parsed")
    {
        const std::string contents("[{}]");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CHECK(values.is_array());
        CHECK(values.size() == 1);

        values = values[0];
        CHECK(values.is_object());
        CHECK(values.size() == 0);
    }

    SECTION("Nested empty JSON array can be parsed")
    {
        const std::string contents("[[]]");

        auto parsed = parser.parse_string(contents);
        REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CHECK(values.is_array());
        CHECK(values.size() == 1);

        values = values[0];
        CHECK(values.is_array());
        CHECK(values.size() == 0);
    }

    SECTION("Empty key/value strings can be parsed")
    {
        {
            const std::string contents("{\"a\" : \"\" }");

            auto parsed = parser.parse_string(contents);
            REQUIRE(parsed.has_value());

            fly::Json values = std::move(parsed.value());
            CHECK(values["a"].is_string());
            CHECK(values["a"].size() == 0);
            CHECK(values["a"] == "");
        }
        {
            const std::string contents("{\"\" : \"a\" }");

            auto parsed = parser.parse_string(contents);
            REQUIRE(parsed.has_value());

            fly::Json values = std::move(parsed.value());
            CHECK(values[""].is_string());
            CHECK(values[""].size() == 1);
            CHECK(values[""] == "a");
        }
        {
            const std::string contents("{\"\" : \"\" }");

            auto parsed = parser.parse_string(contents);
            REQUIRE(parsed.has_value());

            fly::Json values = std::move(parsed.value());
            CHECK(values[""].is_string());
            CHECK(values[""].size() == 0);
            CHECK(values[""] == "");
        }
    }

    SECTION("JSON types that are not objects or arrays cannot be parsed by default")
    {
        validate_fail_raw("\"\"");
        validate_fail_raw("true");
        validate_fail_raw("1");
        validate_fail_raw("-1");
        validate_fail_raw("3.14");
        validate_fail_raw("null");
    }

    SECTION("Badly formed JSON objects")
    {
        validate_fail_raw(":");
        validate_fail_raw(",");
        validate_fail_raw("a");
        validate_fail_raw("\"a\"");
        validate_fail_raw("{");
        validate_fail_raw("}");
        validate_fail_raw("{ : }");
        validate_fail_raw("{ , }");
        validate_fail_raw("{ 1 }");
        validate_fail_raw("{ { } }");
        validate_fail_raw("{ [ ] }");
        validate_fail_raw("{ \"a }");
        validate_fail_raw("{ a\" }");
        validate_fail_raw("{ \"a\" }");
        validate_fail_raw("{ \"a\" : }");
        validate_fail_raw("{ \"a\" , }");
        validate_fail_raw("{ \"a\" : : 1 }");
        validate_fail_raw("{ \"a\" , : 1 }");
        validate_fail_raw("{ \"a\" : , 1 }");
        validate_fail_raw("{ \"a : 1 }");
        validate_fail_raw("{ a\" : 1 }");
        validate_fail_raw("{ \"a\" : 1 ");
        validate_fail_raw("{ \"a\" { }");
        validate_fail_raw("{ \"a\" : { }");
        validate_fail_raw("{ \"a\" [");
        validate_fail_raw("{ \"a\" : [");
        validate_fail_raw("{ \"a\" ]");
        validate_fail_raw("{ \"a\" : ]");
        validate_fail_raw("{ \"a\" tru }");
        validate_fail_raw("{ \"a\" : tru }");
        validate_fail_raw("{ \"a\" flse }");
        validate_fail_raw("{ \"a\" : flse }");
        validate_fail_raw("{ \"a\" 1, }");
        validate_fail_raw("{ \"a\" : 1");
        validate_fail_raw("{ \"a\" : ,");
        validate_fail_raw("{ \"a\" : 1, }");
        validate_fail_raw("{ \"a\" : 1 { }");
        validate_fail_raw("{ \"a\" : 1 { } }");
        validate_fail_raw("{ \"a\" : 1, { }");
        validate_fail_raw("{ \"a\" : \"\\");
        validate_fail_raw("{ \"a\" : \"\x01\" }");
        validate_fail_raw("{ \"\x01\" : \"a\" }");
        validate_fail_raw("{ 1 : 1 }");
    }

    SECTION("Badly formed JSON arrays")
    {
        validate_fail_raw("[");
        validate_fail_raw("]");
        validate_fail_raw("[ : ]");
        validate_fail_raw("[ , ]");
        validate_fail_raw("[ \"a ]");
        validate_fail_raw("[ a\" ]");
        validate_fail_raw("[ \"a\" : ]");
        validate_fail_raw("[ \"a : 1 ]");
        validate_fail_raw("[ a\" : 1 ]");
        validate_fail_raw("[ \"a\", 1");
        validate_fail_raw("[ \"a\" 1 ]");
        validate_fail_raw("[ \"a\" [ ]");
        validate_fail_raw("[ \"a\", [ ]");
        validate_fail_raw("[ \"a\" [");
        validate_fail_raw("[ \"a\", [");
        validate_fail_raw("[ \"a\", ]");
        validate_fail_raw("[ \"a\" true ]");
        validate_fail_raw("[ \"a\", tru ]");
        validate_fail_raw("[ \"a\" false ]");
        validate_fail_raw("[ \"a\", flse ]");
        validate_fail_raw("[ \"a\" 1, ]");
        validate_fail_raw("[ \"a\", ,");
        validate_fail_raw("[ \"a\", 1, ]");
        validate_fail_raw("[ \"a\", 1 [ ]");
        validate_fail_raw("[ \"a\", 1 [ ] ]");
        validate_fail_raw("[ \"a\", \"\\");
        validate_fail_raw("[ \"a\", \"\x01\" ]");
    }

    SECTION("Whitespace is ignored where applicable, and compliant with JSON strings")
    {
        validate_pass_raw("{ \"a\" : 1 }", "a", 1);
        validate_pass_raw("\n{ \n \"a\" \n : \n \t\t 1 \r \n }\n", "a", 1);

        validate_fail_raw("{ \"a\t\" : 1 }");
        validate_fail_raw("{ \"a\n\" : 1 }");
        validate_fail_raw("{ \"a\r\" : 1 }");
        validate_fail_raw("{ \"a\" : \"b\n\" }");
        validate_fail_raw("{ \"a\" : \"b\r\" }");
        validate_fail_raw("{ \"a\" : \"b\t\" }");
    }

    SECTION("Detection of valid numeric JSON types")
    {
        validate_pass("1", 1);
        validate_pass("-1", -1);
        validate_pass("1.2", 1.2);
        validate_pass("-1.2", -1.2);

        validate_pass("1.2e1", 12.0);
        validate_pass("1.2E1", 12.0);
        validate_pass("1.2e+1", 12.0);
        validate_pass("1.2E+1", 12.0);
        validate_pass("1.2e-1", 0.12);
        validate_pass("1.2E-1", 0.12);
    }

    SECTION("Detection of invalid numeric JSON types")
    {
        validate_fail("+1");
        validate_fail("01");
        validate_fail("+1.2");
        validate_fail("1.2.1");

        validate_fail("1abc");
        validate_fail("-1abc");
        validate_fail("1.2+e2");
        validate_fail("1.2+E2");
        validate_fail("1.2-e2");
        validate_fail("1.2-E2");
        validate_fail("1.2e2E2");
        validate_fail("1.2e2e2");
        validate_fail("1.2E2e2");
        validate_fail("1.2E2E2");
        validate_fail("0b1");
        validate_fail("01");
        validate_fail("0x1");
        validate_fail(".1");
        validate_fail("e5");
        validate_fail("E5");
    }

    SECTION("Single-line comments are ignored only when enabled")
    {
        fly::JsonParser comment_parser(fly::JsonParser::Features::AllowComments);
        {
            std::string str = R"(
        // here is a comment1
        // here is a comment2
        {
            "a" : 12,
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"(
        {
            "a" : 12,
            "b" : 13
        }
        // here is a comment1
        // here is a comment2
        )";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12 // here is a comment
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 1);
            CHECK(json["a"] == 12);
        }
        {
            std::string str = R"({
            "a" : 12, // here is a comment
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            // here is a comment
            "a" : 12,
            // here is a comment
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : "abdc // here is a comment efgh",
            "b" : 13
        })";

            {
                auto parsed = parser.parse_string(str);
                REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CHECK(json.size() == 2);
                CHECK(json["a"] == "abdc // here is a comment efgh");
                CHECK(json["b"] == 13);
            }
            {
                auto parsed = comment_parser.parse_string(str);
                REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CHECK(json.size() == 2);
                CHECK(json["a"] == "abdc // here is a comment efgh");
                CHECK(json["b"] == 13);
            }
        }
    }

    SECTION("Multi-line comments are ignored only when enabled")
    {
        fly::JsonParser comment_parser(fly::JsonParser::Features::AllowComments);
        {
            std::string str = R"(
        /* here is a comment1 */
        /* here is a comment2 */
        {
            "a" : 12,
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"(
        {
            "a" : 12,
            "b" : 13
        }
        /* here is a comment1 */
        /* here is a comment2 */
        )";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12, /* here is a comment */
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12/* here is a comment */,
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            /* here is a comment */
            "a" : 12,
            /* here is a comment */
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            /*
                here is a comment
                that crosses multiple lines
                and has JSON embedded in it
                "c" : 14,
                "d" : 15
            */
            "a" : 12,
            /* here is a comment */
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : "abdc /* here is a comment */ efgh",
            "b" : 13
        })";

            {
                auto parsed = parser.parse_string(str);
                REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CHECK(json.size() == 2);
                CHECK(json["a"] == "abdc /* here is a comment */ efgh");
                CHECK(json["b"] == 13);
            }
            {
                auto parsed = comment_parser.parse_string(str);
                REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CHECK(json.size() == 2);
                CHECK(json["a"] == "abdc /* here is a comment */ efgh");
                CHECK(json["b"] == 13);
            }
        }
    }

    SECTION("Badly formed comments cannot be parsed")
    {
        fly::JsonParser comment_parser(fly::JsonParser::Features::AllowComments);
        {
            std::string str = R"(/* here is a bad comment
        {
            "a" : 12
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12
        }  /* here is a bad comment
        )";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12 / here is a bad comment
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({"a" : 12 /)";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12 /* here is a bad comment
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({"a" : 12 /*)";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
    }

    SECTION("Trailing commas in objects are ignored only when enabled")
    {
        fly::JsonParser comma_parser(fly::JsonParser::Features::AllowTrailingComma);
        {
            std::string str = R"({
            "a" : 12,
            "b" : 13,
        })";

            validate_fail_raw(str);

            auto parsed = comma_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12,,
            "b" : 13,
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12,
            "b" : 13,,
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
    }

    SECTION("Trailing commas in arrays are ignored only when enabled")
    {
        fly::JsonParser comma_parser(fly::JsonParser::Features::AllowTrailingComma);
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1, 2,],
        })";

            validate_fail_raw(str);

            auto parsed = comma_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.size() == 2);
            CHECK(json["a"] == 12);
            CHECK(json["b"].is_array());
            CHECK(json["b"].size() == 2);
            CHECK(json["b"][0] == 1);
            CHECK(json["b"][1] == 2);
        }
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1,, 2,],
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1, 2,,],
        })";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
    }

    SECTION("Parsing of any JSON type is valid only when enabled")
    {
        fly::JsonParser type_parser(fly::JsonParser::Features::AllowAnyType);
        {
            std::string str = "this is a string without quotes";

            CHECK_FALSE(parser.parse_string(str).has_value());
            CHECK_FALSE(type_parser.parse_string(str).has_value());
        }
        {
            std::string str = "\"this is a string\"";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_string());
            CHECK(json == str.substr(1, str.size() - 2));
        }
        {
            std::string str = "true";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_boolean());
            CHECK(json == true);
        }
        {
            std::string str = "false";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_boolean());
            CHECK(json == false);
        }
        {
            std::string str = "null";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_null());
            CHECK(json == nullptr);
        }
        {
            std::string str = "12389";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_unsigned_integer());
            CHECK(json == 12389);
        }
        {
            std::string str = "-12389";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_signed_integer());
            CHECK(json == -12389);
        }
        {
            std::string str = "123.89";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CHECK(json.is_float());
            CHECK(double(json) == Approx(123.89));
        }
    }

    SECTION("Parser features may be treated as a bitmask")
    {
        // Strict
        {
            auto features = fly::JsonParser::Features::Strict;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }

        // AllowComments
        {
            auto features = fly::JsonParser::Features::AllowComments;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowComments;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }

        // AllowTrailingComma
        {
            auto features = fly::JsonParser::Features::AllowTrailingComma;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowTrailingComma;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }

        // AllowAnyType
        {
            auto features = fly::JsonParser::Features::AllowAnyType;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowAnyType;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }

        // AllFeatures
        {
            auto features = fly::JsonParser::Features::AllFeatures;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowComments;
            features = features | fly::JsonParser::Features::AllowTrailingComma;
            features = features | fly::JsonParser::Features::AllowAnyType;
            CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }
    }
}
