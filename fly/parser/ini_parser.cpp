#include "fly/parser/ini_parser.h"

#include <cstring>

#include "fly/string/string.h"

namespace fly {

//==============================================================================
IniParser::IniParser(const std::string &path, const std::string &file) :
    Parser(path, file)
{
}

//==============================================================================
void IniParser::ParseInternal(std::ifstream &stream)
{
    std::string line, section;

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
std::string IniParser::onSection(const std::string &line)
{
    std::string section = line;
    String::Trim(section);

    if (trimValue(section, '\'') || trimValue(section, '\"'))
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

        m_values[section][name] = value;
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
