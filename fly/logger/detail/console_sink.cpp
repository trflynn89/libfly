#include "fly/logger/detail/console_sink.hpp"

#include "fly/logger/log.hpp"
#include "fly/logger/styler.hpp"
#include "fly/system/system.hpp"
#include "fly/types/string/string.hpp"

#include <iostream>
#include <optional>
#include <string>

namespace fly::logger::detail {

//==================================================================================================
bool ConsoleSink::initialize()
{
    return true;
}

//==================================================================================================
bool ConsoleSink::stream(fly::logger::Log &&log)
{
    std::ostream *stream = &std::cout;
    fly::logger::Style style = fly::logger::Style::Default;
    std::optional<fly::logger::Color> color;

    switch (log.m_level)
    {
        case fly::logger::Level::Info:
            color.emplace(fly::logger::Color::Green);
            break;

        case fly::logger::Level::Warn:
            stream = &std::cerr;
            color.emplace(fly::logger::Color::Yellow);
            break;

        case fly::logger::Level::Error:
            stream = &std::cerr;
            style = fly::logger::Style::Bold;
            color.emplace(fly::logger::Color::Red);
            break;

        default:
            break;
    }

    {
        auto styler = color ? fly::logger::Styler(std::move(style), *std::move(color)) :
                              fly::logger::Styler(std::move(style));
        *stream << styler << fly::String::format("{} {}", fly::system::local_time(), log.m_trace);
    }

    *stream << ": " << log.m_message << std::endl;
    return true;
}

} // namespace fly::logger::detail
