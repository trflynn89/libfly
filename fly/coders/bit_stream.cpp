#include "fly/coders/bit_stream.h"

#include "fly/literals.h"

namespace fly {

namespace {

    constexpr const byte_type s_magic = 0x1a;
    constexpr const byte_type s_magicMask = 0x1f;
    constexpr const byte_type s_magicShift = 0x03;

    static_assert(s_magic <= s_magicMask, "Magic header has exceeded 5 bits");

    constexpr const byte_type s_remainderMask = 0x07;
    constexpr const byte_type s_remainderShift = 0x00;

    constexpr const byte_type s_byteTypeSize = sizeof(byte_type);
    constexpr const byte_type s_bufferTypeSize = sizeof(buffer_type);

    constexpr const byte_type s_bitsPerWord =
        std::numeric_limits<word_type>::digits;
    constexpr const byte_type s_bitsPerByte =
        std::numeric_limits<byte_type>::digits;

    constexpr const byte_type s_mostSignificantBitPosition =
        s_bufferTypeSize * s_bitsPerByte;

} // namespace

//==============================================================================
BitStream::BitStream(byte_type startingPosition) noexcept :
    m_position(startingPosition),
    m_buffer(0)
{
}

//==============================================================================
BitStreamWriter::BitStreamWriter(std::iostream &stream) noexcept :
    BitStream(s_mostSignificantBitPosition),
    m_stream(stream)
{
    flushHeader(0);
}

//==============================================================================
BitStreamWriter::~BitStreamWriter() noexcept
{
    const byte_type bitsInBuffer = s_mostSignificantBitPosition - m_position;
    const byte_type bitsToFlush = bitsInBuffer + (m_position % s_bitsPerByte);

    if ((bitsInBuffer > 0) && flush(m_buffer, bitsToFlush / s_bitsPerByte))
    {
        const byte_type remainder = (bitsToFlush - bitsInBuffer);
        flushHeader(remainder);
    }
}

//==============================================================================
bool BitStreamWriter::WriteWord(word_type word) noexcept
{
    return WriteBits(word, s_bitsPerWord);
}

//==============================================================================
bool BitStreamWriter::WriteByte(byte_type byte) noexcept
{
    return WriteBits(byte, s_bitsPerByte);
}

//==============================================================================
bool BitStreamWriter::flushHeader(byte_type remainder) noexcept
{
    // Always write the header in the first byte position. Because this is
    // currently only called during construction and destruction, don't bother
    // resetting the position back to where it was originally.
    m_stream.seekp(0);

    const byte_type header =
        (s_magic << s_magicShift) | (remainder << s_remainderShift);

    return flush(header, s_byteTypeSize);
}

//==============================================================================
bool BitStreamWriter::flushBuffer() noexcept
{
    if (flush(m_buffer, s_bufferTypeSize))
    {
        m_position = s_mostSignificantBitPosition;
        m_buffer = 0;

        return true;
    }

    return false;
}

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
