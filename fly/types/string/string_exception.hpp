#pragma once

#include "fly/types/string/detail/string_traits.hpp"

#include <cstdint>
#include <exception>
#include <string>

namespace fly {

/**
 * Generic exception to be raised for errors operating on BasicString types.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version June 6, 2020
 */
class StringException : public std::exception
{
public:
    /**
     * @return C-string representing this exception.
     */
    virtual const char *what() const noexcept override;

protected:
    /**
     * Constructor for subclasses.
     *
     * @param class_name Name of the subclass.
     * @param message Message indicating what error was encountered.
     */
    StringException(const char *class_name, std::string &&message) noexcept;

private:
    const std::string m_message;
};

/**
 * Exception to be raised for errors encountered parsing escaped unicode
 * sequences.
 */
class UnicodeException : public StringException
{
public:
    /**
     * Constructor.
     *
     * @param message Message indicating what error was encountered.
     */
    explicit UnicodeException(const char *message) noexcept;

    /**
     * Constructor.
     *
     * @param message Message indicating what error was encountered.
     * @param arg1 First argument to format the exception message with.
     */
    UnicodeException(const char *message, std::uint32_t arg1) noexcept;

    /**
     * Constructor.
     *
     * @param message Message indicating what error was encountered.
     * @param arg1 First argument to format the exception message with.
     * @param arg2 Second argument to format the exception message with.
     */
    UnicodeException(const char *message, std::uint32_t arg1, std::uint32_t arg2) noexcept;
};

} // namespace fly
