#pragma once

#include <istream>
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
     * Parse a string and retrieve parsed values.
     *
     * @param string String contents to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the string.
     */
    Json Parse(const std::string &);

    /**
     * Parse a file and retrieve parsed values.
     *
     * @param string Directory containing the file to parse.
     * @param string Name of the file to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    Json Parse(const std::string &, const std::string &);

protected:
    /**
     * Parse a stream and retrieve the parsed values.
     *
     * @param istream Stream holding the contents to parse.
     *
     * @throws ParserException Thrown if an error occurs parsing the stream.
     */
    virtual Json ParseInternal(std::istream &) = 0;

    int m_line;
    int m_column;
};

}
