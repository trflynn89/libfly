#include "fly/types/bit_stream/bit_stream_reader.h"
#include "fly/types/bit_stream/bit_stream_writer.h"
#include "fly/types/bit_stream/detail/bit_stream_constants.h"
#include "fly/types/numeric/literals.h"

#include <gtest/gtest.h>

#include <limits>
#include <sstream>

namespace {

constexpr const std::ios::openmode s_inputMode =
    std::ios::in | std::ios::binary;

constexpr const std::ios::openmode s_outputMode =
    std::ios::out | std::ios::binary | std::ios::trunc;

constexpr const std::ios::openmode s_bidrectionalMode =
    s_inputMode | s_outputMode;

} // namespace

//==============================================================================
class BitStreamTest : public ::testing::Test
{
public:
    BitStreamTest() :
        m_inputStream(s_inputMode),
        m_outputStream(s_bidrectionalMode)
    {
    }

protected:
    bool ReadHeader(fly::byte_type &magic, fly::byte_type &remainder)
    {
        const std::string buffer = m_outputStream.str();

        if (buffer.empty())
        {
            return false;
        }

        const fly::byte_type header = static_cast<fly::byte_type>(buffer[0]);

        magic =
            (header >> fly::detail::s_magicShift) & fly::detail::s_magicMask;
        remainder = (header >> fly::detail::s_remainderShift) &
            fly::detail::s_remainderMask;

        return true;
    }

    void VerifyHeader(fly::byte_type expectedRemainder)
    {
        fly::byte_type magic = 0, actualRemainder = 0;
        EXPECT_TRUE(ReadHeader(magic, actualRemainder));

        EXPECT_EQ(magic, fly::detail::s_magic);
        EXPECT_EQ(actualRemainder, expectedRemainder);
    }

    std::istringstream m_inputStream;
    std::stringstream m_outputStream;
};

//==============================================================================
TEST_F(BitStreamTest, EmptyStreamTest)
{
    fly::BitStreamReader stream(m_inputStream);
    fly::byte_type byte;

    // The 1-byte header doesn't exist thus should not have been read.
    EXPECT_EQ(m_inputStream.gcount(), 0);

    // No further reads should succeed.
    EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
    EXPECT_TRUE(m_inputStream.fail());
}

//==============================================================================
TEST_F(BitStreamTest, HeaderTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.Finish());
    }

    // Only a 1-byte header should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 1_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, BadHeaderTest)
{
    fly::byte_type header = (fly::detail::s_magic - 1)
        << fly::detail::s_magicShift;
    m_outputStream << static_cast<std::ios::char_type>(header);
    m_outputStream << "data";

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read, even though it's invalid.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(m_inputStream.fail());
    }
}

//==============================================================================
TEST_F(BitStreamTest, SingleBitTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteBits(1_u8, 1);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 7 remainder bits.
    VerifyHeader(7_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading a single bit should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 1_u8);
        EXPECT_EQ(byte, 1_u8);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, SingleByteTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteByte(0xa);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading a single byte should succeed.
        EXPECT_TRUE(stream.ReadByte(byte));
        EXPECT_EQ(byte, 0xa);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, SingleWordTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteWord(0xae);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 2-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 3_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::word_type word;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading a single word should succeed.
        EXPECT_TRUE(stream.ReadWord(word));
        EXPECT_EQ(word, 0xae);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(word, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, MultiBufferTest)
{
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteBits(0xae1ae1ae1ae1ae1a_u64, length);
        stream.WriteBits(0x1f_u8, 5);
        stream.WriteBits(0xbc9bc9bc9bc9bc9b_u64, length);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should
    // have been written.
    EXPECT_EQ(
        m_outputStream.str().size(),
        2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

    // The header should be the magic value and 3 remainder bits.
    VerifyHeader(3_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::buffer_type buffer;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading all written bits should succeed.
        EXPECT_EQ(stream.ReadBits(buffer, 64), 64_u8);
        EXPECT_EQ(buffer, 0xae1ae1ae1ae1ae1a_u64);

        EXPECT_EQ(stream.ReadBits(buffer, 15), 15_u8);
        EXPECT_EQ(buffer, 0x7ef2);

        EXPECT_EQ(stream.ReadBits(buffer, 54), 54_u8);
        EXPECT_EQ(buffer, 0x1bc9bc9bc9bc9b_u64);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(buffer, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, MultiBufferSplitTest)
{
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteBits(0xae1ae1ae1ae1ae1a_u64, length);
        stream.WriteBits(0x1f_u8, 5);
        stream.WriteBits(0xbc9bc9bc9bc9bc9b_u64, length);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should
    // have been written.
    EXPECT_EQ(
        m_outputStream.str().size(),
        2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

    // The header should be the magic value and 3 remainder bits.
    VerifyHeader(3_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::buffer_type buffer;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading all written bits should succeed. Here, the bits are read in
        // an order such that the second and third read must be split because
        // they each read more than is available in the internal byte buffer.
        EXPECT_EQ(stream.ReadBits(buffer, 6), 6_u8);
        EXPECT_EQ(buffer, 0x2b);

        EXPECT_EQ(stream.ReadBits(buffer, 64), 64_u8);
        EXPECT_EQ(buffer, 0x86b86b86b86b86bf_u64);

        EXPECT_EQ(stream.ReadBits(buffer, 63), 63_u8);
        EXPECT_EQ(buffer, 0x3c9bc9bc9bc9bc9b_u64);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(buffer, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, PeekTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteByte(0xa);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Peeking a single byte multiple times should succeed.
        for (std::uint8_t i = 0; i < 10; ++i)
        {
            EXPECT_EQ(stream.PeekBits(byte, 8_u8), 8_u8);
            EXPECT_EQ(byte, 0xa);
        }

        // After discarding the peeked bits, no further reads should succeed.
        stream.DiscardBits(8_u8);
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, OverPeekTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteBits(0x7f_u8, 7_u8);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 1 remainder bit.
    VerifyHeader(1_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Trying to peek 8 bits now should result in only 7 bits being peeked.
        EXPECT_EQ(stream.PeekBits(byte, 8_u8), 7_u8);
        EXPECT_EQ(byte, 0x7f << 1);

        // After discarding the peeked bits, no further reads should succeed.
        stream.DiscardBits(7_u8);
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, InvalidWriterStreamTest)
{
    // Close the stream before handing it to BitStreamWriter.
    m_outputStream.setstate(std::ios::failbit);

    constexpr auto buffer = std::numeric_limits<fly::buffer_type>::max();
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_outputStream);

        // Fill the internal byte buffer. BitStreamWriter will try to flush the
        // stream, which will fail.
        stream.WriteBits(buffer, length);
        EXPECT_FALSE(stream.Finish());
    }

    // The 1-byte should not have been written.
    EXPECT_TRUE(m_outputStream.str().empty());
}

//==============================================================================
TEST_F(BitStreamTest, FailedWriterStreamTest)
{
    constexpr auto buffer = std::numeric_limits<fly::buffer_type>::max() >> 1;
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits - 1;
    {
        fly::BitStreamWriter stream(m_outputStream);

        // Fill the internal byte buffer with all but one bit.
        stream.WriteBits(buffer, length);

        // Close the stream and write more bits. BitStreamWriter will try to
        // flush the stream, which will fail.
        m_outputStream.setstate(std::ios::failbit);
        stream.WriteBits(3_u8, 2);
        EXPECT_FALSE(stream.Finish());
    }

    // A 1-byte header should have been written. Buffer bytes will be dropped.
    EXPECT_EQ(m_outputStream.str().size(), 1_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, InvalidReaderStreamTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteByte(0xa);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    // Close the stream before handing it to BitStreamReader.
    m_inputStream.setstate(std::ios::failbit);
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header doesn't exist thus should not have been read.
        EXPECT_EQ(m_inputStream.gcount(), 0);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(byte, 1), 0_u8);
    }
}

//==============================================================================
TEST_F(BitStreamTest, FailedReaderStreamTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        stream.WriteByte(0xa);
        EXPECT_TRUE(stream.Finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    VerifyHeader(0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Close the stream and read some bits. BitStreamReader will try to
        // fill the internal byte buffer, which will fail.
        m_inputStream.setstate(std::ios::failbit);
        EXPECT_FALSE(stream.ReadByte(byte));
    }
}
