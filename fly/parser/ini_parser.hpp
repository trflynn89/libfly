#pragma once

#include "fly/parser/parser.hpp"
#include "fly/types/json/json.hpp"

#include <istream>
#include <optional>
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
     * @param stream Stream holding the contents to parse.
     *
     * @return If successful, the parsed values. Otherwise, an unitialized value.
     */
    std::optional<Json> parse_internal(std::istream &stream) override;

private:
    /**
     * Enumeration to indicate the result of a call to IniParser::trim.
     */
    enum class TrimResult
    {
        // The character to be trimmed was found at one end of the string but not the other.
        Imbalanced,

        // The string has been trimmed.
        Trimmed,

        // The character to be trimmed was not found at either end of the string.
        Untrimmed,
    };

    /**
     * Parse a line containing a section name.
     *
     * @param line Line containing the section.
     *
     * @return If successful, the parsed section name. Otherwise, an unitialized value.
     */
    std::optional<std::string> on_section(const std::string &line);

    /**
     * Parse a line containing a name/value pair.
     *
     * @param section Section containing the pair.
     * @param line Line containing the pair.
     *
     * @return If successful, the parsed name/value pair. Otherwise, an unitialized value.
     */
    std::optional<std::pair<std::string, std::string>> on_value(const std::string &line);

    /**
     * If the given string begins and ends with the given character, remove that character from each
     * end of the string.
     *
     * @param str The string the trim.
     * @param ch The character to look for.
     *
     * @return The result of the trim operation.
     */
    TrimResult trim_value(std::string &str, char ch) const;

    /**
     * If the given string begins with the first given character and ends with the second given
     * character, remove those characters from each end of the string.
     *
     * @param str The string the trim.
     * @param start The character to look for at the beginning of the string.
     * @param end The character to look for at the end of the string.
     *
     * @return The result of the trim operation.
     */
    TrimResult trim_value(std::string &str, char start, char end) const;
};

} // namespace fly
