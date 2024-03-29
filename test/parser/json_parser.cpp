#include "fly/parser/json_parser.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/string/format.hpp"

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include <array>
#include <filesystem>
#include <memory>
#include <string>

using namespace std::literals::string_view_literals;

namespace {

std::filesystem::path const &root_data_path()
{
    static auto const path =
        std::filesystem::path(__FILE__).parent_path().parent_path().parent_path() / "build" /
        "data" / "json";
    return path;
}

std::filesystem::path const &unicode_data_path()
{
    static auto const path = std::filesystem::path(__FILE__).parent_path() / "unicode";
    return path;
}

} // namespace

CATCH_TEST_CASE("JsonParser", "[parser]")
{
    fly::parser::JsonParser parser;

    auto validate_fail_raw = [&](std::string const &test) {
        CATCH_CAPTURE(test);
        CATCH_CHECK_FALSE(parser.parse_string(test).has_value());
    };

    auto validate_fail = [&](std::string const &test) {
        validate_fail_raw(fly::string::format("{{ \"a\" : {} }}", test));
    };

    auto validate_pass_raw =
        [&](std::string const &test, std::string const &key, fly::Json const &expected) {
            CATCH_CAPTURE(test);

            std::optional<fly::Json> actual = parser.parse_string(test);
            CATCH_REQUIRE(actual.has_value());

            if (expected.is_float())
            {
                CATCH_CHECK(double(actual->at(key)) == Catch::Approx(double(expected)));
            }
            else
            {
                CATCH_CHECK(actual->at(key) == expected);
            }

            std::optional<fly::Json> repeat = parser.parse_string(actual->serialize());
            CATCH_REQUIRE(repeat.has_value());

            CATCH_CHECK(*actual == *repeat);
        };

    auto validate_pass = [&](std::string const &test, fly::Json const &expected) {
        validate_pass_raw(fly::string::format("{{ \"a\" : {} }}", test), "a", expected);
    };

    CATCH_SECTION("JSON checker test suite")
    {
        static auto const path = root_data_path() / "json_checker" / "test";

        static constexpr std::array exclusions {
            "fail18.json"sv, // The parser has no max-depth.
        };

        for (auto const &it : std::filesystem::directory_iterator(path))
        {
            auto const file = it.path().filename();
            CATCH_CAPTURE(file);

            if (std::find(exclusions.begin(), exclusions.end(), file) != exclusions.end())
            {
                continue;
            }

            if (file.string().starts_with("pass"))
            {
                CATCH_CHECK(parser.parse_file(it.path()).has_value());
            }
            else if (file.string().starts_with("fail"))
            {
                CATCH_CHECK_FALSE(parser.parse_file(it.path()).has_value());
            }
        }
    }

    CATCH_SECTION("Google's json-test-suite")
    {
        static auto const path = root_data_path() / "google_json_test_suite" / "sample.json";
        CATCH_CHECK(parser.parse_file(path).has_value());
    }

    CATCH_SECTION("JSON Parsing Test Suite for RFC-8259 compliance")
    {
        static auto const path = root_data_path() / "nst_json_test_suite";

        static constexpr std::array exclusions {
            "n_structure_100000_opening_arrays.json"sv, // Causes stack overflow.
            "n_structure_open_array_object.json"sv, // Causes stack overflow.
            "i_number_double_huge_neg_exp.json"sv, // Platform dependent (fails Windows).
        };

        static constexpr std::array i_pass {
            "i_structure_500_nested_arrays.json"sv, // No enforced depth limit.
            "i_structure_UTF-8_BOM_empty_object.json"sv, // Byte order mark is handled.
            "i_string_UTF-16LE_with_BOM.json"sv, // Byte order mark is handled.
        };

        // JSONTestSuite contains test files that aren't only objects or arrays.
        fly::parser::JsonParser type_parser(fly::parser::JsonParser::Features::AllowAnyType);

        for (auto const &it : std::filesystem::directory_iterator(path))
        {
            auto const file = it.path().filename();
            CATCH_CAPTURE(file);

            if (std::find(exclusions.begin(), exclusions.end(), file) != exclusions.end())
            {
                continue;
            }

            if (file.string().starts_with('y'))
            {
                CATCH_CHECK(type_parser.parse_file(it.path()).has_value());
            }
            else if (file.string().starts_with('n'))
            {
                CATCH_CHECK_FALSE(type_parser.parse_file(it.path()).has_value());
            }
            else if (file.string().starts_with('i'))
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
        }
    }

    CATCH_SECTION("Big List of Naughty Strings")
    {
        static auto const path = root_data_path() / "blns.json";

        auto parsed = parser.parse_file(path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 515);

        for (std::size_t i = 0; i < values.size(); ++i)
        {
            CATCH_CHECK(values[i].is_string());
        }
    }

    CATCH_SECTION("All Unicode characters")
    {
        static auto const path = root_data_path() / "all_unicode.json";

        auto parsed = parser.parse_file(path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.size() == 1'112'065); // 1,112,064 code points + 1 terminating null value
    }

    CATCH_SECTION("String with UTF-8 encoding (std::string)")
    {
        std::string const contents("{\"encoding\": \"UTF-8\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-8");
    }

    CATCH_SECTION("String with UTF-8 encoding (std::u8string)")
    {
        std::u8string const contents(u8"{\"encoding\": \"UTF-8\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-8");
    }

    CATCH_SECTION("File with UTF-8 byte order mark")
    {
        static auto const path = unicode_data_path() / "utf_8.json";

        auto parsed = parser.parse_file(path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-8");
    }

    CATCH_SECTION("String with UTF-16 encoding")
    {
        std::u16string const contents(u"{\"encoding\": \"UTF-16\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-16");

        parsed = parser.parse_string(std::u16string(1, 0xd800));
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-16 big endian byte order mark")
    {
        static auto const success_path = unicode_data_path() / "utf_16_big_endian.json";
        static auto const invalid_path = unicode_data_path() / "utf_16_big_endian_invalid.json";

        auto parsed = parser.parse_file(success_path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-16 BE");

        parsed = parser.parse_file(invalid_path);
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-16 little endian byte order mark")
    {
        static auto const success_path = unicode_data_path() / "utf_16_little_endian.json";
        static auto const invalid_path = unicode_data_path() / "utf_16_little_endian_invalid.json";

        auto parsed = parser.parse_file(success_path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-16 LE");

        parsed = parser.parse_file(invalid_path);
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("String with UTF-32 encoding")
    {
        std::u32string const contents(U"{\"encoding\": \"UTF-32\"}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-32");

        parsed = parser.parse_string(std::u32string(1, 0xd800));
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-32 big endian byte order mark")
    {
        static auto const success_path = unicode_data_path() / "utf_32_big_endian.json";
        static auto const invalid_path = unicode_data_path() / "utf_32_big_endian_invalid.json";

        auto parsed = parser.parse_file(success_path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-32 BE");

        parsed = parser.parse_file(invalid_path);
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("File with UTF-32 little endian byte order mark")
    {
        static auto const success_path = unicode_data_path() / "utf_32_little_endian.json";
        static auto const invalid_path = unicode_data_path() / "utf_32_big_endian_invalid.json";

        auto parsed = parser.parse_file(success_path);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_REQUIRE(values.size() == 1);

        fly::Json encoded_encoding = values["encoding"];
        CATCH_CHECK(encoded_encoding == "UTF-32 LE");

        parsed = parser.parse_file(invalid_path);
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
        std::string const contents;

        auto parsed = parser.parse_string(contents);
        CATCH_CHECK_FALSE(parsed.has_value());
    }

    CATCH_SECTION("Empty JSON object can be parsed")
    {
        std::string const contents("{}");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.is_object());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Empty JSON array can be parsed")
    {
        std::string const contents("[]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Nested empty JSON object can be parsed")
    {
        std::string const contents("[{}]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 1);

        values = values[0];
        CATCH_CHECK(values.is_object());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Nested empty JSON array can be parsed")
    {
        std::string const contents("[[]]");

        auto parsed = parser.parse_string(contents);
        CATCH_REQUIRE(parsed.has_value());

        fly::Json values = *std::move(parsed);
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 1);

        values = values[0];
        CATCH_CHECK(values.is_array());
        CATCH_CHECK(values.size() == 0);
    }

    CATCH_SECTION("Empty key/value strings can be parsed")
    {
        {
            std::string const contents("{\"a\" : \"\" }");

            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json values = *std::move(parsed);
            CATCH_CHECK(values["a"].is_string());
            CATCH_CHECK(values["a"].size() == 0);
            CATCH_CHECK(values["a"] == "");
        }
        {
            std::string const contents("{\"\" : \"a\" }");

            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json values = *std::move(parsed);
            CATCH_CHECK(values[""].is_string());
            CATCH_CHECK(values[""].size() == 1);
            CATCH_CHECK(values[""] == "a");
        }
        {
            std::string const contents("{\"\" : \"\" }");

            auto parsed = parser.parse_string(contents);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json values = *std::move(parsed);
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
        fly::parser::JsonParser comment_parser(fly::parser::JsonParser::Features::AllowComments);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

                fly::Json json = *std::move(parsed);
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc // here is a comment efgh");
                CATCH_CHECK(json["b"] == 13);
            }
            {
                auto parsed = comment_parser.parse_string(str);
                CATCH_REQUIRE(parsed.has_value());

                fly::Json json = *std::move(parsed);
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc // here is a comment efgh");
                CATCH_CHECK(json["b"] == 13);
            }
        }
    }

    CATCH_SECTION("Multi-line comments are ignored only when enabled")
    {
        fly::parser::JsonParser comment_parser(fly::parser::JsonParser::Features::AllowComments);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

            fly::Json json = *std::move(parsed);
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

                fly::Json json = *std::move(parsed);
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc /* here is a comment */ efgh");
                CATCH_CHECK(json["b"] == 13);
            }
            {
                auto parsed = comment_parser.parse_string(str);
                CATCH_REQUIRE(parsed.has_value());

                fly::Json json = *std::move(parsed);
                CATCH_CHECK(json.size() == 2);
                CATCH_CHECK(json["a"] == "abdc /* here is a comment */ efgh");
                CATCH_CHECK(json["b"] == 13);
            }
        }
    }

    CATCH_SECTION("Badly formed comments cannot be parsed")
    {
        fly::parser::JsonParser comment_parser(fly::parser::JsonParser::Features::AllowComments);
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
        fly::parser::JsonParser comma_parser(fly::parser::JsonParser::Features::AllowTrailingComma);
        {
            std::string str = R"({
            "a" : 12,
            "b" : 13,
        })";

            validate_fail_raw(str);

            auto parsed = comma_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
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
        fly::parser::JsonParser comma_parser(fly::parser::JsonParser::Features::AllowTrailingComma);
        {
            std::string str = R"({
            "a" : 12,
            "b" : [1, 2,],
        })";

            validate_fail_raw(str);

            auto parsed = comma_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
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
        fly::parser::JsonParser type_parser(fly::parser::JsonParser::Features::AllowAnyType);
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

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_string());
            CATCH_CHECK(json == str.substr(1, str.size() - 2));
        }
        {
            std::string str = "true";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_boolean());
            CATCH_CHECK(json == true);
        }
        {
            std::string str = "false";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_boolean());
            CATCH_CHECK(json == false);
        }
        {
            std::string str = "null";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_null());
            CATCH_CHECK(json == nullptr);
        }
        {
            std::string str = "12389";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_unsigned_integer());
            CATCH_CHECK(json == 12389);
        }
        {
            std::string str = "-12389";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_signed_integer());
            CATCH_CHECK(json == -12389);
        }
        {
            std::string str = "123.89";
            validate_fail_raw(str);

            auto parsed = type_parser.parse_string(str);
            CATCH_REQUIRE(parsed.has_value());

            fly::Json json = *std::move(parsed);
            CATCH_CHECK(json.is_float());
            CATCH_CHECK(double(json) == Catch::Approx(123.89));
        }
    }

    CATCH_SECTION("Parser features may be treated as a bitmask")
    {
        using Features = fly::parser::JsonParser::Features;

        // Strict
        {
            auto features = Features::Strict;
            CATCH_CHECK((features & Features::AllowComments) == Features::Strict);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::Strict);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::Strict);
        }

        // AllowComments
        {
            auto features = Features::AllowComments;
            CATCH_CHECK((features & Features::AllowComments) == Features::AllowComments);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::Strict);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::Strict);
        }
        {
            auto features = Features::Strict;
            features = features | Features::AllowComments;
            CATCH_CHECK((features & Features::AllowComments) == Features::AllowComments);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::Strict);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::Strict);
        }

        // AllowTrailingComma
        {
            auto features = Features::AllowTrailingComma;
            CATCH_CHECK((features & Features::AllowComments) == Features::Strict);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::AllowTrailingComma);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::Strict);
        }
        {
            auto features = Features::Strict;
            features = features | Features::AllowTrailingComma;
            CATCH_CHECK((features & Features::AllowComments) == Features::Strict);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::AllowTrailingComma);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::Strict);
        }

        // AllowAnyType
        {
            auto features = Features::AllowAnyType;
            CATCH_CHECK((features & Features::AllowComments) == Features::Strict);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::Strict);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::AllowAnyType);
        }
        {
            auto features = Features::Strict;
            features = features | Features::AllowAnyType;
            CATCH_CHECK((features & Features::AllowComments) == Features::Strict);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::Strict);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::AllowAnyType);
        }

        // AllFeatures
        {
            auto features = Features::AllFeatures;
            CATCH_CHECK((features & Features::AllowComments) == Features::AllowComments);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::AllowTrailingComma);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::AllowAnyType);
        }
        {
            auto features = Features::Strict;
            features = features | Features::AllowComments;
            features = features | Features::AllowTrailingComma;
            features = features | Features::AllowAnyType;
            CATCH_CHECK((features & Features::AllowComments) == Features::AllowComments);
            CATCH_CHECK((features & Features::AllowTrailingComma) == Features::AllowTrailingComma);
            CATCH_CHECK((features & Features::AllowAnyType) == Features::AllowAnyType);
        }
    }
}
