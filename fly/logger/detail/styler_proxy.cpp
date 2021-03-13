#include "fly/logger/detail/styler_proxy.hpp"

#include <iostream>

namespace fly::logger::detail {

//==================================================================================================
StylerProxy::StylerProxy(std::ostream &stream) noexcept :
    m_stream(stream),
    m_stream_is_stdout(m_stream.rdbuf() == std::cout.rdbuf()),
    m_stream_is_stderr(m_stream.rdbuf() == std::cerr.rdbuf())
{
}

} // namespace fly::logger::detail
