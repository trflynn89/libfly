#include "fly/logger/detail/win/styler_proxy_impl.hpp"

namespace fly::detail {

//==================================================================================================
StylerProxyImpl::StylerProxyImpl(
    std::ostream &stream,
    std::stack<Style> &&styles,
    std::stack<Color> &&colors,
    std::stack<Position> &&positions) noexcept :
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

    if (!positions.empty())
    {
        apply_positions(console_info, std::move(positions));
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
void StylerProxyImpl::apply_value<WORD, Style>(WORD &attributes, const Style &modifier)
{
    // https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes
    switch (modifier)
    {
        case Style::Bold:
            attributes |= FOREGROUND_INTENSITY;
            break;
        case Style::Underline:
            attributes |= COMMON_LVB_UNDERSCORE;
            break;
        default:
            break;
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::apply_value<WORD, Color>(WORD &attributes, const Color &modifier)
{
    // https://docs.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes
    auto apply_color = [&attributes, &modifier](bool red, bool green, bool blue) {
        if (modifier.m_plane == Color::Plane::Foreground)
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
        case Color::Black:
            apply_color(false, false, false);
            break;
        case Color::Red:
            apply_color(true, false, false);
            break;
        case Color::Green:
            apply_color(false, true, false);
            break;
        case Color::Blue:
            apply_color(false, false, true);
            break;
        case Color::Yellow:
            apply_color(true, true, false);
            break;
        case Color::Magenta:
            apply_color(true, false, true);
            break;
        case Color::Cyan:
            apply_color(false, true, true);
            break;
        case Color::White:
            apply_color(true, true, true);
            break;
    }
}

//==================================================================================================
template <>
void StylerProxyImpl::apply_value<COORD, Position>(COORD &attributes, const Position &modifier)
{
    switch (modifier)
    {
        case Position::CursorUp:
            attributes.Y -= attributes.Y > 0 ? 1 : 0;
            break;
        case Position::CursorDown:
            attributes.Y += 1;
            break;
        case Position::CursorForward:
            attributes.X += 1;
            break;
        case Position::CursorBackward:
            attributes.X -= attributes.X > 0 ? 1 : 0;
            break;
    }
}

//==================================================================================================
void StylerProxyImpl::apply_styles_and_colors(
    const CONSOLE_SCREEN_BUFFER_INFO &console_info,
    std::stack<Style> &&styles,
    std::stack<Color> &&colors)
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
void StylerProxyImpl::apply_positions(
    const CONSOLE_SCREEN_BUFFER_INFO &console_info,
    std::stack<Position> &&positions)
{
    COORD cursor_position = console_info.dwCursorPosition;

    for (; !positions.empty(); positions.pop())
    {
        apply_value(cursor_position, positions.top());
    }

    ::SetConsoleCursorPosition(m_handle, cursor_position);
}

} // namespace fly::detail
