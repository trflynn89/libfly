#include "fly/coders/base64/base64_coder.hpp"

#include "test/util/path_util.hpp"

#include <gtest/gtest.h>

#include <filesystem>

//==================================================================================================
class Base64CoderTest : public ::testing::Test
{
protected:
    fly::Base64Coder m_coder;
};

//==================================================================================================
TEST_F(Base64CoderTest, Empty)
{
    const std::string raw;
    std::string enc, dec;

    ASSERT_TRUE(m_coder.encode_string(raw, enc));
    ASSERT_TRUE(m_coder.decode_string(enc, dec));

    EXPECT_TRUE(enc.empty());
    EXPECT_EQ(raw, dec);
}

//==================================================================================================
TEST_F(Base64CoderTest, ZeroPadding)
{
    const std::string raw = "Man";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.encode_string(raw, enc));
    ASSERT_TRUE(m_coder.decode_string(enc, dec));

    EXPECT_EQ(enc, "TWFu");
    EXPECT_EQ(raw, dec);
}

//==================================================================================================
TEST_F(Base64CoderTest, SinglePadding)
{
    const std::string raw = "Ma";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.encode_string(raw, enc));
    ASSERT_TRUE(m_coder.decode_string(enc, dec));

    EXPECT_EQ(enc, "TWE=");
    EXPECT_EQ(raw, dec);
}

//==================================================================================================
TEST_F(Base64CoderTest, DoublePadding)
{
    const std::string raw = "M";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.encode_string(raw, enc));
    ASSERT_TRUE(m_coder.decode_string(enc, dec));

    EXPECT_EQ(enc, "TQ==");
    EXPECT_EQ(raw, dec);
}

//==================================================================================================
TEST_F(Base64CoderTest, InvalidSymbol)
{
    std::string dec;

    ASSERT_FALSE(m_coder.decode_string("^", dec));
    ASSERT_FALSE(m_coder.decode_string("ab^", dec));
    ASSERT_FALSE(m_coder.decode_string("^ab", dec));
    ASSERT_FALSE(m_coder.decode_string("ab^ab", dec));
}

//==================================================================================================
TEST_F(Base64CoderTest, InvalidChunkSize)
{
    std::string dec;

    ASSERT_FALSE(m_coder.decode_string("a", dec));
    ASSERT_FALSE(m_coder.decode_string("ab", dec));
    ASSERT_FALSE(m_coder.decode_string("abc", dec));
    ASSERT_FALSE(m_coder.decode_string("abcde", dec));
    ASSERT_FALSE(m_coder.decode_string("abcdef", dec));
    ASSERT_FALSE(m_coder.decode_string("abcdefg", dec));
}

//==================================================================================================
TEST_F(Base64CoderTest, WikiExample)
{
    // Example from: https://en.wikipedia.org/wiki/Base64#Examples
    const std::string raw =
        "Man is distinguished, not only by his reason, but by this singular passion from other "
        "animals, which is a lust of the mind, that by a perseverance of delight in the continued "
        "and indefatigable generation of knowledge, exceeds the short vehemence of any carnal "
        "pleasure.";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.encode_string(raw, enc));
    ASSERT_TRUE(m_coder.decode_string(enc, dec));

    const std::string expected =
        "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIH"
        "Bhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBw"
        "ZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb2"
        "4gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4"
        "=";

    EXPECT_EQ(enc, expected);
    EXPECT_EQ(raw, dec);
}

//==================================================================================================
class Base64CoderFileTest : public Base64CoderTest
{
public:
    Base64CoderFileTest() noexcept :
        Base64CoderTest(),
        m_path(fly::PathUtil::generate_temp_directory()),
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

//==================================================================================================
TEST_F(Base64CoderFileTest, AsciiFile)
{
    // Generated with:
    // tr -dc '[:graph:]' </dev/urandom | head -c 4194304 > test.txt
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.txt";

    ASSERT_TRUE(m_coder.encode_file(raw, m_encoded_file));
    ASSERT_TRUE(m_coder.decode_file(m_encoded_file, m_decoded_file));

    // Generated with:
    // base64 -w0 test.txt > test.txt.base64
    const auto expected = here / "data" / "test.txt.base64";

    EXPECT_TRUE(fly::PathUtil::compare_files(m_encoded_file, expected));
    EXPECT_TRUE(fly::PathUtil::compare_files(raw, m_decoded_file));
}

//==================================================================================================
TEST_F(Base64CoderFileTest, PngFile)
{
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.png";

    ASSERT_TRUE(m_coder.encode_file(raw, m_encoded_file));
    ASSERT_TRUE(m_coder.decode_file(m_encoded_file, m_decoded_file));

    // Generated with:
    // base64 -w0 test.png > test.png.base64
    const auto expected = here / "data" / "test.png.base64";

    EXPECT_TRUE(fly::PathUtil::compare_files(m_encoded_file, expected));
    EXPECT_TRUE(fly::PathUtil::compare_files(raw, m_decoded_file));
}

//==================================================================================================
TEST_F(Base64CoderFileTest, GifFile)
{
    const auto here = std::filesystem::path(__FILE__).parent_path();
    const auto raw = here / "data" / "test.gif";

    ASSERT_TRUE(m_coder.encode_file(raw, m_encoded_file));
    ASSERT_TRUE(m_coder.decode_file(m_encoded_file, m_decoded_file));

    // Generated with:
    // base64 -w0 test.gif > test.gif.base64
    const auto expected = here / "data" / "test.gif.base64";

    EXPECT_TRUE(fly::PathUtil::compare_files(m_encoded_file, expected));
    EXPECT_TRUE(fly::PathUtil::compare_files(raw, m_decoded_file));
}
