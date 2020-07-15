#include "fly/logger/styler.hpp"

#include "fly/fly.hpp"

// Formatter is disabled because it would put a space before and after the solidus.
// clang-format off
#include FLY_OS_IMPL_PATH(logger/detail, styler_proxy)
// clang-format on

namespace fly {

//==================================================================================================
detail::StylerProxy &operator<<(std::ostream &stream, const Styler &styler)
{
    // The input styler parameter must be const in order to allow inline streaming of Styler
    // instances. Otherwise, this method would require an lvalue reference.
    Styler &mutable_styler = const_cast<Styler &>(styler);

    mutable_styler.m_proxy = std::make_unique<detail::StylerProxyImpl>(
        stream,
        std::move(mutable_styler.m_styles),
        std::move(mutable_styler.m_colors),
        std::move(mutable_styler.m_cursors));

    return *(styler.m_proxy);
}

} // namespace fly
