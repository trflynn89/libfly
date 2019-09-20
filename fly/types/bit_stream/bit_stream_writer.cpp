#include "fly/types/bit_stream/bit_stream_writer.h"

#include "fly/literals.h"
#include "fly/types/bit_stream/bit_stream_constants.h"

namespace fly {

//==============================================================================
BitStreamWriter::BitStreamWriter(std::iostream &stream) noexcept :
    BitStream(s_mostSignificantBitPosition),
    m_stream(stream)
{
    flushHeader(0_u8);
}

//==============================================================================
void BitStreamWriter::WriteWord(word_type word) noexcept
{
    WriteBits(word, s_bitsPerWord);
}

//==============================================================================
void BitStreamWriter::WriteByte(byte_type byte) noexcept
{
    WriteBits(byte, s_bitsPerByte);
}

//==============================================================================
bool BitStreamWriter::Finish() noexcept
{
    const byte_type bitsInBuffer = s_mostSignificantBitPosition - m_position;
    const byte_type bitsToFlush = bitsInBuffer + (m_position % s_bitsPerByte);

    if (bitsInBuffer > 0)
    {
        flush(m_buffer, bitsToFlush / s_bitsPerByte);
        m_position = s_mostSignificantBitPosition;
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

    const byte_type header =
        (s_magic << s_magicShift) | (remainder << s_remainderShift);
    flush(header, s_byteTypeSize);

    m_stream.seekp(0, std::ios::end);
}

//==============================================================================
void BitStreamWriter::flushBuffer() noexcept
{
    flush(m_buffer, s_bufferTypeSize);

    m_position = s_mostSignificantBitPosition;
    m_buffer = 0;
}

} // namespace fly
