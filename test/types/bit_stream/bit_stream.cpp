#include "fly/types/bit_stream/bit_stream_reader.hpp"
#include "fly/types/bit_stream/bit_stream_writer.hpp"
#include "fly/types/bit_stream/detail/bit_stream_constants.hpp"
#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_test_macros.hpp"

#include <limits>
#include <sstream>

using namespace fly::literals::numeric_literals;

CATCH_TEST_CASE("BitStream", "[bit_stream]")
{
    std::istringstream input_stream(std::ios::in | std::ios::binary);
    std::ostringstream output_stream(std::ios::out | std::ios::binary);

    auto create_header = [](fly::byte_type remainder) -> fly::byte_type
    {
        return (fly::detail::s_magic << fly::detail::s_magic_shift) |
            (remainder << fly::detail::s_remainder_shift);
    };

    auto verify_header = [&output_stream](fly::byte_type expected_remainder)
    {
        const std::string buffer = output_stream.str();
        CATCH_REQUIRE_FALSE(buffer.empty());

        const fly::byte_type header = static_cast<fly::byte_type>(buffer[0]);

        fly::byte_type magic = (header >> fly::detail::s_magic_shift) & fly::detail::s_magic_mask;
        fly::byte_type remainder =
            (header >> fly::detail::s_remainder_shift) & fly::detail::s_remainder_mask;

        CATCH_CHECK(magic == fly::detail::s_magic);
        CATCH_CHECK(remainder == expected_remainder);
    };

    CATCH_SECTION("Empty reader streams have no header and cannot be read")
    {
        fly::BitStreamReader stream(input_stream);
        fly::byte_type byte;

        // The 1-byte header doesn't exist thus should not have been read.
        CATCH_CHECK(stream.header() == 0);

        // No further reads should succeed.
        CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
        CATCH_CHECK(input_stream.fail());
    }

    CATCH_SECTION("Writing no data to a writer stream must result in only the header being written")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            CATCH_CHECK(stream.finish());
        }

        // Only a 1-byte header should have been written.
        CATCH_CHECK(output_stream.str().size() == 1_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(0_u8));

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Verify detection of bad bit stream headers")
    {
        fly::byte_type header = (fly::detail::s_magic - 1) << fly::detail::s_magic_shift;
        output_stream << static_cast<std::ios::char_type>(header);
        output_stream << "data";

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read, even though it's invalid.
            CATCH_CHECK(stream.header() == header);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(input_stream.fail());
        }
    }

    CATCH_SECTION("Write and read a single bit")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_bits(1_u8, 1);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 1-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 2_u64);

        // The header should be the magic value and 7 remainder bits.
        verify_header(7_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(7_u8));

            // Reading a single bit should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 1_u8);
            CATCH_CHECK(byte == 1_u8);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Write and read a single byte")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_byte(0xa);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 1-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 2_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(0_u8));

            // Reading a single byte should succeed.
            CATCH_CHECK(stream.read_byte(byte));
            CATCH_CHECK(byte == 0xa);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Write and read a single word")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_word(0xae);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 2-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 3_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::word_type word;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(0_u8));

            // Reading a single word should succeed.
            CATCH_CHECK(stream.read_word(word));
            CATCH_CHECK(word == 0xae);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(word, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Write and read multiple byte buffers")
    {
        constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_bits(0xae1ae1ae1ae1ae1a_u64, length);
            stream.write_bits(0x1f_u8, 5);
            stream.write_bits(0xbc9bc9bc9bc9bc9b_u64, length);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should have been
        // written.
        CATCH_CHECK(
            output_stream.str().size() ==
            2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

        // The header should be the magic value and 3 remainder bits.
        verify_header(3_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::buffer_type buffer;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(3_u8));

            // Reading all written bits should succeed.
            CATCH_CHECK(stream.read_bits(buffer, 64) == 64_u8);
            CATCH_CHECK(buffer == 0xae1ae1ae1ae1ae1a_u64);

            CATCH_CHECK(stream.read_bits(buffer, 15) == 15_u8);
            CATCH_CHECK(buffer == 0x7ef2);

            CATCH_CHECK(stream.read_bits(buffer, 54) == 54_u8);
            CATCH_CHECK(buffer == 0x1bc9bc9bc9bc9b_u64);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(buffer, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Write and read multiple byte buffers, reading in an order that splits a read")
    {
        constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_bits(0xae1ae1ae1ae1ae1a_u64, length);
            stream.write_bits(0x1f_u8, 5);
            stream.write_bits(0xbc9bc9bc9bc9bc9b_u64, length);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header, 2 full internal byte buffers, and a 1-byte buffer should have been
        // written.
        CATCH_CHECK(
            output_stream.str().size() ==
            2_u64 + ((length * 2) / std::numeric_limits<fly::byte_type>::digits));

        // The header should be the magic value and 3 remainder bits.
        verify_header(3_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::buffer_type buffer;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(3_u8));

            // Reading all written bits should succeed. Here, the bits are read in an order such
            // that the second and third read must be split because they each read more than is
            // available in the internal byte buffer.
            CATCH_CHECK(stream.read_bits(buffer, 6) == 6_u8);
            CATCH_CHECK(buffer == 0x2b);

            CATCH_CHECK(stream.read_bits(buffer, 64) == 64_u8);
            CATCH_CHECK(buffer == 0x86b86b86b86b86bf_u64);

            CATCH_CHECK(stream.read_bits(buffer, 63) == 63_u8);
            CATCH_CHECK(buffer == 0x3c9bc9bc9bc9bc9b_u64);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(buffer, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Verify peeking bits does not discard bits")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_byte(0xa);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 1-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 2_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(0_u8));

            // Peeking a single byte multiple times should succeed.
            for (std::uint8_t i = 0; i < 10; ++i)
            {
                CATCH_CHECK(stream.peek_bits(byte, 8_u8) == 8_u8);
                CATCH_CHECK(byte == 0xa);
            }

            // After discarding the peeked bits, no further reads should succeed.
            stream.discard_bits(8_u8);
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Try to peek more bits than are available")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_bits(0x7f_u8, 7_u8);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 1-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 2_u64);

        // The header should be the magic value and 1 remainder bit.
        verify_header(1_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(1_u8));

            // Trying to peek 8 bits now should result in only 7 bits being peeked.
            CATCH_CHECK(stream.peek_bits(byte, 8_u8) == 7_u8);
            CATCH_CHECK(byte == 0x7f << 1);

            // After discarding the peeked bits, no further reads should succeed.
            stream.discard_bits(7_u8);
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Verify detection of a writer stream that is initially invalid")
    {
        // Close the stream before handing it to BitStreamWriter.
        output_stream.setstate(std::ios::failbit);

        constexpr auto buffer = std::numeric_limits<fly::buffer_type>::max();
        constexpr auto length = std::numeric_limits<fly::buffer_type>::digits;
        {
            fly::BitStreamWriter stream(output_stream);

            // Fill the internal byte buffer. BitStreamWriter will try to flush the stream, which
            // will fail.
            stream.write_bits(buffer, length);
            CATCH_CHECK_FALSE(stream.finish());
        }

        // The 1-byte should not have been written.
        CATCH_CHECK(output_stream.str().empty());
    }

    CATCH_SECTION("Verify detection of a writer stream that becomes invalid")
    {
        constexpr auto buffer = std::numeric_limits<fly::buffer_type>::max() >> 1;
        constexpr auto length = std::numeric_limits<fly::buffer_type>::digits - 1;
        {
            fly::BitStreamWriter stream(output_stream);

            // Fill the internal byte buffer with all but one bit.
            stream.write_bits(buffer, length);

            // Close the stream and write more bits. BitStreamWriter will try to flush the stream,
            // which will fail.
            output_stream.setstate(std::ios::failbit);
            stream.write_bits(3_u8, 2);
            CATCH_CHECK_FALSE(stream.finish());
        }

        // A 1-byte header should have been written. Buffer bytes will be dropped.
        CATCH_CHECK(output_stream.str().size() == 1_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(0_u8));

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
            CATCH_CHECK(stream.fully_consumed());
        }
    }

    CATCH_SECTION("Verify detection of a reader stream that is initially invalid")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_byte(0xa);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 1-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 2_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        // Close the stream before handing it to BitStreamReader.
        input_stream.setstate(std::ios::failbit);
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header doesn't exist thus should not have been read.
            CATCH_CHECK(stream.header() == 0);

            // No further reads should succeed.
            CATCH_CHECK(stream.read_bits(byte, 1) == 0_u8);
        }
    }

    CATCH_SECTION("Verify detection of a reader stream that becomes invalid")
    {
        {
            fly::BitStreamWriter stream(output_stream);
            stream.write_byte(0xa);
            CATCH_CHECK(stream.finish());
        }

        // A 1-byte header and a 1-byte buffer should have been written.
        CATCH_CHECK(output_stream.str().size() == 2_u64);

        // The header should be the magic value and 0 remainder bits.
        verify_header(0_u8);

        input_stream.str(output_stream.str());
        {
            fly::BitStreamReader stream(input_stream);
            fly::byte_type byte;

            // The 1-byte header should have been read.
            CATCH_CHECK(stream.header() == create_header(0_u8));

            // Close the stream and read some bits. BitStreamReader will try to fill the internal
            // byte buffer, which will fail.
            input_stream.setstate(std::ios::failbit);
            CATCH_CHECK_FALSE(stream.read_byte(byte));
        }
    }
}
