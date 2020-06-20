#include "fly/parser/ini_parser.hpp"

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
    auto parsed = m_parser.parse_file(std::filesystem::path("foo_abc") / "a.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, NonExistingFile)
{
    auto parsed = m_parser.parse_file(std::filesystem::temp_directory_path() / "a.json");
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, EmptyFile)
{
    const std::string contents;

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, EmptySection)
{
    const std::string contents("[section]");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 0);
}

//==================================================================================================
TEST_F(IniParserTest, NonEmptySection)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
    EXPECT_EQ(values.size(), 1);
    EXPECT_EQ(values["section"].size(), 2);
}

//==================================================================================================
TEST_F(IniParserTest, NonExistingSection)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
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

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
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

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
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

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
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

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
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

    auto parsed1 = m_parser.parse_string(contents1);
    ASSERT_TRUE(parsed1.has_value());

    fly::Json values1 = std::move(parsed1.value());
    EXPECT_EQ(values1.size(), 1);
    EXPECT_EQ(values1["section"].size(), 1);
    EXPECT_EQ(values1["section"]["name"], "Jane Doe");

    const std::string contents2(
        "[  \tsection]\n"
        "name=John Doe\n"
        "[section  ]\n"
        "name=Jane Doe\n");

    auto parsed2 = m_parser.parse_string(contents1);
    ASSERT_TRUE(parsed2.has_value());

    fly::Json values2 = std::move(parsed2.value());
    EXPECT_EQ(values2.size(), 1);
    EXPECT_EQ(values2["section"].size(), 1);
    EXPECT_EQ(values2["section"]["name"], "Jane Doe");
}

//==================================================================================================
TEST_F(IniParserTest, DuplicateValue)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "name=Jane Doe\n");

    auto parsed = m_parser.parse_string(contents);
    ASSERT_TRUE(parsed.has_value());

    fly::Json values = std::move(parsed.value());
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

    EXPECT_FALSE(m_parser.parse_string(contents1).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents2).has_value());
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

    EXPECT_FALSE(m_parser.parse_string(contents1).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents2).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents3).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents4).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents5).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents6).has_value());
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

    EXPECT_FALSE(m_parser.parse_string(contents1).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents2).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents3).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents4).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents5).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents6).has_value());
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

    auto parsed1 = m_parser.parse_string(contents1);
    ASSERT_TRUE(parsed1.has_value());

    fly::Json values1 = std::move(parsed1.value());
    EXPECT_EQ(values1.size(), 1);
    EXPECT_EQ(values1["section"].size(), 1);

    auto parsed2 = m_parser.parse_string(contents1);
    ASSERT_TRUE(parsed2.has_value());

    fly::Json values2 = std::move(parsed2.value());
    EXPECT_EQ(values2.size(), 1);
    EXPECT_EQ(values2["section"].size(), 1);
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

    EXPECT_FALSE(m_parser.parse_string(contents1).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents2).has_value());
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

    EXPECT_FALSE(m_parser.parse_string(contents1).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents2).has_value());
    EXPECT_FALSE(m_parser.parse_string(contents3).has_value());
}

//==================================================================================================
TEST_F(IniParserTest, MultipleParse)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA");

    for (int i = 0; i < 5; ++i)
    {
        auto parsed = m_parser.parse_string(contents);
        ASSERT_TRUE(parsed.has_value());

        fly::Json values = std::move(parsed.value());
        EXPECT_EQ(values.size(), 1);
        EXPECT_EQ(values["section"].size(), 2);
    }
}

//==================================================================================================
TEST_F(IniParserTest, BadValue)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=\xff");

    EXPECT_FALSE(m_parser.parse_string(contents).has_value());
}
