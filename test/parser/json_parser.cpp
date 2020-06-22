#include "fly/parser/json_parser.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <sstream>
#include <vector>

//==================================================================================================
class JsonParserTest : public ::testing::Test
{
protected:
    void validate_fail(const std::string &test)
    {
        validate_fail_raw(fly::String::format("{ \"a\" : %s }", test));
    }

    void validate_fail_raw(const std::string &test)
    {
        SCOPED_TRACE(test);
        EXPECT_FALSE(m_parser.parse_string(test).has_value());
    }

    void validate_pass(const std::string &test, const fly::Json &expected)
    {
        validate_pass_raw(fly::String::format("{ \"a\" : %s }", test), "a", expected);
    }

    void
    validate_pass_raw(const std::string &test, const std::string &key, const fly::Json &expected)
    {
        SCOPED_TRACE(test);

        std::optional<fly::Json> actual = m_parser.parse_string(test);
        ASSERT_TRUE(actual.has_value());

        if (expected.is_float())
        {
            EXPECT_DOUBLE_EQ(double(actual->at(key)), double(expected));
        }
        else
        {
            EXPECT_EQ(actual->at(key), expected);
        }

        std::stringstream ss;
        ss << actual.value();

        std::optional<fly::Json> repeat = m_parser.parse_string(ss.str());
        ASSERT_TRUE(repeat.has_value());

        EXPECT_EQ(actual.value(), repeat.value());
    }

    fly::JsonParser m_parser;
};

//==================================================================================================
TEST_F(JsonParserTest, JsonChecker)
{
    // https://www.json.org/JSON_checker/
    // The following files are excluded from this test:
    // - fail18.json: The parser has no max-depth

    // Get the path to the JSON checker directory.
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "json_checker";

    // Validate each JSON file in the JSON checker directory.
    for (const auto &it : std::filesystem::directory_iterator(path))
    {
        const auto file = it.path().filename();
        SCOPED_TRACE(file);

        if (fly::String::starts_with(file.string(), "pass"))
        {
            EXPECT_TRUE(m_parser.parse_file(it.path()).has_value());
        }
        else if (fly::String::starts_with(file.string(), "fail"))
        {
            EXPECT_FALSE(m_parser.parse_file(it.path()).has_value());
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==================================================================================================
TEST_F(JsonParserTest, GoogleJsonTestSuite)
{
    // https://code.google.com/archive/p/json-test-suite/
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "google_json_test_suite";

    EXPECT_TRUE(m_parser.parse_file(path / "sample.json").has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, NstJsonTestSuiteParsing)
{
    // https://github.com/nst/JSONTestSuite
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
    fly::JsonParser parser(fly::JsonParser::Features::AllowAnyType);

    // Get the path to the JSONTestSuite directory.
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "nst_json_test_suite";

    // Validate each JSON file in the JSONTestSuite directory.
    for (const auto &it : std::filesystem::directory_iterator(path))
    {
        const auto file = it.path().filename();
        SCOPED_TRACE(file);

        if (fly::String::starts_with(file.string(), 'y'))
        {
            EXPECT_TRUE(parser.parse_file(it.path()).has_value());
        }
        else if (fly::String::starts_with(file.string(), 'n'))
        {
            EXPECT_FALSE(parser.parse_file(it.path()).has_value());
        }
        else if (fly::String::starts_with(file.string(), 'i'))
        {
            if (std::find(i_pass.begin(), i_pass.end(), file) != i_pass.end())
            {
                EXPECT_TRUE(parser.parse_file(it.path()).has_value());
            }
            else
            {
                EXPECT_FALSE(parser.parse_file(it.path()).has_value());
            }
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==================================================================================================
TEST_F(JsonParserTest, BigListOfNaughtyStrings)
{
    // https://github.com/minimaxir/big-list-of-naughty-strings
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "big_list_of_naughty_strings";

    auto parsed = m_parser.parse_file(path / "blns.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 507);

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        EXPECT_TRUE(values[i].is_string());
    }
}

//==================================================================================================
TEST_F(JsonParserTest, AllUnicode)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";

    auto parsed = m_parser.parse_file(path / "all_unicode.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 1112064);
}

//==================================================================================================
TEST_F(JsonParserTest, UTF8String)
{
    const std::string contents(u8"{\"encoding\": \"UTF-8\"}");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-8");
}

//==================================================================================================
TEST_F(JsonParserTest, UTF8File)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";

    auto parsed = m_parser.parse_file(path / "utf_8.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-8");
}

//==================================================================================================
TEST_F(JsonParserTest, UTF16String)
{
    const std::u16string contents(u"{\"encoding\": \"UTF-16\"}");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-16");

    parsed = m_parser.parse_string(std::u16string(1, 0xd800));
    ASSERT_FALSE(parsed.has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, UTF16BigEndianFile)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";

    auto parsed = m_parser.parse_file(path / "utf_16_big_endian.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-16 BE");

    parsed = m_parser.parse_file(path / "utf_16_big_endian_invalid.json");
    ASSERT_FALSE(parsed.has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, UTF16LittleEndianFile)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";

    auto parsed = m_parser.parse_file(path / "utf_16_little_endian.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-16 LE");

    parsed = m_parser.parse_file(path / "utf_16_little_endian_invalid.json");
    ASSERT_FALSE(parsed.has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, UTF32String)
{
    const std::u32string contents(U"{\"encoding\": \"UTF-32\"}");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-32");

    parsed = m_parser.parse_string(std::u32string(1, 0xd800));
    ASSERT_FALSE(parsed.has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, UTF32BigEndianFile)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";

    auto parsed = m_parser.parse_file(path / "utf_32_big_endian.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-32 BE");

    parsed = m_parser.parse_file(path / "utf_32_big_endian_invalid.json");
    ASSERT_FALSE(parsed.has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, UTF32LittleEndianFile)
{
    const auto here = std::filesystem::path(__FILE__);
    const auto path = here.parent_path() / "json" / "unicode";

    auto parsed = m_parser.parse_file(path / "utf_32_little_endian.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    ASSERT_EQ(values.size(), 1);

    fly::Json encoded_encoding = values["encoding"];
    EXPECT_EQ(encoded_encoding, "UTF-32 LE");

    parsed = m_parser.parse_file(path / "utf_32_big_endian_invalid.json");
    ASSERT_FALSE(parsed.has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, NonExistingPath)
{
    EXPECT_FALSE(m_parser.parse_file(std::filesystem::path("foo_abc") / "a.json").has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, NonExistingFile)
{
    EXPECT_FALSE(
        m_parser.parse_file(std::filesystem::temp_directory_path() / "a.json").has_value());
}

//==================================================================================================
TEST_F(JsonParserTest, EmptyFile)
{
    validate_fail_raw("");
}

//==================================================================================================
TEST_F(JsonParserTest, EmptyObject)
{
    const std::string contents("{}");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_TRUE(values.is_object());
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(JsonParserTest, EmptyArray)
{
    const std::string contents("[]");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_TRUE(values.is_array());
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(JsonParserTest, EmptyNestedObjectArray)
{
    {
        const std::string contents("[{}]");

        auto parsed = m_parser.parse_string(contents);
        ASSERT_TRUE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        EXPECT_TRUE(values.is_array());
        EXPECT_EQ(values.size(), 1);

        values = values[0];
        EXPECT_TRUE(values.is_object());
        EXPECT_EQ(values.size(), 0);
    }
    {
        const std::string contents("[[]]");

        auto parsed = m_parser.parse_string(contents);
        ASSERT_TRUE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        EXPECT_TRUE(values.is_array());
        EXPECT_EQ(values.size(), 1);

        values = values[0];
        EXPECT_TRUE(values.is_array());
        EXPECT_EQ(values.size(), 0);
    }
}

//==================================================================================================
TEST_F(JsonParserTest, EmptyString)
{
    {
        const std::string contents("{\"a\" : \"\" }");

        auto parsed = m_parser.parse_string(contents);
        ASSERT_TRUE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        EXPECT_TRUE(values["a"].is_string());
        EXPECT_EQ(values["a"].size(), 0);
        EXPECT_EQ(values["a"], "");
    }
    {
        const std::string contents("{\"\" : \"a\" }");

        auto parsed = m_parser.parse_string(contents);
        ASSERT_TRUE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        EXPECT_TRUE(values[""].is_string());
        EXPECT_EQ(values[""].size(), 1);
        EXPECT_EQ(values[""], "a");
    }
    {
        const std::string contents("{\"\" : \"\" }");

        auto parsed = m_parser.parse_string(contents);
        ASSERT_TRUE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        EXPECT_TRUE(values[""].is_string());
        EXPECT_EQ(values[""].size(), 0);
        EXPECT_EQ(values[""], "");
    }
}

//==================================================================================================
TEST_F(JsonParserTest, NonObjectOrArray)
{
    validate_fail_raw("\"\"");
    validate_fail_raw("true");
    validate_fail_raw("1");
    validate_fail_raw("-1");
    validate_fail_raw("3.14");
    validate_fail_raw("null");
}

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
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

//==================================================================================================
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12 // here is a comment
        })";

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 1);
        EXPECT_EQ(json["a"], 12);
    }
    {
        std::string str = R"({
            "a" : 12, // here is a comment
            "b" : 13
        })";

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : "abdc // here is a comment efgh",
            "b" : 13
        })";

        {
            auto parsed = m_parser.parse_string(str);
            ASSERT_TRUE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            EXPECT_EQ(json.size(), 2);
            EXPECT_EQ(json["a"], "abdc // here is a comment efgh");
            EXPECT_EQ(json["b"], 13);
        }
        {
            auto parsed = parser.parse_string(str);
            ASSERT_TRUE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            EXPECT_EQ(json.size(), 2);
            EXPECT_EQ(json["a"], "abdc // here is a comment efgh");
            EXPECT_EQ(json["b"], 13);
        }
    }
}

//==================================================================================================
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12, /* here is a comment */
            "b" : 13
        })";

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12/* here is a comment */,
            "b" : 13
        })";

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
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

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : "abdc /* here is a comment */ efgh",
            "b" : 13
        })";

        {
            auto parsed = m_parser.parse_string(str);
            ASSERT_TRUE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            EXPECT_EQ(json.size(), 2);
            EXPECT_EQ(json["a"], "abdc /* here is a comment */ efgh");
            EXPECT_EQ(json["b"], 13);
        }
        {
            auto parsed = parser.parse_string(str);
            ASSERT_TRUE(parsed.has_value());

            fly::Json json = std::move(parsed.value());
            EXPECT_EQ(json.size(), 2);
            EXPECT_EQ(json["a"], "abdc /* here is a comment */ efgh");
            EXPECT_EQ(json["b"], 13);
        }
    }
}

//==================================================================================================
TEST_F(JsonParserTest, InvalidComment)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowComments);
    {
        std::string str = R"(/* here is a bad comment
        {
            "a" : 12
        })";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({
            "a" : 12
        }  /* here is a bad comment
        )";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({
            "a" : 12 / here is a bad comment
        })";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({"a" : 12 /)";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({
            "a" : 12 /* here is a bad comment
        })";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({"a" : 12 /*)";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
}

//==================================================================================================
TEST_F(JsonParserTest, TrailingCommaObject)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowTrailingComma);
    {
        std::string str = R"({
            "a" : 12,
            "b" : 13,
        })";

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_EQ(json.size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12,,
            "b" : 13,
        })";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : 13,,
        })";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
}

//==================================================================================================
TEST_F(JsonParserTest, TrailingCommaArray)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowTrailingComma);
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1, 2,],
        })";

        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
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

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1, 2,,],
        })";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
}

//==================================================================================================
TEST_F(JsonParserTest, AnyType)
{
    fly::JsonParser parser(fly::JsonParser::Features::AllowAnyType);
    {
        std::string str = "this is a string without quotes";

        EXPECT_FALSE(m_parser.parse_string(str).has_value());
        EXPECT_FALSE(parser.parse_string(str).has_value());
    }
    {
        std::string str = "\"this is a string\"";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_string());
        EXPECT_EQ(json, str.substr(1, str.size() - 2));
    }
    {
        std::string str = "true";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_boolean());
        EXPECT_EQ(json, true);
    }
    {
        std::string str = "false";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_boolean());
        EXPECT_EQ(json, false);
    }
    {
        std::string str = "null";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_null());
        EXPECT_EQ(json, nullptr);
    }
    {
        std::string str = "12389";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_unsigned_integer());
        EXPECT_EQ(json, 12389);
    }
    {
        std::string str = "-12389";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_signed_integer());
        EXPECT_EQ(json, -12389);
    }
    {
        std::string str = "123.89";
        validate_fail_raw(str);

        auto parsed = parser.parse_string(str);
        ASSERT_TRUE(parsed.has_value());

        fly::Json json = std::move(parsed.value());
        EXPECT_TRUE(json.is_float());
        EXPECT_DOUBLE_EQ(double(json), 123.89);
    }
}

//==================================================================================================
TEST(JsonParserFeatures, CombinedFeatures)
{
    // Strict
    {
        auto features = fly::JsonParser::Features::Strict;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::Strict);
    }

    // AllowComments
    {
        auto features = fly::JsonParser::Features::AllowComments;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
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
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::Strict);
    }

    // AllowTrailingComma
    {
        auto features = fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::Strict);
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
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::Strict);
    }

    // AllowAnyType
    {
        auto features = fly::JsonParser::Features::AllowAnyType;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::AllowAnyType);
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowAnyType;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::AllowAnyType);
    }

    // AllFeatures
    {
        auto features = fly::JsonParser::Features::AllFeatures;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::AllowAnyType);
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowComments;
        features = features | fly::JsonParser::Features::AllowTrailingComma;
        features = features | fly::JsonParser::Features::AllowAnyType;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma);
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowAnyType,
            fly::JsonParser::Features::AllowAnyType);
    }
}
