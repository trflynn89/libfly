#include "fly/logger/detail/nix/styler_proxy_impl.hpp"

namespace fly::detail {

//==================================================================================================
StylerProxyImpl::StylerProxyImpl(
    std::ostream &stream,
    std::stack<Style> &&styles,
    std::stack<Color> &&colors,
    std::stack<Cursor> &&cursors) noexcept :
    StylerProxy(stream)
{
    if (m_stream_is_stdout || m_stream_is_stderr)
    {
        if (!styles.empty() || !colors.empty())
        {
            apply_styles_and_colors(std::move(styles), std::move(colors));
            m_did_apply_style_or_color = true;
        }

        if (!cursors.empty())
        {
            apply_cursors(std::move(cursors));
        }
    }
}

//==================================================================================================
StylerProxyImpl::~StylerProxyImpl()
{
    if (m_did_apply_style_or_color)
    {
        m_stream << "\x1b[0m";
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::stream_value<Style>(const Style &modifier)
{
    // https://en.wikipedia.org/wiki/ANSI_escape_code#SGR_parameters
    switch (modifier)
    {
        case Style::Default:
            m_stream << 0;
            break;
        case Style::Bold:
            m_stream << 1;
            break;
        case Style::Dim:
            m_stream << 2;
            break;
        case Style::Italic:
            m_stream << 3;
            break;
        case Style::Underline:
            m_stream << 4;
            break;
        case Style::Blink:
            m_stream << 5;
            break;
        case Style::Strike:
            m_stream << 9;
            break;
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::stream_value<Color>(const Color &modifier)
{
    if (modifier.m_color <= Color::White)
    {
        // https://en.wikipedia.org/wiki/ANSI_escape_code#3/4_bit
        if (modifier.m_plane == Color::Foreground)
        {
            m_stream << "3";
        }
        else
        {
            m_stream << "4";
        }
    }
    else
    {
        // https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
        if (modifier.m_plane == Color::Foreground)
        {
            m_stream << "38;5;";
        }
        else
        {
            m_stream << "48;5;";
        }
    }

    m_stream << static_cast<std::uint32_t>(modifier.m_color);
}

//==================================================================================================
template <>
void StylerProxyImpl::stream_value<Cursor>(const Cursor &modifier)
{
    // https://en.wikipedia.org/wiki/ANSI_escape_code#Terminal_output_sequences
    m_stream << "\x1b[" << static_cast<std::uint32_t>(modifier.m_distance);

    switch (modifier.m_direction)
    {
        case Cursor::Up:
            m_stream << 'A';
            break;
        case Cursor::Down:
            m_stream << 'B';
            break;
        case Cursor::Forward:
            m_stream << 'C';
            break;
        case Cursor::Backward:
            m_stream << 'D';
            break;
    }
}

//==================================================================================================
void StylerProxyImpl::apply_styles_and_colors(
    std::stack<Style> &&styles,
    std::stack<Color> &&colors)
{
    m_stream << "\x1b[";

    bool first_modifier = true;

    for (; !styles.empty(); styles.pop(), first_modifier = false)
    {
        if (!first_modifier)
        {
            m_stream << ';';
        }

        stream_value(styles.top());
    }

    for (; !colors.empty(); colors.pop(), first_modifier = false)
    {
        if (!first_modifier)
        {
            m_stream << ';';
        }

        stream_value(colors.top());
    }

    m_stream << 'm';
}

//==================================================================================================
void StylerProxyImpl::apply_cursors(std::stack<Cursor> &&cursors)
{
    for (; !cursors.empty(); cursors.pop())
    {
        stream_value(cursors.top());
    }
}

} // namespace fly::detail
