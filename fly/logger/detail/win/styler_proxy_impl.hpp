#pragma once

#include "fly/logger/detail/styler_proxy.hpp"
#include "fly/logger/styler.hpp"

#include <Windows.h>

#include <ostream>
#include <stack>

namespace fly::logger::detail {

/**
 * Windows implementation of the StylerProxy interface. This implementation uses the Windows Console
 * API to manipulate the std::ostream.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 11, 2020
 */
class StylerProxyImpl : public StylerProxy
{
public:
    /**
     * Constructor. Manipulate the provided stream with the Windows Console API if it is a standard
     * output or error stream.
     *
     * @param stream The stream to manipulate.
     * @param styles The list of styles to apply to the stream.
     * @param colors The list of colors to apply to the stream.
     * @param cursors The list of cursor positions to apply to the stream.
     */
    StylerProxyImpl(
        std::ostream &stream,
        std::stack<fly::logger::Style> &&styles,
        std::stack<fly::logger::Color> &&colors,
        std::stack<fly::logger::Cursor> &&cursors) noexcept;

    /**
     * Destructor. Reset the stream's style and color to its original state.
     */
    ~StylerProxyImpl() override;

private:
    /**
     * Apply a modifier value as a bitmask to a set of modifier attributes.
     *
     * @tparam Attributes The type of the bitmask to modify.
     * @tparam Modifier The type of the modifier to apply.
     *
     * @param attributes The bitmask to apply the modifier to.
     * @param modifier The modifier to apply.
     */
    template <typename Attributes, typename Modifier>
    void apply_value(Attributes &attributes, Modifier const &modifier);

    /**
     * Apply the provided styles and colors to the stream.
     *
     * @param console_info Structure holding the current information about the console screen.
     * @param styles The list of styles to apply to the stream.
     * @param colors The list of colors to apply to the stream.
     */
    void apply_styles_and_colors(
        CONSOLE_SCREEN_BUFFER_INFO const &console_info,
        std::stack<fly::logger::Style> &&styles,
        std::stack<fly::logger::Color> &&colors);

    /**
     * Apply the provided cursor positions to the stream.
     *
     * @param console_info Structure holding the current information about the console screen.
     * @param cursors The list of cursor positions to apply to the stream.
     */
    void apply_cursors(
        CONSOLE_SCREEN_BUFFER_INFO const &console_info,
        std::stack<fly::logger::Cursor> &&cursors);

    HANDLE m_handle {INVALID_HANDLE_VALUE};
    WORD m_original_attributes {0};

    bool m_did_apply_style_or_color {false};
};

} // namespace fly::logger::detail
