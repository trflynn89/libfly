#pragma once

#include "fly/types/bit_stream/bit_stream_types.h"
#include "fly/types/bit_stream/detail/bit_stream.h"
#include "fly/types/bit_stream/detail/bit_stream_traits.h"
#include "fly/types/numeric/endian.h"

#include <cstdint>
#include <istream>

namespace fly {

/**
 * Implementation of the BitStream interface for reading from a binary stream.
 *
 * The stream is read in a lazy manner; bytes are not read from the stream until
 * they are needed. The number of bytes read from the stream at once is defined
 * by the size of buffer_type. That buffer is stored in-memory until it has been
 * entirely consumed by the caller, at which point it is refilled.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class BitStreamReader : public detail::BitStream
{
public:
    /**
     * Constructor. Decode the header byte from the stream. If the header byte
     * is invalid, the stream's fail bit is set.
     *
     * @param iostream The stream to read binary data from.
     */
    BitStreamReader(std::istream &) noexcept;

    /**
     * Read a multibyte word from the byte buffer. Fill the buffer from the
     * stream if it is fully consumed during this operation.
     *
     * @param word_type The location to store the read word.
     *
     * @return bool True if the word was successfully read and filling the byte
     *              buffer was successful (if needed).
     */
    bool ReadWord(word_type &) noexcept;

    /**
     * Read a full byte from the byte buffer. Fill the buffer from the stream if
     * it is fully consumed during this operation.
     *
     * @param byte_type The location to store the read byte.
     *
     * @return bool True if the byte was successfully read and filling the byte
     *              buffer was successful (if needed).
     */
    bool ReadByte(byte_type &) noexcept;

    /**
     * Read a number of bits from the byte buffer. The least-significant bits in
     * the provided data type will be filled, starting from the position pointed
     * to by the provided number of bits. Fill the buffer from the stream if the
     * number of bits to read exceeds the number of bits available.
     *
     * @tparam DataType The data type of the location to store the read bits.
     *
     * @param byte_type The number of bits to read.
     * @param DataType The location to store the read bits.
     *
     * @return byte_type The number of bits successfully read.
     */
    template <typename DataType>
    byte_type ReadBits(byte_type, DataType &) noexcept;

    /**
     * Read a number of bits from the byte buffer without discarding those bits.
     * The least-significant bits in the provided data type will be filled,
     * starting from the position pointed to by the provided number of bits.
     * Fill the buffer from the stream if the number of bits to peek exceeds the
     * number of bits available.
     *
     * @tparam DataType The data type of the location to store the peeked bits.
     *
     * @param byte_type The number of bits to peek.
     * @param DataType The location to store the peeked bits.
     *
     * @return byte_type The number of bits successfully peeked.
     */
    template <typename DataType>
    byte_type PeekBits(byte_type, DataType &) noexcept;

    /**
     * Discard a number of bits from the byte buffer. Should only be used after
     * a successful call to PeekBits.
     *
     * @param byte_type The number of bits to discard.
     */
    void DiscardBits(byte_type) noexcept;

    /**
     * Check if the stream has reached end-of-file and the byte buffer has been
     * fully consumed.
     *
     * @return bool True if the stream has been fully consumed.
     */
    bool FullyConsumed() const noexcept;

private:
    /**
     * Read from the stream to fill the byte buffer.
     *
     * @return bool True if any bits were actually read
     */
    bool refillBuffer() noexcept;

    /**
     * Read from the stream to fill a byte buffer.
     *
     * @tparam DataType The type of the byte buffer to fill.
     *
     * @param byte_type The number of bytes to read.
     * @param DataType The byte buffer to fill.
     *
     * @return byte_type The number of bytes actually read.
     */
    template <typename DataType>
    byte_type fill(byte_type, DataType &) noexcept;

    std::istream &m_stream;
    byte_type m_remainder;
};

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::ReadBits(byte_type size, DataType &bits) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    if constexpr (detail::BitStreamTraits::is_buffer_type_v<DataType>)
    {
        // See PeekBits for why 64-bit read operations must be split.
        const byte_type sizeHigh = size / 2;
        const byte_type sizeLow = size - sizeHigh;
        std::uint32_t bitsHigh, bitsLow;

        const byte_type bitsReadHigh = PeekBits(sizeHigh, bitsHigh);
        DiscardBits(bitsReadHigh);

        const byte_type bitsReadLow = PeekBits(sizeLow, bitsLow);
        DiscardBits(bitsReadLow);

        bits = (static_cast<DataType>(bitsHigh) << sizeLow) | bitsLow;
        return bitsReadHigh + bitsReadLow;
    }
    else
    {
        const byte_type bitsRead = PeekBits(size, bits);
        DiscardBits(bitsRead);

        return bitsRead;
    }
}

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::PeekBits(byte_type size, DataType &bits) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    // Peek operations the site of the byte buffer are not supported because the
    // byte buffer could be in a state where it cannot be refilled.
    //
    // For example, consider a 64-bit byte buffer, and reading 6 bits and then
    // 64 bits. After the 6-bit read, there will be 58 bits left in the buffer;
    // not enough for the next 64 bit read. The buffer must then be refilled,
    // but it cannot because there is less than 1 byte free in the byte buffer.
    //
    // Ideally, that operation could be split into two peeks (fill the given
    // bits with the 58 available, then refill the entire byte buffer). But this
    // would invalidate the PeekBits/DiscardBits semantic. Would need to support
    // putting bits back onto the stream.
    static_assert(
        !detail::BitStreamTraits::is_buffer_type_v<DataType>,
        "PeekBits only supports types smaller than buffer_type");

    byte_type peeked = 0;
    bits = 0;

    // If there are more bits to peek than are available in the byte buffer,
    // break the peek into two.
    if (size > m_position)
    {
        // Fill the remainder of the byte buffer with as many bits as are
        // available, and flush it onto the stream.
        bits = static_cast<DataType>(m_buffer) & BitMask<DataType>(m_position);
        peeked = m_position;

        // Then update the input bits to retain only those bits that have not
        // been written yet.
        size -= m_position;
        bits <<= size;

        if (!refillBuffer())
        {
            return peeked;
        }
    }

    const byte_type diff = m_position - peeked - size;

    // bits |= (m_buffer & BitMask<buffer_type>(position)) >> diff;
    bits |= static_cast<DataType>(m_buffer >> diff) & BitMask<DataType>(size);
    peeked += size;

    return peeked;
}

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::fill(byte_type bytes, DataType &buffer) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    m_stream.read(
        reinterpret_cast<std::ios::char_type *>(&buffer),
        static_cast<std::streamsize>(bytes));

    buffer = endian_swap<Endian::Big>(buffer);

    return static_cast<byte_type>(m_stream.gcount());
}

} // namespace fly
