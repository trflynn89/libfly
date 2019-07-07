#pragma once

#include "fly/coders/endian.h"

#include <cstdint>
#include <istream>

namespace fly {

typedef std::uint8_t byte_type;
typedef std::uint64_t buffer_type;

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
     * Write a single bit to the byte buffer. Flush the buffer to the stream if
     * it is filled during this operation.
     *
     * @param bool The bit to write.
     *
     * @return bool True if the bit was successfully written and flushing the
     *              byte buffer was successful (if needed).
     */
    bool WriteBit(bool) noexcept;

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
     * Read a single bit from the byte buffer. Fill the buffer from the stream
     * if it is fully consumed during this operation.
     *
     * @param bool The location to store the read bit.
     *
     * @return bool True if the bit was successfully read and filling the byte
     *              buffer was successful (if needed).
     */
    bool ReadBit(bool &) noexcept;

    /**
     * Check if the stream has reached end-of-file.
     *
     * @return bool True if the stream has reached end-of-file.
     */
    bool ReachedEndOfFile() const noexcept;

private:
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
