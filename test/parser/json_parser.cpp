#include "test/parser/parser_test.h"

#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "fly/parser/exceptions.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

//==============================================================================
class JsonParserTest : public ParserTest
{
public:
    JsonParserTest() :
        ParserTest(),
        m_spParser(std::make_shared<fly::JsonParser>(m_path, m_file))
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

        CreateFile(test);
        EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
    }

    void ValidatePass(const std::string &test, const fly::Json &expected)
    {
        ValidatePassRaw(fly::String::Format("{ \"a\" : %s }", test), "a", expected);
    }

    void ValidatePassRaw(const std::string &test, const std::string &key, const fly::Json &expected)
    {
        SCOPED_TRACE(test);

        CreateFile(test);
        EXPECT_NO_THROW(m_spParser->Parse());

        fly::Json actual = m_spParser->GetValues();

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

        CreateFile(ss.str());
        EXPECT_NO_THROW(m_spParser->Parse());

        fly::Json repeat = m_spParser->GetValues();
        EXPECT_EQ(actual, repeat);
    }

    fly::JsonParserPtr m_spParser;
};

//==============================================================================
TEST_F(JsonParserTest, JsonCheckerTest)
{
    // Run the parser against test files from http://www.json.org/JSON_checker/
    // The following files are excluded from this test:
    //      - fail18.json: The parser has no max-depth

    // Get the path to the JSON checker directory
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "json_checker");

    // Validate each JSON file in the JSON checker directory
    std::vector<std::string> directories;
    std::vector<std::string> files;

    ASSERT_TRUE(fly::Path::ListPath(path, directories, files));

    for (const std::string &file : files)
    {
        m_spParser = std::make_shared<fly::JsonParser>(path, file);
        SCOPED_TRACE(file);

        if (fly::String::WildcardMatch(file, "pass*.json"))
        {
            EXPECT_NO_THROW(m_spParser->Parse());
        }
        else if (fly::String::WildcardMatch(file, "fail*.json"))
        {
            EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==============================================================================
TEST_F(JsonParserTest, AllUnicodeTest)
{
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    std::string path = fly::Path::Join(segments[0], "unicode");

    m_spParser = std::make_shared<fly::JsonParser>(path, "all_unicode.json");
    EXPECT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 1112064);
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingPathTest)
{
    m_spParser = std::make_shared<fly::JsonParser>(m_path + "foo", m_file);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_TRUE(m_spParser->GetValues().IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingFileTest)
{
    m_spParser = std::make_shared<fly::JsonParser>(m_path, m_file + "foo");

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_TRUE(m_spParser->GetValues().IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, EmptyFileTest)
{
    const std::string contents;
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_TRUE(m_spParser->GetValues().IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, EmptyObjectTest)
{
    const std::string contents("{}");
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    fly::Json json = m_spParser->GetValues();
    EXPECT_TRUE(json.IsObject());
    EXPECT_TRUE(json.Size() == 0);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyArrayTest)
{
    const std::string contents("[]");
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    fly::Json json = m_spParser->GetValues();
    EXPECT_TRUE(json.IsArray());
    EXPECT_TRUE(json.Size() == 0);
}

//==============================================================================
TEST_F(JsonParserTest, EmptyNestedObjectArrayTest)
{
    {
        const std::string contents("[{}]");
        CreateFile(contents);

        ASSERT_NO_THROW(m_spParser->Parse());

        fly::Json json = m_spParser->GetValues();
        EXPECT_TRUE(json.IsArray());
        EXPECT_TRUE(json.Size() == 1);

        json = json[0];
        EXPECT_TRUE(json.IsObject());
        EXPECT_TRUE(json.Size() == 0);
    }
    {
        const std::string contents("[[]]");
        CreateFile(contents);

        ASSERT_NO_THROW(m_spParser->Parse());

        fly::Json json = m_spParser->GetValues();
        EXPECT_TRUE(json.IsArray());
        EXPECT_TRUE(json.Size() == 1);

        json = json[0];
        EXPECT_TRUE(json.IsArray());
        EXPECT_TRUE(json.Size() == 0);
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
