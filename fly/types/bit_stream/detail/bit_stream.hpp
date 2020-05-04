#pragma once

#include "fly/types/bit_stream/bit_stream_types.hpp"
#include "fly/types/bit_stream/detail/bit_stream_traits.hpp"

#include <limits>

namespace fly::detail {

/**
 * Base class for writing to and reading from a binary stream. The first byte of
 * the binary stream is reserved as a header for internal use.
 *
 * The BitStream implementations allow reading and writing content bit-by-bit.
 * Of course, files cannot contain partial bytes. If a BitStream is closed with
 * a partial byte remaining to be written, that byte is zero-filled, and the
 * number of extra bits written is encoded into the header.
 *
 * The format of the header byte is then:
 *
 *     |    5 bits    |           3 bits           |
 *     ---------------------------------------------
 *     | Magic number | Number of zero-filled bits |
 *
 * Each BitStream implementation essentially serves as a wrapper around an
 * already existing std::istream or std::ostream. It is expected that the
 * pre-existing stream outlive the wrapper BitStream instance.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class BitStream
{
public:
    /**
     * Destructor.
     */
    virtual ~BitStream() = default;

protected:
    /**
     * Protected constructor to prevent instantiating this class directly.
     *
     * @param byte_type Initial cursor position.
     */
    explicit BitStream(byte_type) noexcept;

    /**
     * Create a bit-mask with the least-significant bits set. The size of the
     * mask is determined by the template DataType parameter.
     *
     * @tparam DataType The data type storing the number of bits to set.
     *
     * @param DataType The number of bits to set.
     *
     * @return DataType The created mask.
     */
    template <typename DataType>
    constexpr inline static DataType BitMask(const DataType);

    byte_type m_position;
    buffer_type m_buffer;
};

//==============================================================================
template <typename DataType>
constexpr inline DataType BitStream::BitMask(const DataType bits)
{
    static_assert(
        BitStreamTraits::is_unsigned_integer_v<DataType>,
        "DataType must be an unsigned integer type");

    constexpr auto filled = std::numeric_limits<DataType>::max();
    constexpr auto digits = std::numeric_limits<DataType>::digits;

    return static_cast<DataType>(-(bits != 0)) & (filled >> (digits - bits));
}

} // namespace fly::detail
