#include "fly/parser/parser.h"

#include "fly/logger/logger.h"
#include "fly/string/string.h"

namespace fly {

//==============================================================================
Parser::Parser(const std::string &path, const std::string &file) :
    m_path(path),
    m_file(file),
    m_line(0),
    m_column(0)
{
}

//==============================================================================
Json Parser::GetValues() const
{
    std::shared_lock<std::shared_timed_mutex> lock(m_valuesMutex);
    return m_values;
}

//==============================================================================
Json Parser::GetValues(const std::string &section) const
{
    std::shared_lock<std::shared_timed_mutex> lock(m_valuesMutex);
    Json values;

    try
    {
        values = m_values[section];
    }
    catch (const JsonException &)
    {
    }

    return values;
}

//==============================================================================
ParserException::ParserException(
    const std::string &file,
    int line,
    const std::string &message
) :
    m_message(
        String::Format(
            "ParserException: Error parsing %s at [line %d]: %s",
            file, line, message
        )
    )
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
    m_message(
        String::Format(
            "ParserException: Error parsing %s at [line %d, column %d]: %s",
            file, line, column, message
        )
    )
{
    LOGW(-1, "%s", m_message);
}

//==============================================================================
const char *ParserException::what() const noexcept
{
    return m_message.c_str();
}

}
