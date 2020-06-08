#include "fly/parser/parser.hpp"

#include "fly/parser/parser_exception.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <vector>

namespace {

//==================================================================================================
class EofParser : public fly::Parser
{
public:
    void compare_parsed(std::vector<int> chars) const noexcept
    {
        EXPECT_EQ(m_chars, chars);
    }

protected:
    /**
     * Dummy parser to fail if the given stream contains any characters.
     */
    fly::Json parse_internal(std::istream &stream) override
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
            throw fly::UnexpectedCharacterException(m_line, m_column, m_chars[0]);
        }

        return fly::Json();
    }

private:
    std::vector<int> m_chars;
};

} // namespace

//==================================================================================================
class ParserTest : public ::testing::Test
{
protected:
    EofParser m_parser;
};

//==================================================================================================
TEST_F(ParserTest, NonByteOrderMark)
{
    EXPECT_THROW(m_parser.parse_string("\xEE"), fly::ParserException);
    m_parser.compare_parsed({0xEE});
}

//==================================================================================================
TEST_F(ParserTest, Utf8ByteOrderMark)
{
    EXPECT_THROW(m_parser.parse_string("\xEF"), fly::ParserException);
    m_parser.compare_parsed({0xEF});

    EXPECT_THROW(m_parser.parse_string("\xEF\xEE"), fly::ParserException);
    m_parser.compare_parsed({0xEF, 0xEE});

    EXPECT_THROW(m_parser.parse_string("\xEF\xBB"), fly::ParserException);
    m_parser.compare_parsed({0xEF, 0xBB});

    EXPECT_THROW(m_parser.parse_string("\xEF\xBB\xEE"), fly::ParserException);
    m_parser.compare_parsed({0xEF, 0xBB, 0xEE});

    EXPECT_NO_THROW(m_parser.parse_string("\xEF\xBB\xBF"));
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf16BigEndianByteOrderMark)
{
    EXPECT_THROW(m_parser.parse_string("\xFE"), fly::ParserException);
    m_parser.compare_parsed({0xFE});

    EXPECT_THROW(m_parser.parse_string("\xFE\xEE"), fly::ParserException);
    m_parser.compare_parsed({0xFE, 0xEE});

    EXPECT_NO_THROW(m_parser.parse_string("\xFE\xFF"));
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf16LittleEndianByteOrderMark)
{
    EXPECT_THROW(m_parser.parse_string("\xFF"), fly::ParserException);
    m_parser.compare_parsed({0xFF});

    EXPECT_THROW(m_parser.parse_string("\xFF\xEE"), fly::ParserException);
    m_parser.compare_parsed({0xFF, 0xEE});

    EXPECT_NO_THROW(m_parser.parse_string("\xFF\xFE"));
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf32BigEndianByteOrderMark)
{
    EXPECT_THROW(m_parser.parse_string(std::string("\x00", 1)), fly::ParserException);
    m_parser.compare_parsed({0x00});

    EXPECT_THROW(m_parser.parse_string(std::string("\x00\xEE", 2)), fly::ParserException);
    m_parser.compare_parsed({0x00, 0xEE});

    EXPECT_THROW(m_parser.parse_string(std::string("\x00\x00", 2)), fly::ParserException);
    m_parser.compare_parsed({0x00, 0x00});

    EXPECT_THROW(m_parser.parse_string(std::string("\x00\x00\xEE", 3)), fly::ParserException);
    m_parser.compare_parsed({0x00, 0x00, 0xEE});

    EXPECT_THROW(m_parser.parse_string(std::string("\x00\x00\xFE", 3)), fly::ParserException);
    m_parser.compare_parsed({0x00, 0x00, 0xFE});

    EXPECT_THROW(m_parser.parse_string(std::string("\x00\x00\xFE\xEE", 4)), fly::ParserException);
    m_parser.compare_parsed({0x00, 0x00, 0xFE, 0xEE});

    EXPECT_NO_THROW(m_parser.parse_string(std::string("\x00\x00\xFE\xFF", 4)));
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf32LittleEndianByteOrderMark)
{
    EXPECT_THROW(m_parser.parse_string("\xFF\xFE\xEE"), fly::ParserException);
    m_parser.compare_parsed({0xEE}); // 0xFF 0xFE is parsed as UTF-16

    EXPECT_THROW(m_parser.parse_string(std::string("\xFF\xFE\x00", 3)), fly::ParserException);
    m_parser.compare_parsed({0x00}); // 0xFF 0xFE is parsed as UTF-16

    EXPECT_THROW(m_parser.parse_string(std::string("\xFF\xFE\x00\xEE", 4)), fly::ParserException);
    m_parser.compare_parsed({0x00, 0xEE}); // 0xFF 0xFE is parsed as UTF-16

    EXPECT_NO_THROW(m_parser.parse_string(std::string("\xFF\xFE\x00\x00", 4)));
    m_parser.compare_parsed({});
}
