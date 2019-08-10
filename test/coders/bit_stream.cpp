#include "fly/coders/bit_stream.h"

#include <gtest/gtest.h>

#include <sstream>

//==============================================================================
TEST(BitStreamTest, StreamTest)
{
    std::stringstream os(std::ios::in | std::ios::out | std::ios::binary);
    {
        fly::BitStreamWriter bs(os);

        bs.WriteByte(0x48);
        bs.WriteByte(0x65);
        bs.WriteByte(0x6c);
        bs.WriteByte(0x6c);
        bs.WriteByte(0x6f);

        bs.WriteBits(0, 1);
        bs.WriteBits(0, 1);
        bs.WriteBits(1, 1);
        bs.WriteBits(0, 1);
        bs.WriteBits(0, 1);
        bs.WriteBits(0, 1);
        bs.WriteBits(1, 1);
    }

    std::stringstream is(os.str(), std::ios::in | std::ios::binary);
    {
        fly::BitStreamReader bs(is);
        fly::byte_type byte;

        EXPECT_TRUE(bs.ReadByte(byte));
        EXPECT_EQ(byte, 0x48);
        EXPECT_TRUE(bs.ReadByte(byte));
        EXPECT_EQ(byte, 0x65);
        EXPECT_TRUE(bs.ReadByte(byte));
        EXPECT_EQ(byte, 0x6c);
        EXPECT_TRUE(bs.ReadByte(byte));
        EXPECT_EQ(byte, 0x6c);
        EXPECT_TRUE(bs.ReadByte(byte));
        EXPECT_EQ(byte, 0x6f);

        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 0);
        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 0);
        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 1);
        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 0);
        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 0);
        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 0);
        EXPECT_TRUE(bs.ReadBits(1, byte));
        EXPECT_EQ(byte, 1);

        EXPECT_TRUE(bs.FullyConsumed());
        EXPECT_FALSE(bs.ReadBits(1, byte));
        EXPECT_FALSE(bs.ReadByte(byte));
    }
}
