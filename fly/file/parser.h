#pragma once

#include <exception>
#include <string>
#include <utility>
#include <vector>

#include <fly/fly.h>

namespace fly {

DEFINE_CLASS_PTRS(Parser);

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
     * A parsed name-value pair.
     */
    typedef std::pair<std::string, std::string> Value;

    /**
     * A list of parsed name-value pairs.
     */
    typedef std::vector<Value> ValueList;

    /**
     * Constructor.
     *
     * @param string Directory containing the file to parse.
     * @param string Name of the file to parse.
     */
    FLY_API Parser(const std::string &, const std::string &);

    /**
     * Parse the configured file and store parsed values.
     *
     * @throws ParserException Thrown if an error occurs parsing the file.
     */
    FLY_API virtual void Parse() = 0;

    /**
     * Get a section's parsed values.
     *
     * @param string The name of the section containing the values.
     *
     * @return A list of parsed values.
     */
    FLY_API virtual ValueList GetValues(const std::string &) const = 0;

protected:
    const std::string m_path;
    const std::string m_file;

    int m_line;
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
     * @return A C-string representing this exception.
     */
    virtual const char *what() const noexcept;

private:
    const std::string m_message;
};

}
