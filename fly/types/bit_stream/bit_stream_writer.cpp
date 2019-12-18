#include "fly/types/bit_stream/bit_stream_writer.h"

#include "fly/types/bit_stream/detail/bit_stream_constants.h"
#include "fly/types/literals/literals.h"

namespace fly {

//==============================================================================
BitStreamWriter::BitStreamWriter(std::iostream &stream) noexcept :
    BitStream(detail::s_mostSignificantBitPosition),
    m_stream(stream)
{
    flushHeader(0_u8);
}

//==============================================================================
void BitStreamWriter::WriteWord(word_type word) noexcept
{
    WriteBits(word, detail::s_bitsPerWord);
}

//==============================================================================
void BitStreamWriter::WriteByte(byte_type byte) noexcept
{
    WriteBits(byte, detail::s_bitsPerByte);
}

//==============================================================================
bool BitStreamWriter::Finish() noexcept
{
    const byte_type bitsInBuffer =
        detail::s_mostSignificantBitPosition - m_position;

    if (bitsInBuffer > 0)
    {
        const byte_type bitsToFlush =
            bitsInBuffer + (m_position % detail::s_bitsPerByte);

        flush(m_buffer, bitsToFlush / detail::s_bitsPerByte);
        m_position = detail::s_mostSignificantBitPosition;
        m_buffer = 0;

        const byte_type remainder = (bitsToFlush - bitsInBuffer);
        flushHeader(remainder);
    }

    return m_stream.good();
}

//==============================================================================
void BitStreamWriter::flushHeader(byte_type remainder) noexcept
{
    m_stream.seekp(0);

    const byte_type header = (detail::s_magic << detail::s_magicShift) |
        (remainder << detail::s_remainderShift);
    flush(header, detail::s_byteTypeSize);

    m_stream.seekp(0, std::ios::end);
}

//==============================================================================
void BitStreamWriter::flushBuffer() noexcept
{
    flush(m_buffer, detail::s_bufferTypeSize);

    m_position = detail::s_mostSignificantBitPosition;
    m_buffer = 0;
}

} // namespace fly
