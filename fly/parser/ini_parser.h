#pragma once

#include <fstream>
#include <string>

#include "fly/fly.h"
#include "fly/parser/parser.h"

namespace fly {

FLY_CLASS_PTRS(IniParser);

/**
 * Implementation of the Parser interface for the .ini format.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class IniParser : public Parser
{
public:
    /**
     * Constructor.
     */
    IniParser();

protected:
    /**
     * Parse a stream and store the parsed values.
     *
     * @param istream Stream holding the contents to parse.
     *
     * @throws ParserException Thrown if an error occurs parsing the stream.
     */
    virtual void ParseInternal(std::istream &);

private:
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
     *                         the string, but not the other.
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
     *                         was found, but not the other.
     */
    bool trimValue(std::string &, char, char) const;
};

}
