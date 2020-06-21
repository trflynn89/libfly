#include "fly/parser/parser.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <sstream>
#include <vector>

namespace {

//==================================================================================================
class EofParser : public fly::Parser
{
public:
    void compare_parsed(std::vector<int> chars) const
    {
        EXPECT_EQ(m_chars, chars);
    }

protected:
    /**
     * Dummy parser to fail if the given stream contains any characters.
     */
    std::optional<fly::Json> parse_internal(std::istream &stream) override
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
            return std::nullopt;
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
    EXPECT_FALSE(m_parser.parse_string("\xEE").has_value());
    m_parser.compare_parsed({0xEE});
}

//==================================================================================================
TEST_F(ParserTest, Utf8ByteOrderMark)
{
    EXPECT_FALSE(m_parser.parse_string("\xEF").has_value());
    m_parser.compare_parsed({0xEF});

    EXPECT_FALSE(m_parser.parse_string("\xEF\xEE").has_value());
    m_parser.compare_parsed({0xEF, 0xEE});

    EXPECT_FALSE(m_parser.parse_string("\xEF\xBB").has_value());
    m_parser.compare_parsed({0xEF, 0xBB});

    EXPECT_FALSE(m_parser.parse_string("\xEF\xBB\xEE").has_value());
    m_parser.compare_parsed({0xEF, 0xBB, 0xEE});

    EXPECT_TRUE(m_parser.parse_string("\xEF\xBB\xBF").has_value());
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf16BigEndianByteOrderMark)
{
    std::optional<fly::Json> parsed;

    EXPECT_FALSE(m_parser.parse_string("\xFE").has_value());
    m_parser.compare_parsed({0xFE});

    EXPECT_FALSE(m_parser.parse_string("\xFE\xEE").has_value());
    m_parser.compare_parsed({0xFE, 0xEE});

    EXPECT_TRUE(m_parser.parse_string("\xFE\xFF").has_value());
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf16LittleEndianByteOrderMark)
{
    std::optional<fly::Json> parsed;

    EXPECT_FALSE(m_parser.parse_string("\xFF").has_value());
    m_parser.compare_parsed({0xFF});

    EXPECT_FALSE(m_parser.parse_string("\xFF\xEE").has_value());
    m_parser.compare_parsed({0xFF, 0xEE});

    EXPECT_TRUE(m_parser.parse_string("\xFF\xFE").has_value());
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf32BigEndianByteOrderMark)
{
    std::optional<fly::Json> parsed;

    EXPECT_FALSE(m_parser.parse_string(std::string("\x00", 1)).has_value());
    m_parser.compare_parsed({0x00});

    EXPECT_FALSE(m_parser.parse_string(std::string("\x00\xEE", 2)).has_value());
    m_parser.compare_parsed({0x00, 0xEE});

    EXPECT_FALSE(m_parser.parse_string(std::string("\x00\x00", 2)).has_value());
    m_parser.compare_parsed({0x00, 0x00});

    EXPECT_FALSE(m_parser.parse_string(std::string("\x00\x00\xEE", 3)).has_value());
    m_parser.compare_parsed({0x00, 0x00, 0xEE});

    EXPECT_FALSE(m_parser.parse_string(std::string("\x00\x00\xFE", 3)).has_value());
    m_parser.compare_parsed({0x00, 0x00, 0xFE});

    EXPECT_FALSE(m_parser.parse_string(std::string("\x00\x00\xFE\xEE", 4)).has_value());
    m_parser.compare_parsed({0x00, 0x00, 0xFE, 0xEE});

    EXPECT_TRUE(m_parser.parse_string(std::string("\x00\x00\xFE\xFF", 4)).has_value());
    m_parser.compare_parsed({});
}

//==================================================================================================
TEST_F(ParserTest, Utf32LittleEndianByteOrderMark)
{
    std::optional<fly::Json> parsed;

    EXPECT_FALSE(m_parser.parse_string(std::string("\xFF\xFE\x61\x00", 4)).has_value());
    m_parser.compare_parsed({0x61}); // 0xff 0xfe is interpreted as UTF-16 little endian

    EXPECT_FALSE(m_parser.parse_string(std::string("\xFF\xFE\x00\x61", 4)).has_value());
    m_parser.compare_parsed({0xE6, 0x84, 0x80}); // 0xff 0xfe is interpreted as UTF-16 little endian

    EXPECT_TRUE(m_parser.parse_string(std::string("\xFF\xFE\x00\x00", 4)).has_value());
    m_parser.compare_parsed({});
}
