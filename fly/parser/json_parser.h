#pragma once

#include <map>
#include <shared_mutex>
#include <sstream>
#include <stack>
#include <string>

#include "fly/fly.h"
#include "fly/parser/json.h"
#include "fly/parser/parser.h"

namespace fly {

FLY_CLASS_PTRS(JsonParser);

/**
 * Implementation of the Parser interface for .json files.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2017
 */
class JsonParser : public Parser
{
public:
    enum JsonToken
    {
        JSON_NEW_LINE = 0x0A, // \n
        JSON_QUOTE = 0x22, // "
        JSON_COMMA = 0x2c, // ,
        JSON_COLON = 0x3a, // :

        JSON_START_BRACKET = 0x5b, // [
        JSON_CLOSE_BRACKET = 0x5d, // ]

        JSON_START_BRACE = 0x7b, // {
        JSON_CLOSE_BRACE = 0x7d, // }
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

    /**
     * Parse the configured file and store parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    virtual void Parse();

    /**
     * Get a copy of the parsed JSON object.
     *
     * @return Json The parsed JSON object.
     */
    Json GetJson() const;

    /**
     * Get a section's parsed values.
     *
     * @param string The name of the section containing the values.
     *
     * @return A list of parsed values.
     */
    virtual Parser::ValueList GetValues(const std::string &) const;

private:
    void onStartBraceOrBracket(const char &, const JsonToken &);
    void onCloseBraceOrBracket(const char &, const JsonToken &);
    void onQuotation(const char &);
    void onComma(const char &);
    void onColon(const char &);

    void onCharacter(const char &);

    bool storeValue();

    std::stack<JsonState> m_states;

    Json m_root;
    Json *m_pValue;
    std::stack<Json *> m_pParents;

    std::stringstream m_parsing;
    bool m_parsingString;
    bool m_parsedString;
    bool m_expectingValue;

    mutable std::shared_timed_mutex m_sectionsMutex;
};

}
