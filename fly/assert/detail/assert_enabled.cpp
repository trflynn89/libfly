#include "fly/assert/detail/assert_enabled.hpp"

#include "fly/logger/styler.hpp"
#include "fly/system/system.hpp"

#include <iostream>

namespace fly::detail {

//==================================================================================================
std::string Capture::format(std::string_view capture_name) const
{
    return m_format(capture_name, m_value);
}

//==================================================================================================
Assertion::Assertion(
    std::string_view expression,
    std::string_view file,
    std::string_view function,
    std::uint32_t line,
    std::span<std::string_view const> capture_names) :
    m_expression(expression),
    m_file(file),
    m_function(function),
    m_line(line),
    m_capture_names(capture_names)
{
}

//==================================================================================================
void Assertion::log_assertion(std::string_view message, std::span<Capture const> captures) const
{
    std::cerr << '\n';
    std::cerr << logger::Styler(logger::Style::Bold, logger::Color::Red) << "Assertion failed:";

    if (!message.empty())
    {
        std::cerr << logger::Styler(logger::Style::Bold) << ' ' << message;
    }

    std::cerr << fly::string::format("\n\tFLY_ASSERT({})\n\n", m_expression);
    std::cerr << fly::string::format("\tat {}:{}\n", m_file, m_line);
    std::cerr << fly::string::format("\tin {}\n\n", m_function);

    if (!captures.empty())
    {
        std::cerr << logger::Styler(logger::Style::Bold) << "Captures:\n";

        for (std::size_t i = 0; i < captures.size(); ++i)
        {
            std::cerr << captures[i].format(m_capture_names[i]);
        }

        std::cerr << '\n';
    }

    std::cerr << logger::Styler(logger::Style::Bold) << "Call stack:\n";
    fly::system::print_backtrace();
    std::cerr << '\n';
}

} // namespace fly::detail
