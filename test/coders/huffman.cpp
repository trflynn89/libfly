#include "fly/coders/huffman/huffman_config.h"
#include "fly/coders/huffman/huffman_decoder.h"
#include "fly/coders/huffman/huffman_encoder.h"
#include "fly/types/string.h"
#include "test/util/path_util.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <string>

//==============================================================================
class HuffmanCoderTest : public ::testing::Test
{
public:
    HuffmanCoderTest() :
        m_spConfig(std::make_shared<fly::HuffmanConfig>()),
        m_encoder(m_spConfig)
    {
    }

protected:
    std::shared_ptr<fly::HuffmanConfig> m_spConfig;
    fly::HuffmanEncoder m_encoder;
    fly::HuffmanDecoder m_decoder;
};

//==============================================================================
TEST_F(HuffmanCoderTest, EmptyTest)
{
    const std::string raw;
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.EncodeString(raw, enc));
    ASSERT_TRUE(m_decoder.DecodeString(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneSymbolTest)
{
    const std::string raw = "a";
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.EncodeString(raw, enc));
    ASSERT_TRUE(m_decoder.DecodeString(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneUniqueSymbolTest)
{
    const std::string raw = "aaaaaaaaaa";
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.EncodeString(raw, enc));
    ASSERT_TRUE(m_decoder.DecodeString(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, MirrorTest)
{
    const std::string raw = "abcdefabcbbb";
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.EncodeString(raw, enc));
    ASSERT_TRUE(m_decoder.DecodeString(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, LargeMirrorTest)
{
    const std::string raw = fly::String::GenerateRandomString(100 << 10);
    std::string enc, dec;

    ASSERT_TRUE(m_encoder.EncodeString(raw, enc));
    ASSERT_TRUE(m_decoder.DecodeString(enc, dec));

    EXPECT_GT(raw.size(), enc.size());
    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, UnicodeTest)
{
    std::string raw = "🍕א😅😅🍕❤️א🍕";
    std::string enc, dec;

    for (int i = 0; i < 10; ++i)
    {
        raw += raw;
    }

    ASSERT_TRUE(m_encoder.EncodeString(raw, enc));
    ASSERT_TRUE(m_decoder.DecodeString(enc, dec));

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
        m_encodedFile(m_path / "encoded.txt"),
        m_decodedFile(m_path / "decoded.txt")
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
    std::filesystem::path m_encodedFile;
    std::filesystem::path m_decodedFile;
};

//==============================================================================
TEST_F(HuffmanCoderFileTest, AsciiFileTest)
{
    // Generated with:
    // tr -dc '[:graph:]' </dev/urandom | head -c 4194304 > test.txt
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.txt";

    ASSERT_TRUE(m_encoder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_decoder.DecodeFile(m_encodedFile, m_decodedFile));

    EXPECT_GT(
        std::filesystem::file_size(raw),
        std::filesystem::file_size(m_encodedFile));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}

//==============================================================================
TEST_F(HuffmanCoderFileTest, BinaryFileTest)
{
    // Generated with:
    // dd if=/dev/urandom of=test.bin count=1 bs=4194304
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.bin";

    ASSERT_TRUE(m_encoder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_decoder.DecodeFile(m_encodedFile, m_decodedFile));

    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}

//==============================================================================
TEST_F(HuffmanCoderFileTest, Enwik8FileTest)
{
    // Downloaded from: http://mattmahoney.net/dc/enwik8.zip
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "enwik8";

    if (!std::filesystem::exists(raw))
    {
        // It's a 100MB file...not checking it into git for now.
        return;
    }

    ASSERT_TRUE(m_encoder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_decoder.DecodeFile(m_encodedFile, m_decodedFile));

    EXPECT_GT(
        std::filesystem::file_size(raw),
        std::filesystem::file_size(m_encodedFile));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}
