#include "fly/types/bit_stream/detail/bit_stream.h"

namespace fly::detail {

//==============================================================================
BitStream::BitStream(byte_type startingPosition) noexcept :
    m_position(startingPosition),
    m_buffer(0)
{
}

} // namespace fly::detail
