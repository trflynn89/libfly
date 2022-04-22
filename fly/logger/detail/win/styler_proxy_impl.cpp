#include "fly/logger/detail/win/styler_proxy_impl.hpp"

namespace fly::logger::detail {

//==================================================================================================
StylerProxyImpl::StylerProxyImpl(
    std::ostream &stream,
    std::stack<fly::logger::Style> &&styles,
    std::stack<fly::logger::Color> &&colors,
    std::stack<fly::logger::Cursor> &&cursors) noexcept :
    StylerProxy(stream)
{
    if (m_stream_is_stdout)
    {
        m_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if (m_stream_is_stderr)
    {
        m_handle = ::GetStdHandle(STD_ERROR_HANDLE);
    }

    if (m_handle == INVALID_HANDLE_VALUE)
    {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO console_info;
    if (!::GetConsoleScreenBufferInfo(m_handle, &console_info))
    {
        m_handle = INVALID_HANDLE_VALUE;
        return;
    }

    if (!styles.empty() || !colors.empty())
    {
        apply_styles_and_colors(console_info, std::move(styles), std::move(colors));
    }

    if (!cursors.empty())
    {
        apply_cursors(console_info, std::move(cursors));
    }
}

//==================================================================================================
StylerProxyImpl::~StylerProxyImpl()
{
    if (m_did_apply_style_or_color)
    {
        ::SetConsoleTextAttribute(m_handle, m_original_attributes);
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::apply_value<WORD, fly::logger::Style>(
    WORD &attributes,
    fly::logger::Style const &modifier)
{
    // https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes
    switch (modifier)
    {
        case fly::logger::Style::Bold:
            attributes |= FOREGROUND_INTENSITY;
            break;
        case fly::logger::Style::Underline:
            attributes |= COMMON_LVB_UNDERSCORE;
            break;
        default:
            break;
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::apply_value<WORD, fly::logger::Color>(
    WORD &attributes,
    fly::logger::Color const &modifier)
{
    // https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes
    auto apply_color = [&attributes, &modifier](bool red, bool green, bool blue) {
        if (modifier.m_plane == fly::logger::Color::Foreground)
        {
            attributes = red ? (attributes | FOREGROUND_RED) : (attributes & ~FOREGROUND_RED);
            attributes = green ? (attributes | FOREGROUND_GREEN) : (attributes & ~FOREGROUND_GREEN);
            attributes = blue ? (attributes | FOREGROUND_BLUE) : (attributes & ~FOREGROUND_BLUE);
        }
        else
        {
            attributes = red ? (attributes | BACKGROUND_RED) : (attributes & ~BACKGROUND_RED);
            attributes = green ? (attributes | BACKGROUND_GREEN) : (attributes & ~BACKGROUND_GREEN);
            attributes = blue ? (attributes | BACKGROUND_BLUE) : (attributes & ~BACKGROUND_BLUE);
        }
    };

    switch (modifier.m_color)
    {
        case fly::logger::Color::Black:
            apply_color(false, false, false);
            break;
        case fly::logger::Color::Red:
            apply_color(true, false, false);
            break;
        case fly::logger::Color::Green:
            apply_color(false, true, false);
            break;
        case fly::logger::Color::Blue:
            apply_color(false, false, true);
            break;
        case fly::logger::Color::Yellow:
            apply_color(true, true, false);
            break;
        case fly::logger::Color::Magenta:
            apply_color(true, false, true);
            break;
        case fly::logger::Color::Cyan:
            apply_color(false, true, true);
            break;
        case fly::logger::Color::White:
            apply_color(true, true, true);
            break;
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::apply_value<COORD, fly::logger::Cursor>(
    COORD &attributes,
    fly::logger::Cursor const &modifier)
{
    std::uint8_t const &distance = modifier.m_distance;

    switch (modifier.m_direction)
    {
        case fly::logger::Cursor::Up:
            attributes.Y = (attributes.Y > distance) ? (attributes.Y - distance) : 0;
            break;
        case fly::logger::Cursor::Down:
            attributes.Y += distance;
            break;
        case fly::logger::Cursor::Forward:
            attributes.X += distance;
            break;
        case fly::logger::Cursor::Backward:
            attributes.X = (attributes.X > distance) ? (attributes.X - distance) : 0;
            break;
    }
}

//==================================================================================================
void StylerProxyImpl::apply_styles_and_colors(
    CONSOLE_SCREEN_BUFFER_INFO const &console_info,
    std::stack<fly::logger::Style> &&styles,
    std::stack<fly::logger::Color> &&colors)
{
    m_original_attributes = console_info.wAttributes;
    WORD attributes = m_original_attributes;

    for (; !styles.empty(); styles.pop())
    {
        apply_value(attributes, styles.top());
    }

    for (; !colors.empty(); colors.pop())
    {
        apply_value(attributes, colors.top());
    }

    m_did_apply_style_or_color = ::SetConsoleTextAttribute(m_handle, attributes);
}

//==================================================================================================
void StylerProxyImpl::apply_cursors(
    CONSOLE_SCREEN_BUFFER_INFO const &console_info,
    std::stack<fly::logger::Cursor> &&cursors)
{
    COORD cursor_position = console_info.dwCursorPosition;

    for (; !cursors.empty(); cursors.pop())
    {
        apply_value(cursor_position, cursors.top());
    }

    ::SetConsoleCursorPosition(m_handle, cursor_position);
}

} // namespace fly::logger::detail
