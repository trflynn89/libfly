#include "fly/types/bit_stream/bit_stream_reader.h"

#include "fly/literals.h"
#include "fly/types/bit_stream/bit_stream_constants.h"

namespace fly {

//==============================================================================
BitStreamReader::BitStreamReader(std::istream &stream) noexcept :
    BitStream(0),
    m_stream(stream),
    m_remainder(0)
{
    byte_type header = 0;
    byte_type magic = 0;

    // Cannot use ReadByte because the remainder bits are not known yet.
    const byte_type bytesRead = fill(header, s_byteTypeSize);

    if (bytesRead == 1_u8)
    {
        magic = (header >> s_magicShift) & s_magicMask;
        m_remainder = (header >> s_remainderShift) & s_remainderMask;
    }

    if (magic != s_magic)
    {
        m_stream.setstate(std::ios::failbit);
    }
}

//==============================================================================
bool BitStreamReader::ReadWord(word_type &word) noexcept
{
    return ReadBits(s_bitsPerWord, word) == s_bitsPerWord;
}

//==============================================================================
bool BitStreamReader::ReadByte(byte_type &byte) noexcept
{
    return ReadBits(s_bitsPerByte, byte) == s_bitsPerByte;
}

//==============================================================================
void BitStreamReader::DiscardBits(byte_type size) noexcept
{
    m_position -= size;
}

//==============================================================================
bool BitStreamReader::FullyConsumed() const noexcept
{
    if (m_stream.eof() || (m_stream.peek() == EOF))
    {
        return m_position == 0;
    }

    return false;
}

//==============================================================================
bool BitStreamReader::refillBuffer() noexcept
{
    const byte_type bitsToFill = s_mostSignificantBitPosition - m_position;
    buffer_type buffer = 0;

    const byte_type bytesRead = fill(buffer, bitsToFill / s_bitsPerByte);
    const byte_type bitsRead = bytesRead * s_bitsPerByte;

    if (bitsRead == 0)
    {
        return m_position > 0;
    }
    else
    {
        m_position += bitsRead;

        // It is undefined behavior to bit-shift by the size of the value being
        // shifted, i.e. when bitsRead == s_mostSignificantBitPosition. Because
        // bitsRead is at least 1 here, the left-shift can be broken into two
        // operations in order to avoid that undefined behavior.
        m_buffer = (m_buffer << 1) << (bitsRead - 1);
        m_buffer |= buffer >> (s_mostSignificantBitPosition - bitsRead);
    }

    if (m_stream.peek() == EOF)
    {
        // At end-of-file, discard any encoded zero-filled bits.
        m_position -= m_remainder;
        m_buffer >>= m_remainder;
    }

    return true;
}

} // namespace fly
