#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_test_macros.hpp"

#include <cstdint>
#include <type_traits>

using namespace fly::literals::numeric_literals;

CATCH_TEST_CASE("Literals", "[numeric]")
{
    CATCH_SECTION("Signed 8-bit integer literals")
    {
        {
            auto value = 1_i8;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
        {
            auto value = 0b1_i8;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
        {
            auto value = 0B1_i8;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
        {
            auto value = 01_i8;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
        {
            auto value = 0x1_i8;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
        {
            auto value = 0X1_i8;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
        {
            auto value = 1'1'1_i8;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int8_t>));
        }
    }

    CATCH_SECTION("Signed 16-bit integer literals")
    {
        {
            auto value = 1_i16;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
        {
            auto value = 0b1_i16;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
        {
            auto value = 0B1_i16;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
        {
            auto value = 01_i16;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
        {
            auto value = 0x1_i16;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
        {
            auto value = 0X1_i16;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
        {
            auto value = 1'1'1_i16;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int16_t>));
        }
    }

    CATCH_SECTION("Signed 32-bit integer literals")
    {
        {
            auto value = 1_i32;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
        {
            auto value = 0b1_i32;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
        {
            auto value = 0B1_i32;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
        {
            auto value = 01_i32;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
        {
            auto value = 0x1_i32;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
        {
            auto value = 0X1_i32;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
        {
            auto value = 1'1'1_i32;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int32_t>));
        }
    }

    CATCH_SECTION("Signed 64-bit integer literals")
    {
        {
            auto value = 1_i64;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
        {
            auto value = 0b1_i64;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
        {
            auto value = 0B1_i64;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
        {
            auto value = 01_i64;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
        {
            auto value = 0x1_i64;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
        {
            auto value = 0X1_i64;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
        {
            auto value = 1'1'1_i64;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_signed_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::int64_t>));
        }
    }

    CATCH_SECTION("Unsigned 8B-bit integer literals")
    {
        {
            auto value = 1_u8;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
        {
            auto value = 0b1_u8;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
        {
            auto value = 0B1_u8;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
        {
            auto value = 01_u8;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
        {
            auto value = 0x1_u8;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
        {
            auto value = 0X1_u8;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
        {
            auto value = 1'1'1_u8;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint8_t>));
        }
    }

    CATCH_SECTION("Unsigned 16-bit integer literals")
    {
        {
            auto value = 1_u16;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
        {
            auto value = 0b1_u16;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
        {
            auto value = 0B1_u16;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
        {
            auto value = 01_u16;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
        {
            auto value = 0x1_u16;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
        {
            auto value = 0X1_u16;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
        {
            auto value = 1'1'1_u16;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint16_t>));
        }
    }

    CATCH_SECTION("Unsigned 32-bit integer literals")
    {
        {
            auto value = 1_u32;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
        {
            auto value = 0b1_u32;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
        {
            auto value = 0B1_u32;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
        {
            auto value = 01_u32;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
        {
            auto value = 0x1_u32;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
        {
            auto value = 0X1_u32;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
        {
            auto value = 1'1'1_u32;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint32_t>));
        }
    }

    CATCH_SECTION("Unsigned 64-bit integer literals")
    {
        {
            auto value = 1_u64;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
        {
            auto value = 0b1_u64;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
        {
            auto value = 0B1_u64;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
        {
            auto value = 01_u64;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
        {
            auto value = 0x1_u64;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
        {
            auto value = 0X1_u64;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
        {
            auto value = 1'1'1_u64;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::uint64_t>));
        }
    }

    CATCH_SECTION("Unsigned maximum-size integer literal")
    {
        {
            auto value = 1_zu;
            CATCH_CHECK(value == decltype(value)(1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
        {
            auto value = 0b1_zu;
            CATCH_CHECK(value == decltype(value)(0b1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
        {
            auto value = 0B1_zu;
            CATCH_CHECK(value == decltype(value)(0B1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
        {
            auto value = 01_zu;
            CATCH_CHECK(value == decltype(value)(01));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
        {
            auto value = 0x1_zu;
            CATCH_CHECK(value == decltype(value)(0x1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
        {
            auto value = 0X1_zu;
            CATCH_CHECK(value == decltype(value)(0X1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
        {
            auto value = 1'1'1_zu;
            CATCH_CHECK(value == decltype(value)(1'1'1));
            CATCH_CHECK(std::is_unsigned_v<decltype(value)>);
            CATCH_CHECK(std::is_integral_v<decltype(value)>);
            CATCH_CHECK((std::is_same_v<decltype(value), std::size_t>));
        }
    }

#if 0

    CATCH_SECTION("CompileFailures")
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
}
