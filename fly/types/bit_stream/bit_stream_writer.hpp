#pragma once

#include "fly/types/bit_stream/detail/bit_stream.hpp"
#include "fly/types/bit_stream/detail/concepts.hpp"
#include "fly/types/bit_stream/types.hpp"
#include "fly/types/numeric/endian.hpp"

#include <bit>
#include <ostream>

namespace fly {

/**
 * Implementation of the BitStream interface for writing to a binary stream.
 *
 * Bits are written to an in-memory byte buffer until that buffer is full, at which point that
 * buffer is flushed to the stream. When done writing, callers should invoke the finish() method to
 * flush the BitStream header and any bytes remaining in the buffer.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class BitStreamWriter : public detail::BitStream
{
public:
    /**
     * Constructor. Write the header byte onto the stream.
     *
     * @param stream The stream to write binary data into.
     */
    explicit BitStreamWriter(std::ostream &stream) noexcept;

    /**
     * Write a multibyte word to the byte buffer.
     *
     * Flush the buffer to the stream if it is filled during this operation.
     *
     * @param word The word to write.
     */
    void write_word(word_type word);

    /**
     * Write a full byte to the byte buffer.
     *
     * Flush the buffer to the stream if it is filled during this operation.
     *
     * @param byte The byte to write.
     */
    void write_byte(byte_type byte);

    /**
     * Write a number of bits to the byte buffer. The least-significant bits in the provided data
     * type will be written, starting from the position pointed to by the provided number of bits.
     *
     * Flush the buffer to the stream if it is filled during this operation.
     *
     * @tparam DataType The data type storing the bits to write.
     *
     * @param bits The bits to write.
     * @param size The number of bits to write.
     */
    template <detail::BitStreamInteger DataType>
    void write_bits(DataType bits, byte_type size);

    /**
     * If needed, zero-fill the byte buffer, flush it to the stream, and update the header byte.
     *
     * @return True if the stream remains in a good state.
     */
    bool finish();

private:
    /**
     * Flush the header byte onto the stream.
     *
     * @param remainder The number of zero-filled bits in the byte buffer.
     */
    void flush_header(byte_type remainder);

    /**
     * Flush the byte buffer onto the stream.
     */
    void flush_buffer();

    /**
     * Flush a byte buffer to the stream.
     *
     * @tparam DataType The type of the byte buffer to flush.
     *
     * @param buffer The byte buffer to flush.
     * @param bytes The number of bytes to flush.
     */
    template <detail::BitStreamInteger DataType>
    void flush(DataType const &buffer, byte_type bytes);

    std::ostream &m_stream;
};

//==================================================================================================
template <detail::BitStreamInteger DataType>
void BitStreamWriter::write_bits(DataType bits, byte_type size)
{
    // If there are more bits to write than are available in the byte buffer, break the bits into
    // two chunks.
    if (size > m_position)
    {
        byte_type const rshift = size - m_position;

        // Fill the remainder of the byte buffer with as many bits as are available, and flush it
        // onto the stream.
        m_buffer |= static_cast<buffer_type>(bits) >> rshift;
        flush_buffer();

        // Then update the input bits to retain only those bits that have not been written yet.
        bits &= bit_mask<DataType>(rshift);
        size = rshift;
    }

    byte_type const lshift = m_position - size;

    m_buffer |= static_cast<buffer_type>(bits) << lshift;
    m_position = lshift;
}

//==================================================================================================
template <detail::BitStreamInteger DataType>
void BitStreamWriter::flush(DataType const &buffer, byte_type bytes)
{
    if (m_stream)
    {
        DataType const data = endian_swap_if_non_native<std::endian::big>(buffer);

        m_stream_buffer->sputn(
            reinterpret_cast<std::ios::char_type const *>(&data),
            static_cast<std::streamsize>(bytes));
    }
}

} // namespace fly
