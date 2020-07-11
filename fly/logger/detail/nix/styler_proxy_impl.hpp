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
     * output or error stream, and is directed to a terminal.
     *
     * @param stream The stream to manipulate.
     * @param styles The list of styles to apply to the stream.
     * @param colors The list of colors to apply to the stream.
     */
    StylerProxyImpl(
        std::ostream &stream,
        std::stack<Style> &&styles,
        std::stack<Color> &&colors) noexcept;

    /**
     * Destructor. Reset the stream to its original state.
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

    bool m_did_modify_stream;
};

} // namespace fly::detail
