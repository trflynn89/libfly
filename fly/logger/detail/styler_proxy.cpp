#include "fly/logger/detail/styler_proxy.hpp"

namespace fly::detail {

//==================================================================================================
StylerProxy::StylerProxy(std::ostream &stream) noexcept : m_stream(stream)
{
}

} // namespace fly::detail
