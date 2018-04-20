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
    m_expectingValue = false;
    m_line = 1;
    m_column = 0;

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
            ++m_column;

            if (m_parsingString)
            {
                if (std::isspace(c) && (token != JSON_SPACE))
                {
                    throw ParserException(m_file, m_line, m_column, String::Format(
                        "Unexpected character '%x'", token
                    ));
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
                m_column = 0;
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
        throw ParserException(m_file, m_line, m_column, ex.what());
    }

    if (m_states.top() != JSON_NO_STATE)
    {
        throw ParserException(m_file, m_line, m_column,
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
        ));

    case JSON_PARSING_ARRAY:
        m_states.push(JSON_PARSING_VALUE);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        break;

    case JSON_PARSING_COLON:
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Expected name or value before character '%c' (%x)", char(c), c
        ));
    }
    else if (m_pParents.empty())
    {
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
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
            throw ParserException(m_file, m_line, m_column, String::Format(
                "Unexpected character '%c' (%x)", char(c), c
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Unexpected character '%c' (%x)", char(c), c
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
            throw ParserException(m_file, m_line, m_column, String::Format(
                "Expected name or value before character '%c' (%x)", char(c), c
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
        if (m_parsingString)
        {
            pushValue(c);

            // Blindly ignore the escaped character, the Json class will check
            // whether it is valid
            if (c == JSON_REVERSE_SOLIDUS)
            {
                if ((c = stream.get()) == EOF)
                {
                    throw ParserException(m_file, m_line, m_column, String::Format(
                        "Expected escaped character after '%c' (%x)", char(c), c
                    ));
                }

                pushValue(c);
            }
        }
        else if (!std::isspace(c))
        {
            pushValue(c);
        }

        break;

    default:
        if (!std::isspace(c))
        {
            throw ParserException(m_file, m_line, m_column, String::Format(
                "Unexpected character '%c' (%x)", char(c), c
            ));
        }
    }
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
        throw ParserException(m_file, m_line, m_column, String::Format(
            "Octal value '%s' not allowed", value
        ));
    }

    // Parsed a number - validate no postive sign given
    else if ((value.length() > 1) && (value[0] == '+'))
    {
        throw ParserException(m_file, m_line, m_column, String::Format(
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
            throw ParserException(m_file, m_line, m_column, String::Format(
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

}
