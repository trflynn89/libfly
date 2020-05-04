#pragma once

#include "fly/types/bit_stream/bit_stream_types.hpp"
#include "fly/types/bit_stream/detail/bit_stream.hpp"
#include "fly/types/bit_stream/detail/bit_stream_traits.hpp"
#include "fly/types/numeric/endian.hpp"

#include <algorithm>
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
     * @param istream The stream to read binary data from.
     */
    explicit BitStreamReader(std::istream &) noexcept;

    /**
     * Read a multibyte word from the byte buffer.
     *
     * Fill the buffer from the stream if the number of bits to read exceeds the
     * number of bits available.
     *
     * @param word_type The location to store the read word.
     *
     * @return bool True if the word was successfully read and filling the byte
     *              buffer was successful (if needed).
     */
    bool ReadWord(word_type &) noexcept;

    /**
     * Read a full byte from the byte buffer.
     *
     * Fill the buffer from the stream if the number of bits to read exceeds the
     * number of bits available.
     *
     * @param byte_type The location to store the read byte.
     *
     * @return bool True if the byte was successfully read and filling the byte
     *              buffer was successful (if needed).
     */
    bool ReadByte(byte_type &) noexcept;

    /**
     * Read a number of bits from the byte buffer. There is no guarantee that
     * the requested number of bits will actually be read, as there may be less
     * than that number available between the byte buffer and stream. If any
     * bits were read, the least-significant bits in the provided data type will
     * be filled, starting from the position pointed to by the requested number
     * of bits.
     *
     * Fill the buffer from the stream if the number of bits to read exceeds the
     * number of bits available.
     *
     * @tparam DataType The data type of the location to store the read bits.
     *
     * @param DataType The location to store the read bits.
     * @param byte_type The number of bits to read.
     *
     * @return byte_type The number of bits successfully read.
     */
    template <typename DataType>
    byte_type ReadBits(DataType &, byte_type) noexcept;

    /**
     * Read a number of bits from the byte buffer without discarding those bits.
     * There is no guarantee that the requested number of bits will actually be
     * peeked, as there may be less than that number available between the byte
     * buffer and stream. If any bits were peeked, the least-significant bits in
     * the provided data type will be filled, starting from the position pointed
     * to by the requested number of bits.
     *
     * Fill the buffer from the stream if the number of bits to peek exceeds the
     * number of bits available.
     *
     * @param DataType The location to store the peeked bits.
     * @param byte_type The number of bits to peek.
     *
     * @return byte_type The number of bits successfully peeked.
     */
    template <typename DataType>
    byte_type PeekBits(DataType &, byte_type) noexcept;

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
     */
    void refillBuffer() noexcept;

    /**
     * Read from the stream to fill a byte buffer.
     *
     * @tparam DataType The type of the byte buffer to fill.
     *
     * @param DataType The byte buffer to fill.
     * @param byte_type The number of bytes to read.
     *
     * @return byte_type The number of bytes actually read.
     */
    template <typename DataType>
    byte_type fill(DataType &, byte_type) noexcept;

    std::istream &m_stream;
    byte_type m_remainder;
};

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::ReadBits(DataType &bits, byte_type size) noexcept
{
    static_assert(
        detail::BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    if constexpr (detail::BitStreamTraits::is_buffer_type_v<DataType>)
    {
        // See PeekBits for why buffer_type reads must be split.
        const byte_type sizeHigh = size / 2;
        const byte_type sizeLow = size - sizeHigh;
        std::uint32_t bitsHigh, bitsLow;

        const byte_type bitsReadHigh = PeekBits(bitsHigh, sizeHigh);
        DiscardBits(bitsReadHigh);

        const byte_type bitsReadLow = PeekBits(bitsLow, sizeLow);
        DiscardBits(bitsReadLow);

        bits = (static_cast<DataType>(bitsHigh) << sizeLow) | bitsLow;
        return bitsReadHigh + bitsReadLow;
    }
    else
    {
        const byte_type bitsRead = PeekBits(bits, size);
        DiscardBits(bitsRead);

        return bitsRead;
    }
}

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::PeekBits(DataType &bits, byte_type size) noexcept
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
    // Ideally, given the split peek operations below, the given bits could be
    // filled with the 58 available, then the byte buffer entirely refilled.
    // But the caller then cannot discard more than 6 bits, which invalidates
    // the whole PeekBits/DiscardBits semantic. BitStreamReader would need to
    // support putting bits back onto the stream.
    static_assert(
        !detail::BitStreamTraits::is_buffer_type_v<DataType>,
        "PeekBits only supports types smaller than buffer_type");

    byte_type peeked = 0, lshift = 0;
    bits = 0;

    // If there are more bits to peek than are available in the byte buffer,
    // break the peek into two peeks.
    if (size > m_position)
    {
        peeked = m_position;

        const DataType buffer = static_cast<DataType>(m_buffer);
        lshift = size - m_position;

        // Fill the input bits with the remainder of byte buffer and refill the
        // buffer from the stream.
        bits = (buffer & BitMask<DataType>(m_position)) << lshift;
        refillBuffer();

        // Then update the input to only peek any remaining bits next.
        size = std::min(lshift, static_cast<byte_type>(m_position - peeked));
        lshift -= size;
    }

    const byte_type rshift = m_position - peeked - size;
    const DataType buffer = static_cast<DataType>(m_buffer >> rshift);

    bits |= (buffer & BitMask<DataType>(size)) << lshift;
    peeked += size;

    return peeked;
}

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::fill(DataType &buffer, byte_type bytes) noexcept
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
