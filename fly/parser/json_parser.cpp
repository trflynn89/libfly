#include "fly/parser/json_parser.hpp"

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"
#include "fly/types/string/string.hpp"

#include <string_view>

namespace fly {

#define JLOG(...)                                                                                  \
    LOGW(                                                                                          \
        "[line %d, column %d]: " FLY_FORMAT_STRING(__VA_ARGS__),                                   \
        m_line,                                                                                    \
        m_column FLY_FORMAT_ARGS(__VA_ARGS__));

//==================================================================================================
JsonParser::JsonParser(const Features features) noexcept : Parser(), m_features(features)
{
}

//==================================================================================================
std::optional<Json> JsonParser::parse_internal()
{
    std::optional<Json> json;

    try
    {
        json = parse_json();
    }
    catch (const JsonException &ex)
    {
        JLOG("%s", ex.what());
        return std::nullopt;
    }

    if (consume_whitespace_and_comments() == ParseState::Invalid)
    {
        return std::nullopt;
    }

    if (json)
    {
        if (!m_stream->eof())
        {
            JLOG("Extraneous symbols found after JSON value");
            return std::nullopt;
        }
        else if (!json->is_object() && !json->is_array())
        {
            if (!is_feature_allowed(Features::AllowAnyType))
            {
                JLOG(
                    "Parsed non-object/non-array value, but Features::AllowAnyType is not enabled");
                return std::nullopt;
            }
        }
    }

    return json;
}

//==================================================================================================
std::optional<Json> JsonParser::parse_json()
{
    if (consume_whitespace_and_comments() == ParseState::Invalid)
    {
        return std::nullopt;
    }

    switch (m_stream->peek<Token>())
    {
        case Token::StartBrace:
            return parse_object();

        case Token::StartBracket:
            return parse_array();

        default:
            return parse_value();
    }
}

//==================================================================================================
std::optional<Json> JsonParser::parse_object()
{
    Json object = JsonTraits::object_type();
    ParseState state;

    // Discard the opening brace, which has already been peeked.
    discard();

    auto parse_state =
        std::bind(&JsonParser::done_parsing_object_or_array, this, Token::CloseBrace);

    while ((state = parse_state()) == ParseState::KeepParsing)
    {
        if (object && ((state = consume_comma(parse_state)) != ParseState::KeepParsing))
        {
            break;
        }

        JsonTraits::string_type key;
        if (consume_value(JsonType::JsonString, key) == JsonType::Invalid)
        {
            return std::nullopt;
        }
        else if (consume_token(Token::Colon) == ParseState::Invalid)
        {
            return std::nullopt;
        }
        else if (std::optional<Json> value = parse_json(); value)
        {
            object[std::move(key)] = std::move(value.value());
        }
        else
        {
            return std::nullopt;
        }
    }

    return (state == ParseState::Invalid) ? std::nullopt : std::optional<Json>(object);
}

//==================================================================================================
std::optional<Json> JsonParser::parse_array()
{
    Json array = JsonTraits::array_type();
    ParseState state;

    // Discard the opening bracket, which has already been peeked.
    discard();

    auto parse_state =
        std::bind(&JsonParser::done_parsing_object_or_array, this, Token::CloseBracket);

    while ((state = parse_state()) == ParseState::KeepParsing)
    {
        if (array && ((state = consume_comma(parse_state)) != ParseState::KeepParsing))
        {
            break;
        }

        if (std::optional<Json> value = parse_json(); value)
        {
            array[array.size()] = std::move(value.value());
        }
        else
        {
            return std::nullopt;
        }
    }

    return (state == ParseState::Invalid) ? std::nullopt : std::optional<Json>(array);
}

//==================================================================================================
JsonParser::ParseState JsonParser::done_parsing_object_or_array(const Token &end_token)
{
    if (consume_whitespace_and_comments() == ParseState::Invalid)
    {
        return ParseState::Invalid;
    }

    const Token token = m_stream->peek<Token>();

    if (token == end_token)
    {
        discard();
        return ParseState::StopParsing;
    }
    else if (token == Token::EndOfFile)
    {
        return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
std::optional<Json> JsonParser::parse_value()
{
    const bool is_string = m_stream->peek<Token>() == Token::Quote;

    auto json_type = is_string ? JsonType::JsonString : JsonType::Other;
    JsonTraits::string_type value;

    if (consume_value(json_type, value) == JsonType::Invalid)
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

    return std::nullopt;
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_token(const Token &token)
{
    consume_whitespace();

    if (const Token parsed = consume(); parsed != token)
    {
        JLOG(
            "Unexpected character '%c', was expecting '%c'",
            static_cast<JsonTraits::string_type::value_type>(parsed),
            static_cast<JsonTraits::string_type::value_type>(token));

        return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_comma(const ParseStateGetter &parse_state)
{
    if (consume_token(Token::Comma) == ParseState::Invalid)
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
JsonParser::JsonType JsonParser::consume_value(JsonType type, JsonTraits::string_type &value)
{
    const bool is_string = type == JsonType::JsonString;

    Json::stream_type parsing;
    Token token;

    auto stop_parsing = [&token, &is_string, this]() -> bool
    {
        if (is_string)
        {
            return token == Token::Quote;
        }

        return is_whitespace(token) || (token == Token::Comma) || (token == Token::Solidus) ||
            (token == Token::CloseBracket) || (token == Token::CloseBrace);
    };

    if (is_string && (consume_token(Token::Quote) == ParseState::Invalid))
    {
        return JsonType::Invalid;
    }

    while (((token = m_stream->peek<Token>()) != Token::EndOfFile) && !stop_parsing())
    {
        parsing << static_cast<Json::stream_type::char_type>(token);
        discard();

        // Blindly ignore escaped symbols, the Json class will check whether they are valid. Just
        // read at least one more symbol to prevent breaking out of the loop too early if the next
        // symbol is a quote.
        if (is_string && (token == Token::ReverseSolidus))
        {
            if ((token = consume()) == Token::EndOfFile)
            {
                JLOG("Expected character after reverse solidus");
                return JsonType::Invalid;
            }

            parsing << static_cast<Json::stream_type::char_type>(token);
        }
    }

    if (is_string && (consume_token(Token::Quote) == ParseState::Invalid))
    {
        return JsonType::Invalid;
    }

    value = parsing.str();
    return type;
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_whitespace_and_comments()
{
    consume_whitespace();

    while (m_stream->peek<Token>() == Token::Solidus)
    {
        if (consume_comment() == ParseState::Invalid)
        {
            return ParseState::Invalid;
        }

        consume_whitespace();
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
void JsonParser::consume_whitespace()
{
    Token token;

    while (((token = m_stream->peek<Token>()) != Token::EndOfFile) && is_whitespace(token))
    {
        discard();
    }
}

//==================================================================================================
JsonParser::ParseState JsonParser::consume_comment()
{
    if (!is_feature_allowed(Features::AllowComments))
    {
        JLOG("Found comment, but Features::AllowComments is not enabled");
        return ParseState::Invalid;
    }

    // Discard the opening solidus, which has already been peeked.
    discard();

    Token token;

    switch (token = consume())
    {
        case Token::Solidus:
            do
            {
                token = consume();
            } while ((token != Token::EndOfFile) && (token != Token::NewLine));

            break;

        case Token::Asterisk:
        {
            bool parsing_comment = true;

            do
            {
                token = consume();

                if ((token == Token::Asterisk) && (m_stream->peek<Token>() == Token::Solidus))
                {
                    parsing_comment = false;
                    discard();
                    break;
                }
            } while (token != Token::EndOfFile);

            if (parsing_comment)
            {
                return ParseState::Invalid;
            }

            break;
        }

        default:
            JLOG(
                "Invalid start sequence for comments: '/%c'",
                static_cast<JsonTraits::string_type::value_type>(token));
            return ParseState::Invalid;
    }

    return ParseState::KeepParsing;
}

//==================================================================================================
JsonParser::Token JsonParser::consume()
{
    const Token token = m_stream->get<Token>();

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
void JsonParser::discard()
{
    FLY_UNUSED(consume());
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
