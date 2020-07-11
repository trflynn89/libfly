#pragma once

#include "fly/logger/detail/styler_proxy.hpp"
#include "fly/logger/styler.hpp"

#include <Windows.h>

#include <ostream>
#include <stack>

namespace fly::detail {

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
     * Apply a modifier value as a bitmask to a set of modifier attributes.
     *
     * @tparam Modifier The type of the modifier to apply.
     *
     * @param attributes The bitmask to apply the modifier to.
     * @param modifier The modifier to apply.
     */
    template <typename Modifier>
    void apply_value(WORD &attributes, const Modifier &modifier);

    HANDLE m_handle;
    WORD m_original_attributes;
};

} // namespace fly::detail
