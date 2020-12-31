#include "fly/parser/ini_parser.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/string/string.hpp"

#include <vector>

namespace fly {

#define ILOG(...)                                                                                  \
    LOGW("[line %d]: " FLY_FORMAT_STRING(__VA_ARGS__), line() FLY_FORMAT_ARGS(__VA_ARGS__));

//==================================================================================================
std::optional<Json> IniParser::parse_internal()
{
    Json values = JsonTraits::object_type();
    Json::iterator current;

    std::string data;

    while (getline(data))
    {
        String::trim(data);

        if (data.empty() || data.starts_with(';'))
        {
            // Ignore comments and blank lines.
            continue;
        }

        switch (trim_value(data, '[', ']'))
        {
            case TrimResult::Imbalanced:
                return std::nullopt;

            case TrimResult::Trimmed:
                if (auto section = on_section(data); section)
                {
                    try
                    {
                        auto it =
                            values.insert_or_assign(*std::move(section), JsonTraits::object_type());
                        current = it.first;
                    }
                    catch (const JsonException &ex)
                    {
                        ILOG("%s", ex.what());
                        return std::nullopt;
                    }
                }
                else
                {
                    return std::nullopt;
                }

                break;

            case TrimResult::Untrimmed:
                if (values.empty())
                {
                    ILOG("A section must be defined before name=value pairs");
                    return std::nullopt;
                }
                else if (auto value = on_name_value_pair(data); value)
                {
                    try
                    {
                        current->insert_or_assign(
                            std::move(value->first),
                            std::move(value->second));
                    }
                    catch (const JsonException &ex)
                    {
                        ILOG("%s", ex.what());
                        return std::nullopt;
                    }
                }
                else
                {
                    return std::nullopt;
                }

                break;
        }
    }

    return values.empty() ? std::nullopt : std::optional<Json>(std::move(values));
}

//==================================================================================================
bool IniParser::getline(std::string &result)
{
    static constexpr const int s_new_line = 0x0a;

    result.clear();
    int ch;

    while (!eof() && ((ch = get()) != s_new_line))
    {
        result += ch;
    }

    return !eof() || !result.empty();
}

//==================================================================================================
std::optional<std::string> IniParser::on_section(std::string &section)
{
    String::trim(section);

    if ((trim_value(section, '\'') != TrimResult::Untrimmed) ||
        (trim_value(section, '\"') != TrimResult::Untrimmed))
    {
        ILOG("Section names must not be quoted");
        return std::nullopt;
    }

    return section;
}

//==================================================================================================
std::optional<std::pair<std::string, std::string>>
IniParser::on_name_value_pair(const std::string &name_value)
{
    static constexpr std::uint32_t s_size = 2;

    std::vector<std::string> pair = String::split(name_value, '=', s_size);

    if (pair.size() == s_size)
    {
        std::string name(std::move(pair[0])), value(std::move(pair[1]));

        String::trim(name);
        String::trim(value);

        if ((trim_value(name, '\'') != TrimResult::Untrimmed) ||
            (trim_value(name, '\"') != TrimResult::Untrimmed))
        {
            ILOG("Value names must not be quoted");
            return std::nullopt;
        }
        else if (
            (trim_value(value, '\'') == TrimResult::Imbalanced) ||
            (trim_value(value, '\"') == TrimResult::Imbalanced))
        {
            return std::nullopt;
        }

        return std::make_pair(std::move(name), std::move(value));
    }
    else
    {
        ILOG("Require name/value pairs of the form name=value");
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
    bool starts_with_char = str.starts_with(start);
    bool ends_with_char = str.ends_with(end);

    if (starts_with_char && ends_with_char)
    {
        str = str.substr(1, str.size() - 2);
        return TrimResult::Trimmed;
    }
    else if (starts_with_char || ends_with_char)
    {
        ILOG("Imbalanced characters: \"%c\" and \"%c\"", start, end);
        return TrimResult::Imbalanced;
    }

    return TrimResult::Untrimmed;
}

} // namespace fly
