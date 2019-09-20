#include "fly/types/literals.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>

//==============================================================================
TEST(LiteralsTest, Signed8BitTest)
{
    auto value = 1_i8;
    EXPECT_TRUE(std::is_signed_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::int8_t>));
}

//==============================================================================
TEST(LiteralsTest, Signed16BitTest)
{
    auto value = 1_i16;
    EXPECT_TRUE(std::is_signed_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::int16_t>));
}

//==============================================================================
TEST(LiteralsTest, Signed32BitTest)
{
    auto value = 1_i32;
    EXPECT_TRUE(std::is_signed_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::int32_t>));
}

//==============================================================================
TEST(LiteralsTest, Signed64BitTest)
{
    auto value = 1_i64;
    EXPECT_TRUE(std::is_signed_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::int64_t>));
}

//==============================================================================
TEST(LiteralsTest, Unsigned8BitTest)
{
    auto value = 1_u8;
    EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::uint8_t>));
}

//==============================================================================
TEST(LiteralsTest, Unsigned16BitTest)
{
    auto value = 1_u16;
    EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::uint16_t>));
}

//==============================================================================
TEST(LiteralsTest, Unsigned32BitTest)
{
    auto value = 1_u32;
    EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::uint32_t>));
}

//==============================================================================
TEST(LiteralsTest, Unsigned64BitTest)
{
    auto value = 1_u64;
    EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::uint64_t>));
}

//==============================================================================
TEST(LiteralsTest, SizeTTest)
{
    auto value = 1_zu;
    EXPECT_TRUE(std::is_unsigned_v<decltype(value)>);
    EXPECT_TRUE(std::is_integral_v<decltype(value)>);
    EXPECT_TRUE((std::is_same_v<decltype(value), std::size_t>));
}
