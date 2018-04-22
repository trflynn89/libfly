#pragma once

#include <fstream>
#include <stack>
#include <string>

#include "fly/fly.h"
#include "fly/parser/parser.h"

namespace fly {

FLY_CLASS_PTRS(Json);
FLY_CLASS_PTRS(JsonParser);

/**
 * Implementation of the Parser interface for .json files.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2017
 */
class JsonParser : public Parser
{
    enum class Token
    {
        NewLine = 0x0A, // \n
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

public:
    /**
     * Constructor.
     *
     * @param string Directory containing the file to parse.
     * @param string Name of the file to parse.
     */
    JsonParser(const std::string &, const std::string &);

protected:
    /**
     * Parse the configured file and store parsed values.
     *
     * @param ifstream Stream holding the open file.
     *
     * @throws ParserException If an error occurs parsing the file.
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     * @throws BadConversionException If a parsed object was invalid.
     */
    virtual void ParseInternal(std::ifstream &);

private:
    /**
     * Handle the start of an object or array.
     *
     * @param int The current parsed character ({ or [).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onStartBraceOrBracket(Token, int);

    /**
     * Handle the end of an object or array.
     *
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
     * Handle a new line character.
     *
     * @param int The current parsed character (\n).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onNewLine(int);

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
     * @param int The current parsed character.
     * @param int The next parsed character (or EOF).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     */
    void onCharacter(Token, int, std::ifstream &);

    /**
     * Push a parsed character onto the parsing stream.
     *
     * @param int The current parsed character.
     */
    void pushValue(int);

    /**
     * Store the current value of the parsing stream as a JSON object. Ensures
     * that the syntax of the value is compliant with http://www.json.org.
     *
     * @return bool True if a value was stored.
     *
     * @throws BadConversionException If a parsed object was invalid.
     */
    bool popValue();

    std::stack<State> m_states;

    Json *m_pValue;
    std::stack<Json *> m_pParents;

    Json::stream_type m_parsing;
    bool m_parsingString;
    bool m_parsedString;
    bool m_expectingValue;
};

}
