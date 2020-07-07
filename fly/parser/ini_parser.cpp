#include "fly/parser/ini_parser.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/string/string.hpp"

#include <vector>

namespace fly {

//==================================================================================================
std::optional<Json> IniParser::parse_internal(std::istream &stream)
{
    std::string line, section;
    Json values;

    while (stream && std::getline(stream, line))
    {
        String::trim(line);
        ++m_line;

        if (line.empty() || String::starts_with(line, ';'))
        {
            // Ignore comments and blank lines.
            continue;
        }

        const TrimResult result = trim_value(line, '[', ']');

        if (result == TrimResult::Imbalanced)
        {
            return std::nullopt;
        }
        else if (result == TrimResult::Trimmed)
        {
            std::optional<std::string> maybe_section = on_section(line);

            if (maybe_section)
            {
                section = std::move(maybe_section.value());

                try
                {
                    values[section] = JsonTraits::object_type();
                }
                catch (const JsonException &ex)
                {
                    LOGW("[line %d]: %s", m_line, ex.what());
                    return std::nullopt;
                }
            }
            else
            {
                return std::nullopt;
            }
        }
        else if (!section.empty())
        {
            std::optional<std::pair<std::string, std::string>> maybe_value = on_value(line);

            if (maybe_value)
            {
                std::pair<std::string, std::string> value = std::move(maybe_value.value());

                try
                {
                    values[section][std::move(value.first)] = std::move(value.second);
                }
                catch (const JsonException &ex)
                {
                    LOGW("[line %d]: %s", m_line, ex.what());
                    return std::nullopt;
                }
            }
            else
            {
                return std::nullopt;
            }
        }
        else
        {
            LOGW("[line %d]: A section must be defined before name=value pairs", m_line);
            return std::nullopt;
        }
    }

    if (values.is_object())
    {
        return values;
    }

    return std::nullopt;
}

//==================================================================================================
std::optional<std::string> IniParser::on_section(const std::string &line)
{
    std::string section = line;
    String::trim(section);

    if ((trim_value(section, '\'') != TrimResult::Untrimmed) ||
        (trim_value(section, '\"') != TrimResult::Untrimmed))
    {
        LOGW("[line %d]: Section names must not be quoted", m_line);
        return std::nullopt;
    }

    return section;
}

//==================================================================================================
std::optional<std::pair<std::string, std::string>> IniParser::on_value(const std::string &line)
{
    static constexpr std::uint32_t s_size = 2;

    std::vector<std::string> name_value = String::split(line, '=', s_size);

    if (name_value.size() == s_size)
    {
        std::string name(name_value[0]), value(name_value[1]);

        String::trim(name);
        String::trim(value);

        if ((trim_value(name, '\'') != TrimResult::Untrimmed) ||
            (trim_value(name, '\"') != TrimResult::Untrimmed))
        {
            LOGW("[line %d]: Value names must not be quoted", m_line);
            return std::nullopt;
        }
        else if (
            (trim_value(value, '\'') == TrimResult::Imbalanced) ||
            (trim_value(value, '\"') == TrimResult::Imbalanced))
        {
            return std::nullopt;
        }

        return {{name, value}};
    }
    else
    {
        LOGW("[line %d]: Require name/value pairs of the form name=value", m_line);
        return std::nullopt;
    }
}

//==================================================================================================
IniParser::TrimResult IniParser::trim_value(std::string &str, char ch) const
{
    return trim_value(str, ch, ch);
}

//==================================================================================================
IniParser::TrimResult IniParser::trim_value(std::string &str, char start, char end) const
{
    bool starts_with_char = String::starts_with(str, start);
    bool ends_with_char = String::ends_with(str, end);

    if (starts_with_char && ends_with_char)
    {
        str = str.substr(1, str.size() - 2);
        return TrimResult::Trimmed;
    }
    else if (starts_with_char || ends_with_char)
    {
        LOGW("[line %d]: Imbalanced characters: \"%c\" and \"%c\"", m_line, start, end);
        return TrimResult::Imbalanced;
    }

    return TrimResult::Untrimmed;
}

} // namespace fly
