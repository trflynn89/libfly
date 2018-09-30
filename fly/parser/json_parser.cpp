#include "fly/parser/json_parser.h"

#include <cctype>
#include <cstring>

#include "fly/parser/exceptions.h"
#include "fly/types/string.h"

namespace fly {

//==============================================================================
JsonParser::JsonParser() : Parser(), m_features(Features::Strict)
{
}

//==============================================================================
JsonParser::JsonParser(Features features) : Parser(), m_features(features)
{
}

//==============================================================================
Json JsonParser::ParseInternal(std::istream &stream)
{
    m_states = decltype(m_states)();
    m_states.push(State::NoState);

    Json values;
    m_pValue = &values;

    m_pParents = decltype(m_pParents)();
    m_pParents.push(m_pValue);

    m_parsing.str(std::string());
    m_parsingStarted = false;
    m_parsingComplete = false;
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
            case Token::Tab:
            case Token::NewLine:
            case Token::CarriageReturn:
            case Token::Space:
                onWhitespace(token, c);
                break;

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

            case Token::Comma:
                onComma(c);
                break;

            case Token::Colon:
                onColon(c);
                break;

            case Token::Solidus:
                onSolidus(c, stream);
                break;

            default:
                onCharacter(token, c, stream);
                break;
            }
        }
    }
    catch (const JsonException &ex)
    {
        throw ParserException(m_line, m_column, ex.what());
    }

    if (m_states.top() != State::NoState)
    {
        throw ParserException(m_line, m_column,
            "Finished parsing with incomplete JSON object"
        );
    }

    return values;
}

//==============================================================================
void JsonParser::onWhitespace(Token token, int c)
{
    if (m_parsingString)
    {
        if (token != Token::Space)
        {
            throw UnexpectedCharacterException(m_line, m_column, c);
        }

        pushValue(c);
    }
    else
    {
        if (m_parsingStarted)
        {
            m_parsingComplete = true;
        }

        if (token == Token::NewLine)
        {
            ++m_line;
            m_column = 0;
        }
    }
}

//==============================================================================
void JsonParser::onStartBraceOrBracket(Token token, int c)
{
    switch (m_states.top())
    {
    case State::NoState:
        if (m_pValue == nullptr)
        {
            throw UnexpectedCharacterException(m_line, m_column, c);
        }

        break;

    case State::ParsingColon:
    case State::ParsingObject:
        throw UnexpectedCharacterException(m_line, m_column, c);

    case State::ParsingArray:
        m_states.push(State::ParsingValue);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        break;

    case State::ParsingName:
    case State::ParsingValue:
        if (m_parsingString)
        {
            pushValue(c);
            return;
        }
        else if (m_parsingStarted)
        {
            throw UnexpectedCharacterException(m_line, m_column, c);
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
        throw UnexpectedCharacterException(m_line, m_column, c);

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

    if ((!storeValue() && m_expectingValue) || m_pParents.empty())
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
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
            throw UnexpectedCharacterException(m_line, m_column, c);
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

        m_pValue = &((*m_pValue)[popValue()]);
        m_pParents.push(m_pValue);

        m_expectingValue = false;

        break;

    case State::ParsingValue:
        if (m_parsingString)
        {
            m_parsingComplete = true;
            m_parsedString = true;

            m_states.pop();
            m_states.push(State::ParsingComma);
        }
        else if (m_parsingStarted)
        {
            throw UnexpectedCharacterException(m_line, m_column, c);
        }

        break;

    default:
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    m_parsingString = !m_parsingString;
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
            throw UnexpectedCharacterException(m_line, m_column, c);
        }
        break;

    default:
        throw UnexpectedCharacterException(m_line, m_column, c);
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
        else if (storeValue())
        {
            m_states.pop();
        }
        else if (m_expectingValue)
        {
            throw UnexpectedCharacterException(m_line, m_column, c);
        }

        break;

    case State::ParsingComma:
        storeValue();
        m_states.pop();

        if (m_states.top() == State::ParsingValue)
        {
            m_states.pop();
        }

        break;

    default:
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    if (isFeatureAllowed(Features::AllowTrailingComma))
    {
        m_expectingValue = false;
    }
    else
    {
        m_expectingValue = !m_parsingString;
    }
}

//==============================================================================
void JsonParser::onSolidus(int c, std::istream &stream)
{
    if (m_parsingString)
    {
        pushValue(c);
    }
    else if (isFeatureAllowed(Features::AllowComments))
    {
        Token token = static_cast<Token>(stream.get());

        switch (token)
        {
        case Token::Solidus:
            do
            {
                c = stream.get();
            } while ((c != EOF) && (static_cast<Token>(c) != Token::NewLine));

            break;

        case Token::Asterisk:
        {
            bool parsing = true;

            do
            {
                c = stream.get();

                if ((static_cast<Token>(c) == Token::Asterisk) &&
                    (static_cast<Token>(stream.peek()) == Token::Solidus))
                {
                    parsing = false;
                    stream.get();
                }
            } while ((c != EOF) && parsing);

            if (parsing)
            {
                throw UnexpectedCharacterException(m_line, m_column, c);
            }

            break;
        }

        default:
            throw UnexpectedCharacterException(m_line, m_column, c);
        }
    }
    else
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
    }
}

//==============================================================================
void JsonParser::onCharacter(Token token, int c, std::istream &stream)
{
    if (std::isspace(c))
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    switch (m_states.top())
    {
    case State::ParsingArray:
        m_states.push(State::ParsingValue);

        m_pValue = &((*m_pValue)[m_pValue->Size()]);
        m_pParents.push(m_pValue);

        pushValue(c);
        break;

    case State::ParsingValue:
    case State::ParsingName:
        pushValue(c);

        // Blindly ignore the escaped character, the Json class will check
        // whether it is valid. Just read at least one more character to prevent
        // the parser from failing if the next character is a quote.
        if (m_parsingString && (token == Token::ReverseSolidus))
        {
            if ((c = stream.get()) == EOF)
            {
                throw UnexpectedCharacterException(m_line, m_column, c);
            }

            pushValue(c);
        }

        break;

    default:
        throw UnexpectedCharacterException(m_line, m_column, c);
    }
}

//==============================================================================
void JsonParser::pushValue(int c)
{
    if (m_parsingComplete)
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    m_parsing << static_cast<Json::stream_type::char_type>(c);
    m_parsingStarted = true;
}

//==============================================================================
std::string JsonParser::popValue()
{
    const std::string value = m_parsing.str();

    m_parsing.str(std::string());
    m_parsingStarted = false;
    m_parsingComplete = false;

    return value;
}

//==============================================================================
bool JsonParser::storeValue()
{
    const std::string value = popValue();

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

    // Parsed a number
    else
    {
        bool isFloat = false, isSigned = false;
        validateNumber(value, isFloat, isSigned);

        try
        {
            if (isFloat)
            {
                *m_pValue = String::Convert<Json::float_type>(value);
            }
            else if (isSigned)
            {
                *m_pValue = String::Convert<Json::signed_type>(value);
            }
            else
            {
                *m_pValue = String::Convert<Json::unsigned_type>(value);
            }
        }
        catch (...)
        {
            throw BadConversionException(m_line, m_column, value);
        }
    }

    m_pParents.pop();
    m_pValue = (m_pParents.empty() ? nullptr : m_pParents.top());

    m_expectingValue = false;

    return true;
}

//==============================================================================
void JsonParser::validateNumber(
    const std::string &value,
    bool &isFloat,
    bool &isSigned
) const
{
    isSigned = (value[0] == '-');

    const std::string signless = (isSigned ? value.substr(1) : value);

    auto is_octal = [&signless]() -> bool
    {
        return (
            (signless.size() > 1) &&
            (signless[0] == '0') &&
            std::isdigit((unsigned char)(signless[1]))
        );
    };

    auto is_float = [this, &signless]() -> bool
    {
        const std::string::size_type d = signless.find('.');
        const std::string::size_type e = signless.find('e');
        const std::string::size_type E = signless.find('E');

        if (d != std::string::npos)
        {
            std::string::size_type end = signless.size();

            if ((e != std::string::npos) || (E != std::string::npos))
            {
                end = std::min(e, E);
            }

            if ((d + 1) >= end)
            {
                throw BadConversionException(m_line, m_column, signless);
            }

            return true;
        }

        return ((e != std::string::npos) || (E != std::string::npos));
    };

    if (!std::isdigit((unsigned char)(signless[0])))
    {
        throw BadConversionException(m_line, m_column, value);
    }
    else if (is_octal())
    {
        throw BadConversionException(m_line, m_column, value);
    }

    isFloat = is_float();
}

//==============================================================================
bool JsonParser::isFeatureAllowed(Features feature) const
{
    return ((m_features & feature) != Features::Strict);
}

//==============================================================================
JsonParser::Features operator & (JsonParser::Features a, JsonParser::Features b)
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) &
        static_cast<std::underlying_type_t<JsonParser::Features>>(b)
    );
}

//==============================================================================
JsonParser::Features operator | (JsonParser::Features a, JsonParser::Features b)
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) |
        static_cast<std::underlying_type_t<JsonParser::Features>>(b)
    );
}

}
