#include "fly/logger/detail/nix/styler_proxy_impl.hpp"

namespace fly::detail {

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
    // https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
    if (modifier.m_color <= Color::White)
    {
        if (modifier.m_plane == Color::Plane::Foreground)
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
        if (modifier.m_plane == Color::Plane::Foreground)
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
StylerProxyImpl::StylerProxyImpl(
    std::ostream &stream,
    std::stack<Style> &&styles,
    std::stack<Color> &&colors) noexcept :
    StylerProxy(stream),
    m_did_modify_stream(false)
{
    if (m_stream_is_stdout || m_stream_is_stderr)
    {
        m_did_modify_stream = true;
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
}

//==================================================================================================
StylerProxyImpl::~StylerProxyImpl()
{
    if (m_did_modify_stream)
    {
        m_stream << "\x1b[0m";
    }
}

} // namespace fly::detail
