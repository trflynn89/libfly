#include "fly/types/bit_stream/detail/bit_stream.hpp"

namespace fly::detail {

//==============================================================================
BitStream::BitStream(byte_type starting_position) noexcept :
    m_position(starting_position),
    m_buffer(0)
{
}

} // namespace fly::detail
