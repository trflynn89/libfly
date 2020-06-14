#include "fly/types/bit_stream/bit_stream_reader.hpp"

#include "fly/types/bit_stream/detail/bit_stream_constants.hpp"
#include "fly/types/numeric/literals.hpp"

namespace fly {

//==================================================================================================
BitStreamReader::BitStreamReader(std::istream &stream) noexcept :
    BitStream(stream.rdbuf(), 0),
    m_stream(stream),
    m_header(0),
    m_remainder(0)
{
    byte_type magic = 0;

    // Cannot use read_byte because the remainder bits are not known yet.
    const byte_type bytes_read = fill(m_header, detail::s_byte_type_size);

    if (bytes_read == 1_u8)
    {
        magic = (m_header >> detail::s_magic_shift) & detail::s_magic_mask;
        m_remainder = (m_header >> detail::s_remainder_shift) & detail::s_remainder_mask;
    }

    if (magic != detail::s_magic)
    {
        m_stream.setstate(std::ios::failbit);
    }
}

//==================================================================================================
bool BitStreamReader::read_word(word_type &word)
{
    return read_bits(word, detail::s_bits_per_word) == detail::s_bits_per_word;
}

//==================================================================================================
bool BitStreamReader::read_byte(byte_type &byte)
{
    return read_bits(byte, detail::s_bits_per_byte) == detail::s_bits_per_byte;
}

//==================================================================================================
void BitStreamReader::discard_bits(byte_type size)
{
    m_position -= size;
}

//==================================================================================================
bool BitStreamReader::fully_consumed() const
{
    if (m_stream_buffer->sgetc() == EOF)
    {
        return m_position == 0;
    }

    return false;
}

//==================================================================================================
byte_type BitStreamReader::header() const
{
    return m_header;
}

//==================================================================================================
void BitStreamReader::refill_buffer()
{
    const byte_type bits_to_fill = detail::s_most_significant_bit_position - m_position;
    buffer_type buffer = 0;

    const byte_type bytes_read = fill(buffer, bits_to_fill / detail::s_bits_per_byte);

    if (bytes_read > 0)
    {
        const byte_type bits_read = bytes_read * detail::s_bits_per_byte;
        m_position += bits_read;

        // It is undefined behavior to bit-shift by the size of the value being shifted, i.e. when
        // bitsRead == detail::s_mostSignificantBitPosition. Because bitsRead is at least 1 here,
        // the left-shift can be broken into two operations in order to avoid that undefined
        // behavior.
        m_buffer = (m_buffer << 1) << (bits_read - 1);
        m_buffer |= buffer >> (detail::s_most_significant_bit_position - bits_read);

        if (m_stream_buffer->sgetc() == EOF)
        {
            // At end-of-file, discard any encoded zero-filled bits.
            m_position -= m_remainder;
            m_buffer >>= m_remainder;
        }
    }
}

} // namespace fly
