#pragma once

#include <exception>
#include <string>

namespace fly {

class Json;

/**
 * Exception to be raised if an error was encountered creating, accessing, or
 * modifying a Json instance.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version September 24, 2017
 */
class JsonException : public std::exception
{
public:
    /**
     * Constructor.
     *
     * @param message Message indicating what error was encountered.
     */
    JsonException(const std::string &message) noexcept;

    /**
     * Constructor.
     *
     * @param json The Json instance for which the error was encountered.
     * @param message Message indicating what error was encountered.
     */
    JsonException(const Json &json, const std::string &message) noexcept;

    /**
     * @return C-string representing this exception.
     */
    virtual const char *what() const noexcept override;

private:
    const std::string m_message;
};

} // namespace fly
