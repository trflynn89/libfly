#include "parser.h"

#include <fly/logger/logger.h>
#include <fly/string/string.h>

namespace fly {

//==============================================================================
Parser::Parser(const std::string &path, const std::string &file) :
    m_path(path),
    m_file(file),
    m_line(0)
{
}

//==============================================================================
ParserException::ParserException(
    const std::string &file,
    int line,
    const std::string &message
) :
    m_message(String::Format("ParserException: Error parsing %s on line %d: %s",
        file, line, message))
{
    LOGW(-1, "%s", m_message);
}

//==============================================================================
const char *ParserException::what() const noexcept
{
    return m_message.c_str();
}

}
