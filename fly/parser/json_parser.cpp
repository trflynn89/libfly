#include "fly/parser/json_parser.h"

#include <cctype>
#include <cstring>
#include <fstream>

#include "fly/path/path.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

namespace fly {

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
    char c;

    try
    {
        while (stream.get(c))
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
                    onEscapedCharacter(c);
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
                onCharacter(c);
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
void JsonParser::onStartBraceOrBracket(const char &c, const JsonToken &token)
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
            m_parsing << c;
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
void JsonParser::onCloseBraceOrBracket(const char &c, const JsonToken &token)
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
            m_parsing << c;
            return;
        }

        break;

    default:
        break;
    }

    if (!storeValue() && m_expectingValue)
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
void JsonParser::onQuotation(const char &c)
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
void JsonParser::onColon(const char &c)
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
        m_parsing << c;
        break;
    }
}

//==============================================================================
void JsonParser::onComma(const char &c)
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
            storeValue();
            break;

        default:
            m_states.pop();
            break;
        }

        break;

    case JSON_PARSING_NAME:
        m_parsing << c;
        break;

    case JSON_PARSING_VALUE:
        if (m_parsingString)
        {
            m_parsing << c;
        }
        else if (storeValue())
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
void JsonParser::onCharacter(const char &c)
{
    switch (m_states.top())
    {
    case JSON_PARSING_ARRAY:
        if (!std::isspace(c))
        {
            m_states.push(JSON_PARSING_VALUE);

            m_pValue = &((*m_pValue)[m_pValue->Size()]);
            m_pParents.push(m_pValue);

            m_parsing << c;
        }

        break;

    case JSON_PARSING_VALUE:
    case JSON_PARSING_NAME:
        if (m_parsingString || !std::isspace(c))
        {
            m_parsing << c;
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
void JsonParser::onEscapedCharacter(const char &c)
{
    switch (c)
    {
    case JSON_QUOTE:
    case JSON_REVERSE_SOLIDUS:
    case JSON_SOLIDUS:
        m_parsing << c;
        break;

    case JSON_B:
        m_parsing << '\b';
        break;

    case JSON_F:
        m_parsing << '\f';
        break;

    case JSON_N:
        m_parsing << '\n';
        break;

    case JSON_R:
        m_parsing << '\r';
        break;

    case JSON_T:
        m_parsing << '\t';
        break;

    case JSON_U:
        // TODO: Hexadecimal
        break;

    default:
        throw ParserException(m_file, m_line, String::Format(
            "Unescapable character '%c'", c
        ));
    }
}

//==============================================================================
bool JsonParser::storeValue()
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

}
