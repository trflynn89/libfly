#include "fly/parser/json_parser.h"

#include <cctype>
#include <cstring>

#include "fly/parser/exceptions.h"
#include "fly/string/string.h"
#include "fly/types/json.h"

namespace fly {

//==============================================================================
JsonParser::JsonParser(const std::string &path, const std::string &file) :
    Parser(path, file),
    m_states(),
    m_pValue(),
    m_pParents(),
    m_parsing(),
    m_parsingString(false),
    m_parsedString(false),
    m_expectingValue(false)
{
}

//==============================================================================
void JsonParser::ParseInternal(std::ifstream &stream)
{
    m_states = decltype(m_states)();
    m_states.push(State::NoState);

    m_pValue = &m_values;

    m_pParents = decltype(m_pParents)();
    m_pParents.push(m_pValue);

    m_parsing.str(std::string());
    m_parsingString = false;
    m_parsedString = false;
    m_expectingValue = false;

    try
    {
        int c = 0;

        while ((c = stream.get()) != EOF)
        {
            Token token = static_cast<Token>(c);
            ++m_column;

            switch (token)
            {
            case Token::StartBrace:
            case Token::StartBracket:
                onStartBraceOrBracket(token, c);
                break;

            case Token::CloseBrace:
            case Token::CloseBracket:
                onCloseBraceOrBracket(token, c);
                break;

            case Token::Quote:
                onQuotation(c);
                break;

            case Token::NewLine:
                onNewLine(c);
                break;

            case Token::Comma:
                onComma(c);
                break;

            case Token::Colon:
                onColon(c);
                break;

            default:
                onCharacter(token, c, stream);
                break;
            }
        }
    }
    catch (const JsonException &ex)
    {
        throw ParserException(m_file, m_line, m_column, ex.what());
    }

    if (m_states.top() != State::NoState)
    {
        throw ParserException(m_file, m_line, m_column,
            "Finished parsing file with incomplete JSON object"
        );
    }
}

//==============================================================================
void JsonParser::onStartBraceOrBracket(Token token, int c)
{
    switch (m_states.top())
    {
    case State::ParsingObject:
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);

    case State::ParsingArray:
        m_states.push(State::ParsingValue);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        break;

    case State::ParsingColon:
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);

    case State::ParsingName:
    case State::ParsingValue:
        if (m_parsingString)
        {
            pushValue(c);
            return;
        }

        break;

    default:
        break;
    }

    if (token == Token::StartBrace)
    {
        *m_pValue = Json::object_type();
        m_states.push(State::ParsingObject);
    }
    else
    {
        *m_pValue = Json::array_type();
        m_states.push(State::ParsingArray);
    }

    m_expectingValue = false;
}

//==============================================================================
void JsonParser::onCloseBraceOrBracket(Token token, int c)
{
    switch (m_states.top())
    {
    case State::NoState:
    case State::ParsingColon:
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);

    case State::ParsingName:
    case State::ParsingValue:
        if (m_parsingString)
        {
            pushValue(c);
            return;
        }

        break;

    default:
        break;
    }

    if ((!popValue() && m_expectingValue) || m_pParents.empty())
    {
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);
    }

    m_pParents.pop();
    m_pValue = (m_pParents.empty() ? nullptr : m_pParents.top());

    const State expected = (
        (token == Token::CloseBrace) ? State::ParsingObject : State::ParsingArray
    );

    const State unexpected = (
        (token == Token::CloseBrace) ? State::ParsingArray : State::ParsingObject
    );

    while (m_states.top() != expected)
    {
        if (m_states.top() == unexpected)
        {
            throw UnexpectedCharacterException(m_file, m_line, m_column, c);
        }

        m_states.pop();
    }

    m_states.pop();

    if (m_states.top() != State::NoState)
    {
        m_states.push(State::ParsingComma);
    }
}

//==============================================================================
void JsonParser::onQuotation(int c)
{
    switch (m_states.top())
    {
    case State::ParsingObject:
        m_states.push(State::ParsingName);
        break;

    case State::ParsingArray:
        m_states.push(State::ParsingValue);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        break;

    case State::ParsingName:
        m_states.pop();
        m_states.push(State::ParsingColon);

        m_pValue = &((*m_pValue)[m_parsing.str()]);
        m_pParents.push(m_pValue);

        m_parsing.str(std::string());

        m_expectingValue = false;

        break;

    case State::ParsingValue:
        if (m_parsingString)
        {
            m_parsedString = true;

            m_states.pop();
            m_states.push(State::ParsingComma);
        }

        break;

    default:
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);
    }

    m_parsingString = !m_parsingString;
}

//==============================================================================
void JsonParser::onNewLine(int c)
{
    if (m_parsingString)
    {
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);
    }

    ++m_line;
    m_column = 0;
}

//==============================================================================
void JsonParser::onColon(int c)
{
    switch (m_states.top())
    {
    case State::ParsingColon:
        m_states.pop();
        m_states.push(State::ParsingValue);

        m_expectingValue = true;

        break;

    case State::ParsingName:
    case State::ParsingValue:
        if (m_parsingString)
        {
            pushValue(c);
        }
        else
        {
            throw UnexpectedCharacterException(m_file, m_line, m_column, c);
        }
        break;

    default:
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);
    }
}

//==============================================================================
void JsonParser::onComma(int c)
{
    switch (m_states.top())
    {
    case State::ParsingName:
        pushValue(c);
        break;

    case State::ParsingValue:
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
            throw UnexpectedCharacterException(m_file, m_line, m_column, c);
        }

        break;

    case State::ParsingComma:
        popValue();
        m_states.pop();

        if (m_states.top() == State::ParsingValue)
        {
            m_states.pop();
        }

        break;

    default:
        throw UnexpectedCharacterException(m_file, m_line, m_column, c);
    }

    m_expectingValue = !m_parsingString;
}

//==============================================================================
void JsonParser::onCharacter(Token token, int c, std::ifstream &stream)
{
    switch (m_states.top())
    {
    case State::ParsingArray:
        if (!std::isspace(c))
        {
            m_states.push(State::ParsingValue);

            m_pValue = &((*m_pValue)[m_pValue->Size()]);
            m_pParents.push(m_pValue);

            pushValue(c);
        }

        break;

    case State::ParsingValue:
    case State::ParsingName:
        if (m_parsingString)
        {
            if (std::isspace(c) && (token != Token::Space))
            {
                throw UnexpectedCharacterException(m_file, m_line, m_column, c);
            }

            pushValue(c);

            // Blindly ignore the escaped character, the Json class will check
            // whether it is valid. Just read at least one more character to
            // prevent the parser from failing if the next character is a quote.
            if (token == Token::ReverseSolidus)
            {
                if ((c = stream.get()) == EOF)
                {
                    throw UnexpectedCharacterException(m_file, m_line, m_column, c);
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
            throw UnexpectedCharacterException(m_file, m_line, m_column, c);
        }
    }
}

//==============================================================================
void JsonParser::pushValue(int c)
{
    m_parsing << static_cast<Json::stream_type::char_type>(c);
}

//==============================================================================
bool JsonParser::popValue()
{
    const std::string value = m_parsing.str();

    // Parsed a string value
    if (m_parsedString)
    {
        m_parsedString = false;
        *m_pValue = value;
    }

    // No parsed value
    else if (value.empty())
    {
        return false;
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
        throw BadConversionException(m_file, m_line, m_column, value);
    }

    // Parsed a number - validate no postive sign given
    else if ((value.length() > 1) && (value[0] == '+'))
    {
        throw BadConversionException(m_file, m_line, m_column, value);
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
            throw BadConversionException(m_file, m_line, m_column, value);
        }

        try
        {
            *m_pValue = String::Convert<Json::float_type>(value);
        }
        catch (...)
        {
            throw BadConversionException(m_file, m_line, m_column, value);
        }
    }

    // Parsed a signed integer
    else if (value[0] == '-')
    {
        try
        {
            *m_pValue = String::Convert<Json::signed_type>(value);
        }
        catch (...)
        {
            throw BadConversionException(m_file, m_line, m_column, value);
        }
    }

    // Parsed an unsigned integer
    else
    {
        try
        {
            *m_pValue = String::Convert<Json::unsigned_type>(value);
        }
        catch (...)
        {
            throw BadConversionException(m_file, m_line, m_column, value);
        }
    }

    // Check for failed construction of the parsed type
    if (m_pValue->HasValidationError())
    {
        throw ParserException(
            m_file, m_line, m_column, m_pValue->GetValidationError()
        );
    }

    m_pParents.pop();
    m_pValue = m_pParents.top();

    m_parsing.str(std::string());

    m_expectingValue = false;

    return true;
}

}
