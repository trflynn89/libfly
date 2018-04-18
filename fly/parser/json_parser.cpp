#include "fly/parser/json_parser.h"

#include <cctype>
#include <cstring>

#include "fly/path/path.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

namespace fly {

#define IS_HIGH_SURROGATE(c) ((c >= 0xD800) && (c <= 0xDBFF))
#define IS_LOW_SURROGATE(c) ((c >= 0xDC00) && (c <= 0xDFFF))
#define IS_UNICODE(c) ((c >= 0x0) && (c <= 0x10FFFF))

//==============================================================================
JsonParser::JsonParser(const std::string &path, const std::string &file) :
    Parser(path, file),
    m_states(),
    m_pValue(),
    m_pParents(),
    m_parsingString(false),
    m_parsedString(false),
    m_expectingEscape(false),
    m_expectingValue(false)
{
}

//==============================================================================
void JsonParser::Parse()
{
    std::unique_lock<std::shared_timed_mutex> lock(m_valuesMutex);

    std::string fullPath = Path::Join(m_path, m_file);
    std::ifstream stream(fullPath.c_str(), std::ios::in);

    m_parsing.str(std::string());
    m_parsingString = false;
    m_parsedString = false;
    m_expectingEscape = false;
    m_expectingValue = false;
    m_line = 1;

    m_pValue = &m_values;
    *m_pValue = nullptr;

    m_pParents = decltype(m_pParents)();
    m_pParents.push(m_pValue);

    m_states = decltype(m_states)();
    m_states.push(JSON_NO_STATE);

    JsonToken token;
    int c;

    try
    {
        while ((c = stream.get()) != EOF)
        {
            token = static_cast<JsonToken>(c);

            if (m_parsingString)
            {
                if (std::isspace(c) && (token != JSON_SPACE))
                {
                    throw ParserException(m_file, m_line, String::Format(
                        "Unexpected character '%x'", token
                    ));
                }
                else if (m_expectingEscape)
                {
                    m_expectingEscape = false;
                    onEscapedCharacter(c, stream);
                    continue;
                }
                else if (token == JSON_REVERSE_SOLIDUS)
                {
                    m_expectingEscape = true;
                    continue;
                }
            }

            switch (token)
            {
            case JSON_START_BRACE:
            case JSON_START_BRACKET:
                onStartBraceOrBracket(c, token);
                break;

            case JSON_CLOSE_BRACE:
            case JSON_CLOSE_BRACKET:
                onCloseBraceOrBracket(c, token);
                break;

            case JSON_QUOTE:
                onQuotation(c);
                break;

            case JSON_NEW_LINE:
                ++m_line;
                break;

            case JSON_COMMA:
                onComma(c);
                break;

            case JSON_COLON:
                onColon(c);
                break;

            default:
                onCharacter(c, stream);
            }
        }
    }
    catch (const ParserException &ex)
    {
        throw ex;
    }
    catch (const std::exception &ex)
    {
        throw ParserException(m_file, m_line, ex.what());
    }

    if (m_states.top() != JSON_NO_STATE)
    {
        throw ParserException(m_file, m_line,
            "Finished parsing file with incomplete JSON object"
        );
    }
}

//==============================================================================
void JsonParser::onStartBraceOrBracket(int c, const JsonToken &token)
{
    switch (m_states.top())
    {
    case JSON_PARSING_OBJECT:
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));

    case JSON_PARSING_ARRAY:
        m_states.push(JSON_PARSING_VALUE);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        break;

    case JSON_PARSING_COLON:
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));

    case JSON_PARSING_NAME:
    case JSON_PARSING_VALUE:
        if (m_parsingString)
        {
            pushValue(c);
            return;
        }

        break;

    default:
        break;
    }

    if (token == JSON_START_BRACE)
    {
        *m_pValue = Json::object_type();
        m_states.push(JSON_PARSING_OBJECT);
    }
    else
    {
        *m_pValue = Json::array_type();
        m_states.push(JSON_PARSING_ARRAY);
    }

    m_expectingValue = false;
}

//==============================================================================
void JsonParser::onCloseBraceOrBracket(int c, const JsonToken &token)
{
    switch (m_states.top())
    {
    case JSON_NO_STATE:
    case JSON_PARSING_COLON:
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));

    case JSON_PARSING_NAME:
    case JSON_PARSING_VALUE:
        if (m_parsingString)
        {
            pushValue(c);
            return;
        }

        break;

    default:
        break;
    }

    if (!popValue() && m_expectingValue)
    {
        throw ParserException(m_file, m_line, String::Format(
            "Expected name or value before character '%c'", c
        ));
    }
    else if (m_pParents.empty())
    {
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));
    }

    m_pParents.pop();
    m_pValue = (m_pParents.empty() ? nullptr : m_pParents.top());

    const JsonState expected = (
        (token == JSON_CLOSE_BRACE) ? JSON_PARSING_OBJECT : JSON_PARSING_ARRAY
    );

    const JsonState unexpected = (
        (token == JSON_CLOSE_BRACE) ? JSON_PARSING_ARRAY : JSON_PARSING_OBJECT
    );

    while (m_states.top() != expected)
    {
        if (m_states.top() == unexpected)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Unexpected character '%c'", c
            ));
        }

        m_states.pop();
    }

    m_states.pop();

    if (m_states.top() != JSON_NO_STATE)
    {
        m_states.push(JSON_PARSING_COMMA);
    }
}

//==============================================================================
void JsonParser::onQuotation(int c)
{
    switch (m_states.top())
    {
    case JSON_PARSING_OBJECT:
        m_states.push(JSON_PARSING_NAME);
        break;

    case JSON_PARSING_ARRAY:
        m_states.push(JSON_PARSING_VALUE);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        break;

    case JSON_PARSING_NAME:
        m_states.pop();
        m_states.push(JSON_PARSING_COLON);

        m_pValue = &((*m_pValue)[m_parsing.str()]);
        m_pParents.push(m_pValue);

        m_parsing.str(std::string());

        m_expectingValue = false;

        break;

    case JSON_PARSING_VALUE:
        if (m_parsingString)
        {
            m_parsedString = true;

            m_states.pop();
            m_states.push(JSON_PARSING_COMMA);
        }

        break;

    default:
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));
    }

    m_parsingString = !m_parsingString;
}

//==============================================================================
void JsonParser::onColon(int c)
{
    switch (m_states.top())
    {
    case JSON_NO_STATE:
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));

    case JSON_PARSING_COLON:
        m_states.pop();
        m_states.push(JSON_PARSING_VALUE);

        m_expectingValue = true;

        break;

    default:
        pushValue(c);
        break;
    }
}

//==============================================================================
void JsonParser::onComma(int c)
{
    switch (m_states.top())
    {
    case JSON_NO_STATE:
    case JSON_PARSING_OBJECT:
    case JSON_PARSING_ARRAY:
        throw ParserException(m_file, m_line, String::Format(
            "Unexpected character '%c'", c
        ));

    case JSON_PARSING_COMMA:
        m_states.pop();

        switch (m_states.top())
        {
        case JSON_PARSING_OBJECT:
        case JSON_PARSING_ARRAY:
            popValue();
            break;

        default:
            m_states.pop();
            break;
        }

        break;

    case JSON_PARSING_NAME:
        pushValue(c);
        break;

    case JSON_PARSING_VALUE:
        if (m_parsingString)
        {
            pushValue(c);
        }
        else if (popValue())
        {
            m_states.pop();
        }
        else if (m_expectingValue)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected name or value before character '%c'", c
            ));
        }

        break;

    default:
        break;
    }

    m_expectingValue = !m_parsingString;
}

//==============================================================================
void JsonParser::onCharacter(int c, std::ifstream &stream)
{
    switch (m_states.top())
    {
    case JSON_PARSING_ARRAY:
        if (!std::isspace(c))
        {
            m_states.push(JSON_PARSING_VALUE);

            m_pValue = &((*m_pValue)[m_pValue->Size()]);
            m_pParents.push(m_pValue);

            pushValue(c);
        }

        break;

    case JSON_PARSING_VALUE:
    case JSON_PARSING_NAME:
        if (m_parsingString || !std::isspace(c))
        {
            validateCharacter(c, stream);
        }

        break;

    default:
        if (!std::isspace(c))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Unexpected character '%c'", c
            ));
        }
    }
}

//==============================================================================
void JsonParser::onEscapedCharacter(int c, std::ifstream &stream)
{
    switch (c)
    {
    case JSON_QUOTE:
    case JSON_REVERSE_SOLIDUS:
    case JSON_SOLIDUS:
        pushValue(c);
        break;

    case JSON_B:
        pushValue('\b');
        break;

    case JSON_F:
        pushValue('\f');
        break;

    case JSON_N:
        pushValue('\n');
        break;

    case JSON_R:
        pushValue('\r');
        break;

    case JSON_T:
        pushValue('\t');
        break;

    case JSON_U:
        readUnicodeCharacter(stream);
        break;

    default:
        throw ParserException(m_file, m_line, String::Format(
            "Unescapable character '%c'", c
        ));
    }
}

//==============================================================================
void JsonParser::readUnicodeCharacter(std::ifstream &stream)
{
    const int highSurrogateCodepoint = readUnicodeCodepoint(stream);
    int codepoint = highSurrogateCodepoint;

    if (IS_HIGH_SURROGATE(highSurrogateCodepoint))
    {
        int c = 0;

        if (((c = stream.get()) == EOF) || (JsonToken(c) != JSON_REVERSE_SOLIDUS))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected low surrogate to follow high surrogate %x but found %c",
                highSurrogateCodepoint, c
            ));
        }
        else if (((c = stream.get()) == EOF) || (JsonToken(c) != JSON_U))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected low surrogate to follow high surrogate %x but found %c",
                highSurrogateCodepoint, c
            ));
        }

        const int lowSurrogateCodepoint = readUnicodeCodepoint(stream);

        if (IS_LOW_SURROGATE(lowSurrogateCodepoint))
        {
            codepoint =
                // high surrogate occupies the most significant 22 bits
                (highSurrogateCodepoint << 10)
                // low surrogate occupies the least significant 15 bits
                + lowSurrogateCodepoint
                // there is still the 0xD800, 0xDC00 and 0x10000 noise
                // in the result so we have to subtract with:
                // (0xD800 << 10) + DC00 - 0x10000 = 0x35FDC00
                - 0x35FDC00;
        }
        else
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected low surrogate to follow high surrogate %x but found %x",
                highSurrogateCodepoint, lowSurrogateCodepoint
            ));
        }
    }
    else if (IS_LOW_SURROGATE(highSurrogateCodepoint))
    {
        throw ParserException(m_file, m_line, String::Format(
            "Expected high surrogate to preceed low surrogate %x",
            highSurrogateCodepoint
        ));
    }

    if (!IS_UNICODE(codepoint))
    {
        throw ParserException(m_file, m_line, String::Format(
            "Non-unicode character %x", codepoint
        ));
    }

    if (codepoint < 0x80)
    {
        pushValue(codepoint);
    }
    else if (codepoint <= 0x7FF)
    {
        pushValue(0xC0 | (codepoint >> 6));
        pushValue(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0xFFFF)
    {
        pushValue(0xE0 | (codepoint >> 12));
        pushValue(0x80 | ((codepoint >> 6) & 0x3F));
        pushValue(0x80 | (codepoint & 0x3F));
    }
    else
    {
        pushValue(0xF0 | (codepoint >> 18));
        pushValue(0x80 | ((codepoint >> 12) & 0x3F));
        pushValue(0x80 | ((codepoint >> 6) & 0x3F));
        pushValue(0x80 | (codepoint & 0x3F));
    }
}

//==============================================================================
int JsonParser::readUnicodeCodepoint(std::ifstream &stream) const
{
    int codepoint = 0;
    int c = 0;

    for (int i = 0; i < 4; ++i)
    {
        const int shift = (4 * (3 - i));

        if ((c = stream.get()) == EOF)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected exactly 4 hexadecimals after \\u"
            ));
        }

        if ((c >= '0') && (c <= '9'))
        {
            codepoint += ((c - 0x30) << shift);
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            codepoint += ((c - 0x37) << shift);
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            codepoint += ((c - 0x57) << shift);
        }
        else
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected '%c' to be a hexadecimal", c
            ));
        }
    }

    return codepoint;
}

//==============================================================================
void JsonParser::pushValue(int c)
{
    m_parsing << char(c);
}

//==============================================================================
bool JsonParser::popValue()
{
    const std::string value = m_parsing.str();

    if (value.empty())
    {
        return false;
    }

    // Parsed a string value
    if (m_parsedString)
    {
        m_parsedString = false;
        *m_pValue = value;
    }

    // Parsed a boolean value
    else if (value == "true")
    {
        *m_pValue = true;
    }
    else if (value == "false")
    {
        *m_pValue = false;
    }

    // Parsed a null value
    else if (value == "null")
    {
        *m_pValue = nullptr;
    }

    // Parsed a number - validate non-octal
    else if (
        (value.length() > 1) && (value[0] == '0') &&
        (
            ((value.find('.') == std::string::npos)) ||
            ((value.find('.') != 1))
        )
    )
    {
        throw ParserException(m_file, m_line, String::Format(
            "Octal value '%s' not allowed", value
        ));
    }

    // Parsed a number - validate no postive sign given
    else if ((value.length() > 1) && (value[0] == '+'))
    {
        throw ParserException(m_file, m_line, String::Format(
            "Specifying +integer '%s' not allowed", value
        ));
    }

    // Parsed a float
    else if (
        (value.find('.') != std::string::npos) ||
        (value.find('e') != std::string::npos) ||
        (value.find('E') != std::string::npos)
    )
    {
        if ((value[0] == '.') || (value[0] == 'e') || (value[0] == 'E'))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected starting digit for '%s'", value
            ));
        }

        *m_pValue = String::Convert<Json::float_type>(value);
    }

    // Parsed a signed integer
    else if (value[0] == '-')
    {
        *m_pValue = String::Convert<Json::signed_type>(value);
    }

    // Parsed an unsigned integer
    else
    {
        *m_pValue = String::Convert<Json::unsigned_type>(value);
    }

    m_pParents.pop();
    m_pValue = m_pParents.top();

    m_parsing.str(std::string());

    m_expectingValue = false;

    return true;
}

//==============================================================================
void JsonParser::validateCharacter(int c, std::ifstream &stream)
{
    // Invalid control characters
    if ((c >= 0x00) && (c <= 0x1F))
    {
        throw ParserException(m_file, m_line, String::Format(
            "Invalid control character '%x'", int(c)
        ));
    }

    // Quote or reverse solidus
    else if ((c == 0x22) && (c == 0x5C))
    {
        throw ParserException(m_file, m_line, String::Format(
            "Invalid unescaped character '%x'", int(c)
        ));
    }

    // Valid ASCII character
    else if ((c >= 0x20) && (c <= 0x7F))
    {
        pushValue(c);
    }

    // U+0080..U+07FF: bytes C2..DF 80..BF
    else if ((c >= 0xC2) && (c <= 0xDF))
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // U+0800..U+0FFF: bytes E0 A0..BF 80..BF
    else if (c == 0xE0)
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0xA0) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // U+1000..U+CFFF: bytes E1..EC 80..BF 80..BF
    // U+E000..U+FFFF: bytes EE..EF 80..BF 80..BF
    else if (((c >= 0xE1) && (c <= 0xEC)) || (c == 0xEE) || (c == 0xEF))
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // U+D000..U+D7FF: bytes ED 80..9F 80..BF
    else if (c == 0xED)
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0x9F))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // U+10000..U+3FFFF F0 90..BF 80..BF 80..BF
    else if (c == 0xF0)
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x90) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // U+40000..U+FFFFF F1..F3 80..BF 80..BF 80..BF
    else if ((c >= 0xF1) && (c <= 0xF3))
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // U+100000..U+10FFFF F4 80..8F 80..BF 80..BF
    else if (c == 0xF4)
    {
        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0x8F))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);

        if (((c = stream.get()) == EOF) || (c < 0x80) || (c > 0xBF))
        {
            throw ParserException(m_file, m_line, String::Format(
                "Invalid control character '%x'", int(c)
            ));
        }

        pushValue(c);
    }

    // remaining bytes (80..C1 and F5..FF) are ill-formed
    else
    {
        throw ParserException(m_file, m_line, String::Format(
            "Invalid control character '%x'", int(c)
        ));
    }
}

}
