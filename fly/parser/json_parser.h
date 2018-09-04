#pragma once

#include <fstream>
#include <stack>
#include <string>

#include "fly/fly.h"
#include "fly/parser/parser.h"
#include "fly/types/json.h"

namespace fly {

FLY_CLASS_PTRS(JsonParser);

/**
 * Implementation of the Parser interface for the .json format.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2017
 */
class JsonParser : public Parser
{
    enum class Token
    {
        Tab = 0x09, // \t
        NewLine = 0x0A, // \n
        CarriageReturn = 0x0D, // \r
        Space = 0x20, // <space>

        Quote = 0x22, // "
        Comma = 0x2c, // ,
        Colon = 0x3a, // :

        StartBracket = 0x5b, // [
        CloseBracket = 0x5d, // ]

        StartBrace = 0x7b, // {
        CloseBrace = 0x7d, // }

        ReverseSolidus = 0x5C, /* \ */
    };

    enum class State
    {
        NoState,
        ParsingObject,
        ParsingArray,
        ParsingName,
        ParsingValue,
        ParsingColon,
        ParsingComma
    };

protected:
    /**
     * Parse a stream and retrieve the parsed values.
     *
     * @param istream Stream holding the contents to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException If an error occurs parsing the stream.
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     * @throws BadConversionException If a parsed object was invalid.
     */
    Json ParseInternal(std::istream &) override;

private:
    /**
     * Handle a whitespace character.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character.
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onWhitespace(Token, int);

    /**
     * Handle the start of an object or array.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character ({ or [).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onStartBraceOrBracket(Token, int);

    /**
     * Handle the end of an object or array.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character (} or ]).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     * @throws BadConversionException If a parsed object was invalid.
     */
    void onCloseBraceOrBracket(Token, int);

    /**
     * Handle the start or end of a string, for either a name or value.
     *
     * @param int The current parsed character (").
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onQuotation(int);

    /**
     * Handle a colon between name-value pairs.
     *
     * @param int The current parsed character (:).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onColon(int);

    /**
     * Handle a comma, either in an array or after an object.
     *
     * @param int The current parsed character (,).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     * @throws BadConversionException If a parsed object was invalid.
     */
    void onComma(int);

    /**
     * Handle any other character. If the character is a reverse solidus, accept
     * the next character as well.
     *
     * @param Token The current parsed token.
     * @param int The current parsed character.
     * @param istream Stream holding the contents to parse.
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onCharacter(Token, int, std::istream &);

    /**
     * Push a parsed character onto the parsing stream.
     *
     * @param int The current parsed character.
     */
    void pushValue(int);

    /**
     * Retrieve and clear the current value of the parsing stream.
     *
     * @return string The current value of the parsing stream.
     */
    std::string popValue();

    /**
     * Store the current value of the parsing stream as a JSON object. Ensures
     * that the syntax of the value is compliant with http://www.json.org.
     *
     * @return bool True if a value was stored.
     *
     * @throws BadConversionException If a parsed object was invalid.
     */
    bool storeValue();

    /**
     * Validate that a parsed number is compliant with http://www.json.org.
     *
     * @param string The parsed number to validate.
     * @param bool Set to true if the parsed number is a floating point.
     * @param bool Set to true if the parsed number is signed.
     *
     * @throws BadConversionException If a parsed number was invalid.
     */
    void validateNumber(const std::string &, bool &, bool &) const;

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

}
