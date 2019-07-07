#include "fly/coders/bit_stream.h"

namespace fly {

namespace {

    constexpr const byte_type s_magic = 0x1a;
    constexpr const byte_type s_magicMask = 0x1f;
    constexpr const byte_type s_magicShift = 0x03;

    constexpr const byte_type s_remainderMask = 0x07;
    constexpr const byte_type s_remainderShift = 0x00;

    constexpr const byte_type s_byteTypeSize = sizeof(byte_type);
    constexpr const byte_type s_bufferTypeSize = sizeof(buffer_type);

    constexpr const byte_type s_bitsPerByte = s_byteTypeSize * 8;

    constexpr const byte_type s_mostSignificantBitPosition =
        s_bufferTypeSize * s_bitsPerByte;

} // namespace

//==============================================================================
BitStream::BitStream(byte_type startingPosition) noexcept :
    m_position(startingPosition),
    m_buffer(0)
{
    static_assert(s_magic <= s_magicMask, "Magic header has exceeded 5 bits");
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

    if (m_stream.good() && (bitsInBuffer > 0))
    {
        if (flush(m_buffer, bitsToFlush / s_bitsPerByte))
        {
            const byte_type remainder = (bitsToFlush - bitsInBuffer);
            flushHeader(remainder);
        }
    }
}

//==============================================================================
bool BitStreamWriter::WriteByte(byte_type byte) noexcept
{
    for (byte_type shift = s_bitsPerByte; shift != 0; --shift)
    {
        const bool bit = (byte >> (shift - 1)) & 0x1;

        if (!WriteBit(bit))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool BitStreamWriter::WriteBit(bool bit) noexcept
{
    m_buffer |= (static_cast<buffer_type>(bit) << --m_position);

    if (m_position == 0)
    {
        if (!flush(m_buffer, s_bufferTypeSize))
        {
            return false;
        }

        m_position = s_mostSignificantBitPosition;
        m_buffer = 0;
    }

    return true;
}

//==============================================================================
bool BitStreamWriter::flushHeader(byte_type remainder) noexcept
{
    if (m_stream.good())
    {
        // Always write the header in the first byte position. Because this is
        // currently only called during construction and destruction, don't
        // bother resetting the position back to where it was originally.
        m_stream.seekp(0);

        const byte_type header =
            (s_magic << s_magicShift) | (remainder << s_remainderShift);

        return flush(header, s_byteTypeSize);
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

    const byte_type bytesRead = fill(header, s_byteTypeSize);

    if (bytesRead == 1)
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
bool BitStreamReader::ReadByte(byte_type &byte) noexcept
{
    byte = 0;

    for (byte_type shift = s_bitsPerByte; shift != 0; --shift)
    {
        bool bit;

        if (ReadBit(bit))
        {
            byte |= (bit << (shift - 1));
        }
        else
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool BitStreamReader::ReadBit(bool &bit) noexcept
{
    if (m_position == 0)
    {
        const byte_type bytesRead = fill(m_buffer, s_bufferTypeSize);

        if (bytesRead == 0)
        {
            return false;
        }

        byte_type shift = (s_bufferTypeSize - bytesRead) * s_bitsPerByte;

        if (ReachedEndOfFile())
        {
            // At end-of-file, discard any encoded zero-filled bits.
            shift += m_remainder;
        }

        if (shift > 0)
        {
            m_position = s_mostSignificantBitPosition - shift;
            m_buffer >>= shift;
        }
        else
        {
            m_position = s_mostSignificantBitPosition;
        }
    }

    bit = (m_buffer >> --m_position) & 0x1;
    return true;
}

//==============================================================================
bool BitStreamReader::ReachedEndOfFile() const noexcept
{
    return m_stream.eof() || (m_stream.good() && (m_stream.peek() == EOF));
}

} // namespace fly
