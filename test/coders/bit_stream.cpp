#include "fly/coders/bit_stream.h"

#include "fly/literals.h"

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

// Keep in sync with //fly/coders/bit_stream.cpp.
constexpr const fly::byte_type s_magic = 0x1a;
constexpr const fly::byte_type s_magicMask = 0x1f;
constexpr const fly::byte_type s_magicShift = 0x03;
constexpr const fly::byte_type s_remainderMask = 0x07;
constexpr const fly::byte_type s_remainderShift = 0x00;

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
        magic = (header >> s_magicShift) & s_magicMask;
        remainder = (header >> s_remainderShift) & s_remainderMask;

        return true;
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
    EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
    EXPECT_TRUE(m_inputStream.fail());
}

//==============================================================================
TEST_F(BitStreamTest, HeaderTest)
{
    {
        fly::BitStreamWriter {m_outputStream};
    }

    // Only a 1-byte header should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 1_u64);

    // The header should be the magic value and no remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, BadHeaderTest)
{
    // The header should be the magic value and no remainder bits.
    fly::byte_type header = (s_magic - 1) << s_magicShift;
    m_outputStream << static_cast<std::ios::char_type>(header);
    m_outputStream << "data";

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read, even though it's invalid.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
        EXPECT_TRUE(m_inputStream.fail());
    }
}

//==============================================================================
TEST_F(BitStreamTest, SingleBitTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.WriteBits(1_u8, 1));
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 7 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 7_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading a single bit should succeed.
        EXPECT_EQ(stream.ReadBits(1, byte), 1_u8);
        EXPECT_EQ(byte, 1_u8);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, SingleByteTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.WriteByte(0xa));
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 0_u8);

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
        EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, SingleWordTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.WriteWord(0xae));
    }

    // A 1-byte header and a 2-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 3_u64);

    // The header should be the magic value and 0 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 0_u8);

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
        EXPECT_EQ(stream.ReadBits(1, word), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, MultiBufferTest)
{
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.WriteBits(0x1ae1ae1a_zu, length));
        EXPECT_TRUE(stream.WriteBits(0xbc9bc9ba_zu, length));
        EXPECT_TRUE(stream.WriteBits(0x1f_u8, 6));
    }

    // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should
    // have been written.
    EXPECT_EQ(
        m_outputStream.str().size(),
        2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

    // The header should be the magic value and 2 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 2_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::buffer_type buffer;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // Reading each full buffer should succeed.
        EXPECT_TRUE(stream.ReadBits(length, buffer));
        EXPECT_EQ(buffer, 0x1ae1ae1a);

        EXPECT_TRUE(stream.ReadBits(length, buffer));
        EXPECT_EQ(buffer, 0xbc9bc9ba);

        // Reading the last bits should succeed.
        EXPECT_TRUE(stream.ReadBits(6, buffer));
        EXPECT_EQ(buffer, 0x1f);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(1, buffer), 0_u8);
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
        EXPECT_FALSE(stream.WriteBits(buffer, length));
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
        EXPECT_TRUE(stream.WriteBits(buffer, length));

        // Close the stream and write more bits. BitStreamWriter will try to
        // flush the stream, which will fail.
        m_outputStream.setstate(std::ios::failbit);
        EXPECT_FALSE(stream.WriteBits(3_u8, 2));
    }

    // A 1-byte header should have been written. Buffer bytes will be dropped.
    EXPECT_EQ(m_outputStream.str().size(), 1_u64);

    // The header should be the magic value and 0 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 0_u8);

    m_inputStream.str(m_outputStream.str());
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(m_inputStream.gcount(), 1);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
        EXPECT_TRUE(stream.FullyConsumed());
    }
}

//==============================================================================
TEST_F(BitStreamTest, InvalidReaderStreamTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.WriteByte(0xa));
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 0_u8);

    // Close the stream before handing it to BitStreamReader.
    m_inputStream.setstate(std::ios::failbit);
    {
        fly::BitStreamReader stream(m_inputStream);
        fly::byte_type byte;

        // The 1-byte header doesn't exist thus should not have been read.
        EXPECT_EQ(m_inputStream.gcount(), 0);

        // No further reads should succeed.
        EXPECT_EQ(stream.ReadBits(1, byte), 0_u8);
    }
}

//==============================================================================
TEST_F(BitStreamTest, FailedReaderStreamTest)
{
    {
        fly::BitStreamWriter stream(m_outputStream);
        EXPECT_TRUE(stream.WriteByte(0xa));
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_outputStream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    fly::byte_type magic = 0, remainder = 0;
    EXPECT_TRUE(ReadHeader(magic, remainder));
    EXPECT_EQ(magic, s_magic);
    EXPECT_EQ(remainder, 0_u8);

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
