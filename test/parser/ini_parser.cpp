#include "fly/parser/ini_parser.hpp"

#include "fly/parser/parser_exception.hpp"

#include <gtest/gtest.h>

#include <filesystem>

//==================================================================================================
class IniParserTest : public ::testing::Test
{
protected:
    fly::IniParser m_parser;
};

//==================================================================================================
TEST_F(IniParserTest, NonExistingPath)
{
    fly::Json values;

    ASSERT_NO_THROW(values = m_parser.parse_file(std::filesystem::path("foo_abc") / "a.json"));
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, NonExistingFile)
{
    fly::Json values;

    ASSERT_NO_THROW(
        values = m_parser.parse_file(std::filesystem::temp_directory_path() / "a.json"));
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, EmptyFile)
{
    const std::string contents;

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, EmptySection)
{
    const std::string contents("[section]");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, NonEmptySection)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));

    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 2);
}

//==================================================================================================
TEST_F(IniParserTest, NonExisting)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));

    EXPECT_EQ(values["section"].size(), 2);
    EXPECT_EQ(values["bad-section"].size(), 0);
    EXPECT_EQ(values["section-bad"].size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, Comment)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "; [other-section]\n"
        "; name=Jane Doe\n");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));

    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 1);
    EXPECT_EQ(values["other-section"].size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, ErrantSpaces)
{
    const std::string contents(
        "   [section   ]  \n"
        "\t\t\n   name=John Doe\t  \n"
        "\taddress  = USA\t \r \n");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));

    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 2);
}

//==================================================================================================
TEST_F(IniParserTest, QuotedValue)
{
    const std::string contents(
        "[section]\n"
        "name=\"  John Doe  \"\n"
        "address= \t '\\tUSA'");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));

    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 2);
}

//==================================================================================================
TEST_F(IniParserTest, MutlipleSectionType)
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
        "noage=1\n");

    fly::Json values;
    ASSERT_NO_THROW(values = m_parser.parse_string(contents));

    EXPECT_EQ(values.size(), 3);
    EXPECT_EQ(values["section1"].size(), 2);
    EXPECT_EQ(values["section2"].size(), 2);
    EXPECT_EQ(values["section3"].size(), 2);
}

//==================================================================================================
TEST_F(IniParserTest, DuplicateSection)
{
    const std::string contents1(
        "[section]\n"
        "name=John Doe\n"
        "[section]\n"
        "name=Jane Doe\n");

    fly::Json values;
    EXPECT_NO_THROW(values = m_parser.parse_string(contents1));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 1);
    EXPECT_EQ(values["section"]["name"], "Jane Doe");

    const std::string contents2(
        "[  \tsection]\n"
        "name=John Doe\n"
        "[section  ]\n"
        "name=Jane Doe\n");

    EXPECT_NO_THROW(values = m_parser.parse_string(contents2));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 1);
    EXPECT_EQ(values["section"]["name"], "Jane Doe");
}

//==================================================================================================
TEST_F(IniParserTest, DuplicateValue)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "name=Jane Doe\n");

    fly::Json values;
    EXPECT_NO_THROW(values = m_parser.parse_string(contents));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 1);
    EXPECT_EQ(values["section"]["name"], "Jane Doe");
}

//==================================================================================================
TEST_F(IniParserTest, ImbalancedBrace)
{
    const std::string contents1(
        "[section\n"
        "name=John Doe\n");

    const std::string contents2(
        "section]\n"
        "name=John Doe\n");

    EXPECT_THROW(m_parser.parse_string(contents1), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents2), fly::ParserException);
}

//==================================================================================================
TEST_F(IniParserTest, ImbalancedQuote)
{
    const std::string contents1(
        "[section]\n"
        "name=\"John Doe\n");

    const std::string contents2(
        "[section]\n"
        "name=John Doe\"\n");
    const std::string contents3(
        "[section]\n"
        "name='John Doe\n");

    const std::string contents4(
        "[section]\n"
        "name=John Doe'\n");

    const std::string contents5(
        "[section]\n"
        "name=\"John Doe'\n");

    const std::string contents6(
        "[section]\n"
        "name='John Doe\"\n");

    EXPECT_THROW(m_parser.parse_string(contents1), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents2), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents3), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents4), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents5), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents6), fly::ParserException);
}

//==================================================================================================
TEST_F(IniParserTest, MisplacedQuote)
{
    const std::string contents1(
        "[section]\n"
        "\"name\"=John Doe\n");

    const std::string contents2(
        "[section]\n"
        "\'name\'=John Doe\n");

    const std::string contents3(
        "[\"section\"]\n"
        "name=John Doe\n");

    const std::string contents4(
        "[\'section\']\n"
        "name=John Doe\n");

    const std::string contents5(
        "\"[section]\"\n"
        "name=John Doe\n");

    const std::string contents6(
        "\'[section]\'\n"
        "name=John Doe\n");

    EXPECT_THROW(m_parser.parse_string(contents1), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents2), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents3), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents4), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents5), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents6), fly::ParserException);
}

//==================================================================================================
TEST_F(IniParserTest, MultipleAssignment)
{
    const std::string contents1(
        "[section]\n"
        "name=John=Doe\n");
    const std::string contents2(
        "[section]\n"
        "name=\"John=Doe\"\n");

    fly::Json values;

    ASSERT_NO_THROW(values = m_parser.parse_string(contents1));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 1);

    ASSERT_NO_THROW(values = m_parser.parse_string(contents2));
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 1);
}

//==================================================================================================
TEST_F(IniParserTest, MissingAssignment)
{
    const std::string contents1(
        "[section]\n"
        "name\n");

    const std::string contents2(
        "[section]\n"
        "name=\n");

    EXPECT_THROW(m_parser.parse_string(contents1), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents2), fly::ParserException);
}

//==================================================================================================
TEST_F(IniParserTest, EarlyAssignment)
{
    const std::string contents1(
        "name=John Doe\n"
        "[section]\n");

    const std::string contents2(
        "name=\n"
        "[section]\n");

    const std::string contents3(
        "name\n"
        "[section]\n");

    EXPECT_THROW(m_parser.parse_string(contents1), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents2), fly::ParserException);
    EXPECT_THROW(m_parser.parse_string(contents3), fly::ParserException);
}

//==================================================================================================
TEST_F(IniParserTest, MultipleParse)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA");

    fly::Json values;

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_NO_THROW(values = m_parser.parse_string(contents));

        EXPECT_EQ(values.size(), 1);
        EXPECT_EQ(values["section"].size(), 2);
    }
}
