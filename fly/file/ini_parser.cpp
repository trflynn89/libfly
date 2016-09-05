#include "ini_parser.h"

#include <cstring>
#include <fstream>

#include <fly/string/string.h>
#include <fly/system/system.h>

namespace fly {

//==============================================================================
IniParser::IniParser(const std::string &path, const std::string &file) :
    Parser(path, file)
{
}

//==============================================================================
void IniParser::Parse()
{
    std::string fullPath = System::Join(m_path, m_file);
    std::ifstream stream(fullPath.c_str(), std::ios::in);

    std::string line, section;
    m_line = 0;

    std::unique_lock<std::shared_timed_mutex> lock(m_sectionsMutex);
    m_sections.clear();

    while (stream.good() && std::getline(stream, line))
    {
        String::Trim(line);
        ++m_line;

        if (line.empty() || String::StartsWith(line, ';'))
        {
            // Ignore comments and blank lines
        }
        else if (trimValue(line, '[', ']'))
        {
            section = onSection(line);
        }
        else if (!section.empty())
        {
            onValue(section, line);
        }
        else
        {
            throw ParserException(m_file, m_line,
                "A section must be defined before name=value pairs"
            );
        }
    }
}

//==============================================================================
Parser::ValueList IniParser::GetValues(const std::string &section) const
{
    Parser::ValueList values;

    std::shared_lock<std::shared_timed_mutex> lock(m_sectionsMutex);
    IniSection::const_iterator it = m_sections.find(section);

    if (it != m_sections.end())
    {
        values = it->second;
    }

    return values;
}

//==============================================================================
IniParser::IniSection::size_type IniParser::GetSize() const
{
    std::shared_lock<std::shared_timed_mutex> lock(m_sectionsMutex);
    return m_sections.size();
}

//==============================================================================
Parser::ValueList::size_type IniParser::GetSize(const std::string &section) const
{
    Parser::ValueList::size_type size = 0;

    std::shared_lock<std::shared_timed_mutex> lock(m_sectionsMutex);
    IniSection::const_iterator it = m_sections.find(section);

    if (it != m_sections.end())
    {
        size = it->second.size();
    }

    return size;
}

//==============================================================================
std::string IniParser::onSection(const std::string &line)
{
    std::string section = line;
    String::Trim(section);

    if (m_sections.find(section) != m_sections.end())
    {
        throw ParserException(m_file, m_line,
            "Section names must be unique"
        );
    }
    else if (trimValue(section, '\'') || trimValue(section, '\"'))
    {
        throw ParserException(m_file, m_line,
            "Section names must not be quoted"
        );
    }

    return section;
}

//==============================================================================
void IniParser::onValue(const std::string &section, const std::string &line)
{
    static const size_t size = 2;

    std::vector<std::string> nameValue = String::Split(line, '=', size);

    if (nameValue.size() == size)
    {
        std::string name(nameValue[0]), value(nameValue[1]);

        String::Trim(name);
        String::Trim(value);

        if (trimValue(name, '\'') || trimValue(name, '\"'))
        {
            throw ParserException(m_file, m_line,
                "Value names must not be quoted"
            );
        }

        trimValue(value, '\'');
        trimValue(value, '\"');

        Parser::ValueList &list = m_sections[section];

        for (const Parser::Value &value : list)
        {
            if (name.compare(value.first) == 0)
            {
                throw ParserException(m_file, m_line,
                    "Value names must be unique within a section"
                );
            }
        }

        list.push_back(Parser::Value(name, value));
    }
    else
    {
        throw ParserException(m_file, m_line,
            "Require name/value pairs of the form name=value"
        );
    }
}

//==============================================================================
bool IniParser::trimValue(std::string &str, char ch) const
{
    return trimValue(str, ch, ch);
}

//==============================================================================
bool IniParser::trimValue(std::string &str, char start, char end) const
{
    bool startsWithChar = String::StartsWith(str, start);
    bool endsWithChar = String::EndsWith(str, end);

    if (startsWithChar || endsWithChar)
    {
        if (startsWithChar && endsWithChar)
        {
            str = str.substr(1, str.size() - 2);
        }
        else
        {
            throw ParserException(m_file, m_line, String::Format(
                "Imbalanced characters: \"%c\" and \"%c\"", start, end
            ));
        }
    }

    return (startsWithChar && endsWithChar);
}

}
