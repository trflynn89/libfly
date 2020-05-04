#include "fly/coders/base64/base64_coder.hpp"

#include "test/util/path_util.hpp"

#include <gtest/gtest.h>

#include <filesystem>

//==============================================================================
class Base64CoderTest : public ::testing::Test
{
protected:
    fly::Base64Coder m_coder;
};

//==============================================================================
TEST_F(Base64CoderTest, EmptyTest)
{
    const std::string raw;
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_TRUE(enc.empty());
    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(Base64CoderTest, ZeroPaddingTest)
{
    const std::string raw = "Man";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(enc, "TWFu");
    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(Base64CoderTest, SinglePaddingTest)
{
    const std::string raw = "Ma";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(enc, "TWE=");
    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(Base64CoderTest, DoublePaddingTest)
{
    const std::string raw = "M";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(enc, "TQ==");
    EXPECT_EQ(raw, dec);
}

//==============================================================================
TEST_F(Base64CoderTest, InvalidSymbolTest)
{
    std::string dec;

    ASSERT_FALSE(m_coder.DecodeString("^", dec));
    ASSERT_FALSE(m_coder.DecodeString("ab^", dec));
    ASSERT_FALSE(m_coder.DecodeString("^ab", dec));
    ASSERT_FALSE(m_coder.DecodeString("ab^ab", dec));
}

//==============================================================================
TEST_F(Base64CoderTest, InvalidChunkSizeTest)
{
    std::string dec;

    ASSERT_FALSE(m_coder.DecodeString("a", dec));
    ASSERT_FALSE(m_coder.DecodeString("ab", dec));
    ASSERT_FALSE(m_coder.DecodeString("abc", dec));
    ASSERT_FALSE(m_coder.DecodeString("abcde", dec));
    ASSERT_FALSE(m_coder.DecodeString("abcdef", dec));
    ASSERT_FALSE(m_coder.DecodeString("abcdefg", dec));
}

//==============================================================================
TEST_F(Base64CoderTest, WikiExampleTest)
{
    // Example from: https://en.wikipedia.org/wiki/Base64#Examples
    const std::string raw =
        "Man is distinguished, not only by his reason, but by this singular "
        "passion from other animals, which is a lust of the mind, that by a "
        "perseverance of delight in the continued and indefatigable generation "
        "of knowledge, exceeds the short vehemence of any carnal pleasure.";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(raw, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    const std::string expected =
        "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieS"
        "B0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBh"
        "IGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodC"
        "BpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25v"
        "d2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbG"
        "Vhc3VyZS4=";

    EXPECT_EQ(enc, expected);
    EXPECT_EQ(raw, dec);
}

//==============================================================================
class Base64CoderFileTest : public Base64CoderTest
{
public:
    Base64CoderFileTest() noexcept :
        Base64CoderTest(),
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
TEST_F(Base64CoderFileTest, AsciiFileTest)
{
    // Generated with:
    // tr -dc '[:graph:]' </dev/urandom | head -c 4194304 > test.txt
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.txt";

    ASSERT_TRUE(m_coder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_coder.DecodeFile(m_encodedFile, m_decodedFile));

    // Generated with:
    // base64 -w0 test.txt > test.txt.base64
    const auto expected = here / "data" / "test.txt.base64";

    EXPECT_TRUE(fly::PathUtil::CompareFiles(m_encodedFile, expected));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}

//==============================================================================
TEST_F(Base64CoderFileTest, PngFileTest)
{
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.png";

    ASSERT_TRUE(m_coder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_coder.DecodeFile(m_encodedFile, m_decodedFile));

    // Generated with:
    // base64 -w0 test.png > test.png.base64
    const auto expected = here / "data" / "test.png.base64";

    EXPECT_TRUE(fly::PathUtil::CompareFiles(m_encodedFile, expected));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}

//==============================================================================
TEST_F(Base64CoderFileTest, GifFileTest)
{
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.gif";

    ASSERT_TRUE(m_coder.EncodeFile(raw, m_encodedFile));
    ASSERT_TRUE(m_coder.DecodeFile(m_encodedFile, m_decodedFile));

    // Generated with:
    // base64 -w0 test.gif > test.gif.base64
    const auto expected = here / "data" / "test.gif.base64";

    EXPECT_TRUE(fly::PathUtil::CompareFiles(m_encodedFile, expected));
    EXPECT_TRUE(fly::PathUtil::CompareFiles(raw, m_decodedFile));
}
