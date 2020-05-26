#pragma once

#include "fly/types/bit_stream/bit_stream_types.hpp"
#include "fly/types/bit_stream/detail/bit_stream.hpp"
#include "fly/types/bit_stream/detail/bit_stream_traits.hpp"
#include "fly/types/numeric/endian.hpp"

#include <ostream>

namespace fly {

/**
 * Implementation of the BitStream interface for writing to a binary stream.
 *
 * Bits are written to an in-memory byte buffer until that buffer is full, at
 * which point that buffer is flushed to the stream. When done writing, callers
 * should invoke the finish() method to flush the BitStream header and any bytes
 * remaining in the buffer.
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
    void write_word(word_type word) noexcept;

    /**
     * Write a full byte to the byte buffer.
     *
     * Flush the buffer to the stream if it is filled during this operation.
     *
     * @param byte The byte to write.
     */
    void write_byte(byte_type byte) noexcept;

    /**
     * Write a number of bits to the byte buffer. The least-significant bits in
     * the provided data type will be written, starting from the position
     * pointed to by the provided number of bits.
     *
     * Flush the buffer to the stream if it is filled during this operation.
     *
     * @tparam DataType The data type storing the bits to write.
     *
     * @param bits The bits to write.
     * @param size The number of bits to write.
     */
    template <typename DataType>
    void write_bits(DataType bits, byte_type size) noexcept;

    /**
     * If needed, zero-fill the byte buffer, flush it to the stream, and update
     * the header byte.
     *
     * @return True if the stream remains in a good state.
     */
    bool finish() noexcept;

private:
    /**
     * Flush the header byte onto the stream.
     *
     * @param remainder The number of zero-filled bits in the byte buffer.
     */
    void flush_header(byte_type remainder) noexcept;

    /**
     * Flush the byte buffer onto the stream.
     */
    void flush_buffer() noexcept;

    /**
     * Flush a byte buffer to the stream.
     *
     * @tparam DataType The type of the byte buffer to flush.
     *
     * @param buffer The byte buffer to flush.
     * @param bytes The number of bytes to flush.
     */
    template <typename DataType>
    void flush(const DataType &buffer, byte_type bytes) noexcept;

    std::ostream &m_stream;
};

//==============================================================================
template <typename DataType>
void BitStreamWriter::write_bits(DataType bits, byte_type size) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    // If there are more bits to write than are available in the byte buffer,
    // break the bits into two chunks.
    if (size > m_position)
    {
        const byte_type rshift = size - m_position;

        // Fill the remainder of the byte buffer with as many bits as are
        // available, and flush it onto the stream.
        m_buffer |= static_cast<buffer_type>(bits) >> rshift;
        flush_buffer();

        // Then update the input bits to retain only those bits that have not
        // been written yet.
        bits &= bit_mask<DataType>(rshift);
        size = rshift;
    }

    const byte_type lshift = m_position - size;

    m_buffer |= static_cast<buffer_type>(bits) << lshift;
    m_position = lshift;
}

//==============================================================================
template <typename DataType>
void BitStreamWriter::flush(const DataType &buffer, byte_type bytes) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    if (m_stream)
    {
        const DataType data = endian_swap<Endian::Big>(buffer);

        m_stream_buffer->sputn(
            reinterpret_cast<const std::ios::char_type *>(&data),
            static_cast<std::streamsize>(bytes));
    }
}

} // namespace fly
