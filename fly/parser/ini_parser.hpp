#pragma once

#include "fly/parser/parser.hpp"
#include "fly/types/json/json.hpp"

#include <istream>
#include <string>

namespace fly {

/**
 * Implementation of the Parser interface for the .ini format.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 18, 2016
 */
class IniParser : public Parser
{
protected:
    /**
     * Parse a stream and retrieve the parsed values.
     *
     * @param istream Stream holding the contents to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the stream.
     */
    Json ParseInternal(std::istream &) noexcept(false) override;

private:
    /**
     * Parse a line containing a section name.
     *
     * @param string Line containing the section.
     *
     * @return The parsed section name.
     *
     * @throws ParserException Thrown if the section name is quoted.
     */
    std::string onSection(const std::string &) noexcept(false);

    /**
     * Parse a line containing a name/value pair.
     *
     * @param Json Section containing the pair.
     * @param string Line containing the pair.
     *
     * @throws ParserException Thrown if the value name is quoted, or the line
     *                         both a name and value are not found.
     */
    void onValue(Json &, const std::string &) noexcept(false);

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
    bool trimValue(std::string &, char) const noexcept(false);

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
    bool trimValue(std::string &, char, char) const noexcept(false);
};

} // namespace fly
