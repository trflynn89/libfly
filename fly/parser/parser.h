#pragma once

#include "fly/types/json.h"

#include <filesystem>
#include <istream>
#include <string>

namespace fly {

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
     * Destructor.
     */
    virtual ~Parser() = default;

    /**
     * Parse a string and retrieve parsed values.
     *
     * @param string String contents to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the string.
     */
    Json ParseString(const std::string &) noexcept(false);

    /**
     * Parse a file and retrieve parsed values.
     *
     * @param path Path to the file to parse.
     *
     * @return Json The parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    Json ParseFile(const std::filesystem::path &) noexcept(false);

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
    virtual Json ParseInternal(std::istream &) noexcept(false) = 0;

    int m_line;
    int m_column;

private:
    /**
     * Before passing a stream to the parser implementation, discard any byte
     * order marks (supports UTF-8, UTF-16, and UTF-32).
     *
     * @param istream Stream holding the contents to parse.
     */
    void consumeByteOrderMark(std::istream &) const noexcept;
};

} // namespace fly
