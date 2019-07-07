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

    EXPECT_EQ(pre, dec);
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
    const std::string pre = fly::String::GenerateRandomString(100 << 10);
    std::cout << "." << std::endl;
    std::string enc, dec;

    auto start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.EncodeString(pre, enc));
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed1 = end - start;
    std::cout << elapsed1.count() << std::endl;

    start = std::chrono::system_clock::now();
    ASSERT_TRUE(m_coder.DecodeString(enc, dec));
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed2 = end - start;
    std::cout << elapsed2.count() << std::endl;

    EXPECT_GT(pre.size(), enc.size());
    EXPECT_EQ(pre, dec);
}
