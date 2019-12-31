#pragma once

#include "fly/types/bit_stream/bit_stream_types.h"
#include "fly/types/bit_stream/detail/bit_stream.h"
#include "fly/types/bit_stream/detail/bit_stream_traits.h"
#include "fly/types/numeric/endian.h"

#include <iostream>

namespace fly {

/**
 * Implementation of the BitStream interface for writing to a binary stream.
 *
 * Bits are written to an in-memory byte buffer until that buffer is full, at
 * which point that buffer is flushed to the stream. When done writing, callers
 * should invoke the Finish() method to flush the BitStream header and any bytes
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
     * The wrapped stream must be both an input and output stream. This allows
     * going back and rewriting the header byte to contain the number of
     * zero-filled bits.
     *
     * @param iostream The stream to write binary data into.
     */
    BitStreamWriter(std::iostream &) noexcept;

    /**
     * Write a multibyte word to the byte buffer. Flush the buffer to the stream
     * if it is filled during this operation.
     *
     * @param word_type The word to write.
     */
    void WriteWord(word_type) noexcept;

    /**
     * Write a full byte to the byte buffer. Flush the buffer to the stream if
     * it is filled during this operation.
     *
     * @param byte_type The byte to write.
     */
    void WriteByte(byte_type) noexcept;

    /**
     * Write a number of bits to the byte buffer. The least-significant bits in
     * the provided data type will be written, starting from the position
     * pointed to by the provided number of bits. Flush the buffer to the stream
     * if it is filled during this operation.
     *
     * @tparam DataType The data type storing the bits to write.
     *
     * @param DataType The bits to write.
     * @param byte_type The number of bits to write.
     */
    template <typename DataType>
    void WriteBits(DataType, byte_type) noexcept;

    /**
     * If needed, zero-fill the byte buffer, flush it to the stream, and update
     * the header byte.
     *
     * @return bool True if the stream remains in a good state.
     */
    bool Finish() noexcept;

private:
    /**
     * Flush the header byte onto the stream.
     *
     * @param byte_type The number of zero-filled bits in the byte buffer.
     */
    void flushHeader(byte_type) noexcept;

    /**
     * Flush the byte buffer onto the stream.
     */
    void flushBuffer() noexcept;

    /**
     * Flush a byte buffer to the stream.
     *
     * @tparam DataType The type of the byte buffer to flush.
     *
     * @param DataType The byte buffer to flush.
     * @param byte_type The number of bytes to flush.
     */
    template <typename DataType>
    void flush(const DataType &, byte_type) noexcept;

    std::iostream &m_stream;
};

//==============================================================================
template <typename DataType>
void BitStreamWriter::WriteBits(DataType bits, byte_type size) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    // If there are more bits to write than are available in the byte buffer,
    // break the bits into two chunks.
    if (size > m_position)
    {
        const byte_type diff = size - m_position;

        // Fill the remainder of the byte buffer with as many bits as are
        // available, and flush it onto the stream.
        m_buffer |= (static_cast<buffer_type>(bits) >> diff);
        flushBuffer();

        // Then update the input bits to retain only those bits that have not
        // been written yet.
        bits &= GenerateMask<DataType>(diff);
        size = diff;
    }

    const byte_type diff = m_position - size;

    m_buffer |= static_cast<buffer_type>(bits) << diff;
    m_position = diff;
}

//==============================================================================
template <typename DataType>
void BitStreamWriter::flush(const DataType &buffer, byte_type bytes) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    const DataType data = byte_swap<Endian::Big>(buffer);
    m_stream.write(reinterpret_cast<const std::ios::char_type *>(&data), bytes);
}

} // namespace fly
