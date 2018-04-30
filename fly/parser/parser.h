#pragma once

#include <istream>
#include <shared_mutex>
#include <string>

#include "fly/fly.h"
#include "fly/types/json.h"

namespace fly {

FLY_CLASS_PTRS(Parser);

/**
 * Virtual interface to parse a file or string. Parsers for specific formats
 * should inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class Parser
{
public:
    /**
     * Constructor.
     */
    Parser();

    /**
     * Parse a string and store parsed values.
     *
     * @param string String contents to parse.
     *
     * @throws ParserException Thrown if an error occurs parsing the string.
     */
    void Parse(const std::string &);

    /**
     * Parse a file and store parsed values.
     *
     * @param string Directory containing the file to parse.
     * @param string Name of the file to parse.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    void Parse(const std::string &, const std::string &);

    /**
     * Get the entire set of parsed values, stored as a JSON object.
     *
     * @return Json The parsed values.
     */
    Json GetValues() const;

    /**
     * Try to get a sub-object from the set of parsed values. If the given
     * key doesn't exist, returns a NULL JSON object.
     *
     * @param string The key to retrieve.
     *
     * @return The parsed values.
     */
    Json GetValues(const std::string &) const;

protected:
    /**
     * Parse a stream and store the parsed values.
     *
     * @param istream Stream holding the contents to parse.
     *
     * @throws ParserException Thrown if an error occurs parsing the stream.
     */
    virtual void ParseInternal(std::istream &) = 0;

    Json m_values;

    int m_line;
    int m_column;

private:
    mutable std::shared_timed_mutex m_valuesMutex;
};

}
