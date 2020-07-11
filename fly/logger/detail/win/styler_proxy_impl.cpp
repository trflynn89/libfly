#include "fly/logger/detail/win/styler_proxy_impl.hpp"

#include "fly/fly.hpp"

namespace fly::detail {

//==================================================================================================
StylerProxyImpl::StylerProxyImpl(
    std::ostream &stream,
    std::stack<Style> &&styles,
    std::stack<Color> &&colors) noexcept :
    StylerProxy(stream)
{
    FLY_UNUSED(styles);
    FLY_UNUSED(colors);
}

//==================================================================================================
StylerProxyImpl::~StylerProxyImpl()
{
}

} // namespace fly::detail
