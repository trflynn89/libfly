#pragma once

#include "fly/parser/parser.hpp"
#include "fly/types/json/json.hpp"

#include <cstdint>
#include <istream>
#include <limits>
#include <stack>
#include <string>
#include <type_traits>

namespace fly {

/**
 * Implementation of the Parser interface for the .json format.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 19, 2017
 */
class JsonParser : public Parser
{
    enum class Token : std::uint8_t
    {
        Tab = 0x09, // \t
        NewLine = 0x0a, // \n
        CarriageReturn = 0x0d, // \r
        Space = 0x20, // <space>

        Asterisk = 0x2a, // *
        Quote = 0x22, // "
        Comma = 0x2c, // ,
        Solidus = 0x2f, // /
        Colon = 0x3a, // :

        StartBracket = 0x5b, // [
        CloseBracket = 0x5d, // ]

        StartBrace = 0x7b, // {
        CloseBrace = 0x7d, // }

        ReverseSolidus = 0x5c, /* \ */
    };

    enum class State : std::uint8_t
    {
        NoState,
        ParsingObject,
        ParsingArray,
        ParsingName,
        ParsingValue,
        ParsingColon,
        ParsingComma,
    };

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
     * Constructor. Create a parser with strict https://www.json.org compliance.
     */
    JsonParser() noexcept;

    /**
     * Constructor. Create a parser with the specific features.
     *
     * @param Features The extra features to allow.
     */
    explicit JsonParser(Features) noexcept;

protected:
    /**
     * Parse a stream and retrieve the parsed values.
     *
     * @param istream Stream holding the contents to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException If an error occurs parsing the stream.
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     * @throws BadConversionException A parsed object was invalid.
     */
    Json ParseInternal(std::istream &) noexcept(false) override;

private:
    /**
     * Handle a whitespace character.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character.
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     */
    void onWhitespace(Token, int) noexcept(false);

    /**
     * Handle the start of an object or array.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character ({ or [).
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     */
    void onStartBraceOrBracket(Token, int) noexcept(false);

    /**
     * Handle the end of an object or array.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character (} or ]).
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     * @throws BadConversionException A parsed object was invalid.
     */
    void onCloseBraceOrBracket(Token, int) noexcept(false);

    /**
     * Handle the start or end of a string, for either a name or value.
     *
     * @param int The current parsed character (").
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     */
    void onQuotation(int) noexcept(false);

    /**
     * Handle a colon between name-value pairs.
     *
     * @param int The current parsed character (:).
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     */
    void onColon(int) noexcept(false);

    /**
     * Handle a comma, either in an array or after an object.
     *
     * @param int The current parsed character (,).
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     * @throws BadConversionException A parsed object was invalid.
     */
    void onComma(int) noexcept(false);

    /**
     * Handle the start of a comment, if comments are allowed. Consumes the
     * stream until the end of the comment.
     *
     * @param int The current parsed character.
     * @param istream Stream holding the contents to parse.
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     */
    void onSolidus(int, std::istream &) noexcept(false);

    /**
     * Handle any other character. If the character is a reverse solidus, accept
     * the next character as well.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character.
     * @param istream Stream holding the contents to parse.
     *
     * @throws UnexpectedCharacterException A parsed character was unexpected.
     */
    void onCharacter(Token, int, std::istream &) noexcept(false);

    /**
     * Push a parsed character onto the parsing stream.
     *
     * @param int The current parsed character.
     *
     * @throws UnexpectedCharacterException Any character was unexpected.
     */
    void pushValue(int) noexcept(false);

    /**
     * Retrieve and clear the current value of the parsing stream.
     *
     * @return string The current value of the parsing stream.
     */
    std::string popValue() noexcept;

    /**
     * Store the current value of the parsing stream as a JSON object. Ensures
     * that the syntax of the value is compliant with https://www.json.org.
     *
     * @return bool True if a value was stored.
     *
     * @throws BadConversionException A parsed object was invalid.
     */
    bool storeValue() noexcept(false);

    /**
     * Validate that a parsed number is compliant with https://www.json.org.
     *
     * @param string The parsed number to validate.
     * @param bool Set to true if the parsed number is a floating point.
     * @param bool Set to true if the parsed number is signed.
     *
     * @throws BadConversionException A parsed number was invalid.
     */
    void validateNumber(const std::string &, bool &, bool &) const
        noexcept(false);

    /**
     * Check if a feature has been allowed.
     *
     * @param Features The feature to check.
     *
     * @return True if the feature is allowed.
     */
    bool isFeatureAllowed(Features) const noexcept(false);

    Features m_features;

    std::stack<State> m_states;

    Json *m_pValue;
    std::stack<Json *> m_pParents;

    Json::stream_type m_parsing;

    bool m_parsingStarted;
    bool m_parsingComplete;
    bool m_parsingString;
    bool m_parsedString;
    bool m_expectingValue;
};

/**
 * Combine two Features instances into a single instance via bitwise-and.
 */
JsonParser::Features
operator&(JsonParser::Features, JsonParser::Features) noexcept;

/**
 * Combine two Features instances into a single instance via bitwise-or.
 */
JsonParser::Features
operator|(JsonParser::Features, JsonParser::Features) noexcept;

} // namespace fly
