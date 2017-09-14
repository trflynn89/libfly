#pragma once

#include <map>
#include <shared_mutex>
#include <sstream>
#include <string>

#include "fly/fly.h"
#include "fly/parser/json.h"
#include "fly/parser/parser.h"

namespace fly {

DEFINE_CLASS_PTRS(JsonParser);

/**
 * Implementation of the Parser interface for .json files.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 19, 2017
 */
class JsonParser : public Parser
{
    /**
     * A map of parsed section names to that section's list of name-value pairs.
     */
    typedef std::map<std::string, Parser::ValueList> IniSection;

    enum JsonToken
    {
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
        JSON_PARSING_NAME,
        JSON_PARSING_OBJECT,
        JSON_PARSING_VALUE,
        JSON_PARSING_ARRAY
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
     * Get a section's parsed values.
     *
     * @param string The name of the section containing the values.
     *
     * @return A list of parsed values.
     */
    virtual Parser::ValueList GetValues(const std::string &) const;

    /**
     * Get the number of sections that have been parsed.
     *
     * @return The number of parsed sections.
     */
    IniSection::size_type GetSize() const;

    /**
     * Get the number of name/value pairs that have been parsed for a section.
     *
     * @param string The name of the section to count.
     *
     * @return The number of parsed values.
     */
    Parser::ValueList::size_type GetSize(const std::string &) const;

private:
    void onStartBrace();
    void onCloseBrace();
    void onQuotation();

    void onCharacter(const char &);

    JsonState m_state;

    Json m_root;
    Json *m_pValue;

    std::stringstream m_parsing;
    size_t m_objectDepth;

    /**
     * Parse a line containing a section name.
     *
     * @param string Line containing the section.
     *
     * @return The parsed section name.
     */
    std::string onSection(const std::string &);

    /**
     * Parse a line containing a name/value pair.
     *
     * @param string Section containing the pair.
     * @param string Line containing the pair.
     */
    void onValue(const std::string &, const std::string &);

    /**
     * If the given string begins and ends with the given character, remove that
     * character from each end of the string.
     *
     * @param string The string the trim.
     * @param char The character to look for.
     *
     * @return True if the string was trimmed.
     *
     * @throws ParserException Thrown if the character was found at one end of
     *     the string, but not the other.
     */
    bool trimValue(std::string &, char) const;

    /**
     * If the given string begins with the first given character and ends with
     * the second given character, remove those characters from each end of the
     * string.
     *
     * @param string The string the trim.
     * @param char The character to look for at the beginning of the string.
     * @param char The character to look for at the end of the string.
     *
     * @return True if the string was trimmed.
     *
     * @throws ParserException Thrown if the one of the start/end characters
     *     was found, but not the other.
     */
    bool trimValue(std::string &, char, char) const;

    mutable std::shared_timed_mutex m_sectionsMutex;
    IniSection m_sections;
};

}
