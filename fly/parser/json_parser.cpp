#include "fly/parser/json_parser.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"

#include <string_view>

namespace fly {

//==================================================================================================
JsonParser::JsonParser() noexcept : Parser(), m_features(Features::Strict)
{
}

//==================================================================================================
JsonParser::JsonParser(const Features features) noexcept : Parser(), m_features(features)
{
}

//==================================================================================================
Json JsonParser::parse_internal(std::istream &stream) noexcept(false)
{
    Json json;

    try
    {
        json = parse_json(stream);
    }
    catch (const JsonException &ex)
    {
        throw ParserException(m_line, m_column, ex.what());
    }

    consume_whitespace_and_comments(stream);

    if (!stream.eof())
    {
        throw ParserException(m_line, m_column, "Extraneous symbols found after JSON value");
    }
    else if (!json.is_object() && !json.is_array())
    {
        if (!is_feature_allowed(Features::AllowAnyType))
        {
            throw ParserException(m_line, m_column, "JSON value must be an object or array");
        }
    }

    return json;
}

//==================================================================================================
Json JsonParser::parse_json(std::istream &stream) noexcept(false)
{
    consume_whitespace_and_comments(stream);

    switch (peek(stream))
    {
        case Token::StartBrace:
            return parse_object(stream);

        case Token::StartBracket:
            return parse_array(stream);

        default:
            return parse_value(stream);
    }
}

//==================================================================================================
Json JsonParser::parse_object(std::istream &stream) noexcept(false)
{
    Json object = JsonTraits::object_type();
    consume_token(stream, Token::StartBrace);

    auto stop_parsing = [&stream, this]() noexcept(false) {
        consume_whitespace_and_comments(stream);
        const Token token = peek(stream);

        return (token == Token::EndOfFile) || (token == Token::CloseBrace);
    };

    while (!stop_parsing())
    {
        if (object && consume_comma(stream, stop_parsing))
        {
            break;
        }

        if (peek(stream) == Token::Solidus)
        {
            consume_comment(stream);
        }

        auto key = consume_value(stream, JsonType::JsonString);
        consume_token(stream, Token::Colon);

        object[std::move(key)] = parse_json(stream);
    }

    consume_token(stream, Token::CloseBrace);
    return object;
}

//==================================================================================================
Json JsonParser::parse_array(std::istream &stream) noexcept(false)
{
    Json array = JsonTraits::array_type();
    consume_token(stream, Token::StartBracket);

    auto stop_parsing = [&stream, this]() noexcept(false) {
        consume_whitespace_and_comments(stream);
        const Token token = peek(stream);

        return (token == Token::EndOfFile) || (token == Token::CloseBracket);
    };

    while (!stop_parsing())
    {
        if (array && consume_comma(stream, stop_parsing))
        {
            break;
        }

        array[array.size()] = parse_json(stream);
    }

    consume_token(stream, Token::CloseBracket);
    return array;
}

//==================================================================================================
Json JsonParser::parse_value(std::istream &stream) noexcept(false)
{
    const bool is_string = peek(stream) == Token::Quote;

    const auto json_type = is_string ? JsonType::JsonString : JsonType::Other;
    const JsonTraits::string_type value = consume_value(stream, json_type);

    if (is_string)
    {
        return value;
    }
    else if (value == "true")
    {
        return true;
    }
    else if (value == "false")
    {
        return false;
    }
    else if (value == "null")
    {
        return nullptr;
    }
    else
    {
        const NumericType numeric_type = validate_number(value);

        try
        {
            switch (numeric_type)
            {
                case NumericType::SignedInteger:
                    return String::convert<JsonTraits::signed_type>(value);

                case NumericType::UnsignedInteger:
                    return String::convert<JsonTraits::unsigned_type>(value);

                case NumericType::FloatingPoint:
                    return String::convert<JsonTraits::float_type>(value);
            }
        }
        catch (...)
        {
        }
    }

    throw BadConversionException(m_line, m_column, value);
}

//==================================================================================================
void JsonParser::consume_token(std::istream &stream, const Token &token) noexcept(false)
{
    consume_whitespace(stream);

    const Token parsed = consume(stream);

    if (parsed != token)
    {
        throw UnexpectedTokenException(m_line, m_column, parsed);
    }
}

//==================================================================================================
bool JsonParser::consume_comma(
    std::istream &stream,
    const std::function<bool()> &stop_parsing) noexcept(false)
{
    consume_token(stream, Token::Comma);

    if (stop_parsing())
    {
        if (is_feature_allowed(Features::AllowTrailingComma))
        {
            return true;
        }

        throw UnexpectedTokenException(m_line, m_column, Token::Comma);
    }

    return false;
}

//==================================================================================================
JsonTraits::string_type
JsonParser::consume_value(std::istream &stream, JsonType type) noexcept(false)
{
    Json::stream_type parsing;
    Token token;

    auto stop_parsing = [&token, &type, this]() noexcept {
        if (type == JsonType::JsonString)
        {
            return token == Token::Quote;
        }

        const bool is_comment =
            is_feature_allowed(Features::AllowComments) && (token == Token::Solidus);

        return is_whitespace(token) || (token == Token::Comma) || (token == Token::CloseBracket) ||
            (token == Token::CloseBrace) || is_comment;
    };

    if (type == JsonType::JsonString)
    {
        consume_token(stream, Token::Quote);
    }

    while (((token = peek(stream)) != Token::EndOfFile) && !stop_parsing())
    {
        parsing << static_cast<Json::stream_type::char_type>(token);
        discard(stream);

        // Blindly ignore escaped symbols, the Json class will check whether
        // they are valid. Just read at least one more symbol to prevent
        // breaking out of the loop too early if the next symbol is a quote.
        if ((type == JsonType::JsonString) && (token == Token::ReverseSolidus))
        {
            if ((token = consume(stream)) == Token::EndOfFile)
            {
                throw UnexpectedTokenException(m_line, m_column, token);
            }

            parsing << static_cast<Json::stream_type::char_type>(token);
        }
    }

    if (type == JsonType::JsonString)
    {
        consume_token(stream, Token::Quote);
    }

    return parsing.str();
}

//==================================================================================================
void JsonParser::consume_whitespace_and_comments(std::istream &stream) noexcept(false)
{
    consume_whitespace(stream);

    if (is_feature_allowed(Features::AllowComments))
    {
        while (peek(stream) == Token::Solidus)
        {
            consume_comment(stream);
            consume_whitespace(stream);
        }
    }
}

//==================================================================================================
void JsonParser::consume_whitespace(std::istream &stream) noexcept
{
    Token token;

    while (((token = peek(stream)) != Token::EndOfFile) && is_whitespace(token))
    {
        discard(stream);
    }
}

//==================================================================================================
void JsonParser::consume_comment(std::istream &stream) noexcept(false)
{
    if (!is_feature_allowed(Features::AllowComments))
    {
        throw UnexpectedTokenException(m_line, m_column, Token::Solidus);
    }

    consume_token(stream, Token::Solidus);
    Token token;

    switch (token = consume(stream))
    {
        case Token::Solidus:
            do
            {
                token = consume(stream);
            } while ((token != Token::EndOfFile) && (token != Token::NewLine));

            break;

        case Token::Asterisk:
            do
            {
                token = consume(stream);

                if ((token == Token::Asterisk) && (peek(stream) == Token::Solidus))
                {
                    consume_token(stream, Token::Solidus);
                    break;
                }
            } while (token != Token::EndOfFile);

            break;

        default:
            throw UnexpectedTokenException(m_line, m_column, token);
    }
}

//==================================================================================================
JsonParser::Token JsonParser::peek(std::istream &stream) noexcept
{
    return static_cast<Token>(stream.peek());
}

//==================================================================================================
JsonParser::Token JsonParser::consume(std::istream &stream) noexcept
{
    const Token token = static_cast<Token>(stream.get());

    if (token == Token::NewLine)
    {
        m_column = 1;
        ++m_line;
    }
    else
    {
        ++m_column;
    }

    return token;
}

//==================================================================================================
void JsonParser::discard(std::istream &stream) noexcept
{
    FLY_UNUSED(consume(stream));
}

//==================================================================================================
JsonParser::NumericType JsonParser::validate_number(const JsonTraits::string_type &value) const
    noexcept(false)
{
    const bool is_signed = value[0] == '-';

    const auto signless = std::basic_string_view<JsonTraits::string_type::value_type>(value).substr(
        is_signed ? 1 : 0);

    auto is_octal = [&signless]() noexcept -> bool {
        return (signless.size() > 1) && (signless[0] == '0') &&
            std::isdigit(static_cast<unsigned char>(signless[1]));
    };

    auto is_float = [this, &value, &signless]() noexcept(false) -> bool {
        const JsonTraits::string_type::size_type d = signless.find('.');
        const JsonTraits::string_type::size_type e1 = signless.find('e');
        const JsonTraits::string_type::size_type e2 = signless.find('E');

        if (d != JsonTraits::string_type::npos)
        {
            JsonTraits::string_type::size_type end = signless.size();

            if ((e1 != JsonTraits::string_type::npos) || (e2 != JsonTraits::string_type::npos))
            {
                end = std::min(e1, e2);
            }

            if ((d + 1) >= end)
            {
                throw BadConversionException(m_line, m_column, value);
            }

            return true;
        }

        return (e1 != JsonTraits::string_type::npos) || (e2 != JsonTraits::string_type::npos);
    };

    if (!std::isdigit(static_cast<unsigned char>(signless[0])))
    {
        throw BadConversionException(m_line, m_column, value);
    }
    else if (is_octal())
    {
        throw BadConversionException(m_line, m_column, value);
    }
    else if (is_float())
    {
        return NumericType::FloatingPoint;
    }
    else if (is_signed)
    {
        return NumericType::SignedInteger;
    }
    else
    {
        return NumericType::UnsignedInteger;
    }
}

//==================================================================================================
bool JsonParser::is_whitespace(const Token &token) const noexcept
{
    switch (token)
    {
        case Token::Tab:
        case Token::NewLine:
        case Token::VerticalTab:
        case Token::CarriageReturn:
        case Token::Space:
            return true;

        default:
            return false;
    }
}

//==================================================================================================
bool JsonParser::is_feature_allowed(Features feature) const noexcept
{
    return (m_features & feature) != Features::Strict;
}

//==================================================================================================
JsonParser::Features operator&(JsonParser::Features a, JsonParser::Features b) noexcept
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) &
        static_cast<std::underlying_type_t<JsonParser::Features>>(b));
}

//==================================================================================================
JsonParser::Features operator|(JsonParser::Features a, JsonParser::Features b) noexcept
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) |
        static_cast<std::underlying_type_t<JsonParser::Features>>(b));
}

//==================================================================================================
JsonParser::UnexpectedTokenException::UnexpectedTokenException(
    std::uint32_t line,
    std::uint32_t column,
    JsonParser::Token token) :
    UnexpectedCharacterException(line, column, static_cast<int>(token))
{
}

} // namespace fly
