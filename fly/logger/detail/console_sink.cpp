#include "fly/logger/detail/console_sink.hpp"

#include "fly/logger/log.hpp"
#include "fly/logger/styler.hpp"
#include "fly/system/system.hpp"
#include "fly/types/string/string.hpp"

#include <iostream>
#include <optional>
#include <string>

namespace fly::detail {

//==================================================================================================
bool ConsoleSink::initialize()
{
    return true;
}

//==================================================================================================
bool ConsoleSink::stream(fly::Log &&log)
{
    std::ostream *stream = &std::cout;
    fly::Style style = fly::Style::Default;
    std::optional<fly::Color> color;

    switch (log.m_level)
    {
        case Log::Level::Info:
            color.emplace(fly::Color::Green);
            break;

        case Log::Level::Warn:
            stream = &std::cerr;
            color.emplace(fly::Color::Yellow);
            break;

        case Log::Level::Error:
            stream = &std::cerr;
            style = fly::Style::Bold;
            color.emplace(fly::Color::Red);
            break;

        default:
            break;
    }

    {
        auto styler = color ? fly::Styler(std::move(style), *std::move(color)) : fly::Styler(style);
        *stream << styler;

        String::format(*stream, "{} {}", fly::System::local_time(), log.m_trace);
    }

    *stream << ": " << log.m_message << std::endl;
    return true;
}

} // namespace fly::detail
