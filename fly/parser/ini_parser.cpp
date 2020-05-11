#include "fly/parser/ini_parser.hpp"

#include "fly/parser/exceptions.hpp"
#include "fly/types/string/string.hpp"

#include <vector>

namespace fly {

//==============================================================================
Json IniParser::parse_internal(std::istream &stream) noexcept(false)
{
    std::string line, section;
    Json values;

    while (stream.good() && std::getline(stream, line))
    {
        String::Trim(line);
        ++m_line;

        if (line.empty() || String::StartsWith(line, ';'))
        {
            // Ignore comments and blank lines
        }
        else if (trim_value(line, '[', ']'))
        {
            section = on_section(line);
        }
        else if (!section.empty())
        {
            on_value(values[section], line);
        }
        else
        {
            throw ParserException(
                m_line,
                "A section must be defined before name=value pairs");
        }
    }

    return values;
}

//==============================================================================
std::string IniParser::on_section(const std::string &line) noexcept(false)
{
    std::string section = line;
    String::Trim(section);

    if (trim_value(section, '\'') || trim_value(section, '\"'))
    {
        throw ParserException(m_line, "Section names must not be quoted");
    }

    return section;
}

//==============================================================================
void IniParser::on_value(Json &section, const std::string &line) noexcept(false)
{
    static constexpr std::uint32_t s_size = 2;

    std::vector<std::string> name_value = String::Split(line, '=', s_size);

    if (name_value.size() == s_size)
    {
        std::string name(name_value[0]), value(name_value[1]);

        String::Trim(name);
        String::Trim(value);

        if (trim_value(name, '\'') || trim_value(name, '\"'))
        {
            throw ParserException(m_line, "Value names must not be quoted");
        }

        trim_value(value, '\'');
        trim_value(value, '\"');

        section[name] = value;
    }
    else
    {
        throw ParserException(
            m_line,
            "Require name/value pairs of the form name=value");
    }
}

//==============================================================================
bool IniParser::trim_value(std::string &str, char ch) const noexcept(false)
{
    return trim_value(str, ch, ch);
}

//==============================================================================
bool IniParser::trim_value(std::string &str, char start, char end) const
    noexcept(false)
{
    bool starts_with_char = String::StartsWith(str, start);
    bool ends_with_char = String::EndsWith(str, end);

    if (starts_with_char && ends_with_char)
    {
        str = str.substr(1, str.size() - 2);
    }
    else if (starts_with_char || ends_with_char)
    {
        throw ParserException(
            m_line,
            String::Format(
                "Imbalanced characters: \"%c\" and \"%c\"",
                start,
                end));
    }

    return starts_with_char && ends_with_char;
}

} // namespace fly
