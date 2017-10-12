#include "fly/parser/json_parser.h"

#include <cctype>
#include <cstring>
#include <fstream>

#include "fly/path/path.h"
#include "fly/string/string.h"

namespace fly {

//==============================================================================
JsonParser::JsonParser(const std::string &path, const std::string &file) :
    Parser(path, file),
    m_states(),
    m_root(),
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
    std::unique_lock<std::shared_timed_mutex> lock(m_sectionsMutex);

    std::string fullPath = Path::Join(m_path, m_file);
    std::ifstream stream(fullPath.c_str(), std::ios::in);

    m_parsing.str(std::string());
    m_parsingString = false;
    m_parsedString = false;
    m_expectingValue = false;
    m_line = 1;

    m_pValue = &m_root;
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

            switch (token)
            {
            case JSON_START_BRACE:
                onStartBrace();
                break;

            case JSON_CLOSE_BRACE:
                onCloseBrace(c);
                break;

            case JSON_START_BRACKET:
                onStartBracket();
                break;

            case JSON_CLOSE_BRACKET:
                onCloseBracket(c);
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
    catch (const JsonException &ex)
    {
        throw ParserException(m_file, m_line, ex.what());
    }

    std::cout << "Finished Parsing:" << std::endl;
    std::cout << m_root << std::endl;
}

//==============================================================================
void JsonParser::onStartBrace()
{
    if (m_states.top() == JSON_PARSING_ARRAY)
    {
        m_states.push(JSON_PARSING_VALUE);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);
    }

    *m_pValue = Json::object_type();
    m_states.push(JSON_PARSING_OBJECT);

    m_expectingValue = false;
}

//==============================================================================
void JsonParser::onCloseBrace(const char &c)
{
    if (!storeValue() && m_expectingValue)
    {
        throw ParserException(m_file, m_line, String::Format(
            "Expected name or value before character '%c'", c
        ));
    }

    m_pParents.pop();
    m_pValue = (m_pParents.empty() ? nullptr : m_pParents.top());

    while (m_states.top() != JSON_PARSING_OBJECT)
    {
        m_states.pop();
    }

    m_states.pop();
    m_states.push(JSON_PARSING_COMMA);
}

//==============================================================================
void JsonParser::onStartBracket()
{
    if (m_states.top() == JSON_PARSING_ARRAY)
    {
        m_states.push(JSON_PARSING_VALUE);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);
    }

    *m_pValue = Json::array_type();
    m_states.push(JSON_PARSING_ARRAY);

    m_expectingValue = false;
}

//==============================================================================
void JsonParser::onCloseBracket(const char &c)
{
    if (!storeValue() && m_expectingValue)
    {
        throw ParserException(m_file, m_line, String::Format(
            "Expected name or value before character '%c'", c
        ));
    }

    m_pParents.pop();
    m_pValue = (m_pParents.empty() ? nullptr : m_pParents.top());

    while (m_states.top() != JSON_PARSING_ARRAY)
    {
        m_states.pop();
    }

    m_states.pop();
    m_states.push(JSON_PARSING_COMMA);
}

//==============================================================================
void JsonParser::onQuotation(const char &c)
{
    m_parsingString = !m_parsingString;

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
        if (!m_parsingString)
        {
            m_parsedString = true;

            m_states.pop();
            m_states.push(JSON_PARSING_COMMA);
        }

        break;

    default:
        throw ParserException(m_file, m_line, String::Format(
            "Expected comma before character '%c'", c
        ));
    }
}

//==============================================================================
void JsonParser::onColon(const char &c)
{
    switch (m_states.top())
    {
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
    case JSON_PARSING_COMMA:
        m_states.pop();

        switch (m_states.top())
        {
        case JSON_PARSING_OBJECT:
        case JSON_PARSING_ARRAY:
            storeValue();
            break;

        case JSON_PARSING_VALUE:
            m_states.pop();
            break;

        default:
            break;
        }

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

        break;

    default:
        if (m_expectingValue)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Expected name or value before character '%c'", c
            ));
        }

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

    // Parsed a float
    else if (value.find('.') != std::string::npos)
    {
        try
        {
            *m_pValue = String::Convert<Json::float_type>(value, true);
        }
        catch (...)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Could not convert '%s' to a float type", value
            ));
        }
    }

    // Parsed a signed integer
    else if ((value[0] == '-') || (value[0] == '+'))
    {
        try
        {
            *m_pValue = String::Convert<Json::signed_type>(value, true);
        }
        catch (...)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Could not convert '%s' to an signed type", value
            ));
        }
    }

    // Parsed an unsigned integer
    else
    {
        try
        {
            *m_pValue = String::Convert<Json::unsigned_type>(value, true);
        }
        catch (...)
        {
            throw ParserException(m_file, m_line, String::Format(
                "Could not convert '%s' to an unsigned type", value
            ));
        }
    }

    m_pParents.pop();
    m_pValue = m_pParents.top();

    m_parsing.str(std::string());

    m_expectingValue = false;

    return true;
}

//==============================================================================
Parser::ValueList JsonParser::GetValues(const std::string &) const
{
    return Parser::ValueList();
}

}
