#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "fly/parser/exceptions.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/types/json.h"
#include "fly/types/string.h"

//==============================================================================
class JsonParserTest : public ::testing::Test
{
public:
    JsonParserTest() : m_spParser(std::make_shared<fly::JsonParser>())
    {
    }

protected:
    void ValidateFail(const std::string &test)
    {
        ValidateFailRaw(fly::String::Format("{ \"a\" : %s }", test));
    }

    void ValidateFailRaw(const std::string &test)
    {
        SCOPED_TRACE(test);
        EXPECT_THROW(m_spParser->Parse(test), fly::ParserException);
    }

    void ValidatePass(const std::string &test, const fly::Json &expected)
    {
        ValidatePassRaw(fly::String::Format("{ \"a\" : %s }", test), "a", expected);
    }

    void ValidatePassRaw(const std::string &test, const std::string &key, const fly::Json &expected)
    {
        fly::Json actual, repeat;

        SCOPED_TRACE(test);
        EXPECT_NO_THROW(actual = m_spParser->Parse(test));

        if (expected.IsFloat())
        {
            EXPECT_DOUBLE_EQ(double(actual[key]), double(expected));
        }
        else
        {
            EXPECT_EQ(actual[key], expected);
        }

        std::stringstream ss;
        ss << actual;

        EXPECT_NO_THROW(repeat = m_spParser->Parse(ss.str()));
        EXPECT_EQ(actual, repeat);
    }

    fly::ParserPtr m_spParser;
};

//==============================================================================
TEST_F(JsonParserTest, JsonCheckerTest)
{
    // Run the parser against test files from http://www.json.org/JSON_checker/
    // The following files are excluded from this test:
    // - fail18.json: The parser has no max-depth

    // Get the path to the JSON checker directory
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "json", "json_checker");

    // Validate each JSON file in the JSON checker directory
    std::vector<std::string> directories;
    std::vector<std::string> files;

    ASSERT_TRUE(fly::Path::ListPath(path, directories, files));

    for (const std::string &file : files)
    {
        SCOPED_TRACE(file);

        if (fly::String::StartsWith(file, "pass"))
        {
            EXPECT_NO_THROW(m_spParser->Parse(path, file));
        }
        else if (fly::String::StartsWith(file, "fail"))
        {
            EXPECT_THROW(m_spParser->Parse(path, file), fly::ParserException);
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==============================================================================
TEST_F(JsonParserTest, GoogleJsonTestSuiteTest)
{
    // https://code.google.com/archive/p/json-test-suite/
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "json", "google_json_test_suite");

    EXPECT_NO_THROW(m_spParser->Parse(path, "sample.json"));
}

//==============================================================================
TEST_F(JsonParserTest, NstJsonTestSuiteParsingTest)
{
    // Run the parser against test files from https://github.com/nst/JSONTestSuite
    // The following files are excluded from this test:
    // - n_single_space.json: Empty files are allowed
    // - n_structure_100000_opening_arrays.json: Too nested, causes stack overflow
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
    // - i_number_double_huge_neg_exp.json: Depends on platform (fails on Windows)

    // Indeterminate files expected to pass
    std::vector<std::string> iPass = {
        "i_structure_500_nested_arrays.json", // No enforced depth limit
        "i_structure_UTF-8_BOM_empty_object.json", // Byte order mark ignored
    };

    // Get the path to the JSONTestSuite directory
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "json", "nst_json_test_suite");

    // Validate each JSON file in the JSONTestSuite directory
    std::vector<std::string> directories;
    std::vector<std::string> files;

    ASSERT_TRUE(fly::Path::ListPath(path, directories, files));

    for (const std::string &file : files)
    {
        SCOPED_TRACE(file);

        if (fly::String::StartsWith(file, 'y'))
        {
            EXPECT_NO_THROW(m_spParser->Parse(path, file));
        }
        else if (fly::String::StartsWith(file, 'n'))
        {
            EXPECT_THROW(m_spParser->Parse(path, file), fly::ParserException);
        }
        else if (fly::String::StartsWith(file, 'i'))
        {
            if (std::find(iPass.begin(), iPass.end(), file) != iPass.end())
            {
                EXPECT_NO_THROW(m_spParser->Parse(path, file));
            }
            else
            {
                EXPECT_THROW(m_spParser->Parse(path, file), fly::ParserException);
            }
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==============================================================================
TEST_F(JsonParserTest, BigListOfNaughtyStringsTest)
{
    // https://github.com/minimaxir/big-list-of-naughty-strings
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "json", "big_list_of_naughty_strings");
    fly::Json values;

    EXPECT_NO_THROW(values = m_spParser->Parse(path, "blns.json"));
    EXPECT_EQ(values.Size(), 507);

    for (size_t i = 0; i < values.Size(); ++i)
    {
        EXPECT_TRUE(values[i].IsString());
    }
}

//==============================================================================
TEST_F(JsonParserTest, AllUnicodeTest)
{
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "json", "unicode");
    fly::Json values;

    EXPECT_NO_THROW(values = m_spParser->Parse(path, "all_unicode.json"));
    EXPECT_EQ(values.Size(), 1112064);
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingPathTest)
{
    fly::Json values;

    ASSERT_NO_THROW(m_spParser->Parse("foo_abc", "abc.json"));
    EXPECT_TRUE(values.IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingFileTest)
{
    fly::Json values;

    ASSERT_NO_THROW(m_spParser->Parse(fly::Path::GetTempDirectory(), "abc.json"));
    EXPECT_TRUE(values.IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, EmptyFileTest)
{
    const std::string contents;
    fly::Json values;

    ASSERT_NO_THROW(values = m_spParser->Parse(contents));
    EXPECT_TRUE(values.IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, EmptyObjectTest)
{
    const std::string contents("{}");
    fly::Json values;

    ASSERT_NO_THROW(values = m_spParser->Parse(contents));
    EXPECT_TRUE(values.IsObject());
    EXPECT_EQ(values.Size(), 0);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyArrayTest)
{
    const std::string contents("[]");
    fly::Json values;

    ASSERT_NO_THROW(values = m_spParser->Parse(contents));
    EXPECT_TRUE(values.IsArray());
    EXPECT_EQ(values.Size(), 0);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyNestedObjectArrayTest)
{
    fly::Json values;
    {
        const std::string contents("[{}]");
        ASSERT_NO_THROW(values = m_spParser->Parse(contents));

        EXPECT_TRUE(values.IsArray());
        EXPECT_EQ(values.Size(), 1);

        values = values[0];
        EXPECT_TRUE(values.IsObject());
        EXPECT_EQ(values.Size(), 0);
    }
    {
        const std::string contents("[[]]");
        ASSERT_NO_THROW(values = m_spParser->Parse(contents));

        EXPECT_TRUE(values.IsArray());
        EXPECT_EQ(values.Size(), 1);

        values = values[0];
        EXPECT_TRUE(values.IsArray());
        EXPECT_EQ(values.Size(), 0);
    }
}

//==============================================================================
TEST_F(JsonParserTest, EmptyStringTest)
{
    fly::Json values;
    {
        const std::string contents("{\"a\" : \"\" }");
        ASSERT_NO_THROW(values = m_spParser->Parse(contents));

        EXPECT_TRUE(values["a"].IsString());
        EXPECT_EQ(values["a"].Size(), 0);
        EXPECT_EQ(values["a"], "");
    }
    {
        const std::string contents("{\"\" : \"a\" }");
        ASSERT_NO_THROW(values = m_spParser->Parse(contents));

        EXPECT_TRUE(values[""].IsString());
        EXPECT_EQ(values[""].Size(), 1);
        EXPECT_EQ(values[""], "a");
    }
    {
        const std::string contents("{\"\" : \"\" }");
        ASSERT_NO_THROW(values = m_spParser->Parse(contents));

        EXPECT_TRUE(values[""].IsString());
        EXPECT_EQ(values[""].Size(), 0);
        EXPECT_EQ(values[""], "");
    }
}

//==============================================================================
TEST_F(JsonParserTest, NonObjectOrArrayTest)
{
    ValidateFailRaw("\"\"");
    ValidateFailRaw("true");
    ValidateFailRaw("1");
    ValidateFailRaw("-1");
    ValidateFailRaw("3.14");
    ValidateFailRaw("null");
}

//==============================================================================
TEST_F(JsonParserTest, BadlyFormedObjectTest)
{
    ValidateFailRaw(":");
    ValidateFailRaw(",");
    ValidateFailRaw("a");
    ValidateFailRaw("\"a\"");
    ValidateFailRaw("{");
    ValidateFailRaw("}");
    ValidateFailRaw("{ : }");
    ValidateFailRaw("{ , }");
    ValidateFailRaw("{ 1 }");
    ValidateFailRaw("{ { } }");
    ValidateFailRaw("{ [ ] }");
    ValidateFailRaw("{ \"a }");
    ValidateFailRaw("{ a\" }");
    ValidateFailRaw("{ \"a\" }");
    ValidateFailRaw("{ \"a\" : }");
    ValidateFailRaw("{ \"a\" , }");
    ValidateFailRaw("{ \"a\" : : 1 }");
    ValidateFailRaw("{ \"a\" , : 1 }");
    ValidateFailRaw("{ \"a\" : , 1 }");
    ValidateFailRaw("{ \"a : 1 }");
    ValidateFailRaw("{ a\" : 1 }");
    ValidateFailRaw("{ \"a\" : 1 ");
    ValidateFailRaw("{ \"a\" { }");
    ValidateFailRaw("{ \"a\" : { }");
    ValidateFailRaw("{ \"a\" [");
    ValidateFailRaw("{ \"a\" : [");
    ValidateFailRaw("{ \"a\" ]");
    ValidateFailRaw("{ \"a\" : ]");
    ValidateFailRaw("{ \"a\" tru }");
    ValidateFailRaw("{ \"a\" : tru }");
    ValidateFailRaw("{ \"a\" flse }");
    ValidateFailRaw("{ \"a\" : flse }");
    ValidateFailRaw("{ \"a\" 1, }");
    ValidateFailRaw("{ \"a\" : 1");
    ValidateFailRaw("{ \"a\" : ,");
    ValidateFailRaw("{ \"a\" : 1, }");
    ValidateFailRaw("{ \"a\" : 1 { }");
    ValidateFailRaw("{ \"a\" : 1 { } }");
    ValidateFailRaw("{ \"a\" : 1, { }");
    ValidateFailRaw("{ \"a\" : \"\\");
    ValidateFailRaw("{ \"a\" : \"\x01\" }");
    ValidateFailRaw("{ \"\x01\" : \"a\" }");
    ValidateFailRaw("{ 1 : 1 }");
}

//==============================================================================
TEST_F(JsonParserTest, BadlyFormedArrayTest)
{
    ValidateFailRaw("[");
    ValidateFailRaw("]");
    ValidateFailRaw("[ : ]");
    ValidateFailRaw("[ , ]");
    ValidateFailRaw("[ \"a ]");
    ValidateFailRaw("[ a\" ]");
    ValidateFailRaw("[ \"a\" : ]");
    ValidateFailRaw("[ \"a : 1 ]");
    ValidateFailRaw("[ a\" : 1 ]");
    ValidateFailRaw("[ \"a\", 1");
    ValidateFailRaw("[ \"a\" 1 ]");
    ValidateFailRaw("[ \"a\" [ ]");
    ValidateFailRaw("[ \"a\", [ ]");
    ValidateFailRaw("[ \"a\" [");
    ValidateFailRaw("[ \"a\", [");
    ValidateFailRaw("[ \"a\", ]");
    ValidateFailRaw("[ \"a\" true ]");
    ValidateFailRaw("[ \"a\", tru ]");
    ValidateFailRaw("[ \"a\" false ]");
    ValidateFailRaw("[ \"a\", flse ]");
    ValidateFailRaw("[ \"a\" 1, ]");
    ValidateFailRaw("[ \"a\", ,");
    ValidateFailRaw("[ \"a\", 1, ]");
    ValidateFailRaw("[ \"a\", 1 [ ]");
    ValidateFailRaw("[ \"a\", 1 [ ] ]");
    ValidateFailRaw("[ \"a\", \"\\");
    ValidateFailRaw("[ \"a\", \"\x01\" ]");
}

//==============================================================================
TEST_F(JsonParserTest, WhiteSpaceTest)
{
    ValidatePassRaw("{ \"a\" : 1 }", "a", 1);
    ValidatePassRaw("\n{ \n \"a\" \n : \n \t\t 1 \r \n }\n", "a", 1);

    ValidateFailRaw("{ \"a\t\" : 1 }");
    ValidateFailRaw("{ \"a\n\" : 1 }");
    ValidateFailRaw("{ \"a\r\" : 1 }");
    ValidateFailRaw("{ \"a\" : \"b\n\" }");
    ValidateFailRaw("{ \"a\" : \"b\r\" }");
    ValidateFailRaw("{ \"a\" : \"b\t\" }");
}

//==============================================================================
TEST_F(JsonParserTest, NumericConversionTest)
{
    ValidatePass("1", 1);
    ValidatePass("-1", -1);
    ValidatePass("1.2", 1.2);
    ValidatePass("-1.2", -1.2);

    ValidateFail("+1");
    ValidateFail("01");
    ValidateFail("+1.2");
    ValidateFail("1.2.1");

    ValidatePass("1.2e1", 12.0);
    ValidatePass("1.2E1", 12.0);
    ValidatePass("1.2e+1", 12.0);
    ValidatePass("1.2E+1", 12.0);
    ValidatePass("1.2e-1", 0.12);
    ValidatePass("1.2E-1", 0.12);

    ValidateFail("1abc");
    ValidateFail("-1abc");
    ValidateFail("1.2+e2");
    ValidateFail("1.2+E2");
    ValidateFail("1.2-e2");
    ValidateFail("1.2-E2");
    ValidateFail("1.2e2E2");
    ValidateFail("1.2e2e2");
    ValidateFail("1.2E2e2");
    ValidateFail("1.2E2E2");
    ValidateFail("01.1");
    ValidateFail(".1");
    ValidateFail("e5");
    ValidateFail("E5");
}

//==============================================================================
TEST_F(JsonParserTest, SingleLineCommentTest)
{
    auto spParser = std::make_shared<fly::JsonParser>(
        fly::JsonParser::Features::AllowComments
    );
    {
        std::string str = R"({
            "a" : 12, // here is a comment
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
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

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : "abdc // here is a comment efgh",
            "b" : 13
        })";

        fly::Json json;

        EXPECT_NO_THROW(m_spParser->Parse(str));
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
        EXPECT_EQ(json["a"], "abdc // here is a comment efgh");
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12 / here is a bad comment
        })";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
    {
        std::string str = R"({"a" : 12 /)";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
}

//==============================================================================
TEST_F(JsonParserTest, MultiLineCommentTest)
{
    auto spParser = std::make_shared<fly::JsonParser>(
        fly::JsonParser::Features::AllowComments
    );
    {
        std::string str = R"({
            "a" : 12, /* here is a comment */
            "b" : 13
        })";

        fly::Json json;

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
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

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
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

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : "abdc /* here is a comment */ efgh",
            "b" : 13
        })";

        fly::Json json;

        EXPECT_NO_THROW(m_spParser->Parse(str));
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
        EXPECT_EQ(json["a"], "abdc /* here is a comment */ efgh");
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12 /* here is a bad comment
        })";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
    {
        std::string str = R"({"a" : 12 /*)";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
}

//==============================================================================
TEST_F(JsonParserTest, TrailingCommaObjectTest)
{
    auto spParser = std::make_shared<fly::JsonParser>(
        fly::JsonParser::Features::AllowTrailingComma
    );
    {
        std::string str = R"({
            "a" : 12,
            "b" : 13,
        })";

        fly::Json json;

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_EQ(json["b"], 13);
    }
    {
        std::string str = R"({
            "a" : 12,,
            "b" : 13,
        })";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : 13,,
        })";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
}

//==============================================================================
TEST_F(JsonParserTest, TrailingCommaArrayTest)
{
    auto spParser = std::make_shared<fly::JsonParser>(
        fly::JsonParser::Features::AllowTrailingComma
    );
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1, 2,],
        })";

        fly::Json json;

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_NO_THROW(json = spParser->Parse(str));
        EXPECT_EQ(json.Size(), 2);
        EXPECT_EQ(json["a"], 12);
        EXPECT_TRUE(json["b"].IsArray());
        EXPECT_EQ(json["b"].Size(), 2);
        EXPECT_EQ(json["b"][0], 1);
        EXPECT_EQ(json["b"][1], 2);
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1,, 2,],
        })";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
    {
        std::string str = R"({
            "a" : 12,
            "b" : [1, 2,,],
        })";

        EXPECT_THROW(m_spParser->Parse(str), fly::ParserException);
        EXPECT_THROW(spParser->Parse(str), fly::ParserException);
    }
}

//==============================================================================
TEST(JsonParserFeatures, CombinedFeaturesTest)
{
    {
        auto features = fly::JsonParser::Features::Strict;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict
        );
    }

    {
        auto features = fly::JsonParser::Features::AllowComments;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict
        );
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowComments;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::Strict
        );
    }

    {
        auto features = fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma
        );
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::Strict
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma
        );
    }

    {
        auto features = fly::JsonParser::Features::AllFeatures;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma
        );
    }
    {
        auto features = fly::JsonParser::Features::Strict;
        features = features | fly::JsonParser::Features::AllowComments;
        features = features | fly::JsonParser::Features::AllowTrailingComma;
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowComments,
            fly::JsonParser::Features::AllowComments
        );
        EXPECT_EQ(
            features & fly::JsonParser::Features::AllowTrailingComma,
            fly::JsonParser::Features::AllowTrailingComma
        );
    }
}
