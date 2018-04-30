#include "fly/parser/parser.h"

#include <fstream>
#include <sstream>

#include "fly/path/path.h"

namespace fly {

//==============================================================================
Parser::Parser() :
    m_line(0),
    m_column(0)
{
}

//==============================================================================
Json Parser::Parse(const std::string &contents)
{
    std::istringstream stream(contents);

    m_line = 1;
    m_column = 0;

    return ParseInternal(stream);
}

//==============================================================================
Json Parser::Parse(const std::string &path, const std::string &file)
{
    const std::string fullPath = Path::Join(path, file);
    std::ifstream stream(fullPath, std::ios::in);

    m_line = 1;
    m_column = 0;

    return ParseInternal(stream);
}

}
