#include "fly/coders/huffman/huffman_config.hpp"
#include "fly/coders/huffman/huffman_decoder.hpp"
#include "fly/coders/huffman/huffman_encoder.hpp"
#include "fly/types/bit_stream/bit_stream_writer.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "test/util/path_util.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

namespace {

/**
 * Subclass of the Huffman coder config to contain invalid values.
 */
class BadHuffmanConfig : public fly::HuffmanConfig
{
public:
    BadHuffmanConfig() noexcept : fly::HuffmanConfig()
    {
        m_default_encoder_max_code_length = std::numeric_limits<decltype(
            m_default_encoder_max_code_length)>::max();
    }
};

/**
 * Subclass of the Huffman coder config to reduce Huffman code lengths.
 */
class SmallCodeLengthConfig : public fly::HuffmanConfig
{
public:
    SmallCodeLengthConfig() noexcept : fly::HuffmanConfig()
    {
        m_default_encoder_max_code_length = 3;
    }
};

} // namespace

namespace fly {

//==============================================================================
class HuffmanCoderTest : public ::testing::Test
{
public:
    HuffmanCoderTest() :
        m_config(std::make_shared<fly::HuffmanConfig>()),
        m_encoder(m_config)
    {
    }

protected:
    std::string create_stream_with_remainder(
        std::vector<fly::byte_type> bytes,
        fly::byte_type remainder)
    {
        std::stringstream stream(
            std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

        fly::BitStreamWriter output(stream);

        for (const fly::byte_type &byte : bytes)
        {
            output.write_byte(byte);
        }
        if (remainder > 0_u8)
        {
            output.write_bits(0_u8, remainder);
        }

        if (output.finish())
        {
            return stream.str();
        }

        return std::string();
    }

    std::string create_stream(std::vector<fly::byte_type> bytes)
    {
        return create_stream_with_remainder(std::move(bytes), 0_u8);
    }

    std::vector<HuffmanCode> get_decoded_huffman_codes()
    {
        std::vector<HuffmanCode> codes;

        for (std::uint16_t i = 0; i < m_decoder.m_huffman_codes_size; ++i)
        {
            const HuffmanCode &code = m_decoder.m_huffman_codes[i];
            codes.emplace_back(code.m_symbol, code.m_code, code.m_length);
        }

        return codes;
    }

    std::shared_ptr<fly::HuffmanConfig> m_config;
    fly::HuffmanEncoder m_encoder;
    fly::HuffmanDecoder m_decoder;
};

//==============================================================================
TEST_F(HuffmanCoderTest, InvalidConfig)
{
    const std::string raw;
    std::string enc;

    auto spConfig = std::make_shared<BadHuffmanConfig>();
    fly::HuffmanEncoder encoder(spConfig);

    EXPECT_FALSE(encoder.encode_string(raw, enc));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderMissingVersion)
{
    const std::string enc;
    std::string dec;

    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderInvalidVersion)
{
    std::vector<fly::byte_type> bytes = {
        0_u8, // Version
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderMissingChunkSize)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderZeroChunkSize)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        0_u8, // Chunk size KB (low)
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderMissingMaxCodeLength)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderZeroMaxCodeLength)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        0_u8, // Maximum Huffman code length
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, HeaderInvalidMaxCodeLength)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        255_u8, // Maximum Huffman code length
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, IncompleteCodeLengthCounts)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
    };

    const std::string enc =
        create_stream_with_remainder(std::move(bytes), 1_u8);
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, ZeroCodeLengthCounts)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
        0_u8, // Number of code length counts
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, InvalidCodeLengthCounts)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
        8_u8, // Number of code length counts
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, MissingCodeLengthCount)
{
    fly::byte_type numberOfCodeLengthCounts = 5_u8;

    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
        numberOfCodeLengthCounts, // Number of code length counts
    };

    for (fly::byte_type i = 0; i < numberOfCodeLengthCounts; ++i)
    {
        const std::string enc = create_stream(bytes);
        std::string dec;

        EXPECT_FALSE(enc.empty());
        EXPECT_FALSE(m_decoder.decode_string(enc, dec));

        bytes.push_back(0_u8);
        bytes.push_back(1_u8);
    }
}

//==============================================================================
TEST_F(HuffmanCoderTest, MissingSymbol)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
        2_u8, // Number of code length counts
        0_u8, // Code length count 1 (high)
        0_u8, // Code length count 1 (low)
        0_u8, // Code length count 2 (high)
        1_u8, // Code length count 2 (low)
    };

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, TooManyCodes)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
        2_u8, // Number of code length counts
        0_u8, // Code length count 1 (high)
        0_u8, // Code length count 1 (low)
        std::numeric_limits<std::uint8_t>::max(), // Code length count 2 (high)
        std::numeric_limits<std::uint8_t>::max(), // Code length count 2 (low)
    };

    for (auto i = 0; i < std::numeric_limits<std::uint16_t>::max(); ++i)
    {
        bytes.push_back(1_u8);
    }

    const std::string enc = create_stream(std::move(bytes));
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, MissingSymbols)
{
    std::vector<fly::byte_type> bytes = {
        1_u8, // Version
        0_u8, // Chunk size KB (high)
        1_u8, // Chunk size KB (low)
        4_u8, // Maximum Huffman code length
        1_u8, // Number of code length counts
        0_u8, // Code length count 1 (high)
        1_u8, // Code length count 1 (low),
        0x41, // Single symbol (A)
    };

    const std::string enc = create_stream_with_remainder(std::move(bytes), 1);
    std::string dec;

    EXPECT_FALSE(enc.empty());
    EXPECT_FALSE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, Empty)
{
    const std::string raw;
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneSymbol)
{
    const std::string raw = "a";
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneUniqueSymbol)
{
    const std::string raw = "aaaaaaaaaa";
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, Mirror)
{
    const std::string raw = "abcdefabcbbb";
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, LengthLimited)
{
    const std::string raw = "abcdefabcbbb";
    std::string enc, dec;

    auto config = std::make_shared<SmallCodeLengthConfig>();
    fly::HuffmanEncoder encoder(config);

    ASSERT_TRUE(encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));

    EXPECT_EQ(raw, dec);

    // Validate the Kraft–McMillan inequality.
    const std::uint16_t maxAllowedKraft =
        (1_u16 << config->encoder_max_code_length()) - 1;
    std::uint16_t kraft = 0_u16;

    for (const HuffmanCode &code : get_decoded_huffman_codes())
    {
        kraft += 1_u16 << (config->encoder_max_code_length() - code.m_length);
    }

    EXPECT_LE(kraft, maxAllowedKraft);
}

//==============================================================================
TEST_F(HuffmanCoderTest, LargeMirror)
{
    const std::string raw = fly::String::generate_random_string(100 << 10);
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));

    EXPECT_GT(raw.size(), enc.size());
    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, Unicode)
{
    std::string raw = "🍕א😅😅🍕❤️א🍕";
    std::string enc, dec;

    for (int i = 0; i < 10; ++i)
    {
        raw += raw;
    }

    ASSERT_TRUE(m_encoder.encode_string(raw, enc));
    ASSERT_TRUE(m_decoder.decode_string(enc, dec));

    EXPECT_GT(raw.size(), enc.size());
    EXPECT_EQ(raw, dec);
}

//==============================================================================
class HuffmanCoderFileTest : public HuffmanCoderTest
{
public:
    HuffmanCoderFileTest() noexcept :
        HuffmanCoderTest(),
        m_path(fly::PathUtil::GenerateTempDirectory()),
        m_encoded_file(m_path / "encoded.txt"),
        m_decoded_file(m_path / "decoded.txt")
    {
    }

    /**
     * Create the directory to contain test output files.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path));
    }

    /**
     * Delete the created directory.
     */
    void TearDown() noexcept override
    {
        std::filesystem::remove_all(m_path);
    }

protected:
    std::filesystem::path m_path;
    std::filesystem::path m_encoded_file;
    std::filesystem::path m_decoded_file;
};

//==============================================================================
TEST_F(HuffmanCoderFileTest, AsciiFile)
{
    // Generated with:
    // tr -dc '[:graph:]' </dev/urandom | head -c 4194304 > test.txt
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.txt";

    ASSERT_TRUE(m_encoder.encode_file(raw, m_encoded_file));
    ASSERT_TRUE(m_decoder.decode_file(m_encoded_file, m_decoded_file));

    EXPECT_GT(
        std::filesystem::file_size(raw),
        std::filesystem::file_size(m_encoded_file));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decoded_file));
}

//==============================================================================
TEST_F(HuffmanCoderFileTest, BinaryFile)
{
    // Generated with:
    // dd if=/dev/urandom of=test.bin count=1 bs=4194304
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.bin";

    ASSERT_TRUE(m_encoder.encode_file(raw, m_encoded_file));
    ASSERT_TRUE(m_decoder.decode_file(m_encoded_file, m_decoded_file));

    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decoded_file));
}

//==============================================================================
TEST_F(HuffmanCoderFileTest, Enwik8File)
{
    // Downloaded from: http://mattmahoney.net/dc/enwik8.zip
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "enwik8";

    if (!std::filesystem::exists(raw))
    {
        // TODO: The enwik8 file is 100MB. Instead of checking it into git and
        // encoding/decoding it with debug mode unit tests, a performance test
        // should be created that downloads the file and runs in release mode.
        return;
    }

    ASSERT_TRUE(m_encoder.encode_file(raw, m_encoded_file));
    ASSERT_TRUE(m_decoder.decode_file(m_encoded_file, m_decoded_file));

    EXPECT_GT(
        std::filesystem::file_size(raw),
        std::filesystem::file_size(m_encoded_file));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decoded_file));
}

} // namespace fly
