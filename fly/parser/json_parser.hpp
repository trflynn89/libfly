#pragma once

#include "fly/parser/parser.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/json/json_traits.hpp"

#include <cstdint>
#include <functional>
#include <istream>
#include <limits>
#include <optional>
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

        // Allows the last value in an object/array to have one trailing comma.
        AllowTrailingComma = 1 << 1,

        // Allow parsing any JSON type, rather than only objects and arrays.
        AllowAnyType = 1 << 2,

        // Allows all of the above features.
        AllFeatures = std::numeric_limits<std::underlying_type_t<Features>>::max()
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
     * @return If successful, the parsed JSON value. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_internal(std::istream &stream) override;

private:
    /**
     * ASCII codes for special JSON tokens.
     */
    enum class Token : std::int16_t
    {
        EndOfFile = std::char_traits<JsonTraits::string_type::value_type>::eof(),

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
     * Enumeration to indicate the type of a JSON value to parse.
     */
    enum class JsonType : std::uint8_t
    {
        Invalid,
        JsonString,
        SignedInteger,
        UnsignedInteger,
        FloatingPoint,
        Other,
    };

    /**
     * Enumeration to indicate the current status of parsing the JSON value.
     */
    enum class ParseState : std::uint8_t
    {
        // A parsing error has occurred and all parsing should stop.
        Invalid,

        // Parsing is complete.
        StopParsing,

        // Parsing is ongoing.
        KeepParsing,
    };

    using ParseStateGetter = std::function<ParseState()>;

    /**
     * Parse a complete JSON value from a stream. May be called recursively for nested values.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, the parsed JSON value. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_json(std::istream &stream);

    /**
     * Parse a JSON object from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, the parsed JSON object. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_object(std::istream &stream);

    /**
     * Parse a JSON array from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, the parsed JSON array. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_array(std::istream &stream);

    /**
     * Determine whether parsing a JSON object or array is complete.
     *
     * @param stream Stream holding the contents to parse.
     * @param token Token indicating the end of the JSON object (}) or array (]).
     *
     * @return The new parsing state.
     */
    ParseState done_parsing_object_or_array(std::istream &stream, const Token &end_token);

    /**
     * Parse a JSON string, number, boolean, or null value from a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, the parsed JSON value. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_value(std::istream &stream);

    /**
     * Extract a single symbol from a stream. Ensure that symbol is equal to an expected token.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The new parsing state.
     */
    ParseState consume_token(std::istream &stream, const Token &token);

    /**
     * Extract a comma from a stream. Handles any trailing commas, allowing a single trailing comma
     * if enabled in the feature set.
     *
     * @param stream Stream holding the contents to parse.
     * @param parse_state Callback to indicate whether the calling parser should stop parsing.
     *
     * @return The new parsing state.
     */
    ParseState consume_comma(std::istream &stream, const ParseStateGetter &parse_state);

    /**
     * Extract a string, number, boolean, or null value from a stream. If parsing a string, escaped
     * symbols are preserved in that string, and the returned value does not contain its surrounding
     * quotes.
     *
     * @param stream Stream holding the contents to parse.
     * @param type The JSON value type to consume.
     * @param value The location to store the parsed value.
     *
     * @return The JSON value type that was parsed. Will be either the type that was provided or
     *         JsonType::Invalid if an error occurred.
     */
    JsonType consume_value(std::istream &stream, JsonType type, JsonTraits::string_type &value);

    /**
     * Extract all consecutive whitespace symbols and comments (if enabled in the feature set) from
     * a stream. The first non-whitespace, non-comment symbol is left on the stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The new parsing state.
     */
    ParseState consume_whitespace_and_comments(std::istream &stream);

    /**
     * Extract all consecutive whitespace symbols from a stream until a non- whitespace symbol is
     * encountered. The non-whitespace symbol is left on the stream.
     *
     * @param stream Stream holding the contents to parse.
     */
    void consume_whitespace(std::istream &stream);

    /**
     * Extract a single- or multi-line comment from a stream, if enabled in the feature set.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The new parsing state.
     */
    ParseState consume_comment(std::istream &stream);

    /**
     * Read the next symbol in a stream without extracting it.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The peeked symbol.
     */
    Token peek(std::istream &stream);

    /**
     * Extract the next symbol in a stream.
     *
     * @param stream Stream holding the contents to parse.
     *
     * @return The extracted symbol.
     */
    Token consume(std::istream &stream);

    /**
     * Extract and discard the next symbol in a stream.
     *
     * @param stream Stream holding the contents to parse.
     */
    void discard(std::istream &stream);

    /**
     * Validate that a parsed number is valid and interpret its numeric JSON type.
     *
     * @param value String storing the parsed number to validate.
     *
     * @return The interpreted JSON value type.
     */
    JsonType validate_number(const JsonTraits::string_type &value) const;

    /**
     * Check if a symbol is a whitespace symbol.
     *
     * @param token The token to inspect.
     *
     * @return True if the symbol is whitespace.
     */
    bool is_whitespace(const Token &token) const;

    /**
     * Check if a feature has been allowed.
     *
     * @param feature The feature to check.
     *
     * @return True if the feature is allowed.
     */
    bool is_feature_allowed(Features feature) const;

    const Features m_features;
};

/**
 * Combine two Features instances into a single instance via bitwise-and.
 */
JsonParser::Features operator&(JsonParser::Features a, JsonParser::Features b);

/**
 * Combine two Features instances into a single instance via bitwise-or.
 */
JsonParser::Features operator|(JsonParser::Features a, JsonParser::Features b);

} // namespace fly
