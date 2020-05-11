#include "fly/parser/json_parser.hpp"

#include "fly/parser/exceptions.hpp"
#include "fly/types/string/string.hpp"

#include <cctype>
#include <cstdio>
#include <string_view>

namespace fly {

//==============================================================================
JsonParser::JsonParser() noexcept : Parser(), m_features(Features::Strict)
{
}

//==============================================================================
JsonParser::JsonParser(Features features) noexcept :
    Parser(),
    m_features(features)
{
}

//==============================================================================
Json JsonParser::parse_internal(std::istream &stream) noexcept(false)
{
    m_states = decltype(m_states)();
    m_states.push(State::NoState);

    Json values;
    m_value = &values;

    m_parents = decltype(m_parents)();
    m_parents.push(m_value);

    m_parsing.str(std::string());
    m_parsing_started = false;
    m_parsing_complete = false;
    m_parsing_string = false;
    m_parsed_string = false;
    m_expecting_value = false;

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
                    on_whitespace(token, c);
                    break;

                case Token::StartBrace:
                case Token::StartBracket:
                    on_start_brace_or_bracket(token, c);
                    break;

                case Token::CloseBrace:
                case Token::CloseBracket:
                    on_close_brace_or_bracket(token, c);
                    break;

                case Token::Quote:
                    on_quotation(c);
                    break;

                case Token::Comma:
                    on_comma(c);
                    break;

                case Token::Colon:
                    on_colon(c);
                    break;

                case Token::Solidus:
                    on_solidus(c, stream);
                    break;

                default:
                    on_character(token, c, stream);
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
        throw ParserException(
            m_line,
            m_column,
            "Finished parsing with incomplete JSON object");
    }

    return values;
}

//==============================================================================
void JsonParser::on_whitespace(Token token, int c) noexcept(false)
{
    if (m_parsing_string)
    {
        if (token != Token::Space)
        {
            throw UnexpectedCharacterException(m_line, m_column, c);
        }

        push_value(c);
    }
    else
    {
        if (m_parsing_started)
        {
            m_parsing_complete = true;
        }

        if (token == Token::NewLine)
        {
            ++m_line;
            m_column = 0;
        }
    }
}

//==============================================================================
void JsonParser::on_start_brace_or_bracket(Token token, int c) noexcept(false)
{
    switch (m_states.top())
    {
        case State::NoState:
            if (m_value == nullptr)
            {
                throw UnexpectedCharacterException(m_line, m_column, c);
            }

            break;

        case State::ParsingColon:
        case State::ParsingObject:
            throw UnexpectedCharacterException(m_line, m_column, c);

        case State::ParsingArray:
            m_states.push(State::ParsingValue);

            m_value = &((*m_value)[m_value->size()]);
            m_parents.push(m_value);

            break;

        case State::ParsingName:
        case State::ParsingValue:
            if (m_parsing_string)
            {
                push_value(c);
                return;
            }
            else if (m_parsing_started)
            {
                throw UnexpectedCharacterException(m_line, m_column, c);
            }

            break;

        default:
            break;
    }

    if (token == Token::StartBrace)
    {
        *m_value = JsonTraits::object_type();
        m_states.push(State::ParsingObject);
    }
    else
    {
        *m_value = JsonTraits::array_type();
        m_states.push(State::ParsingArray);
    }

    m_expecting_value = false;
}

//==============================================================================
void JsonParser::on_close_brace_or_bracket(Token token, int c) noexcept(false)
{
    switch (m_states.top())
    {
        case State::NoState:
        case State::ParsingColon:
            throw UnexpectedCharacterException(m_line, m_column, c);

        case State::ParsingName:
        case State::ParsingValue:
            if (m_parsing_string)
            {
                push_value(c);
                return;
            }

            break;

        default:
            break;
    }

    if ((!store_value() && m_expecting_value) || m_parents.empty())
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    m_parents.pop();
    m_value = (m_parents.empty() ? nullptr : m_parents.top());

    // Formatter doesn't have options for hanging indent after ternary operator
    // clang-format off
    const State expected = (token == Token::CloseBrace) ?
        State::ParsingObject : State::ParsingArray;
    const State unexpected = (token == Token::CloseBrace) ?
        State::ParsingArray : State::ParsingObject;
    // clang-format on

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
void JsonParser::on_quotation(int c) noexcept(false)
{
    switch (m_states.top())
    {
        case State::ParsingObject:
            m_states.push(State::ParsingName);
            break;

        case State::ParsingArray:
            m_states.push(State::ParsingValue);

            m_value = &((*m_value)[m_value->size()]);
            m_parents.push(m_value);

            break;

        case State::ParsingName:
            m_states.pop();
            m_states.push(State::ParsingColon);

            m_value = &((*m_value)[pop_value()]);
            m_parents.push(m_value);

            m_expecting_value = false;

            break;

        case State::ParsingValue:
            if (m_parsing_string)
            {
                m_parsing_complete = true;
                m_parsed_string = true;

                m_states.pop();
                m_states.push(State::ParsingComma);
            }
            else if (m_parsing_started)
            {
                throw UnexpectedCharacterException(m_line, m_column, c);
            }

            break;

        default:
            throw UnexpectedCharacterException(m_line, m_column, c);
    }

    m_parsing_string = !m_parsing_string;
}

//==============================================================================
void JsonParser::on_colon(int c) noexcept(false)
{
    switch (m_states.top())
    {
        case State::ParsingColon:
            m_states.pop();
            m_states.push(State::ParsingValue);

            m_expecting_value = true;

            break;

        case State::ParsingName:
        case State::ParsingValue:
            if (m_parsing_string)
            {
                push_value(c);
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
void JsonParser::on_comma(int c) noexcept(false)
{
    switch (m_states.top())
    {
        case State::ParsingName:
            push_value(c);
            break;

        case State::ParsingValue:
            if (m_parsing_string)
            {
                push_value(c);
            }
            else if (store_value())
            {
                m_states.pop();
            }
            else if (m_expecting_value)
            {
                throw UnexpectedCharacterException(m_line, m_column, c);
            }

            break;

        case State::ParsingComma:
            store_value();
            m_states.pop();

            if (m_states.top() == State::ParsingValue)
            {
                m_states.pop();
            }

            break;

        default:
            throw UnexpectedCharacterException(m_line, m_column, c);
    }

    if (is_feature_allowed(Features::AllowTrailingComma))
    {
        m_expecting_value = false;
    }
    else
    {
        m_expecting_value = !m_parsing_string;
    }
}

//==============================================================================
void JsonParser::on_solidus(int c, std::istream &stream) noexcept(false)
{
    if (m_parsing_string)
    {
        push_value(c);
    }
    else if (is_feature_allowed(Features::AllowComments))
    {
        Token token = static_cast<Token>(stream.get());

        switch (token)
        {
            case Token::Solidus:
                do
                {
                    c = stream.get();
                } while ((c != EOF) &&
                         (static_cast<Token>(c) != Token::NewLine));

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
void JsonParser::on_character(
    Token token,
    int c,
    std::istream &stream) noexcept(false)
{
    if (std::isspace(c))
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    switch (m_states.top())
    {
        case State::ParsingArray:
            m_states.push(State::ParsingValue);

            m_value = &((*m_value)[m_value->size()]);
            m_parents.push(m_value);

            push_value(c);
            break;

        case State::ParsingValue:
        case State::ParsingName:
            push_value(c);

            // Blindly ignore the escaped character, the Json class will
            // check whether it is valid. Just read at least one more
            // character to prevent the parser from failing if the next
            // character is a quote.
            if (m_parsing_string && (token == Token::ReverseSolidus))
            {
                if ((c = stream.get()) == EOF)
                {
                    throw UnexpectedCharacterException(m_line, m_column, c);
                }

                push_value(c);
            }

            break;

        default:
            throw UnexpectedCharacterException(m_line, m_column, c);
    }
}

//==============================================================================
void JsonParser::push_value(int c) noexcept(false)
{
    if (m_parsing_complete)
    {
        throw UnexpectedCharacterException(m_line, m_column, c);
    }

    m_parsing << static_cast<Json::stream_type::char_type>(c);
    m_parsing_started = true;
}

//==============================================================================
std::string JsonParser::pop_value() noexcept
{
    const std::string value = m_parsing.str();

    m_parsing.str(std::string());
    m_parsing_started = false;
    m_parsing_complete = false;

    return value;
}

//==============================================================================
bool JsonParser::store_value() noexcept(false)
{
    const std::string value = pop_value();

    // Parsed a string value
    if (m_parsed_string)
    {
        m_parsed_string = false;
        *m_value = value;
    }

    // No parsed value
    else if (value.empty())
    {
        return false;
    }

    // Parsed a boolean value
    else if (value == "true")
    {
        *m_value = true;
    }
    else if (value == "false")
    {
        *m_value = false;
    }

    // Parsed a null value
    else if (value == "null")
    {
        *m_value = nullptr;
    }

    // Parsed a number
    else
    {
        bool is_float = false, is_signed = false;
        validate_number(value, is_float, is_signed);

        try
        {
            if (is_float)
            {
                *m_value = String::Convert<JsonTraits::float_type>(value);
            }
            else if (is_signed)
            {
                *m_value = String::Convert<JsonTraits::signed_type>(value);
            }
            else
            {
                *m_value = String::Convert<JsonTraits::unsigned_type>(value);
            }
        }
        catch (...)
        {
            throw BadConversionException(m_line, m_column, value);
        }
    }

    m_parents.pop();
    m_value = (m_parents.empty() ? nullptr : m_parents.top());

    m_expecting_value = false;

    return true;
}

//==============================================================================
void JsonParser::validate_number(
    const std::string &value,
    bool &is_float,
    bool &is_signed) const noexcept(false)
{
    is_signed = (value[0] == '-');

    const auto signless = std::string_view(value).substr(is_signed ? 1 : 0);

    auto octal_test = [&signless]() -> bool {
        return (signless.size() > 1) && (signless[0] == '0') &&
            std::isdigit(static_cast<unsigned char>(signless[1]));
    };

    auto float_test = [this, &value, &signless]() -> bool {
        const std::string::size_type d = signless.find('.');
        const std::string::size_type e1 = signless.find('e');
        const std::string::size_type e2 = signless.find('E');

        if (d != std::string::npos)
        {
            std::string::size_type end = signless.size();

            if ((e1 != std::string::npos) || (e2 != std::string::npos))
            {
                end = std::min(e1, e2);
            }

            if ((d + 1) >= end)
            {
                throw BadConversionException(m_line, m_column, value);
            }

            return true;
        }

        return (e1 != std::string::npos) || (e2 != std::string::npos);
    };

    if (!std::isdigit(static_cast<unsigned char>(signless[0])))
    {
        throw BadConversionException(m_line, m_column, value);
    }
    else if (octal_test())
    {
        throw BadConversionException(m_line, m_column, value);
    }

    is_float = float_test();
}

//==============================================================================
bool JsonParser::is_feature_allowed(Features feature) const noexcept(false)
{
    return (m_features & feature) != Features::Strict;
}

//==============================================================================
JsonParser::Features
operator&(JsonParser::Features a, JsonParser::Features b) noexcept
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) &
        static_cast<std::underlying_type_t<JsonParser::Features>>(b));
}

//==============================================================================
JsonParser::Features
operator|(JsonParser::Features a, JsonParser::Features b) noexcept
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) |
        static_cast<std::underlying_type_t<JsonParser::Features>>(b));
}

} // namespace fly
