#include "fly/coders/huffman_coder.h"

#include "fly/types/string.h"
#include "test/util/path_util.h"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>

//==============================================================================
class HuffmanCoderTest : public ::testing::Test
{
protected:
    fly::HuffmanCoder m_coder;
};

//==============================================================================
TEST_F(HuffmanCoderTest, EmptyTest)
{
    const std::string raw;
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneSymbolTest)
{
    const std::string raw = "a";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneUniqueSymbolTest)
{
    const std::string raw = "aaaaaaaaaa";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, MirrorTest)
{
    const std::string raw = "abcdefabcbbb";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, LargeMirrorTest)
{
    const std::string raw = fly::String::GenerateRandomString(100 << 10);
    std::cout << "." << std::endl;
    std::string enc, dec;

    auto start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count()
              << std::endl;

    start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));
    end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count()
              << std::endl;

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

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

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

    ASSERT_TRUE(m_coder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_coder.DecodeFile(m_encodedFile, m_decodedFile));

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

    ASSERT_TRUE(m_coder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_coder.DecodeFile(m_encodedFile, m_decodedFile));

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

    auto start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.EncodeFile(raw, m_encodedFile));
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count()
              << std::endl;

    std::cout << std::filesystem::file_size(raw) << std::endl;
    std::cout << std::filesystem::file_size(m_encodedFile) << std::endl;

    start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.DecodeFile(m_encodedFile, m_decodedFile));
    end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count()
              << std::endl;

    EXPECT_GT(
        std::filesystem::file_size(raw),
        std::filesystem::file_size(m_encodedFile));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}
