#pragma once

#include "fly/parser/exceptions.hpp"
#include "fly/parser/parser.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/json/json_traits.hpp"

#include <cstdint>
#include <functional>
#include <istream>
#include <limits>
#include <string>

namespace fly {

/**
 * Implementation of the Parser interface for the .json format.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 13, 2020
 */
class JsonParser : public Parser
{
public:
    /**
     * Optional parsing features. May be combined with bitwise and/or operators.
     */
    enum class Features : std::uint8_t
    {
        // Strict compliance with https://www.json.org.
        Strict = 0,

        // Allows single-line (//) and multi-line (/* */) comments.
        AllowComments = 1 << 0,

        // Allows the last element in an object/array to have one trailing comma
        AllowTrailingComma = 1 << 1,

        // Allows all of the above features
        AllFeatures =
            std::numeric_limits<std::underlying_type_t<Features>>::max()
    };

    /**
     * Constructor. Create a parser with strict compliance.
     */
    JsonParser() noexcept;

    /**
     * Constructor. Create a parser with the specified features.
     *
     * @param features The extra features to allow.
     */
    explicit JsonParser(const Features features) noexcept;

protected:
    /**
     * Parse a single complete JSON value from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The parsed JSON value.
     *
     * @throws ParserException The parsed JSON value is invalid, or the stream
     *         contains more than a JSON value.
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     * @throws BadConversionException A parsed value could not be converted to
     *         a JSON type.
     */
    Json parse_internal(std::istream &stream) noexcept(false) override;

private:
    /**
     * ASCII codes for special JSON tokens.
     */
    enum class Token : std::int16_t
    {
        EndOfFile =
            std::char_traits<JsonTraits::string_type::value_type>::eof(),

        Tab = 0x09, // \t
        NewLine = 0x0a, // \n
        VerticalTab = 0x0b, // \v
        CarriageReturn = 0x0d, // \r
        Space = 0x20, // <space>

        Quote = 0x22, // "
        Asterisk = 0x2a, // *
        Comma = 0x2c, // ,
        Hyphen = 0x2d, // -
        Solidus = 0x2f, // /
        Colon = 0x3a, // :
        ReverseSolidus = 0x5c, /* \ */

        StartBracket = 0x5b, // [
        CloseBracket = 0x5d, // ]

        StartBrace = 0x7b, // {
        CloseBrace = 0x7d, // }
    };

    /**
     * Helper enumeration to indicate the type of a JSON value.
     */
    enum class JsonType : std::uint8_t
    {
        JsonString,
        Other,
    };

    /**
     * Helper enumeration to indicate the numeric type of a JSON number.
     */
    enum class NumericType : std::uint8_t
    {
        SignedInteger,
        UnsignedInteger,
        FloatingPoint,
    };

    /**
     * Helper class to convert an unexpected Token to the value type needed by
     * UnexpectedCharacterException.
     */
    class UnexpectedTokenException : public UnexpectedCharacterException
    {
    public:
        UnexpectedTokenException(
            std::uint32_t line,
            std::uint32_t column,
            JsonParser::Token token);
    };

    /**
     * Parse a complete JSON value from a stream. May be called recursively for
     * nested values.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The parsed JSON value.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     * @throws BadConversionException A parsed value could not be converted to
     *         a JSON type.
     */
    Json parse_json(std::istream &stream) noexcept(false);

    /**
     * Parse a JSON object from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The parsed JSON object.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     * @throws BadConversionException A parsed value could not be converted to
     *         a JSON type.
     */
    Json parse_object(std::istream &stream) noexcept(false);

    /**
     * Parse a JSON array from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The parsed JSON array.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     * @throws BadConversionException A parsed value could not be converted to
     *         a JSON type.
     */
    Json parse_array(std::istream &stream) noexcept(false);

    /**
     * Parse a JSON string, number, boolean, or null value from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The parsed JSON number, boolean, or null value.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     * @throws BadConversionException A parsed value could not be converted to
     *         a JSON type.
     */
    Json parse_value(std::istream &stream) noexcept(false);

    /**
     * Extract a single symbol from a stream. Ensure that symbol is equal to an
     * expected token.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     */
    void
    consume_token(std::istream &stream, const Token &token) noexcept(false);

    /**
     * Extract a comma from a stream. Handles any trailing commas, allowing a
     * single trailing comma if enabled in the feature set.
     *
     * @param stream Stream holding the contents to parse.
     * @param stop_parsing Callback to indicate whether the calling parser
     *        should stop parsing.
     *
     * @return True if the callback indicated parsing is complete.
     *
     * @throws UnexpectedCharacterException A trailing comma was found, but the
     *         feature is not enabled.
     */
    bool consume_comma(
        std::istream &stream,
        const std::function<bool()> &stop_parsing) noexcept(false);

    /**
     * Extract a string, number, boolean, or null value from a stream. If
     * parsing a string, escaped symbols are preserved in that string, and the
     * returned value does not contain its surrounding quotes.
     *
     * @param stream Stream holding the contents to parse.
     * @param type The JSON value type to consume.
     *
     * @return The parsed value as a string.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     */
    JsonTraits::string_type
    consume_value(std::istream &stream, JsonType type) noexcept(false);

    /**
     * Extract all consecutive whitespace symbols and comments (if enabled in
     * the feature set) from a stream. The first non-whitespace, non-comment
     * symbol is left on the stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected.
     */
    void consume_whitespace_and_comments(std::istream &stream) noexcept(false);

    /**
     * Extract all consecutive whitespace symbols from a stream until a non-
     * whitespace symbol is encountered. The non-whitespace symbol is left on
     * the stream.
     *
     * @param stream Stream holding the contents to parse.
     */
    void consume_whitespace(std::istream &stream) noexcept;

    /**
     * Extract a single- or multi-line comment from a stream, if enabled in the
     * feature set.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @throws UnexpectedCharacterException A parsed symbol was unexpected, or
     *         the comment feature is not enabled.
     */
    void consume_comment(std::istream &stream) noexcept(false);

    /**
     * Read the next symbol in a stream without extracting it.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The peeked symbol.
     */
    Token peek(std::istream &stream) noexcept;

    /**
     * Extract the next symbol in a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The extracted symbol.
     */
    Token consume(std::istream &stream) noexcept;

    /**
     * Extract the next symbol in a stream and discard it.
     *
     * @param stream Stream holding the contents to parse.
     */
    void discard(std::istream &stream) noexcept;

    /**
     * Validate that a parsed number is valid and interpret its numeric JSON
     * type.
     *
     * @param value The parsed number to validate.
     *
     * @return The interpreted numeric type.
     *
     * @throws BadConversionException The parsed number is not valid.
     */
    NumericType validate_number(const JsonTraits::string_type &value) const
        noexcept(false);

    /**
     * Check if a symbol is a whitespace symbol.
     *
     * @param token The token to inspect.
     *
     * @return True if the symbol is whitespace.
     */
    bool is_whitespace(const Token &token) const noexcept;

    /**
     * Check if a feature has been allowed.
     *
     * @param feature The feature to check.
     *
     * @return True if the feature is allowed.
     */
    bool is_feature_allowed(Features feature) const noexcept;

    const Features m_features;
};

/**
 * Combine two Features instances into a single instance via bitwise-and.
 */
JsonParser::Features
operator&(JsonParser::Features a, JsonParser::Features b) noexcept;

/**
 * Combine two Features instances into a single instance via bitwise-or.
 */
JsonParser::Features
operator|(JsonParser::Features a, JsonParser::Features b) noexcept;

} // namespace fly
