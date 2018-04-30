#include "fly/parser/parser.h"

#include <fstream>
#include <sstream>

#include "fly/path/path.h"

namespace fly {

//==============================================================================
Parser::Parser() :
    m_values(),
    m_line(0),
    m_column(0)
{
}

//==============================================================================
void Parser::Parse(const std::string &contents)
{
    std::unique_lock<std::shared_timed_mutex> lock(m_valuesMutex);

    std::istringstream stream(contents);

    m_values = nullptr;

    m_line = 1;
    m_column = 0;

    ParseInternal(stream);
}

//==============================================================================
void Parser::Parse(const std::string &path, const std::string &file)
{
    std::unique_lock<std::shared_timed_mutex> lock(m_valuesMutex);

    std::string fullPath = Path::Join(path, file);
    std::ifstream stream(fullPath, std::ios::in);

    m_values = nullptr;

    m_line = 1;
    m_column = 0;

    ParseInternal(stream);
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

}
