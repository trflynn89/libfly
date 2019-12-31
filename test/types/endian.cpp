#include "fly/types/numeric/endian.h"

#include "fly/traits/traits.h"
#include "fly/types/numeric/literals.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

namespace {

template <
    typename T,
    fly::enable_if_any<
        std::is_same<std::int8_t, T>,
        std::is_same<std::uint8_t, T>> = 0>
T swap(T x)
{
    return x;
}

template <
    typename T,
    fly::enable_if_any<
        std::is_same<std::int16_t, T>,
        std::is_same<std::uint16_t, T>> = 0>
T swap(T x)
{
    T result = 0;

    result |= (x & 0xff00_u16) >> 8;
    result |= (x & 0x00ff_u16) << 8;

    return result;
}

template <
    typename T,
    fly::enable_if_any<
        std::is_same<std::int32_t, T>,
        std::is_same<std::uint32_t, T>> = 0>
T swap(T x)
{
    T result = 0;

    result |= (x & 0xff000000_u32) >> 24;
    result |= (x & 0x00ff0000_u32) >> 8;
    result |= (x & 0x0000ff00_u32) << 8;
    result |= (x & 0x000000ff_u32) << 24;

    return result;
}

template <
    typename T,
    fly::enable_if_any<
        std::is_same<std::int64_t, T>,
        std::is_same<std::uint64_t, T>> = 0>
T swap(T x)
{
    T result = 0;

    result |= (x & 0xff00000000000000_u64) >> 56;
    result |= (x & 0x00ff000000000000_u64) >> 40;
    result |= (x & 0x0000ff0000000000_u64) >> 24;
    result |= (x & 0x000000ff00000000_u64) >> 8;
    result |= (x & 0x00000000ff000000_u64) << 8;
    result |= (x & 0x0000000000ff0000_u64) << 24;
    result |= (x & 0x000000000000ff00_u64) << 40;
    result |= (x & 0x00000000000000ff_u64) << 56;

    return result;
}

} // namespace

//==============================================================================
template <typename DataType>
struct EndianTest : public ::testing::Test
{
protected:
    template <fly::Endian desired>
    void RunTest()
    {
        const DataType iterations = std::numeric_limits<std::uint8_t>::max();
        const DataType step = std::numeric_limits<DataType>::max() / iterations;

        for (DataType data = 0, i = 0; i++ < iterations; data += step)
        {
            DataType expected = data;
            DataType actual = fly::endian_swap<desired>(data);

            if constexpr (desired != fly::Endian::Native)
            {
                expected = swap(expected);
            }

            EXPECT_EQ(expected, actual);
        }
    }
};

using DataTypes = ::testing::Types<
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t,
    std::uint8_t,
    std::uint16_t,
    std::uint32_t,
    std::uint64_t>;

TYPED_TEST_SUITE(EndianTest, DataTypes);

//==============================================================================
TYPED_TEST(EndianTest, BigEndianTest)
{
    TestFixture::template RunTest<fly::Endian::Big>();
}

//==============================================================================
TYPED_TEST(EndianTest, LittleEndianTest)
{
    TestFixture::template RunTest<fly::Endian::Little>();
}

//==============================================================================
TYPED_TEST(EndianTest, NativeEndianTest)
{
    TestFixture::template RunTest<fly::Endian::Native>();
}
