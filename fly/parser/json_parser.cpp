#include "fly/parser/json_parser.hpp"

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"
#include "fly/types/string/string.hpp"

#include <string_view>

namespace fly {

#define JLOG(...)                                                                                  \
    LOGW(                                                                                          \
        "[line %d, column %d]: " _FLY_FORMAT_STRING(__VA_ARGS__),                                  \
        m_line,                                                                                    \
        m_column _FLY_FORMAT_ARGS(__VA_ARGS__));

//==================================================================================================
JsonParser::JsonParser() noexcept : Parser(), m_features(Features::Strict)
{
}

//==================================================================================================
JsonParser::JsonParser(const Features features) noexcept : Parser(), m_features(features)
{
}

//==================================================================================================
std::optional<Json> JsonParser::parse_internal(std::istream &stream)
{
    std::optional<Json> json;

    try
    {
        json = parse_json(stream);
    }
    catch (const JsonException &ex)
    {
        JLOG("%s", ex.what());
        return std::nullopt;
    }

    if (consume_whitespace_and_comments(stream) == ParseState::Invalid)
    {
        return std::nullopt;
    }

    if (!stream.eof())
    {
        JLOG("Extraneous symbols found after JSON value");
        return std::nullopt;
    }
    else if (json && (!json->is_object() && !json->is_array()))
    {
        if (!is_feature_allowed(Features::AllowAnyType))
        {
            JLOG("Parsed non-object/non-array value, but Features::AllowAnyType is not enabled");
            return std::nullopt;
        }
    }

    return json;
}

//==================================================================================================
std::optional<Json> JsonParser::parse_json(std::istream &stream)
{
    if (consume_whitespace_and_comments(stream) == ParseState::Invalid)
    {
        return std::nullopt;
    }

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
std::optional<Json> JsonParser::parse_object(std::istream &stream)
{
    Json object = JsonTraits::object_type();

    if (consume_token(stream, Token::StartBrace) == ParseState::Invalid)
    {
        return std::nullopt;
    }

    while (true)
    {
        const ParseState state = parse_object_loop(stream, object);

        if (state == ParseState::Invalid)
        {
            return std::nullopt;
        }
        else if (state == ParseState::StopParsing)
        {
            break;
        }
    }

    if (consume_token(stream, Token::CloseBrace) == ParseState::Invalid)
    {
        return std::nullopt;
    }

    return object;
}

//==================================================================================================
JsonParser::ParseState JsonParser::parse_object_loop(std::istream &stream, Json &object)
{
    auto parse_state = std::bind(
        &JsonParser::done_parsing_object_or_array,
        this,
        std::ref(stream),
        Token::CloseBrace);

    ParseState state;

    if (state = parse_state(); state != ParseState::KeepParsing)
    {
        return state;
    }
    else if (object)
    {
        if (state = consume_comma(stream, parse_state); state != ParseState::KeepParsing)
        {
            return state;
        }
    }

    if ((peek(stream) == Token::Solidus) && (consume_comment(stream) == ParseState::Invalid))
    {
        return ParseState::Invalid;
    }

    JsonTraits::string_type key;
    if (consume_value(stream, JsonType::JsonString, key) == JsonType::Invalid)
    {
        return ParseState::Invalid;
    }

    if (consume_token(stream, Token::Colon) == ParseState::Invalid)
    {
        return ParseState::Invalid;
    }

    if (std::optional<Json> child = parse_json(stream); child)
    {
        object[std::move(key)] = std::move(child.value());
    }
    else
    {
        return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
std::optional<Json> JsonParser::parse_array(std::istream &stream)
{
    Json array = JsonTraits::array_type();

    if (consume_token(stream, Token::StartBracket) == ParseState::Invalid)
    {
        return std::nullopt;
    }

    while (true)
    {
        const ParseState state = parse_array_loop(stream, array);

        if (state == ParseState::Invalid)
        {
            return std::nullopt;
        }
        else if (state == ParseState::StopParsing)
        {
            break;
        }
    }

    if (consume_token(stream, Token::CloseBracket) == ParseState::Invalid)
    {
        return std::nullopt;
    }

    return array;
}

//==================================================================================================
JsonParser::ParseState JsonParser::parse_array_loop(std::istream &stream, Json &array)
{
    auto parse_state = std::bind(
        &JsonParser::done_parsing_object_or_array,
        this,
        std::ref(stream),
        Token::CloseBracket);

    ParseState state;

    if (state = parse_state(); state != ParseState::KeepParsing)
    {
        return state;
    }
    else if (array)
    {
        if (state = consume_comma(stream, parse_state); state != ParseState::KeepParsing)
        {
            return state;
        }
    }

    if (std::optional<Json> child = parse_json(stream); child)
    {
        array[array.size()] = std::move(child.value());
    }
    else
    {
        return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
JsonParser::ParseState
JsonParser::done_parsing_object_or_array(std::istream &stream, const Token &end_token)
{
    if (consume_whitespace_and_comments(stream) == ParseState::Invalid)
    {
        return ParseState::Invalid;
    }

    const Token token = peek(stream);

    if ((token == Token::EndOfFile) || (token == end_token))
    {
        return ParseState::StopParsing;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
std::optional<Json> JsonParser::parse_value(std::istream &stream)
{
    const bool is_string = peek(stream) == Token::Quote;

    auto json_type = is_string ? JsonType::JsonString : JsonType::Other;
    JsonTraits::string_type value;

    if (consume_value(stream, json_type, value) == JsonType::Invalid)
    {
        return std::nullopt;
    }
    else if (is_string)
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

    switch (validate_number(value))
    {
        case JsonType::SignedInteger:
            if (auto number = String::convert<JsonTraits::signed_type>(value); number)
            {
                return number.value();
            }
            break;

        case JsonType::UnsignedInteger:
            if (auto number = String::convert<JsonTraits::unsigned_type>(value); number)
            {
                return number.value();
            }
            break;

        case JsonType::FloatingPoint:
            if (auto number = String::convert<JsonTraits::float_type>(value); number)
            {
                return number.value();
            }
            break;

        default:
            break;
    }

    JLOG("Could not convert %s to a JSON number", value);
    return std::nullopt;
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_token(std::istream &stream, const Token &token)
{
    consume_whitespace(stream);

    if (const Token parsed = consume(stream); parsed != token)
    {
        JLOG(
            "Unexpected character '%c', was expecting '%c'",
            static_cast<JsonTraits::string_type::value_type>(token),
            static_cast<JsonTraits::string_type::value_type>(parsed));

        return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
JsonParser::ParseState
JsonParser::consume_comma(std::istream &stream, const ParseStateGetter &parse_state)
{
    if (consume_token(stream, Token::Comma) == ParseState::Invalid)
    {
        return ParseState::Invalid;
    }

    const ParseState state = parse_state();

    if (state == ParseState::StopParsing)
    {
        if (is_feature_allowed(Features::AllowTrailingComma))
        {
            return ParseState::StopParsing;
        }

        JLOG("Found trailing comma, but Features::AllowTrailingComma is not enabled");
        return ParseState::Invalid;
    }

    return state;
}

//==================================================================================================
JsonParser::JsonType
JsonParser::consume_value(std::istream &stream, JsonType type, JsonTraits::string_type &value)
{
    const bool is_string = type == JsonType::JsonString;

    Json::stream_type parsing;
    Token token;

    auto stop_parsing = [&token, &is_string, this]() -> bool {
        if (is_string)
        {
            return token == Token::Quote;
        }

        return is_whitespace(token) || (token == Token::Comma) || (token == Token::Solidus) ||
            (token == Token::CloseBracket) || (token == Token::CloseBrace);
    };

    if (is_string && (consume_token(stream, Token::Quote) == ParseState::Invalid))
    {
        return JsonType::Invalid;
    }

    while (((token = peek(stream)) != Token::EndOfFile) && !stop_parsing())
    {
        parsing << static_cast<Json::stream_type::char_type>(token);
        discard(stream);

        // Blindly ignore escaped symbols, the Json class will check whether they are valid. Just
        // read at least one more symbol to prevent breaking out of the loop too early if the next
        // symbol is a quote.
        if (is_string && (token == Token::ReverseSolidus))
        {
            if ((token = consume(stream)) == Token::EndOfFile)
            {
                JLOG("Expected character after reverse solidus");
                return JsonType::Invalid;
            }

            parsing << static_cast<Json::stream_type::char_type>(token);
        }
    }

    if (is_string && (consume_token(stream, Token::Quote) == ParseState::Invalid))
    {
        return JsonType::Invalid;
    }

    value = parsing.str();
    return type;
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_whitespace_and_comments(std::istream &stream)
{
    consume_whitespace(stream);

    if (is_feature_allowed(Features::AllowComments))
    {
        while (peek(stream) == Token::Solidus)
        {
            if (consume_comment(stream) == ParseState::Invalid)
            {
                return ParseState::Invalid;
            }

            consume_whitespace(stream);
        }
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
void JsonParser::consume_whitespace(std::istream &stream)
{
    Token token;

    while (((token = peek(stream)) != Token::EndOfFile) && is_whitespace(token))
    {
        discard(stream);
    }
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_comment(std::istream &stream)
{
    if (!is_feature_allowed(Features::AllowComments))
    {
        JLOG("Found comment, but Features::AllowComments is not enabled");
        return ParseState::Invalid;
    }

    if (consume_token(stream, Token::Solidus) == ParseState::Invalid)
    {
        return ParseState::Invalid;
    }

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
                    FLY_UNUSED(consume_token(stream, Token::Solidus));
                    break;
                }
            } while (token != Token::EndOfFile);

            break;

        default:
            JLOG(
                "Invalid start sequence for comments: '/%c'",
                static_cast<JsonTraits::string_type::value_type>(token));
            return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
JsonParser::Token JsonParser::peek(std::istream &stream)
{
    return static_cast<Token>(stream.peek());
}

//==================================================================================================
JsonParser::Token JsonParser::consume(std::istream &stream)
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
void JsonParser::discard(std::istream &stream)
{
    FLY_UNUSED(consume(stream));
}

//==================================================================================================
JsonParser::JsonType JsonParser::validate_number(const JsonTraits::string_type &value) const
{
    const bool is_signed = value[0] == '-';

    const auto signless = std::basic_string_view<JsonTraits::string_type::value_type>(value).substr(
        is_signed ? 1 : 0);

    const bool is_octal = (signless.size() > 1) && (signless[0] == '0') &&
        std::isdigit(static_cast<unsigned char>(signless[1]));

    if (!std::isdigit(static_cast<unsigned char>(signless[0])))
    {
        JLOG("Could not convert '%s' to a number", value);
        return JsonType::Invalid;
    }
    else if (is_octal)
    {
        JLOG("Octal value '%s' is not number", value);
        return JsonType::Invalid;
    }

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
            JLOG("Float value '%s' is invalid", value);
            return JsonType::Invalid;
        }

        return JsonType::FloatingPoint;
    }
    else if ((e1 != JsonTraits::string_type::npos) || (e2 != JsonTraits::string_type::npos))
    {
        return JsonType::FloatingPoint;
    }

    return is_signed ? JsonType::SignedInteger : JsonType::UnsignedInteger;
}

//==================================================================================================
bool JsonParser::is_whitespace(const Token &token) const
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
bool JsonParser::is_feature_allowed(Features feature) const
{
    return (m_features & feature) != Features::Strict;
}

//==================================================================================================
JsonParser::Features operator&(JsonParser::Features a, JsonParser::Features b)
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) &
        static_cast<std::underlying_type_t<JsonParser::Features>>(b));
}

//==================================================================================================
JsonParser::Features operator|(JsonParser::Features a, JsonParser::Features b)
{
    return static_cast<JsonParser::Features>(
        static_cast<std::underlying_type_t<JsonParser::Features>>(a) |
        static_cast<std::underlying_type_t<JsonParser::Features>>(b));
}

} // namespace fly
