#include "fly/types/bit_stream/bit_stream_writer.hpp"

#include "fly/types/bit_stream/detail/bit_stream_constants.hpp"
#include "fly/types/numeric/literals.hpp"

namespace fly {

//==================================================================================================
BitStreamWriter::BitStreamWriter(std::ostream &stream) noexcept :
    BitStream(stream.rdbuf(), detail::s_most_significant_bit_position),
    m_stream(stream)
{
    flush_header(0_u8);
}

//==================================================================================================
void BitStreamWriter::write_word(word_type word) noexcept
{
    write_bits(word, detail::s_bits_per_word);
}

//==================================================================================================
void BitStreamWriter::write_byte(byte_type byte) noexcept
{
    write_bits(byte, detail::s_bits_per_byte);
}

//==================================================================================================
bool BitStreamWriter::finish() noexcept
{
    const byte_type bits_in_buffer = detail::s_most_significant_bit_position - m_position;

    if (bits_in_buffer > 0)
    {
        const byte_type bits_to_flush = bits_in_buffer + (m_position % detail::s_bits_per_byte);

        flush(m_buffer, bits_to_flush / detail::s_bits_per_byte);
        m_position = detail::s_most_significant_bit_position;
        m_buffer = 0;

        const byte_type remainder = (bits_to_flush - bits_in_buffer);
        flush_header(remainder);
    }

    return m_stream.good();
}

//==================================================================================================
void BitStreamWriter::flush_header(byte_type remainder) noexcept
{
    m_stream_buffer->pubseekpos(0);

    const byte_type header =
        (detail::s_magic << detail::s_magic_shift) | (remainder << detail::s_remainder_shift);
    flush(header, detail::s_byte_type_size);
}

//==================================================================================================
void BitStreamWriter::flush_buffer() noexcept
{
    flush(m_buffer, detail::s_buffer_type_size);

    m_position = detail::s_most_significant_bit_position;
    m_buffer = 0;
}

} // namespace fly
