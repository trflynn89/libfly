#include "fly/types/numeric/endian.hpp"

#include "fly/traits/traits.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <bit>
#include <cstdint>
#include <limits>

namespace {

template <typename T, fly::enable_if<fly::size_of_type_is<T, 1>> = 0>
T swap(T x)
{
    return x;
}

template <typename T, fly::enable_if<fly::size_of_type_is<T, 2>> = 0>
T swap(T x)
{
    T result = 0;

    result |= (x & static_cast<T>(0xff00)) >> 8;
    result |= (x & static_cast<T>(0x00ff)) << 8;

    return result;
}

template <typename T, fly::enable_if<fly::size_of_type_is<T, 4>> = 0>
T swap(T x)
{
    T result = 0;

    result |= (x & static_cast<T>(0xff000000)) >> 24;
    result |= (x & static_cast<T>(0x00ff0000)) >> 8;
    result |= (x & static_cast<T>(0x0000ff00)) << 8;
    result |= (x & static_cast<T>(0x000000ff)) << 24;

    return result;
}

template <typename T, fly::enable_if<fly::size_of_type_is<T, 8>> = 0>
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

template <typename DataType, std::endian Desired>
void run_test()
{
    constexpr DataType iterations = 100;
    const DataType step = std::numeric_limits<DataType>::max() / iterations;

    for (DataType data = 0, i = 0; i++ < iterations; data += step)
    {
        {
            DataType expected = swap(data);
            DataType actual = fly::endian_swap(data);

            CATCH_CHECK(expected == actual);
        }
        {
            DataType expected = data;
            DataType actual = fly::endian_swap_if_non_native<Desired>(data);

            if constexpr (Desired != std::endian::native)
            {
                expected = swap(expected);
            }

            CATCH_CHECK(expected == actual);
        }
    }
}

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "Endian",
    "[numeric]",
    std::int8_t,
    std::int16_t,
    std::int32_t,
    std::int64_t,
    std::uint8_t,
    std::uint16_t,
    std::uint32_t,
    std::uint64_t)
{
    CATCH_SECTION("Byte swap to big-endian")
    {
        run_test<TestType, std::endian::big>();
    }

    CATCH_SECTION("Byte swap to little-endian")
    {
        run_test<TestType, std::endian::little>();
    }

    CATCH_SECTION("Byte swap to the system's native endianness")
    {
        run_test<TestType, std::endian::native>();
    }
}
