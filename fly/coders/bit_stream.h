#pragma once

#include "fly/coders/endian.h"

#include <cstdint>
#include <istream>

namespace fly {

typedef std::uint8_t byte_type;
typedef std::uint16_t word_type;
typedef std::size_t buffer_type;

/**
 * Base class for writing to and reading from a binary stream. The first byte of
 * the binary stream is reserved as a header for internal use.
 *
 * The BitStream implementations allow reading and writing content bit-by-bit.
 * Of course, files cannot contain partial bytes. If a BitStream is closed with
 * a partial byte remaining to be written, that byte is zero-filled, and the
 * number of extra bits written is encoded into the header.
 *
 * The format of the header byte is then:
 *
 *     |    5 bits    |           3 bits           |
 *     ---------------------------------------------
 *     | Magic number | Number of zero-filled bits |
 *
 * Each BitStream implementation essentially serves as a wrapper around an
 * already existing std::istream or std::ostream. It is expected that the
 * pre-existing stream outlive the wrapper BitStream instance.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
class BitStream
{
public:
    /**
     * Destructor.
     */
    virtual ~BitStream() = default;

protected:
    /**
     * Protected constructor to prevent instantiating this class directly.
     *
     * @param byte_type Initial position to store.
     */
    BitStream(byte_type) noexcept;

    /**
     * Create a bit-mask with the least-significant bits set. The size of the
     * mask is determined by the template DataType parameter.
     *
     * @tparam DataType The data type storing the number of bits to set.
     *
     * @param DataType The number of bits to set.
     *
     * @return DataType The created mask.
     */
    template <typename DataType>
    constexpr static DataType GenerateMask(const DataType);

    byte_type m_position;
    buffer_type m_buffer;
};

/**
 * Implementation of the BitStream interface for writing to a binary stream.
 *
 * Bits are written to an in-memory byte buffer until that buffer is full, at
 * which point that buffer is flushed to the stream. At destruction, if the byte
 * buffer contains a partially-filled byte, that byte is zero-filled and flushed
 * to the stream.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
class BitStreamWriter : public BitStream
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
     * Destructor. If needed, zero-fill the byte buffer, flush it to the stream,
     * and update the header byte.
     */
    ~BitStreamWriter() noexcept override;

    /**
     * Write a multibyte word to the byte buffer. Flush the buffer to the stream
     * if it is filled during this operation.
     *
     * @param word_type The word to write.
     *
     * @return bool True if the word was successfully written and flushing the
     *              byte buffer was successful (if needed).
     */
    bool WriteWord(word_type) noexcept;

    /**
     * Write a full byte to the byte buffer. Flush the buffer to the stream if
     * it is filled during this operation.
     *
     * @param byte_type The byte to write.
     *
     * @return bool True if the byte was successfully written and flushing the
     *              byte buffer was successful (if needed).
     */
    bool WriteByte(byte_type) noexcept;

    /**
     * Write a number of bits to the byte buffer. The least-significant bits in
     * the provided data type will be written. Flush the buffer to the stream if
     * it is filled during this operation.
     *
     * @tparam DataType The data type storing the bits to write.
     *
     * @param DataType The bits to write.
     * @param byte_type The number of bits to write.
     *
     * @return bool True if the bits were successfully written and flushing the
     *              byte buffer was successful (if needed).
     */
    template <typename DataType>
    bool WriteBits(DataType, byte_type) noexcept;

private:
    /**
     * Flush the header byte onto the stream.
     *
     * @param byte_type The number of zero-filled bits in the byte buffer.
     *
     * @return bool True if the header byte was successfully flushed.
     */
    bool flushHeader(byte_type) noexcept;

    /**
     * Flush the byte buffer onto the stream.
     *
     * @return bool True if the byte buffer was successfully flushed.
     */
    bool flushBuffer() noexcept;

    /**
     * Flush a byte buffer to the stream.
     *
     * @tparam BufferType The type of the byte buffer to flush.
     *
     * @param BufferType The byte buffer to flush.
     * @param byte_type The number of bytes to flush.
     *
     * @return bool True if the byte buffer was sucessfully flushed and the
     *              stream remains in a good state.
     */
    template <typename BufferType>
    bool flush(const BufferType &, byte_type) noexcept;

    std::iostream &m_stream;
};

/**
 * Implementation of the BitStream interface for reading from a binary stream.
 *
 * The stream is read in a lazy manner; bytes are not read from the stream until
 * they are needed. The number of bytes read from the stream at once is defined
 * by the size of buffer_type. That buffer is stored in-memory until it has been
 * entirely consumed by the caller, at which point it is refilled.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
class BitStreamReader : public BitStream
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
     * Read a number of bits from the byte buffer. The most-significant bits in
     * the provided data type will be written. Fill the buffer from the stream
     * if the number of bits to read exceeds the number of bits available.
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
     * The most-significant bits in the provided data type will be written. Fill
     * the buffer from the stream if the number of bits to peek exceeds the
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
     * @tparam BufferType The type of the byte buffer to fill.
     *
     * @param BufferType The byte buffer to fill.
     * @param byte_type The number of bytes to read.
     *
     * @return byte_type The number of bytes actually read.
     */
    template <typename BufferType>
    byte_type fill(BufferType &, byte_type) noexcept;

    std::istream &m_stream;
    byte_type m_remainder;
};

//==============================================================================
template <typename DataType>
constexpr DataType BitStream::GenerateMask(const DataType bits)
{
    constexpr auto filledMask = std::numeric_limits<DataType>::max();
    constexpr auto numberOfBits = std::numeric_limits<DataType>::digits;

    return filledMask >> (numberOfBits - bits);
}

//==============================================================================
template <typename DataType>
bool BitStreamWriter::WriteBits(DataType bits, byte_type size) noexcept
{
    // If there are more bits to write than are available in the byte buffer,
    // break the bits into two chunks.
    if (size > m_position)
    {
        const byte_type diff = size - m_position;

        // Fill the remainder of the byte buffer with as many bits as are
        // available, and flush it onto the stream.
        m_buffer |= (static_cast<buffer_type>(bits) >> diff);

        if (!flushBuffer())
        {
            return false;
        }

        // Then update the input bits to retain only those bits that have not
        // been written yet.
        bits &= GenerateMask<DataType>(diff);
        size = diff;
    }

    m_buffer |= (static_cast<buffer_type>(bits) << (m_position - size));
    m_position -= size;

    return (m_position == 0) ? flushBuffer() : true;
}

//==============================================================================
template <typename BufferType>
bool BitStreamWriter::flush(const BufferType &buffer, byte_type bytes) noexcept
{
    if (m_stream.good())
    {
        const BufferType data = byte_swap(buffer);
        m_stream.write(reinterpret_cast<const char *>(&data), bytes);

        return m_stream.good();
    }

    return false;
}

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::ReadBits(byte_type size, DataType &bits) noexcept
{
    const byte_type sizeRead = PeekBits(size, bits);
    DiscardBits(sizeRead);

    return sizeRead;
}

//==============================================================================
template <typename DataType>
byte_type BitStreamReader::PeekBits(byte_type size, DataType &bits) noexcept
{
    if ((size > m_position) && !refillBuffer())
    {
        return 0;
    }

    // If there are more bits to peek than are available in the byte buffer,
    // then only the remaining bits can be returned.
    if (size > m_position)
    {
        bits = (m_buffer & GenerateMask<DataType>(m_position))
            << (size - m_position);
        return m_position;
    }
    else
    {
        bits = (m_buffer >> (m_position - size)) & GenerateMask<DataType>(size);
        return size;
    }
}

//==============================================================================
template <typename BufferType>
byte_type BitStreamReader::fill(BufferType &buffer, byte_type bytes) noexcept
{
    if (m_stream.good())
    {
        m_stream.read(reinterpret_cast<char *>(&buffer), bytes);

        const auto bytesRead = static_cast<byte_type>(m_stream.gcount());
        buffer = byte_swap(buffer);

        return bytesRead;
    }

    return 0;
}

} // namespace fly
