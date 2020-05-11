#pragma once

#include <exception>
#include <string>

namespace fly {

/**
 * Exception to be raised if an error was encountered while parsing.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 21, 2018
 */
class ParserException : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param line Line number where error was encountered.
     * @param message Message indicating what error was encountered.
     */
    ParserException(int line, const std::string &message) noexcept;

    /**
     * Constructor.
     *
     * @param line Line number where error was encountered.
     * @param column Column number in line where error was encountered.
     * @param message Message indicating what error was encountered.
     */
    ParserException(int line, int column, const std::string &message) noexcept;

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
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 21, 2018
 */
class UnexpectedCharacterException : public ParserException
{
public:
    /**
     * Constructor.
     *
     * @param line Line number where error was encountered.
     * @param column Column number in line where error was encountered.
     * @param ch Unexpected character code.
     */
    UnexpectedCharacterException(int line, int column, int ch) noexcept;
};

/**
 * Exception to be raised if a value could not be converted as expected.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 21, 2018
 */
class BadConversionException : public ParserException
{
public:
    /**
     * Constructor.
     *
     * @param line Line number where error was encountered.
     * @param column Column number in line where error was encountered.
     * @param value The unconvertable value.
     */
    BadConversionException(
        int line,
        int column,
        const std::string &value) noexcept;
};

} // namespace fly
