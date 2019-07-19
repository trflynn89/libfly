#include "fly/coders/huffman_coder.h"

#include "fly/types/string.h"

#include <gtest/gtest.h>

#include <chrono>
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
    const std::string pre;
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneSymbolTest)
{
    const std::string pre = "a";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(pre, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, OneUniqueSymbolTest)
{
    const std::string pre = "aaaaaaaaaa";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(pre, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, MirrorTest)
{
    const std::string pre = "abcdefabcbbb";
    std::string enc, dec;

    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_EQ(pre, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, LargeMirrorTest)
{
    const std::string pre = fly::String::GenerateRandomString(100 << 20);
    std::cout << "." << std::endl;
    std::string enc, dec;

    auto start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    auto end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count() << std::endl;

    start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));
    end = std::chrono::system_clock::now();
    std::cout << std::chrono::duration<double>(end - start).count() << std::endl;

    EXPECT_GT(pre.size(), enc.size());
    EXPECT_EQ(pre, dec);
}

//==============================================================================
TEST_F(HuffmanCoderTest, UnicodeTest)
{
    std::string pre = "🍕😅😅🍕❤️🍕";
    std::string enc, dec;

    for (int i = 0; i < 10; ++i)
    {
        pre += pre;
    }

    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));

    EXPECT_GT(pre.size(), enc.size());
    EXPECT_EQ(pre, dec);
}
