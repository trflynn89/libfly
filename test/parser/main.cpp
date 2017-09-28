#include <fstream>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/logger/logger.h"
#include "fly/parser/ini_parser.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/string/string.h"

//==============================================================================
class IniParserTest : public ::testing::Test
{
public:
    IniParserTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt"),
        m_spParser(std::make_shared<fly::IniParser>(m_path, m_file))
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
        {
            std::ofstream stream(GetFullPath(), std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(GetFullPath(), std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    /**
     * @return The full path to the file being monitored.
     */
    std::string GetFullPath() const
    {
        static const char sep = fly::Path::GetSeparator();
        return fly::String::Join(sep, m_path, m_file);
    }

    std::string m_path;
    std::string m_file;

    fly::IniParserPtr m_spParser;
};

//==============================================================================
TEST_F(IniParserTest, NonExistingPathTest)
{
    m_spParser = std::make_shared<fly::IniParser>(m_path + "foo", m_file);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, NonExistingFileTest)
{
    m_spParser = std::make_shared<fly::IniParser>(m_path, m_file + "foo");

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, EmptyFileTest)
{
    const std::string contents;
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, EmptySectionTest)
{
    const std::string contents("[section]");
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
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

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 2);
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

    EXPECT_EQ(m_spParser->GetSize("section"), 2);
    EXPECT_EQ(m_spParser->GetSize("bad-section"), 0);
    EXPECT_EQ(m_spParser->GetSize("section-bad"), 0);
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

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 1);
    EXPECT_EQ(m_spParser->GetSize("other-section"), 0);
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

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 2);
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

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 2);
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

    EXPECT_EQ(m_spParser->GetSize(), 3);
    EXPECT_EQ(m_spParser->GetSize("section1"), 2);
    EXPECT_EQ(m_spParser->GetSize("section2"), 2);
    EXPECT_EQ(m_spParser->GetSize("section3"), 2);
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
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    const std::string contents2(
        "[  \tsection]\n"
        "name=John Doe\n"
        "[section  ]\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
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
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
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
    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 1);

    CreateFile(contents2);
    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 1);
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

        EXPECT_EQ(m_spParser->GetSize(), 1);
        EXPECT_EQ(m_spParser->GetSize("section"), 2);
    }
}






TEST(JsonParserTest, Test)
{
    const fly::Json array = { '7', 8, "nine", 10 };

    const fly::Json map = {
        { "name1", "value1" },
        { "name2", 2 },
        { "name3", 3.14159 },
    };

    fly::Json value = {
        true,
        false,
        1,
        "string",
        nullptr,
        array,
        map,
        { 11, "twelve", 13.0, 14 },
        { { "name4", "value4" }, { "name5", 1.23456f } }
    };

    std::cout << value << std::endl;
    std::cout << std::endl;

    std::cout << value[0] << std::endl;
    std::cout << value[1] << std::endl;
    std::cout << value[2] << std::endl;
    std::cout << value[3] << std::endl;
    std::cout << value[4] << std::endl;
    std::cout << value[5] << std::endl;
    std::cout << value[6] << std::endl;
    std::cout << value[7] << std::endl;
    std::cout << value[8] << std::endl;
    std::cout << std::endl;

    EXPECT_THROW(array[4], fly::JsonException);
    EXPECT_THROW(array["a"], fly::JsonException);

    EXPECT_THROW(map[3], fly::JsonException);
    EXPECT_THROW(map["a"], fly::JsonException);

    auto spParser = std::make_shared<fly::JsonParser>("/tmp", "test.json");
    spParser->Parse();
}
