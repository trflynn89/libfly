#include "fly/parser/exceptions.h"

#include <cctype>

#include "fly/logger/logger.h"
#include "fly/string/string.h"

namespace fly {

//==============================================================================
ParserException::ParserException(
    const std::string &file,
    int line,
    const std::string &message
) :
    m_message(String::Format(
        "ParserException: Error parsing %s at [line %d]: %s",
        file, line, message
    ))
{
    LOGW(-1, "%s", m_message);
}

//==============================================================================
ParserException::ParserException(
    const std::string &file,
    int line,
    int column,
    const std::string &message
) :
    m_message(String::Format(
        "ParserException: Error parsing %s at [line %d, column %d]: %s",
        file, line, column, message
    ))
{
    LOGW(-1, "%s", m_message);
}

//==============================================================================
const char *ParserException::what() const noexcept
{
    return m_message.c_str();
}

//==============================================================================
UnexpectedCharacterException::UnexpectedCharacterException(
    const std::string &file,
    int line,
    int column,
    int c
) :
    ParserException(file, line, column, (std::isprint(c) ?
        String::Format("Unexpected character '%c' (%x)", char(c), c) :
        String::Format("Unexpected character '%x'", c)
    ))
{
}

//==============================================================================
BadConversionException::BadConversionException(
    const std::string &file,
    int line,
    int column,
    const std::string &value
) :
    ParserException(file, line, column,
        String::Format("Could not convert '%s' to a value", value)
    )
{
}

}
