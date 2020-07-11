#pragma once

#include "fly/logger/detail/styler_proxy.hpp"
#include "fly/logger/styler.hpp"

#include <ostream>
#include <stack>

namespace fly::detail {

/**
 * Windows implementation of the StylerProxy interface. This implementation is currently empty.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 11, 2020
 */
class StylerProxyImpl : public StylerProxy
{
public:
    /**
     * Constructor.
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
     * Destructor.
     */
    ~StylerProxyImpl() override;
};

} // namespace fly::detail
