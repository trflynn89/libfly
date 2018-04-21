#pragma once

#include <fstream>
#include <sstream>
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
    enum JsonToken
    {
        JSON_NEW_LINE = 0x0A, // \n
        JSON_SPACE = 0x20, // <space>

        JSON_QUOTE = 0x22, // "
        JSON_COMMA = 0x2c, // ,
        JSON_COLON = 0x3a, // :

        JSON_START_BRACKET = 0x5b, // [
        JSON_CLOSE_BRACKET = 0x5d, // ]

        JSON_START_BRACE = 0x7b, // {
        JSON_CLOSE_BRACE = 0x7d, // }

        JSON_REVERSE_SOLIDUS = 0x5C, /* \ */
    };

    enum JsonState
    {
        JSON_NO_STATE,
        JSON_PARSING_OBJECT,
        JSON_PARSING_ARRAY,
        JSON_PARSING_NAME,
        JSON_PARSING_VALUE,
        JSON_PARSING_COLON,
        JSON_PARSING_COMMA
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
    void onStartBraceOrBracket(int);

    /**
     * Handle the end of an object or array.
     *
     * @param int The current parsed character (} or ]).
     *
     * @throws UnexpectedCharacterException If a parsed character was unexpected.
     * @throws BadConversionException If a parsed object was invalid.
     */
    void onCloseBraceOrBracket(int);

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
    void onCharacter(int, std::ifstream &);

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

    std::stack<JsonState> m_states;

    Json *m_pValue;
    std::stack<Json *> m_pParents;

    std::stringstream m_parsing;
    bool m_parsingString;
    bool m_parsedString;
    bool m_expectingValue;
};

}
