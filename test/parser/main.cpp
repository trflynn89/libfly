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
        "address= \t '\tUSA'"
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
    CreateFile("\"\"");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("true");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("1");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("-1");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("3.14");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("null");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, BadlyFormedObjectTest)
{
    CreateFile(":");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(",");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("a");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("\"a\"");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("}");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ { } }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ [ ] }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ a\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ a\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1 ");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" { }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : { }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" [");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : [");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" ]");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : ]");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" tru }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : tru }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" flse }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : flse }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" 1, }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : ,");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1, }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1 { }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1 { } }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1, { }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, WhiteSpaceTest)
{
    CreateFile("{ \"a\" : 1 }");
    EXPECT_NO_THROW(m_spParser->Parse());

    CreateFile("\n{ \n \"a\" \n : \n \t\t 1 \r \n }\n");
    EXPECT_NO_THROW(m_spParser->Parse());

    CreateFile("{ \"a\t\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\n\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\r\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : \"b\n\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : \"b\r\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : \"b\t\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, NumericConversionTest)
{
    fly::Json json;

    CreateFile("{ \"a\" : 1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_EQ(json["a"], 1);

    CreateFile("{ \"a\" : -1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_EQ(json["a"], -1);

    CreateFile("{ \"a\" : +1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 01 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 1.2);

    CreateFile("{ \"a\" : -1.2 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), -1.2);

    CreateFile("{ \"a\" : +1.2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2.1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2e1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 12);

    CreateFile("{ \"a\" : 1.2E1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 12);

    CreateFile("{ \"a\" : 1.2e+1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 12);

    CreateFile("{ \"a\" : 1.2E+1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 12);

    CreateFile("{ \"a\" : 1.2e-1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 0.12);

    CreateFile("{ \"a\" : 1.2E-1 }");
    EXPECT_NO_THROW(m_spParser->Parse());
    json = m_spParser->GetValues();
    EXPECT_DOUBLE_EQ(double(json["a"]), 0.12);

    CreateFile("{ \"a\" : 1.2+e2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2+E2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2-e2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2-E2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2e2E2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2e2e2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2E2e2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1.2E2E2 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 01.1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : .1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : e5 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : E5 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}
