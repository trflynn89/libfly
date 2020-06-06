#include "fly/types/bit_stream/bit_stream_reader.hpp"
#include "fly/types/bit_stream/bit_stream_writer.hpp"
#include "fly/types/bit_stream/detail/bit_stream_constants.hpp"
#include "fly/types/numeric/literals.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <sstream>

namespace {

constexpr const std::ios::openmode s_input_mode = std::ios::in | std::ios::binary;

constexpr const std::ios::openmode s_output_mode =
    std::ios::out | std::ios::binary | std::ios::trunc;

} // namespace

//==================================================================================================
class BitStreamTest : public ::testing::Test
{
public:
    BitStreamTest() : m_input_stream(s_input_mode), m_output_stream(s_output_mode)
    {
    }

protected:
    fly::byte_type create_header(fly::byte_type remainder)
    {
        return (fly::detail::s_magic << fly::detail::s_magic_shift) |
            (remainder << fly::detail::s_remainder_shift);
    }

    bool read_header(fly::byte_type &magic, fly::byte_type &remainder)
    {
        const std::string buffer = m_output_stream.str();

        if (buffer.empty())
        {
            return false;
        }

        const fly::byte_type header = static_cast<fly::byte_type>(buffer[0]);

        magic = (header >> fly::detail::s_magic_shift) & fly::detail::s_magic_mask;
        remainder = (header >> fly::detail::s_remainder_shift) & fly::detail::s_remainder_mask;

        return true;
    }

    void verify_header(fly::byte_type expected_remainder)
    {
        fly::byte_type magic = 0, actual_remainder = 0;
        EXPECT_TRUE(read_header(magic, actual_remainder));

        EXPECT_EQ(magic, fly::detail::s_magic);
        EXPECT_EQ(actual_remainder, expected_remainder);
    }

    std::istringstream m_input_stream;
    std::ostringstream m_output_stream;
};

//==================================================================================================
TEST_F(BitStreamTest, EmptyStream)
{
    fly::BitStreamReader stream(m_input_stream);
    fly::byte_type byte;

    // The 1-byte header doesn't exist thus should not have been read.
    EXPECT_EQ(stream.header(), 0);

    // No further reads should succeed.
    EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
    EXPECT_TRUE(m_input_stream.fail());
}

//==================================================================================================
TEST_F(BitStreamTest, GoodHeader)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        EXPECT_TRUE(stream.finish());
    }

    // Only a 1-byte header should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 1_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(0_u8));

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, BadHeader)
{
    fly::byte_type header = (fly::detail::s_magic - 1) << fly::detail::s_magic_shift;
    m_output_stream << static_cast<std::ios::char_type>(header);
    m_output_stream << "data";

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read, even though it's invalid.
        EXPECT_EQ(stream.header(), header);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(m_input_stream.fail());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, SingleBit)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_bits(1_u8, 1);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 2_u64);

    // The header should be the magic value and 7 remainder bits.
    verify_header(7_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(7_u8));

        // Reading a single bit should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 1_u8);
        EXPECT_EQ(byte, 1_u8);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, SingleByte)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_byte(0xa);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(0_u8));

        // Reading a single byte should succeed.
        EXPECT_TRUE(stream.read_byte(byte));
        EXPECT_EQ(byte, 0xa);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, SingleWord)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_word(0xae);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 2-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 3_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::word_type word;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(0_u8));

        // Reading a single word should succeed.
        EXPECT_TRUE(stream.read_word(word));
        EXPECT_EQ(word, 0xae);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(word, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, MultiBuffer)
{
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_bits(0xae1ae1ae1ae1ae1a_u64, length);
        stream.write_bits(0x1f_u8, 5);
        stream.write_bits(0xbc9bc9bc9bc9bc9b_u64, length);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should
    // have been written.
    EXPECT_EQ(
        m_output_stream.str().size(),
        2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

    // The header should be the magic value and 3 remainder bits.
    verify_header(3_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::buffer_type buffer;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(3_u8));

        // Reading all written bits should succeed.
        EXPECT_EQ(stream.read_bits(buffer, 64), 64_u8);
        EXPECT_EQ(buffer, 0xae1ae1ae1ae1ae1a_u64);

        EXPECT_EQ(stream.read_bits(buffer, 15), 15_u8);
        EXPECT_EQ(buffer, 0x7ef2);

        EXPECT_EQ(stream.read_bits(buffer, 54), 54_u8);
        EXPECT_EQ(buffer, 0x1bc9bc9bc9bc9b_u64);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(buffer, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, MultiBufferSplit)
{
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_bits(0xae1ae1ae1ae1ae1a_u64, length);
        stream.write_bits(0x1f_u8, 5);
        stream.write_bits(0xbc9bc9bc9bc9bc9b_u64, length);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should
    // have been written.
    EXPECT_EQ(
        m_output_stream.str().size(),
        2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

    // The header should be the magic value and 3 remainder bits.
    verify_header(3_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::buffer_type buffer;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(3_u8));

        // Reading all written bits should succeed. Here, the bits are read in
        // an order such that the second and third read must be split because
        // they each read more than is available in the internal byte buffer.
        EXPECT_EQ(stream.read_bits(buffer, 6), 6_u8);
        EXPECT_EQ(buffer, 0x2b);

        EXPECT_EQ(stream.read_bits(buffer, 64), 64_u8);
        EXPECT_EQ(buffer, 0x86b86b86b86b86bf_u64);

        EXPECT_EQ(stream.read_bits(buffer, 63), 63_u8);
        EXPECT_EQ(buffer, 0x3c9bc9bc9bc9bc9b_u64);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(buffer, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, Peek)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_byte(0xa);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(0_u8));

        // Peeking a single byte multiple times should succeed.
        for (std::uint8_t i = 0; i < 10; ++i)
        {
            EXPECT_EQ(stream.peek_bits(byte, 8_u8), 8_u8);
            EXPECT_EQ(byte, 0xa);
        }

        // After discarding the peeked bits, no further reads should succeed.
        stream.discard_bits(8_u8);
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, OverPeek)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_bits(0x7f_u8, 7_u8);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 2_u64);

    // The header should be the magic value and 1 remainder bit.
    verify_header(1_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(1_u8));

        // Trying to peek 8 bits now should result in only 7 bits being peeked.
        EXPECT_EQ(stream.peek_bits(byte, 8_u8), 7_u8);
        EXPECT_EQ(byte, 0x7f << 1);

        // After discarding the peeked bits, no further reads should succeed.
        stream.discard_bits(7_u8);
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, InvalidWriterStream)
{
    // Close the stream before handing it to BitStreamWriter.
    m_output_stream.setstate(std::ios::failbit);

    constexpr auto buffer = std::numeric_limits<fly::buffer_type>::max();
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
    {
        fly::BitStreamWriter stream(m_output_stream);

        // Fill the internal byte buffer. BitStreamWriter will try to flush the
        // stream, which will fail.
        stream.write_bits(buffer, length);
        EXPECT_FALSE(stream.finish());
    }

    // The 1-byte should not have been written.
    EXPECT_TRUE(m_output_stream.str().empty());
}

//==================================================================================================
TEST_F(BitStreamTest, FailedWriterStream)
{
    constexpr auto buffer = std::numeric_limits<fly::buffer_type>::max() >> 1;
    constexpr auto length = std::numeric_limits<fly::buffer_type>::digits - 1;
    {
        fly::BitStreamWriter stream(m_output_stream);

        // Fill the internal byte buffer with all but one bit.
        stream.write_bits(buffer, length);

        // Close the stream and write more bits. BitStreamWriter will try to
        // flush the stream, which will fail.
        m_output_stream.setstate(std::ios::failbit);
        stream.write_bits(3_u8, 2);
        EXPECT_FALSE(stream.finish());
    }

    // A 1-byte header should have been written. Buffer bytes will be dropped.
    EXPECT_EQ(m_output_stream.str().size(), 1_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(0_u8));

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
        EXPECT_TRUE(stream.fully_consumed());
    }
}

//==================================================================================================
TEST_F(BitStreamTest, InvalidReaderStream)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_byte(0xa);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    // Close the stream before handing it to BitStreamReader.
    m_input_stream.setstate(std::ios::failbit);
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header doesn't exist thus should not have been read.
        EXPECT_EQ(stream.header(), 0);

        // No further reads should succeed.
        EXPECT_EQ(stream.read_bits(byte, 1), 0_u8);
    }
}

//==================================================================================================
TEST_F(BitStreamTest, FailedReaderStream)
{
    {
        fly::BitStreamWriter stream(m_output_stream);
        stream.write_byte(0xa);
        EXPECT_TRUE(stream.finish());
    }

    // A 1-byte header and a 1-byte buffer should have been written.
    EXPECT_EQ(m_output_stream.str().size(), 2_u64);

    // The header should be the magic value and 0 remainder bits.
    verify_header(0_u8);

    m_input_stream.str(m_output_stream.str());
    {
        fly::BitStreamReader stream(m_input_stream);
        fly::byte_type byte;

        // The 1-byte header should have been read.
        EXPECT_EQ(stream.header(), create_header(0_u8));

        // Close the stream and read some bits. BitStreamReader will try to
        // fill the internal byte buffer, which will fail.
        m_input_stream.setstate(std::ios::failbit);
        EXPECT_FALSE(stream.read_byte(byte));
    }
}
