#include "fly/parser/exceptions.hpp"
#include "fly/parser/json_parser.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <vector>

namespace {

//==========================================================================
class EofParser : public fly::Parser
{
public:
    void CompareParsed(std::vector<int> chars) const noexcept
    {
        EXPECT_EQ(m_chars, chars);
    }

protected:
    /**
     * Dummy parser to fail if the given stream contains any characters.
     */
    fly::Json ParseInternal(std::istream &stream) override
    {
        m_chars.clear();
        int c = 0;

        while (stream)
        {
            if ((c = stream.get()) != EOF)
            {
                m_chars.push_back(c);
            }
        }

        if (!m_chars.empty())
        {
            throw fly::UnexpectedCharacterException(
                m_line,
                m_column,
                m_chars[0]);
        }

        return fly::Json();
    }

private:
    std::vector<int> m_chars;
};

} // namespace

//==============================================================================
class ParserTest : public ::testing::Test
{
public:
    ParserTest() noexcept : m_spParser(std::make_shared<EofParser>())
    {
    }

protected:
    std::shared_ptr<EofParser> m_spParser;
};

//==============================================================================
TEST_F(ParserTest, NonByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->ParseString("\xEE"), fly::ParserException);
    m_spParser->CompareParsed({0xEE});
}

//==============================================================================
TEST_F(ParserTest, Utf8ByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->ParseString("\xEF"), fly::ParserException);
    m_spParser->CompareParsed({0xEF});

    EXPECT_THROW(m_spParser->ParseString("\xEF\xEE"), fly::ParserException);
    m_spParser->CompareParsed({0xEF, 0xEE});

    EXPECT_THROW(m_spParser->ParseString("\xEF\xBB"), fly::ParserException);
    m_spParser->CompareParsed({0xEF, 0xBB});

    EXPECT_THROW(m_spParser->ParseString("\xEF\xBB\xEE"), fly::ParserException);
    m_spParser->CompareParsed({0xEF, 0xBB, 0xEE});

    EXPECT_NO_THROW(m_spParser->ParseString("\xEF\xBB\xBF"));
    m_spParser->CompareParsed({});
}

//==============================================================================
TEST_F(ParserTest, Utf16BigEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->ParseString("\xFE"), fly::ParserException);
    m_spParser->CompareParsed({0xFE});

    EXPECT_THROW(m_spParser->ParseString("\xFE\xEE"), fly::ParserException);
    m_spParser->CompareParsed({0xFE, 0xEE});

    EXPECT_NO_THROW(m_spParser->ParseString("\xFE\xFF"));
    m_spParser->CompareParsed({});
}

//==============================================================================
TEST_F(ParserTest, Utf16LittleEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->ParseString("\xFF"), fly::ParserException);
    m_spParser->CompareParsed({0xFF});

    EXPECT_THROW(m_spParser->ParseString("\xFF\xEE"), fly::ParserException);
    m_spParser->CompareParsed({0xFF, 0xEE});

    EXPECT_NO_THROW(m_spParser->ParseString("\xFF\xFE"));
    m_spParser->CompareParsed({});
}

//==============================================================================
TEST_F(ParserTest, Utf32BigEndianByteOrderMarkTest)
{
    EXPECT_THROW(
        m_spParser->ParseString(std::string("\x00", 1)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00});

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\x00\xEE", 2)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00, 0xEE});

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\x00\x00", 2)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00, 0x00});

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\x00\x00\xEE", 3)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00, 0x00, 0xEE});

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\x00\x00\xFE", 3)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00, 0x00, 0xFE});

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\x00\x00\xFE\xEE", 4)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00, 0x00, 0xFE, 0xEE});

    EXPECT_NO_THROW(
        m_spParser->ParseString(std::string("\x00\x00\xFE\xFF", 4)));
    m_spParser->CompareParsed({});
}

//==============================================================================
TEST_F(ParserTest, Utf32LittleEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->ParseString("\xFF\xFE\xEE"), fly::ParserException);
    m_spParser->CompareParsed({0xEE}); // 0xFF 0xFE is parsed as UTF-16

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\xFF\xFE\x00", 3)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00}); // 0xFF 0xFE is parsed as UTF-16

    EXPECT_THROW(
        m_spParser->ParseString(std::string("\xFF\xFE\x00\xEE", 4)),
        fly::ParserException);
    m_spParser->CompareParsed({0x00, 0xEE}); // 0xFF 0xFE is parsed as UTF-16

    EXPECT_NO_THROW(
        m_spParser->ParseString(std::string("\xFF\xFE\x00\x00", 4)));
    m_spParser->CompareParsed({});
}
