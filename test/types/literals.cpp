#include "fly/types/numeric/literals.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>

//==============================================================================
TEST(LiteralsTest, Signed8BitTest)
{
    {
        auto value = 1_i8;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
    {
        auto value = 0b1_i8;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
    {
        auto value = 0B1_i8;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
    {
        auto value = 01_i8;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
    {
        auto value = 0x1_i8;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
    {
        auto value = 0X1_i8;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
    {
        auto value = 1'1'1_i8;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Signed16BitTest)
{
    {
        auto value = 1_i16;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
    {
        auto value = 0b1_i16;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
    {
        auto value = 0B1_i16;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
    {
        auto value = 01_i16;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
    {
        auto value = 0x1_i16;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
    {
        auto value = 0X1_i16;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
    {
        auto value = 1'1'1_i16;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Signed32BitTest)
{
    {
        auto value = 1_i32;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
    {
        auto value = 0b1_i32;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
    {
        auto value = 0B1_i32;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
    {
        auto value = 01_i32;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
    {
        auto value = 0x1_i32;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
    {
        auto value = 0X1_i32;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
    {
        auto value = 1'1'1_i32;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Signed64BitTest)
{
    {
        auto value = 1_i64;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
    {
        auto value = 0b1_i64;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
    {
        auto value = 0B1_i64;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
    {
        auto value = 01_i64;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
    {
        auto value = 0x1_i64;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
    {
        auto value = 0X1_i64;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
    {
        auto value = 1'1'1_i64;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_signed_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Unsigned8BitTest)
{
    {
        auto value = 1_u8;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
    {
        auto value = 0b1_u8;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
    {
        auto value = 0B1_u8;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
    {
        auto value = 01_u8;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
    {
        auto value = 0x1_u8;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
    {
        auto value = 0X1_u8;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
    {
        auto value = 1'1'1_u8;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Unsigned16BitTest)
{
    {
        auto value = 1_u16;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
    {
        auto value = 0b1_u16;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
    {
        auto value = 0B1_u16;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
    {
        auto value = 01_u16;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
    {
        auto value = 0x1_u16;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
    {
        auto value = 0X1_u16;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
    {
        auto value = 1'1'1_u16;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Unsigned32BitTest)
{
    {
        auto value = 1_u32;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
    {
        auto value = 0b1_u32;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
    {
        auto value = 0B1_u32;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
    {
        auto value = 01_u32;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
    {
        auto value = 0x1_u32;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
    {
        auto value = 0X1_u32;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
    {
        auto value = 1'1'1_u32;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, Unsigned64BitTest)
{
    {
        auto value = 1_u64;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
    {
        auto value = 0b1_u64;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
    {
        auto value = 0B1_u64;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
    {
        auto value = 01_u64;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
    {
        auto value = 0x1_u64;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
    {
        auto value = 0X1_u64;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
    {
        auto value = 1'1'1_u64;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
    }
}

//==============================================================================
TEST(LiteralsTest, SizeTTest)
{
    {
        auto value = 1_zu;
        EXPECT_EQ(value, decltype(value)(1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
    {
        auto value = 0b1_zu;
        EXPECT_EQ(value, decltype(value)(0b1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
    {
        auto value = 0B1_zu;
        EXPECT_EQ(value, decltype(value)(0B1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
    {
        auto value = 01_zu;
        EXPECT_EQ(value, decltype(value)(01));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
    {
        auto value = 0x1_zu;
        EXPECT_EQ(value, decltype(value)(0x1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
    {
        auto value = 0X1_zu;
        EXPECT_EQ(value, decltype(value)(0X1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
    {
        auto value = 1'1'1_zu;
        EXPECT_EQ(value, decltype(value)(1'1'1));
        EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
        EXPECT_TRUE(std::is_integral_v<decltype(value)>);
        EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
    }
}

#if 0

//==============================================================================
TEST(LiteralsTest, CompileFailures)
{
    // Of course, expected compile failures cannot be tested in a unit test
    // binary. So these are here for manual testing.

    // Literal overflow
    (void)0x0000'0000'0000'0080_i8;
    (void)0x0000'0000'0000'0800_i16;
    (void)0x0000'0000'8000'0000_i32;
    (void)0x8000'0000'0000'0000_i64;

    // Bad character
    (void)1.2_u8;
    (void)0x1.2p0_u8;
}

#endif
