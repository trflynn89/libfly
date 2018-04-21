#pragma once

#include <exception>
#include <string>

namespace fly {

/**
 * Exception to be raised if an error was encountered parsing a file.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version April 21, 2018
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

/**
 * Exception to be raised if an unexpected character was encountered parsing a
 * file.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version April 21, 2018
 */
class UnexpectedCharacterException : public ParserException
{
public:
    /**
     * Constructor.
     *
     * @param string Name of the file failed to parse parse.
     * @param int Line number in file where error was encountered.
     * @param int Column number in line where error was encountered.
     * @param int Unexpected character code.
     */
    UnexpectedCharacterException(const std::string &, int, int, int);
};

/**
 * Exception to be raised if a value could not be converted as expected.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version April 21, 2018
 */
class BadConversionException : public ParserException
{
public:
    /**
     * Constructor.
     *
     * @param string Name of the file failed to parse parse.
     * @param int Line number in file where error was encountered.
     * @param int Column number in line where error was encountered.
     * @param string The unconvertable value.
     */
    BadConversionException(const std::string &, int, int, const std::string &);
};

}
