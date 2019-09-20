#include "fly/types/bit_stream/bit_stream.h"

namespace fly {

//==============================================================================
BitStream::BitStream(byte_type startingPosition) noexcept :
    m_position(startingPosition),
    m_buffer(0)
{
}

} // namespace fly
