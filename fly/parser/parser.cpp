#include "fly/parser/parser.h"

#include "fly/path/path.h"

namespace fly {

//==============================================================================
Parser::Parser(const std::string &path, const std::string &file) :
    m_path(path),
    m_file(file),
    m_values(),
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
void Parser::Parse()
{
    std::unique_lock<std::shared_timed_mutex> lock(m_valuesMutex);

    std::string fullPath = Path::Join(m_path, m_file);
    std::ifstream stream(fullPath, std::ios::in);

    m_values = nullptr;

    m_line = 1;
    m_column = 0;

    ParseInternal(stream);
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

}
