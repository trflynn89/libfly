#pragma once

#include "fly/logger/detail/styler_proxy.hpp"
#include "fly/logger/styler.hpp"

#include <ostream>
#include <stack>

namespace fly::detail {

/**
 * Linux implementation of the StylerProxy interface. This implementation streams ANSI escape
 * sequences onto the std::ostream.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 11, 2020
 */
class StylerProxyImpl : public StylerProxy
{
public:
    /**
     * Constructor. Manipulate the provided stream with ANSI escape sequences if it is a standard
     * output or error stream.
     *
     * @param stream The stream to manipulate.
     * @param styles The list of styles to apply to the stream.
     * @param colors The list of colors to apply to the stream.
     * @param cursors The list of cursor positions to apply to the stream.
     */
    StylerProxyImpl(
        std::ostream &stream,
        std::stack<Style> &&styles,
        std::stack<Color> &&colors,
        std::stack<Cursor> &&cursors) noexcept;

    /**
     * Destructor. Reset the stream's style and color to its original state.
     */
    ~StylerProxyImpl() override;

private:
    /**
     * Stream a modifier value as its ANSI escape sequence.
     *
     * @tparam Modifier The type of the modifier to stream.
     *
     * @param modifier The modifier to stream.
     */
    template <typename Modifier>
    void stream_value(const Modifier &modifier);

    /**
     * Manipulate the stream with ANSI escape sequences of the provided styles and colors.
     *
     * @param styles The list of styles to apply to the stream.
     * @param colors The list of colors to apply to the stream.
     */
    void apply_styles_and_colors(std::stack<Style> &&styles, std::stack<Color> &&colors);

    /**
     * Manipulate the stream with ANSI escape sequences of the provided cursor positions.
     *
     * @param cursors The list of cursor positions to apply to the stream.
     */
    void apply_cursors(std::stack<Cursor> &&cursors);

    bool m_did_apply_style_or_color {false};
};

} // namespace fly::detail
