#include <fstream>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "fly/logger/logger.h"
#include "fly/parser/ini_parser.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

//==============================================================================
TEST(ParserExceptionTest, ExceptionTest)
{
    std::string file("test_file");
    int line = 123;
    std::string message("Bad file!");

    try
    {
        throw fly::ParserException(file, line, message);
    }
    catch (const fly::ParserException &ex)
    {
        std::string what(ex.what());

        EXPECT_NE(what.find(file), std::string::npos);
        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_NE(what.find(message), std::string::npos);
    }
}

//==============================================================================
class ParserTest : public ::testing::Test
{
public:
    ParserTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt")
    {
        LOGC("Using path '%s' : '%s'", m_path, m_file);
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
    /**
     * Create a file with the given contents.
     *
     * @param string Contents of the file to create.
     */
    void CreateFile(const std::string &contents)
    {
        const std::string path = fly::Path::Join(m_path, m_file);
        {
            std::ofstream stream(path, std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(path, std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    std::string m_path;
    std::string m_file;
};

//==============================================================================
class IniParserTest : public ParserTest
{
public:
    IniParserTest() :
        ParserTest(),
        m_spParser(std::make_shared<fly::IniParser>(m_path, m_file))
    {
    }

protected:
    fly::IniParserPtr m_spParser;
};

//==============================================================================
TEST_F(IniParserTest, NonExistingPathTest)
{
    m_spParser = std::make_shared<fly::IniParser>(m_path + "foo", m_file);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 0);
}

//==============================================================================
TEST_F(IniParserTest, NonExistingFileTest)
{
    m_spParser = std::make_shared<fly::IniParser>(m_path, m_file + "foo");

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 0);
}

//==============================================================================
TEST_F(IniParserTest, EmptyFileTest)
{
    const std::string contents;
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 0);
}

//==============================================================================
TEST_F(IniParserTest, EmptySectionTest)
{
    const std::string contents("[section]");
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 0);
}

//==============================================================================
TEST_F(IniParserTest, NonEmptySectionTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 2);
}

//==============================================================================
TEST_F(IniParserTest, NonExistingTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetValues("section").Size(), 2);
    EXPECT_EQ(m_spParser->GetValues("bad-section").Size(), 0);
    EXPECT_EQ(m_spParser->GetValues("section-bad").Size(), 0);
}

//==============================================================================
TEST_F(IniParserTest, CommentTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "; [other-section]\n"
        "; name=Jane Doe\n"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("other-section").Size(), 0);
}

//==============================================================================
TEST_F(IniParserTest, ErrantSpacesTest)
{
    const std::string contents(
        "   [section   ]  \n"
        "\t\t\n   name=John Doe\t  \n"
        "\taddress  = USA\t \r \n"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 2);
}

//==============================================================================
TEST_F(IniParserTest, QuotedValueTest)
{
    const std::string contents(
        "[section]\n"
        "name=\"  John Doe  \"\n"
        "address= \t '\\tUSA'"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 2);
}

//==============================================================================
TEST_F(IniParserTest, MutlipleSectionTypeTest)
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
        "noage=1\n"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetValues().Size(), 3);
    EXPECT_EQ(m_spParser->GetValues("section1").Size(), 2);
    EXPECT_EQ(m_spParser->GetValues("section2").Size(), 2);
    EXPECT_EQ(m_spParser->GetValues("section3").Size(), 2);
}

//==============================================================================
TEST_F(IniParserTest, DuplicateSectionTest)
{
    const std::string contents1(
        "[section]\n"
        "name=John Doe\n"
        "[section]\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents1);
    EXPECT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section")["name"], "Jane Doe");

    const std::string contents2(
        "[  \tsection]\n"
        "name=John Doe\n"
        "[section  ]\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents2);
    EXPECT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section")["name"], "Jane Doe");
}

//==============================================================================
TEST_F(IniParserTest, DuplicateValueTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents);
    EXPECT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section")["name"], "Jane Doe");
}

//==============================================================================
TEST_F(IniParserTest, ImbalancedBraceTest)
{
    const std::string contents1(
        "[section\n"
        "name=John Doe\n"
    );

    const std::string contents2(
        "section]\n"
        "name=John Doe\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, ImbalancedQuoteTest)
{
    const std::string contents1(
        "[section]\n"
        "name=\"John Doe\n"
    );

    const std::string contents2(
        "[section]\n"
        "name=John Doe\"\n"
    );
    const std::string contents3(
        "[section]\n"
        "name='John Doe\n"
    );

    const std::string contents4(
        "[section]\n"
        "name=John Doe'\n"
    );

    const std::string contents5(
        "[section]\n"
        "name=\"John Doe'\n"
    );

    const std::string contents6(
        "[section]\n"
        "name='John Doe\"\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents3);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents4);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents5);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents6);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, MisplacedQuoteTest)
{
    const std::string contents1(
        "[section]\n"
        "\"name\"=John Doe\n"
    );

    const std::string contents2(
        "[section]\n"
        "\'name\'=John Doe\n"
    );

    const std::string contents3(
        "[\"section\"]\n"
        "name=John Doe\n"
    );

    const std::string contents4(
        "[\'section\']\n"
        "name=John Doe\n"
    );

    const std::string contents5(
        "\"[section]\"\n"
        "name=John Doe\n"
    );

    const std::string contents6(
        "\'[section]\'\n"
        "name=John Doe\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents3);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents4);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents5);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents6);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, MultipleAssignmentTest)
{
    const std::string contents1(
        "[section]\n"
        "name=John=Doe\n"
    );
    const std::string contents2(
        "[section]\n"
        "name=\"John=Doe\"\n"
    );

    CreateFile(contents1);
    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 1);

    CreateFile(contents2);
    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetValues().Size(), 1);
    EXPECT_EQ(m_spParser->GetValues("section").Size(), 1);
}

//==============================================================================
TEST_F(IniParserTest, MissingAssignmentTest)
{
    const std::string contents1(
        "[section]\n"
        "name\n"
    );

    const std::string contents2(
        "[section]\n"
        "name=\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, EarlyAssignmentTest)
{
    const std::string contents1(
        "name=John Doe\n"
        "[section]\n"
    );

    const std::string contents2(
        "name=\n"
        "[section]\n"
    );

    const std::string contents3(
        "name\n"
        "[section]\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents3);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, MultipleParseTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA"
    );

    CreateFile(contents);

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_NO_THROW(m_spParser->Parse());

        EXPECT_EQ(m_spParser->GetValues().Size(), 1);
        EXPECT_EQ(m_spParser->GetValues("section").Size(), 2);
    }
}

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
    // Get the path to the unicode directory
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
