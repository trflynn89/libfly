#include <fstream>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/logger/logger.h"
#include "fly/parser/ini_parser.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

#if defined(FLY_WINDOWS)

#include <Windows.h>

#define utf8(str) ConvertToUTF8(L##str)

namespace
{
    const char *ConvertToUTF8(const wchar_t *str)
    {
        static char buff[1024];

        ::WideCharToMultiByte(CP_UTF8, 0, str, -1, buff, sizeof(buff), NULL, NULL);
        return buff;
    }
}
#else

#define utf8(str) str

#endif

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
    void ValidateFailString(const std::string &test)
    {
        ValidateFailNonString(fly::String::Format("\"%s\"", test));
    }

    void ValidateFailNonString(const std::string &test)
    {
        ValidateFailRaw(fly::String::Format("{ \"a\" : %s }", test));
    }

    void ValidateFailRaw(const std::string &test)
    {
        SCOPED_TRACE(test);

        CreateFile(test);
        EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
    }

    void ValidatePassString(const std::string &test)
    {
        ValidatePassString(test, test);
    }

    void ValidatePassString(const std::string &test, const fly::Json &expected)
    {
        ValidatePassNonString(fly::String::Format("\"%s\"", test), expected);
    }

    void ValidatePassNonString(const std::string &test, const fly::Json &expected)
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
    ValidateFailRaw("{ { } }");
    ValidateFailRaw("{ [ ] }");
    ValidateFailRaw("{ \"a }");
    ValidateFailRaw("{ a\" }");
    ValidateFailRaw("{ \"a\" }");
    ValidateFailRaw("{ \"a\" : }");
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
    ValidatePassNonString("1", 1);
    ValidatePassNonString("-1", -1);
    ValidatePassNonString("1.2", 1.2);
    ValidatePassNonString("-1.2", -1.2);

    ValidateFailNonString("+1");
    ValidateFailNonString("01");
    ValidateFailNonString("+1.2");
    ValidateFailNonString("1.2.1");

    ValidatePassNonString("1.2e1", 12.0);
    ValidatePassNonString("1.2E1", 12.0);
    ValidatePassNonString("1.2e+1", 12.0);
    ValidatePassNonString("1.2E+1", 12.0);
    ValidatePassNonString("1.2e-1", 0.12);
    ValidatePassNonString("1.2E-1", 0.12);

    ValidateFailNonString("1.2+e2");
    ValidateFailNonString("1.2+E2");
    ValidateFailNonString("1.2-e2");
    ValidateFailNonString("1.2-E2");
    ValidateFailNonString("1.2e2E2");
    ValidateFailNonString("1.2e2e2");
    ValidateFailNonString("1.2E2e2");
    ValidateFailNonString("1.2E2E2");
    ValidateFailNonString("01.1");
    ValidateFailNonString(".1");
    ValidateFailNonString("e5");
    ValidateFailNonString("E5");
}

//==============================================================================
TEST_F(JsonParserTest, UnicodeConversionTest)
{
    ValidateFailRaw("{ \"a\" : \"\\u");
    ValidateFailString("\\u");
    ValidateFailString("\\u0");
    ValidateFailString("\\u00");
    ValidateFailString("\\u000");
    ValidateFailString("\\u000z");

    ValidatePassString("\\u0040", utf8("\u0040"));
    ValidatePassString("\\u007A", utf8("\u007A"));
    ValidatePassString("\\u007a", utf8("\u007a"));
    ValidatePassString("\\u00c4", utf8("\u00c4"));
    ValidatePassString("\\u00e4", utf8("\u00e4"));
    ValidatePassString("\\u0298", utf8("\u0298"));
    ValidatePassString("\\u0800", utf8("\u0800"));
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
TEST_F(JsonParserTest, MarkusKuhnStressTest)
{
    // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

    // 1  Some correct UTF-8 text
    {
        ValidatePassString("κόσμε");
    }

    // 2  Boundary condition test cases
    {
        // 2.1  First possible sequence of a certain length
        {
            // 2.1.1  1 byte  (U-00000000)
            //ValidatePassString("\\u0000", "\U00000000");

            // 2.1.2  2 bytes (U-00000080)
            ValidatePassString("\xc2\x80");

            // 2.1.3  3 bytes (U-00000800)
            ValidatePassString("\xe0\xa0\x80");

            // 2.1.4  4 bytes (U-00010000)
            ValidatePassString("\xf0\x90\x80\x80");

            // 2.1.5  5 bytes (U-00200000)
            ValidateFailString("\xf8\x88\x80\x80\x80");

            // 2.1.6  6 bytes (U-04000000)
            ValidateFailString("\xfc\x84\x80\x80\x80\x80");
        }

        // 2.2  Last possible sequence of a certain length
        {
            // 2.2.1  1 byte  (U-0000007F)
            ValidatePassString("\x7f");

            // 2.2.2  2 bytes (U-000007FF)
            ValidatePassString("\xdf\xbf");

            // 2.2.3  3 bytes (U-0000FFFF)
            ValidatePassString("\xef\xbf\xbf");

            // 2.1.4  4 bytes (U-00200000)
            ValidateFailString("\xf7\xbf\xbf\xbf");

            // 2.1.5  5 bytes (U-03FFFFFF)
            ValidateFailString("\xfb\xbf\xbf\xbf\xbf");

            // 2.1.6  6 bytes (U-7FFFFFFF)
            ValidateFailString("\xfd\xbf\xbf\xbf\xbf\xbf");
        }

        // 2.3  Other boundary conditions
        {
            // 2.3.1  U-0000D7FF
            ValidatePassString("\xed\x9f\xbf");

            // 2.3.2  U-0000E000
            ValidatePassString("\xee\x80\x80");

            // 2.3.3  U-0000FFFD
            ValidatePassString("\xef\xbf\xbd");

            // 2.3.4  U-0010FFFF
            ValidatePassString("\xf4\x8f\xbf\xbf");

            // 2.3.5  U-00110000
            ValidateFailString("\xf4\x90\x80\x80");
        }
    }

    // 3  Malformed sequences
    {
        // 3.1  Unexpected continuation bytes
        {
            // 3.1.1  First continuation byte 0x80
            ValidateFailString("\x80");

            // 3.1.2 Last  continuation byte 0xbf
            ValidateFailString("\xbf");

            // 3.1.3  2 continuation bytes
            ValidateFailString("\x80\xbf");

            // 3.1.4  3 continuation bytes
            ValidateFailString("\x80\xbf\x80");

            // 3.1.5  4 continuation bytes
            ValidateFailString("\x80\xbf\x80\xbf");

            // 3.1.6  5 continuation bytes
            ValidateFailString("\x80\xbf\x80\xbf\x80");

            // 3.1.7  6 continuation bytes
            ValidateFailString("\x80\xbf\x80\xbf\x80\xbf");

            // 3.1.8  7 continuation bytes
            ValidateFailString("\x80\xbf\x80\xbf\x80\xbf\x80");

            // 3.1.9  Sequence of all 64 possible continuation bytes (0x80-0xbf)
            ValidateFailString("\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf");
        }

        // 3.2  Lonely start characters
        {
            // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf),
            //        each followed by a space character
            ValidateFailString("\xc0 \xc1 \xc2 \xc3 \xc4 \xc5 \xc6 \xc7 \xc8 \xc9 \xca \xcb \xcc \xcd \xce \xcf \xd0 \xd1 \xd2 \xd3 \xd4 \xd5 \xd6 \xd7 \xd8 \xd9 \xda \xdb \xdc \xdd \xde \xdf");

            // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef)
            //        each followed by a space character
            ValidateFailString("\xe0 \xe1 \xe2 \xe3 \xe4 \xe5 \xe6 \xe7 \xe8 \xe9 \xea \xeb \xec \xed \xee \xef");

            // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7),
            //        each followed by a space character
            ValidateFailString("\xf0 \xf1 \xf2 \xf3 \xf4 \xf5 \xf6 \xf7");

            // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb),
            //        each followed by a space character
            ValidateFailString("\xf8 \xf9 \xfa \xfb");

            // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd),
            //        each followed by a space character
            ValidateFailString("\xfc \xfd");
        }

        // 3.3  Sequences with last continuation byte missing
        {
            // 3.3.1  2-byte sequence with last byte missing (U+0000)
            ValidateFailString("\xc0");

            // 3.3.2  3-byte sequence with last byte missing (U+0000)
            ValidateFailString("\xe0\x80");

            // 3.3.3  4-byte sequence with last byte missing (U+0000)
            ValidateFailString("\xf0\x80\x80");

            // 3.3.4  5-byte sequence with last byte missing (U+0000)
            ValidateFailString("\xf8\x80\x80\x80");

            // 3.3.5  6-byte sequence with last byte missing (U+0000)
            ValidateFailString("\xfc\x80\x80\x80\x80");

            // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
            ValidateFailString("\xdf");

            // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
            ValidateFailString("\xef\xbf");

            // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
            ValidateFailString("\xf7\xbf\xbf");

            // 3.3.9  5-byte sequence with last byte missing (U-03FFFFFF)
            ValidateFailString("\xfb\xbf\xbf\xbf");

            // 3.3.10 6-byte sequence with last byte missing (U-7FFFFFFF)
            ValidateFailString("\xfd\xbf\xbf\xbf\xbf");
        }

        // 3.4  Concatenation of incomplete sequences
        {
            // All the 10 sequences of 3.3 concatenated, you should see 10 malformed
            // sequences being signalled
            ValidateFailString("\xc0\xe0\x80\xf0\x80\x80\xf8\x80\x80\x80\xfc\x80\x80\x80\x80\xdf\xef\xbf\xf7\xbf\xbf\xfb\xbf\xbf\xbf\xfd\xbf\xbf\xbf\xbf");
        }

        // 3.5  Impossible bytes
        {
            // 3.5.1  fe
            ValidateFailString("\xfe");

            // 3.5.2  ff
            ValidateFailString("\xff");

            // 3.5.3  fe fe ff ff
            ValidateFailString("\xfe\xfe\xff\xff");
        }
    }

    // 4  Overlong sequences
    {
        // 4.1  Examples of an overlong ASCII character
        {
            // 4.1.1 U+002F = c0 af
            ValidateFailString("\xc0\xaf");

            // 4.1.2 U+002F = e0 80 af
            ValidateFailString("\xe0\x80\xaf");

            // 4.1.3 U+002F = f0 80 80 af
            ValidateFailString("\xf0\x80\x80\xaf");

            // 4.1.4 U+002F = f8 80 80 80 af
            ValidateFailString("\xf8\x80\x80\x80\xaf");

            // 4.1.5 U+002F = fc 80 80 80 80 af
            ValidateFailString("\xfc\x80\x80\x80\x80\xaf");
        }

        // 4.2  Maximum overlong sequences
        {
            // 4.2.1  U-0000007F = c1 bf
            ValidateFailString("\xc1\xbf");

            // 4.2.2  U-000007FF = e0 9f bf
            ValidateFailString("\xe0\x9f\xbf");

            // 4.2.3  U-0000FFFF = f0 8f bf bf
            ValidateFailString("\xf0\x8f\xbf\xbf");

            // 4.2.4  U-001FFFFF = f8 87 bf bf bf
            ValidateFailString("\xf8\x87\xbf\xbf\xbf");

            // 4.2.5  U-03FFFFFF = fc 83 bf bf bf bf
            ValidateFailString("\xfc\x83\xbf\xbf\xbf\xbf");
        }

        // 4.3  Overlong representation of the NUL character
        {
            // 4.3.1  U+0000 = c0 80
            ValidateFailString("\xc0\x80");

            // 4.3.2  U+0000 = e0 80 80
            ValidateFailString("\xe0\x80\x80");

            // 4.3.3  U+0000 = f0 80 80 80
            ValidateFailString("\xf0\x80\x80\x80");

            // 4.3.4  U+0000 = f8 80 80 80 80
            ValidateFailString("\xf8\x80\x80\x80\x80");

            // 4.3.5  U+0000 = fc 80 80 80 80 80
            ValidateFailString("\xfc\x80\x80\x80\x80\x80");
        }
    }

    // 5  Illegal code positions
    {
        // 5.1 Single UTF-16 surrogates
        {
            // 5.1.1  U+D800 = ed a0 80
            ValidateFailString("\xed\xa0\x80");

            // 5.1.2  U+DB7F = ed ad bf
            ValidateFailString("\xed\xad\xbf");

            // 5.1.3  U+DB80 = ed ae 80
            ValidateFailString("\xed\xae\x80");

            // 5.1.4  U+DBFF = ed af bf
            ValidateFailString("\xed\xaf\xbf");

            // 5.1.5  U+DC00 = ed b0 80
            ValidateFailString("\xed\xb0\x80");

            // 5.1.6  U+DF80 = ed be 80
            ValidateFailString("\xed\xbe\x80");

            // 5.1.7  U+DFFF = ed bf bf
            ValidateFailString("\xed\xbf\xbf");
        }

        // 5.2 Paired UTF-16 surrogates
        {
            // 5.2.1  U+D800 U+DC00 = ed a0 80 ed b0 80
            ValidateFailString("\xed\xa0\x80\xed\xb0\x80");

            // 5.2.2  U+D800 U+DFFF = ed a0 80 ed bf bf
            ValidateFailString("\xed\xa0\x80\xed\xbf\xbf");

            // 5.2.3  U+DB7F U+DC00 = ed ad bf ed b0 80
            ValidateFailString("\xed\xad\xbf\xed\xb0\x80");

            // 5.2.4  U+DB7F U+DFFF = ed ad bf ed bf bf
            ValidateFailString("\xed\xad\xbf\xed\xbf\xbf");

            // 5.2.5  U+DB80 U+DC00 = ed ae 80 ed b0 80
            ValidateFailString("\xed\xae\x80\xed\xb0\x80");

            // 5.2.6  U+DB80 U+DFFF = ed ae 80 ed bf bf
            ValidateFailString("\xed\xae\x80\xed\xbf\xbf");

            // 5.2.7  U+DBFF U+DC00 = ed af bf ed b0 80
            ValidateFailString("\xed\xaf\xbf\xed\xb0\x80");

            // 5.2.8  U+DBFF U+DFFF = ed af bf ed bf bf
            ValidateFailString("\xed\xaf\xbf\xed\xbf\xbf");
        }

        // 5.3 Noncharacter code positions
        {
            // 5.3.1  U+FFFE = ef bf be
            ValidatePassString("\xef\xbf\xbe");

            // 5.3.2  U+FFFF = ef bf bf
            ValidatePassString("\xef\xbf\xbf");

            // 5.3.3  U+FDD0 .. U+FDEF
            ValidatePassString("\xEF\xB7\x90");
            ValidatePassString("\xEF\xB7\x91");
            ValidatePassString("\xEF\xB7\x92");
            ValidatePassString("\xEF\xB7\x93");
            ValidatePassString("\xEF\xB7\x94");
            ValidatePassString("\xEF\xB7\x95");
            ValidatePassString("\xEF\xB7\x96");
            ValidatePassString("\xEF\xB7\x97");
            ValidatePassString("\xEF\xB7\x98");
            ValidatePassString("\xEF\xB7\x99");
            ValidatePassString("\xEF\xB7\x9A");
            ValidatePassString("\xEF\xB7\x9B");
            ValidatePassString("\xEF\xB7\x9C");
            ValidatePassString("\xEF\xB7\x9D");
            ValidatePassString("\xEF\xB7\x9E");
            ValidatePassString("\xEF\xB7\x9F");
            ValidatePassString("\xEF\xB7\xA0");
            ValidatePassString("\xEF\xB7\xA1");
            ValidatePassString("\xEF\xB7\xA2");
            ValidatePassString("\xEF\xB7\xA3");
            ValidatePassString("\xEF\xB7\xA4");
            ValidatePassString("\xEF\xB7\xA5");
            ValidatePassString("\xEF\xB7\xA6");
            ValidatePassString("\xEF\xB7\xA7");
            ValidatePassString("\xEF\xB7\xA8");
            ValidatePassString("\xEF\xB7\xA9");
            ValidatePassString("\xEF\xB7\xAA");
            ValidatePassString("\xEF\xB7\xAB");
            ValidatePassString("\xEF\xB7\xAC");
            ValidatePassString("\xEF\xB7\xAD");
            ValidatePassString("\xEF\xB7\xAE");
            ValidatePassString("\xEF\xB7\xAF");

            // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
            ValidatePassString("\xF0\x9F\xBF\xBF");
            ValidatePassString("\xF0\xAF\xBF\xBF");
            ValidatePassString("\xF0\xBF\xBF\xBF");
            ValidatePassString("\xF1\x8F\xBF\xBF");
            ValidatePassString("\xF1\x9F\xBF\xBF");
            ValidatePassString("\xF1\xAF\xBF\xBF");
            ValidatePassString("\xF1\xBF\xBF\xBF");
            ValidatePassString("\xF2\x8F\xBF\xBF");
            ValidatePassString("\xF2\x9F\xBF\xBF");
            ValidatePassString("\xF2\xAF\xBF\xBF");
        }
    }
}
