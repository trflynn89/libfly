#include "fly/parser/json_parser.hpp"

#include "fly/parser/exceptions.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

//==============================================================================
class JsonParserTest : public ::testing::Test
{
protected:
    void validate_fail(const std::string &test) noexcept
    {
        validate_fail_raw(fly::String::format("{ \"a\" : %s }", test));
    }

    void validate_fail_raw(const std::string &test) noexcept
    {
        SCOPED_TRACE(test);
        EXPECT_THROW(m_parser.parse_string(test), fly::ParserException);
    }

    void
    validate_pass(const std::string &test, const fly::Json &expected) noexcept
    {
        validate_pass_raw(
            fly::String::format("{ \"a\" : %s }", test),
            "a",
            expected);
    }

    void validate_pass_raw(
        const std::string &test,
        const std::string &key,
        const fly::Json &expected) noexcept
    {
        fly::Json actual, repeat;

        SCOPED_TRACE(test);
        EXPECT_NO_THROW(actual = m_parser.parse_string(test));

        if (expected.is_float())
        {
            EXPECT_DOUBLE_EQ(double(actual[key]), double(expected));
        }
        else
        {
            EXPECT_EQ(actual[key], expected);
        }

        std::stringstream ss;
        ss << actual;

        EXPECT_NO_THROW(repeat = m_parser.parse_string(ss.str()));
        EXPECT_EQ(actual, repeat);
    }

    fly::JsonParser m_parser;
};

//==============================================================================
TEST_F(JsonParserTest, JsonChecker)
{
    // https://www.json.org/JSON_checker/
    // The following files are excluded from this test:
    // - fail18.json: The parser has no max-depth

    // Get the path to the JSON checker directory
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "json_checker";

    // Validate each JSON file in the JSON checker directory
    for (const auto &it : std::filesystem::directory_iterator(path))
    {
        const auto file = it.path().filename();
        SCOPED_TRACE(file);

        if (fly::String::starts_with(file.string(), "pass"))
        {
            EXPECT_NO_THROW(m_parser.parse_file(it.path()));
        }
        else if (fly::String::starts_with(file.string(), "fail"))
        {
            EXPECT_THROW(m_parser.parse_file(it.path()), fly::ParserException);
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==============================================================================
TEST_F(JsonParserTest, GoogleJsonTestSuite)
{
    // https://code.google.com/archive/p/json-test-suite/
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "google_json_test_suite";

    EXPECT_NO_THROW(m_parser.parse_file(path / "sample.json"));
}

//==============================================================================
TEST_F(JsonParserTest, NstJsonTestSuiteParsing)
{
    // https://github.com/nst/JSONTestSuite
    // The following files are excluded from this test:
    // - n_single_space.json: Empty files are allowed
    // - n_structure_100000_opening_arrays.json: Causes stack overflow
    // - n_structure_no_data.json: Empty files are allowed
    // - n_structure_open_array_object.json: Too nested, causes stack overflow
    // - n_structure_UTF8_BOM_no_data.json: Empty files are allowed
    // - y_string_space.json: Only allow objects and arrays
    // - y_structure_lonely_false.json: Only allow objects and arrays
    // - y_structure_lonely_int.json: Only allow objects and arrays
    // - y_structure_lonely_negative_real.json: Only allow objects and arrays
    // - y_structure_lonely_null.json: Only allow objects and arrays
    // - y_structure_lonely_string.json: Only allow objects and arrays
    // - y_structure_lonely_true.json: Only allow objects and arrays
    // - y_structure_string_empty.json: Only allow objects and arrays
    // - i_number_double_huge_neg_exp.json: Platform dependent (fails Windows)

    // Indeterminate files expected to pass
    std::vector<std::string> i_pass = {
        "i_structure_500_nested_arrays.json", // No enforced depth limit
        "i_structure_UTF-8_BOM_empty_object.json", // Byte order mark ignored
    };

    // Get the path to the JSONTestSuite directory
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "nst_json_test_suite";

    // Validate each JSON file in the JSONTestSuite directory
    for (const auto &it : std::filesystem::directory_iterator(path))
    {
        const auto file = it.path().filename();
        SCOPED_TRACE(file);

        if (fly::String::starts_with(file.string(), 'y'))
        {
            EXPECT_NO_THROW(m_parser.parse_file(it.path()));
        }
        else if (fly::String::starts_with(file.string(), 'n'))
        {
            EXPECT_THROW(m_parser.parse_file(it.path()), fly::ParserException);
        }
        else if (fly::String::starts_with(file.string(), 'i'))
        {
            if (std::find(i_pass.begin(), i_pass.end(), file) != i_pass.end())
            {
                EXPECT_NO_THROW(m_parser.parse_file(it.path()));
            }
            else
            {
                EXPECT_THROW(
                    m_parser.parse_file(it.path()),
                    fly::ParserException);
            }
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==============================================================================
TEST_F(JsonParserTest, BigListOfNaughtyStrings)
{
    // https://github.com/minimaxir/big-list-of-naughty-strings
    const auto here = std::filesystem::path(__FILE__);
    const auto path =
        here.parent_path() / "json" / "big_list_of_naughty_strings";
    fly::Json values;

    EXPECT_NO_THROW(values = m_parser.parse_file(path / "blns.json"));
    EXPECT_EQ(values.size(), 507);

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        EXPECT_TRUE(values[i].is_string());
    }
}

//==============================================================================
TEST_F(JsonParserTest, AllUnicode)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";
    fly::Json values;

    EXPECT_NO_THROW(values = m_parser.parse_file(path / "all_unicode.json"));
    EXPECT_EQ(values.size(), 1112064);
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingPath)
{
    EXPECT_THROW(
        m_parser.parse_file(std::filesystem::path("foo_abc") / "a.json"),
        fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingFile)
{
    EXPECT_THROW(
        m_parser.parse_file(std::filesystem::temp_directory_path() / "a.json"),
        fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyFile)
{
    validate_fail_raw("");
}

//==============================================================================
TEST_F(JsonParserTest, EmptyObject)
{
    const std::string contents("{}");
    fly::Json values;

    ASSERT_NO_THROW(values = m_parser.parse_string(contents));
    EXPECT_TRUE(values.is_object());
    EXPECT_EQ(values.size(), 0);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyArray)
{
    const std::string contents("[]");
    fly::Json values;

    ASSERT_NO_THROW(values = m_parser.parse_string(contents));
    EXPECT_TRUE(values.is_array());
    EXPECT_EQ(values.size(), 0);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyNestedObjectArray)
{
    fly::Json values;
    {
        const std::string contents("[{}]");
        ASSERT_NO_THROW(values = m_parser.parse_string(contents));

        EXPECT_TRUE(values.is_array());
        EXPECT_EQ(values.size(), 1);

        values = values[0];
        EXPECT_TRUE(values.is_object());
        EXPECT_EQ(values.size(), 0);
    }
    {
        const std::string contents("[[]]");
        ASSERT_NO_THROW(values = m_parser.parse_string(contents));

        EXPECT_TRUE(values.is_array());
        EXPECT_EQ(values.size(), 1);

        values = values[0];
        EXPECT_TRUE(values.is_array());
        EXPECT_EQ(values.size(), 0);
    }
}

//==============================================================================
TEST_F(JsonParserTest, EmptyString)
{
    fly::Json values;
    {
        const std::string contents("{\"a\" : \"\" }");
        ASSERT_NO_THROW(values = m_parser.parse_string(contents));

        EXPECT_TRUE(values["a"].is_string());
        EXPECT_EQ(values["a"].size(), 0);
        EXPECT_EQ(values["a"], "");
    }
    {
        const std::string contents("{\"\" : \"a\" }");
        ASSERT_NO_THROW(values = m_parser.parse_string(contents));

        EXPECT_TRUE(values[""].is_string());
        EXPECT_EQ(values[""].size(), 1);
        EXPECT_EQ(values[""], "a");
    }
    {
        const std::string contents("{\"\" : \"\" }");
        ASSERT_NO_THROW(values = m_parser.parse_string(contents));

        EXPECT_TRUE(values[""].is_string());
        EXPECT_EQ(values[""].size(), 0);
        EXPECT_EQ(values[""], "");
    }
}

//==============================================================================
TEST_F(JsonParserTest, NonObjectOrArray)
{
    validate_fail_raw("\"\"");
    validate_fail_raw("true");
    validate_fail_raw("1");
    validate_fail_raw("-1");
    validate_fail_raw("3.14");
    validate_fail_raw("null");
}

//==============================================================================
TEST_F(JsonParserTest, BadlyFormedObject)
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

//==============================================================================
TEST_F(JsonParserTest, BadlyFormedArray)
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

//==============================================================================
TEST_F(JsonParserTest, WhiteSpace)
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

//==============================================================================
TEST_F(JsonParserTest, NumericConversion)
{
    validate_pass("1", 1);
    validate_pass("-1", -1);
    validate_pass("1.2", 1.2);
    validate_pass("-1.2", -1.2);

    validate_fail("+1");
    validate_fail("01");
    validate_fail("+1.2");
    validate_fail("1.2.1");

    validate_pass("1.2e1", 12.0);
    validate_pass("1.2E1", 12.0);
    validate_pass("1.2e+1", 12.0);
    validate_pass("1.2E+1", 12.0);
    validate_pass("1.2e-1", 0.12);
    validate_pass("1.2E-1", 0.12);

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

//==============================================================================
TEST_F(JsonParserTest, SingleLineComment)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowComments);
    {
        std::string str = R"(
        // here is a comment1
        // here is a comment2
        {
            "a" : 12,
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
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

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12 // here is a comment
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 1);
        EXPECT_EQ(json["a"], 12);
    }
    {
        std::string str = R"({
            "a" : 12, // here is a comment
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            // here is a comment
            "a" : 12,
            // here is a comment
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : "abdc // here is a comment efgh",
            "b" : 13
        })";

        fly::Json json;

        EXPECT_NO_THROW(m_parser.parse_string(str));
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], "abdc // here is a comment efgh");
        EXPECT_EQ(json["b"], 13);
    }
}

//==============================================================================
TEST_F(JsonParserTest, MultiLineComment)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowComments);
    {
        std::string str = R"(
        /* here is a comment1 */
        /* here is a comment2 */
        {
            "a" : 12,
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
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

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12, /* here is a comment */
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12/* here is a comment */,
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            /* here is a comment */
            "a" : 12,
            /* here is a comment */
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
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

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : "abdc /* here is a comment */ efgh",
            "b" : 13
        })";

        fly::Json json;

        EXPECT_NO_THROW(m_parser.parse_string(str));
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], "abdc /* here is a comment */ efgh");
        EXPECT_EQ(json["b"], 13);
    }
}

//==============================================================================
TEST_F(JsonParserTest, InvalidComment)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowComments);
    {
        std::string str = R"({
            "a" : 12 / here is a bad comment
        })";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
    {
        std::string str = R"({"a" : 12 /)";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
    {
        std::string str = R"({
            "a" : 12 /* here is a bad comment
        })";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
    {
        std::string str = R"({"a" : 12 /*)";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
}

//==============================================================================
TEST_F(JsonParserTest, TrailingCommaObject)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowTrailingComma);
    {
        std::string str = R"({
            "a" : 12,
            "b" : 13,
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12,,
            "b" : 13,
        })";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : 13,,
        })";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
}

//==============================================================================
TEST_F(JsonParserTest, TrailingCommaArray)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowTrailingComma);
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1, 2,],
        })";

        fly::Json json;

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_NO_THROW(json = parser.parse_string(str));
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_TRUE(json["b"].is_array());
        EXPECT_EQ(json["b"].size(), 2);
        EXPECT_EQ(json["b"][0], 1);
        EXPECT_EQ(json["b"][1], 2);
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1,, 2,],
        })";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1, 2,,],
        })";

        EXPECT_THROW(m_parser.parse_string(str), fly::ParserException);
        EXPECT_THROW(parser.parse_string(str), fly::ParserException);
    }
}

//==============================================================================
TEST(JsonParserFeatures, CombinedFeatures)
{
    {
        auto features = fly::JsonParser::Features::Strict;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
    }

    {
        auto features = fly::JsonParser::Features::AllowComments;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowComments;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
    }

    {
        auto features = fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
    }

    {
        auto features = fly::JsonParser::Features::AllFeatures;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowComments;
        features = features | fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
    }
}
