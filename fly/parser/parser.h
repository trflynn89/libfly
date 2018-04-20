#pragma once

#include <exception>
#include <fstream>
#include <shared_mutex>
#include <string>

#include "fly/fly.h"
#include "fly/types/json.h"

namespace fly {

FLY_CLASS_PTRS(Parser);

/**
 * Virtual interface to parse a file. Parsers for specific file formats should
 * inherit from this class.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 18, 2016
 */
class Parser
{
public:
    /**
     * Constructor.
     *
     * @param string Directory containing the file to parse.
     * @param string Name of the file to parse.
     */
    Parser(const std::string &, const std::string &);

    /**
     * Parse the configured file and store parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    void Parse();

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
     * Parse the configured file and store parsed values.
     *
     * @param ifstream Stream holding the open file.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    virtual void ParseInternal(std::ifstream &) = 0;

    const std::string m_path;
    const std::string m_file;

    Json m_values;

    int m_line;
    int m_column;

private:
    mutable std::shared_timed_mutex m_valuesMutex;
};

/**
 * Exception to be raised if an error was encountered parsing a file.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 16, 2016
 */
class ParserException : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param string Name of the file failed to parse parse.
     * @param int Line number in file where error was encountered.
     * @param string Message indicating what error was encountered.
     */
    ParserException(const std::string &, int, const std::string &);

    /**
     * Constructor.
     *
     * @param string Name of the file failed to parse parse.
     * @param int Line number in file where error was encountered.
     * @param int Column number in line where error was encountered.
     * @param string Message indicating what error was encountered.
     */
    ParserException(const std::string &, int, int, const std::string &);

    /**
     * @return A C-string representing this exception.
     */
    virtual const char *what() const noexcept;

private:
    const std::string m_message;
};

}
