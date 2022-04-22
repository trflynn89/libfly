#pragma once

#include "fly/types/bit_stream/detail/concepts.hpp"
#include "fly/types/bit_stream/types.hpp"

#include <limits>
#include <streambuf>

namespace fly::detail {

/**
 * Base class for writing to and reading from a binary stream. The first byte of the binary stream
 * is reserved as a header for internal use.
 *
 * The BitStream implementations allow reading and writing content bit-by-bit. Of course, files
 * cannot contain partial bytes. If a BitStream is closed with a partial byte remaining to be
 * written, that byte is zero-filled, and the number of extra bits written is encoded into the
 * header.
 *
 * The format of the header byte is then:
 *
 *     |    5 bits    |           3 bits           |
 *     ---------------------------------------------
 *     | Magic number | Number of zero-filled bits |
 *
 * Each BitStream implementation essentially serves as a wrapper around an already existing
 * std::istream or std::ostream. It is expected that the pre-existing stream outlive the wrapper
 * BitStream instance.
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
    virtual ~BitStream();

protected:
    /**
     * Protected constructor to prevent instantiating this class directly.
     *
     * @param stream_buffer Pointer to the stream's underlying stream buffer.
     * @param starting_position Initial cursor position.
     */
    BitStream(std::streambuf *stream_buffer, byte_type starting_position) noexcept;

    /**
     * Create a bit-mask with the least-significant bits set. The size of the mask is determined by
     * the template DataType parameter.
     *
     * @tparam DataType The data type storing the number of bits to set.
     *
     * @param starting_position The number of bits to set.
     *
     * @return The created mask.
     */
    template <detail::BitStreamInteger DataType>
    DataType bit_mask(DataType const bits);

    std::streambuf *m_stream_buffer;

    buffer_type m_buffer {0};
    byte_type m_position {0};
};

//==================================================================================================
template <detail::BitStreamInteger DataType>
inline DataType BitStream::bit_mask(DataType const bits)
{
    static constexpr auto s_filled = std::numeric_limits<DataType>::max();
    static constexpr auto s_digits = std::numeric_limits<DataType>::digits;

    return (bits == 0) ? 0 : s_filled >> (s_digits - bits);
}

} // namespace fly::detail
