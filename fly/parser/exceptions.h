#pragma once

#include <exception>
#include <string>

namespace fly {

/**
 * Exception to be raised if an error was encountered while parsing.
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
     * @param int Line number where error was encountered.
     * @param string Message indicating what error was encountered.
     */
    ParserException(int, const std::string &);

    /**
     * Constructor.
     *
     * @param int Line number where error was encountered.
     * @param int Column number in line where error was encountered.
     * @param string Message indicating what error was encountered.
     */
    ParserException(int, int, const std::string &);

    /**
     * @return A C-string representing this exception.
     */
    const char *what() const noexcept override;

private:
    const std::string m_message;
};

/**
 * Exception to be raised if an unexpected character was encountered.
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
     * @param int Line number where error was encountered.
     * @param int Column number in line where error was encountered.
     * @param int Unexpected character code.
     */
    UnexpectedCharacterException(int, int, int);
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
     * @param int Line number where error was encountered.
     * @param int Column number in line where error was encountered.
     * @param string The unconvertable value.
     */
    BadConversionException(int, int, const std::string &);
};

}
