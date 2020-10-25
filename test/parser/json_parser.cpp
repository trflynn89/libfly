#include "fly/parser/json_parser.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <catch2/catch.hpp>

#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

CATCH_TEST_CASE("JsonParser", "[parser]")
{
    fly::JsonParser parser;

    auto validate_fail_raw = [&](const std::string &test)
    {
        CATCH_CAPTURE(test);
        CATCH_CHECK_FALSE(parser.parse_string(test).has_value());
    };

    auto validate_fail = [&](const std::string &test)
    {
        validate_fail_raw(fly::String::format("{ \"a\" : %s }", test));
    };

    auto validate_pass_raw =
        [&](const std::string &test, const std::string &key, const fly::Json &expected)
    {
        CATCH_CAPTURE(test);

        std::optional<fly::Json> actual = parser.parse_string(test);
        CATCH_REQUIRE(actual.has_value());

        if (expected.is_float())
        {
            CATCH_CHECK(double(actual->at(key)) == Approx(double(expected)));
        }
        else
        {
            CATCH_CHECK(actual->at(key) == expected);
        }

        std::stringstream ss;
        ss << actual.value();

        std::optional<fly::Json> repeat = parser.parse_string(ss.str());
        CATCH_REQUIRE(repeat.has_value());

        CATCH_CHECK(actual.value() == repeat.value());
    };

    auto validate_pass = [&](const std::string &test, const fly::Json &expected)
    {
        validate_pass_raw(fly::String::format("{ \"a\" : %s }", test), "a", expected);
    };

    // https://www.json.org/JSON_checker/
    CATCH_SECTION("JSON_checker test suite")
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
            CATCH_CAPTURE(file);

            if (fly::String::starts_with(file.string(), "pass"))
            {
                CATCH_CHECK(parser.parse_file(it.path()).has_value());
            }
            else if (fly::String::starts_with(file.string(), "fail"))
            {
                CATCH_CHECK_FALSE(parser.parse_file(it.path()).has_value());
            }
            else
            {
                CATCH_FAIL("Unrecognized JSON file: " << file);
            }
        }
    }

    // https://code.google.com/archive/p/json-test-suite/
    CATCH_SECTION("Google's json-test-suite")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "google_json_test_suite";

        CATCH_CHECK(parser.parse_file(path / "sample.json").has_value());
    }

    // https://github.com/nst/JSONTestSuite
    CATCH_SECTION("JSON Parsing Test Suite for RFC-8259 compliance")
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
            CATCH_CAPTURE(file);

            if (fly::String::starts_with(file.string(), 'y'))
            {
                CATCH_CHECK(type_parser.parse_file(it.path()).has_value());
            }
            else if (fly::String::starts_with(file.string(), 'n'))
            {
                CATCH_CHECK_FALSE(type_parser.parse_file(it.path()).has_value());
            }
            else if (fly::String::starts_with(file.string(), 'i'))
            {
                if (std::find(i_pass.begin(), i_pass.end(), file) != i_pass.end())
                {
                    CATCH_CHECK(type_parser.parse_file(it.path()).has_value());
                }
                else
                {
                    CATCH_CHECK_FALSE(type_parser.parse_file(it.path()).has_value());
                }
            }
            else
            {
                CATCH_FAIL("Unrecognized JSON file: " << file);
            }
        }
    }

    // https://github.com/minimaxir/big-list-of-naughty-strings
    CATCH_SECTION("Big List of Naughty Strings")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "big_list_of_naughty_strings";

        auto parsed = parser.parse_file(path / "blns.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_CHECK(values.size() == 507);

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            CATCH_CHECK(values[i].is_string());
        }
    }

    CATCH_SECTION("All Unicode characters")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "all_unicode.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_CHECK(values.size() == 1112064);
    }

    CATCH_SECTION("String with UTF-8 encoding is parsed as-is")
    {
        const std::string contents("{\"encoding\": \"UTF-8\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-8");
    }

    CATCH_SECTION("File with UTF-8 byte order mark is parsed as-is")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_8.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-8");
    }

    CATCH_SECTION("String with UTF-16 encoding is converted to UTF-8")
    {
        const std::u16string contents(u"{\"encoding\": \"UTF-16\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-16");

        parsed = parser.parse_string(std::u16string(1, 0xd800));
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-16 big endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_16_big_endian.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-16 BE");

        parsed = parser.parse_file(path / "utf_16_big_endian_invalid.json");
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-16 little endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_16_little_endian.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-16 LE");

        parsed = parser.parse_file(path / "utf_16_little_endian_invalid.json");
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("String with UTF-32 encoding is converted to UTF-8")
    {
        const std::u32string contents(U"{\"encoding\": \"UTF-32\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-32");

        parsed = parser.parse_string(std::u32string(1, 0xd800));
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-32 big endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_32_big_endian.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-32 BE");

        parsed = parser.parse_file(path / "utf_32_big_endian_invalid.json");
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-32 little endian byte order mark is converted to UTF-8")
    {
        const auto here = std::filesystem::path(__FILE__);
        const auto path = here.parent_path() / "json" / "unicode";

        auto parsed = parser.parse_file(path / "utf_32_little_endian.json");
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-32 LE");

        parsed = parser.parse_file(path / "utf_32_big_endian_invalid.json");
        CATCH_CHECK_FALSE(parsed.has_value());
    }

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

    CATCH_SECTION("Empty JSON object can be parsed")
    {
        const std::string contents("{}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_CHECK(values.is_object());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Empty JSON array can be parsed")
    {
        const std::string contents("[]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Nested empty JSON object can be parsed")
    {
        const std::string contents("[{}]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 1);

        values = values[0];
        CATCH_CHECK(values.is_object());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Nested empty JSON array can be parsed")
    {
        const std::string contents("[[]]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 1);

        values = values[0];
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Empty key/value strings can be parsed")
    {
        {
            const std::string contents("{\"a\" : \"\" }");

            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json values = std::move(parsed.value());
            CATCH_CHECK(values["a"].is_string());
            CATCH_CHECK(values["a"].size() == 0);
            CATCH_CHECK(values["a"] == "");
        }
        {
            const std::string contents("{\"\" : \"a\" }");

            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json values = std::move(parsed.value());
            CATCH_CHECK(values[""].is_string());
            CATCH_CHECK(values[""].size() == 1);
            CATCH_CHECK(values[""] == "a");
        }
        {
            const std::string contents("{\"\" : \"\" }");

            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json values = std::move(parsed.value());
            CATCH_CHECK(values[""].is_string());
            CATCH_CHECK(values[""].size() == 0);
            CATCH_CHECK(values[""] == "");
        }
    }

    CATCH_SECTION("JSON types that are not objects or arrays cannot be parsed by default")
    {
        validate_fail_raw("\"\"");
        validate_fail_raw("true");
        validate_fail_raw("1");
        validate_fail_raw("-1");
        validate_fail_raw("3.14");
        validate_fail_raw("null");
    }

    CATCH_SECTION("Badly formed JSON objects")
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

    CATCH_SECTION("Badly formed JSON arrays")
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

    CATCH_SECTION("Whitespace is ignored where applicable, and compliant with JSON strings")
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

    CATCH_SECTION("Detection of valid numeric JSON types")
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

    CATCH_SECTION("Detection of invalid numeric JSON types")
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

    CATCH_SECTION("Single-line comments are ignored only when enabled")
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12 // here is a comment
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 1);
            CATCH_CHECK(json["a"] == 12);
        }
        {
            std::string str = R"({
            "a" : 12, // here is a comment
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : "abdc // here is a comment efgh",
            "b" : 13
        })";

            {
                auto parsed = parser.parse_string(str);
                CATCH_REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc // here is a comment efgh");
                CATCH_CHECK(json["b"] == 13);
            }
            {
                auto parsed = comment_parser.parse_string(str);
                CATCH_REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc // here is a comment efgh");
                CATCH_CHECK(json["b"] == 13);
            }
        }
    }

    CATCH_SECTION("Multi-line comments are ignored only when enabled")
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12, /* here is a comment */
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12/* here is a comment */,
            "b" : 13
        })";

            validate_fail_raw(str);

            auto parsed = comment_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
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
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : "abdc /* here is a comment */ efgh",
            "b" : 13
        })";

            {
                auto parsed = parser.parse_string(str);
                CATCH_REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc /* here is a comment */ efgh");
                CATCH_CHECK(json["b"] == 13);
            }
            {
                auto parsed = comment_parser.parse_string(str);
                CATCH_REQUIRE(parsed.has_value());

                fly::Json json = std::move(parsed.value());
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc /* here is a comment */ efgh");
                CATCH_CHECK(json["b"] == 13);
            }
        }
    }

    CATCH_SECTION("Badly formed comments cannot be parsed")
    {
        fly::JsonParser comment_parser(fly::JsonParser::Features::AllowComments);
        {
            std::string str = R"(/* here is a bad comment
        {
            "a" : 12
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12
        }  /* here is a bad comment
        )";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12 / here is a bad comment
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({"a" : 12 /)";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12 /* here is a bad comment
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({"a" : 12 /*)";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comment_parser.parse_string(str).has_value());
        }
    }

    CATCH_SECTION("Trailing commas in objects are ignored only when enabled")
    {
        fly::JsonParser comma_parser(fly::JsonParser::Features::AllowTrailingComma);
        {
            std::string str = R"({
            "a" : 12,
            "b" : 13,
        })";

            validate_fail_raw(str);

            auto parsed = comma_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"] == 13);
        }
        {
            std::string str = R"({
            "a" : 12,,
            "b" : 13,
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12,
            "b" : 13,,
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
    }

    CATCH_SECTION("Trailing commas in arrays are ignored only when enabled")
    {
        fly::JsonParser comma_parser(fly::JsonParser::Features::AllowTrailingComma);
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1, 2,],
        })";

            validate_fail_raw(str);

            auto parsed = comma_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.size() == 2);
            CATCH_CHECK(json["a"] == 12);
            CATCH_CHECK(json["b"].is_array());
            CATCH_CHECK(json["b"].size() == 2);
            CATCH_CHECK(json["b"][0] == 1);
            CATCH_CHECK(json["b"][1] == 2);
        }
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1,, 2,],
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1, 2,,],
        })";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(comma_parser.parse_string(str).has_value());
        }
    }

    CATCH_SECTION("Parsing of any JSON type is valid only when enabled")
    {
        fly::JsonParser type_parser(fly::JsonParser::Features::AllowAnyType);
        {
            std::string str = "this is a string without quotes";

            CATCH_CHECK_FALSE(parser.parse_string(str).has_value());
            CATCH_CHECK_FALSE(type_parser.parse_string(str).has_value());
        }
        {
            std::string str = "\"this is a string\"";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_string());
            CATCH_CHECK(json == str.substr(1, str.size() - 2));
        }
        {
            std::string str = "true";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_boolean());
            CATCH_CHECK(json == true);
        }
        {
            std::string str = "false";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_boolean());
            CATCH_CHECK(json == false);
        }
        {
            std::string str = "null";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_null());
            CATCH_CHECK(json == nullptr);
        }
        {
            std::string str = "12389";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_unsigned_integer());
            CATCH_CHECK(json == 12389);
        }
        {
            std::string str = "-12389";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_signed_integer());
            CATCH_CHECK(json == -12389);
        }
        {
            std::string str = "123.89";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            CATCH_CHECK(json.is_float());
            CATCH_CHECK(double(json) == Approx(123.89));
        }
    }

    CATCH_SECTION("Parser features may be treated as a bitmask")
    {
        // Strict
        {
            auto features = fly::JsonParser::Features::Strict;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }

        // AllowComments
        {
            auto features = fly::JsonParser::Features::AllowComments;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowComments;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }

        // AllowTrailingComma
        {
            auto features = fly::JsonParser::Features::AllowTrailingComma;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowTrailingComma;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::Strict);
        }

        // AllowAnyType
        {
            auto features = fly::JsonParser::Features::AllowAnyType;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowAnyType;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::Strict);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }

        // AllFeatures
        {
            auto features = fly::JsonParser::Features::AllFeatures;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }
        {
            auto features = fly::JsonParser::Features::Strict;
            features = features | fly::JsonParser::Features::AllowComments;
            features = features | fly::JsonParser::Features::AllowTrailingComma;
            features = features | fly::JsonParser::Features::AllowAnyType;
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowComments) ==
                fly::JsonParser::Features::AllowComments);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowTrailingComma) ==
                fly::JsonParser::Features::AllowTrailingComma);
            CATCH_CHECK(
                (features & fly::JsonParser::Features::AllowAnyType) ==
                fly::JsonParser::Features::AllowAnyType);
        }
    }
}
