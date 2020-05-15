#include "fly/parser/exceptions.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/string/string.hpp"

#include <cctype>

namespace fly {

//==============================================================================
ParserException::ParserException(
    std::uint32_t line,
    const std::string &message) noexcept :
    m_message(String::format(
        "ParserException: Error parsing at [line %u]: %s",
        line,
        message))
{
    LOGW("%s", m_message);
}

//==============================================================================
ParserException::ParserException(
    std::uint32_t line,
    std::uint32_t column,
    const std::string &message) noexcept :
    m_message(String::format(
        "ParserException: Error parsing at [line %u, column %u]: %s",
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
    std::uint32_t line,
    std::uint32_t column,
    int ch) noexcept :
    ParserException(
        line,
        column,
        std::isprint(ch) ?
            String::format("Unexpected character '%c' (%x)", char(ch), ch) :
            String::format("Unexpected character '%x'", ch))
{
}

//==============================================================================
BadConversionException::BadConversionException(
    std::uint32_t line,
    std::uint32_t column,
    const std::string &value) noexcept :
    ParserException(
        line,
        column,
        String::format("Could not convert '%s' to a value", value))
{
}

} // namespace fly
