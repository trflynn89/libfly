#include "fly/parser/exceptions.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/string/string.hpp"

#include <cctype>

namespace fly {

//==============================================================================
ParserException::ParserException(
    int line,
    const std::string &message) noexcept :
    m_message(String::Format(
        "ParserException: Error parsing at [line %d]: %s",
        line,
        message))
{
    LOGW("%s", m_message);
}

//==============================================================================
ParserException::ParserException(
    int line,
    int column,
    const std::string &message) noexcept :
    m_message(String::Format(
        "ParserException: Error parsing at [line %d, column %d]: %s",
        line,
        column,
        message))
{
    LOGW("%s", m_message);
}

//==============================================================================
const char *ParserException::what() const noexcept
{
    return m_message.c_str();
}

//==============================================================================
UnexpectedCharacterException::UnexpectedCharacterException(
    int line,
    int column,
    int c) noexcept :
    ParserException(
        line,
        column,
        std::isprint(c) ?
            String::Format("Unexpected character '%c' (%x)", char(c), c) :
            String::Format("Unexpected character '%x'", c))
{
}

//==============================================================================
BadConversionException::BadConversionException(
    int line,
    int column,
    const std::string &value) noexcept :
    ParserException(
        line,
        column,
        String::Format("Could not convert '%s' to a value", value))
{
}

} // namespace fly
