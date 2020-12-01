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
    JsonParser() = default;

    /**
     * Constructor. Create a parser with the specified features.
     *
     * @param features The extra features to allow.
     */
    explicit JsonParser(const Features features) noexcept;

protected:
    /**
     * Parse a single complete JSON value from the stream.
     *
     * @return If successful, the parsed JSON value. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_internal() override;

private:
    /**
     * ASCII codes for special JSON tokens.
     */
    enum class Token : std::char_traits<JsonTraits::char_type>::int_type
    {
        EndOfFile = std::char_traits<JsonTraits::char_type>::eof(),

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
     * Enumeration to indicate the type of a JSON number to parse.
     */
    enum class NumberType : std::uint8_t
    {
        Invalid,
        SignedInteger,
        UnsignedInteger,
        FloatingPoint,
    };

    /**
     * Enumeration to indicate the current status of parsing the JSON value.
     */
    enum class ParseState : std::uint8_t
    {
        Invalid,
        StopParsing,
        KeepParsing,
    };

    using ParseStateGetter = std::function<ParseState()>;

    /**
     * Parse a complete JSON value from the stream. May be called recursively for nested values.
     *
     * @return If successful, the parsed JSON value. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_json();

    /**
     * Parse a JSON object from the stream.
     *
     * @return If successful, the parsed JSON object. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_object();

    /**
     * Parse a JSON array from the stream.
     *
     * @return If successful, the parsed JSON array. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_array();

    /**
     * Determine whether parsing a JSON object or array is complete.
     *
     * @param token Token indicating the end of the JSON object (}) or array (]).
     *
     * @return The new parsing state.
     */
    ParseState done_parsing_object_or_array(const Token &end_token);

    /**
     * Parse a JSON string from the stream. Escaped symbols are preserved in the string, and the
     * returned value does not contain its surrounding quotes.
     *
     * This returns an actual string rather than a JSON value because some callers prefer the string
     * type (e.g. to pass the string as the key of a JSON object).
     *
     * @return If successful, the parsed JSON string. Otherwise, an unitialized value.
     */
    std::optional<JsonTraits::string_type> parse_quoted_string();

    /**
     * Parse a JSON number, boolean, or null value from the stream.
     *
     * @return If successful, the parsed JSON value. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_value();

    /**
     * Extract a single symbol from the stream. Ensure that symbol is equal to an expected token.
     *
     * @return The new parsing state.
     */
    ParseState consume_token(const Token &token);

    /**
     * Extract a comma from the stream. Handles any trailing commas, allowing a single trailing
     * comma if enabled in the feature set.
     *
     * @param parse_state Callback to indicate whether the calling parser should stop parsing.
     *
     * @return The new parsing state.
     */
    ParseState consume_comma(const ParseStateGetter &parse_state);

    /**
     * Extract a number, boolean, or null value from the stream.
     *
     * @return The JSON value that was parsed as a string.
     */
    JsonTraits::string_type consume_value();

    /**
     * Extract all consecutive whitespace symbols and comments (if enabled in the feature set) from
     * the stream. The first non-whitespace, non-comment symbol is left on the stream.
     *
     * @return The new parsing state.
     */
    ParseState consume_whitespace_and_comments();

    /**
     * Extract all consecutive whitespace symbols from the stream until a non- whitespace symbol is
     * encountered. The non-whitespace symbol is left on the stream.
     */
    void consume_whitespace();

    /**
     * Extract a single- or multi-line comment from the stream, if enabled in the feature set.
     *
     * @return The new parsing state.
     */
    ParseState consume_comment();

    /**
     * Extract the next symbol in the stream.
     *
     * @return The extracted symbol.
     */
    Token consume();

    /**
     * Extract and discard the next symbol in the stream.
     */
    void discard();

    /**
     * Validate that a parsed number is valid and interpret its numeric JSON type.
     *
     * @param value String storing the parsed number to validate.
     *
     * @return The interpreted JSON value type.
     */
    NumberType validate_number(const JsonTraits::string_type &value) const;

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

    const Features m_features {Features::Strict};
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
