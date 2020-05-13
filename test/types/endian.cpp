#include "fly/types/numeric/endian.hpp"

#include "fly/traits/traits.hpp"
#include "fly/types/numeric/literals.hpp"

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

    result |= (x & static_cast<T>(0xff00)) >> 8;
    result |= (x & static_cast<T>(0x00ff)) << 8;

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

    result |= (x & static_cast<T>(0xff000000)) >> 24;
    result |= (x & static_cast<T>(0x00ff0000)) >> 8;
    result |= (x & static_cast<T>(0x0000ff00)) << 8;
    result |= (x & static_cast<T>(0x000000ff)) << 24;

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

    result |= (x & static_cast<T>(0xff00000000000000)) >> 56;
    result |= (x & static_cast<T>(0x00ff000000000000)) >> 40;
    result |= (x & static_cast<T>(0x0000ff0000000000)) >> 24;
    result |= (x & static_cast<T>(0x000000ff00000000)) >> 8;
    result |= (x & static_cast<T>(0x00000000ff000000)) << 8;
    result |= (x & static_cast<T>(0x0000000000ff0000)) << 24;
    result |= (x & static_cast<T>(0x000000000000ff00)) << 40;
    result |= (x & static_cast<T>(0x00000000000000ff)) << 56;

    return result;
}

} // namespace

//==============================================================================
template <typename DataType>
struct EndianTest : public ::testing::Test
{
protected:
    template <fly::Endian Desired>
    void run_test()
    {
        constexpr DataType iterations = 100;
        const DataType step = std::numeric_limits<DataType>::max() / iterations;

        for (DataType data = 0, i = 0; i++ < iterations; data += step)
        {
            DataType expected = data;
            DataType actual = fly::endian_swap<Desired>(data);

            if constexpr (Desired != fly::Endian::Native)
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

TYPED_TEST_SUITE(EndianTest, DataTypes, );

//==============================================================================
TYPED_TEST(EndianTest, BigEndian)
{
    TestFixture::template run_test<fly::Endian::Big>();
}

//==============================================================================
TYPED_TEST(EndianTest, LittleEndian)
{
    TestFixture::template run_test<fly::Endian::Little>();
}

//==============================================================================
TYPED_TEST(EndianTest, NativeEndian)
{
    TestFixture::template run_test<fly::Endian::Native>();
}
