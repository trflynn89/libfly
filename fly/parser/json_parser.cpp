#include "fly/parser/json_parser.h"

#include <cctype>
#include <cstring>
#include <fstream>

#include "fly/path/path.h"
#include "fly/string/string.h"

namespace fly {

//==============================================================================
JsonParser::JsonParser(const std::string &path, const std::string &file) :
    Parser(path, file),
    m_state(JSON_NO_STATE),
    m_root(),
    m_pValue(&m_root),
    m_objectDepth(0)
{
}

//==============================================================================
void JsonParser::Parse()
{
    std::string fullPath = Path::Join(m_path, m_file);
    std::ifstream stream(fullPath.c_str(), std::ios::in);

    std::string line, section;
    m_line = 0;

    std::unique_lock<std::shared_timed_mutex> lock(m_sectionsMutex);
    m_sections.clear();

    JsonToken token;
    char c;

    while (stream.get(c))
    {
        if ((m_state == JSON_NO_STATE) && std::isspace(c))
        {
            // Ignore whitepsace
            continue;
        }

        token = static_cast<JsonToken>(c);

        switch (token)
        {
        case JSON_START_BRACE:
            onStartBrace();
            break;

        case JSON_CLOSE_BRACE:
            onCloseBrace();
            break;

        case JSON_QUOTE:
            onQuotation();
            break;

        case JSON_COMMA:
            break;

        case JSON_COLON:
            break;

        case JSON_START_BRACKET:
            break;

        case JSON_CLOSE_BRACKET:
            break;

        default:
            onCharacter(c);
        }
    }

/*
    while (stream.good() && std::getline(stream, line))
    {
        String::Trim(line);
        ++m_line;

        if (line.empty())
        {
            // Ignore blank lines
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
*/
}

void JsonParser::onStartBrace()
{
    if (m_pValue)
    {

    }
    else
    {
        m_pValue = &m_root;
    }

    ++m_objectDepth;
}

void JsonParser::onCloseBrace()
{
    if (m_pValue)
    {
        m_pValue = m_pValue->GetParent();
    }

    --m_objectDepth;
}

void JsonParser::onQuotation()
{
    switch (m_state)
    {
    case JSON_NO_STATE:
        m_state = JSON_PARSING_NAME;
        break;

    case JSON_PARSING_NAME:
        std::cout << m_parsing.str() << std::endl;

        m_parsing.str(std::string());
        m_state = JSON_NO_STATE;
        break;

    default:
        break;
    }
}

void JsonParser::onCharacter(const char &c)
{
    switch (m_state)
    {
    case JSON_PARSING_NAME:
        m_parsing << c;
        break;
    case JSON_PARSING_OBJECT:
        break;
    case JSON_PARSING_VALUE:
        break;
    case JSON_PARSING_ARRAY:
        break;

    default:
        break;
    }
}

//==============================================================================
Parser::ValueList JsonParser::GetValues(const std::string &section) const
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
JsonParser::IniSection::size_type JsonParser::GetSize() const
{
    std::shared_lock<std::shared_timed_mutex> lock(m_sectionsMutex);
    return m_sections.size();
}

//==============================================================================
Parser::ValueList::size_type JsonParser::GetSize(const std::string &section) const
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
std::string JsonParser::onSection(const std::string &line)
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
void JsonParser::onValue(const std::string &section, const std::string &line)
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
bool JsonParser::trimValue(std::string &str, char ch) const
{
    return trimValue(str, ch, ch);
}

//==============================================================================
bool JsonParser::trimValue(std::string &str, char start, char end) const
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
