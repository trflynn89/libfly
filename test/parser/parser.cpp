#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include "fly/parser/exceptions.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

namespace
{
    FLY_CLASS_PTRS(EofParser);

    //==========================================================================
    class EofParser : public fly::Parser
    {
    public:
        void CompareParsed(std::vector<int> chars) const
        {
            EXPECT_EQ(m_chars, chars);
        }

    protected:
        /**
         * Dummy parser to fail if the given stream contains any characters.
         */
        virtual fly::Json ParseInternal(std::istream &stream)
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
}

//==============================================================================
class ParserTest : public ::testing::Test
{
public:
    ParserTest() : m_spParser(std::make_shared<EofParser>())
    {
    }

protected:
    EofParserPtr m_spParser;
};

//==============================================================================
TEST_F(ParserTest, NonByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->Parse("\xEE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xEE });
}

//==============================================================================
TEST_F(ParserTest, Utf8ByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->Parse("\xEF"), fly::ParserException);
    m_spParser->CompareParsed({ 0xEF });

    EXPECT_THROW(m_spParser->Parse("\xEF\xEE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xEF, 0xEE });

    EXPECT_THROW(m_spParser->Parse("\xEF\xBB"), fly::ParserException);
    m_spParser->CompareParsed({ 0xEF, 0xBB });

    EXPECT_THROW(m_spParser->Parse("\xEF\xBB\xEE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xEF, 0xBB, 0xEE });

    EXPECT_NO_THROW(m_spParser->Parse("\xEF\xBB\xBF"));
    m_spParser->CompareParsed({ });
}

//==============================================================================
TEST_F(ParserTest, Utf16BigEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->Parse("\xFE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xFE });

    EXPECT_THROW(m_spParser->Parse("\xFE\xEE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xFE, 0xEE });

    EXPECT_NO_THROW(m_spParser->Parse("\xFE\xFF"));
    m_spParser->CompareParsed({ });
}

//==============================================================================
TEST_F(ParserTest, Utf16LittleEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->Parse("\xFF"), fly::ParserException);
    m_spParser->CompareParsed({ 0xFF });

    EXPECT_THROW(m_spParser->Parse("\xFF\xEE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xFF, 0xEE });

    EXPECT_NO_THROW(m_spParser->Parse("\xFF\xFE"));
    m_spParser->CompareParsed({ });
}

//==============================================================================
TEST_F(ParserTest, Utf32BigEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->Parse(std::string("\x00", 1)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00 });

    EXPECT_THROW(m_spParser->Parse(std::string("\x00\xEE", 2)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00, 0xEE });

    EXPECT_THROW(m_spParser->Parse(std::string("\x00\x00", 2)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00, 0x00 });

    EXPECT_THROW(m_spParser->Parse(std::string("\x00\x00\xEE", 3)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00, 0x00, 0xEE });

    EXPECT_THROW(m_spParser->Parse(std::string("\x00\x00\xFE", 3)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00, 0x00, 0xFE });

    EXPECT_THROW(m_spParser->Parse(std::string("\x00\x00\xFE\xEE", 4)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00, 0x00, 0xFE, 0xEE });

    EXPECT_NO_THROW(m_spParser->Parse(std::string("\x00\x00\xFE\xFF", 4)));
    m_spParser->CompareParsed({ });
}

//==============================================================================
TEST_F(ParserTest, Utf32LittleEndianByteOrderMarkTest)
{
    EXPECT_THROW(m_spParser->Parse("\xFF\xFE\xEE"), fly::ParserException);
    m_spParser->CompareParsed({ 0xEE }); // The 0xFF 0xFE is parsed as UTF-16

    EXPECT_THROW(m_spParser->Parse(std::string("\xFF\xFE\x00", 3)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00 }); // The 0xFF 0xFE is parsed as UTF-16

    EXPECT_THROW(m_spParser->Parse(std::string("\xFF\xFE\x00\xEE", 4)), fly::ParserException);
    m_spParser->CompareParsed({ 0x00, 0xEE }); // The 0xFF 0xFE is parsed as UTF-16

    EXPECT_NO_THROW(m_spParser->Parse(std::string("\xFF\xFE\x00\x00", 4)));
    m_spParser->CompareParsed({ });
}
