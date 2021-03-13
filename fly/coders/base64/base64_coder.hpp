#pragma once

#include "fly/coders/coder.hpp"

#include <array>
#include <istream>
#include <ostream>

namespace fly::coders {

/**
 * A Base64 encoder and decoder.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 3, 2020
 */
class Base64Coder : public Encoder, public Decoder
{
protected:
    /**
     * Base64 encode a stream.
     *
     * @param decoded Stream holding the contents to encode.
     * @param encoded Stream to store the encoded contents.
     *
     * @return True if the input stream was successfully encoded.
     */
    bool encode_internal(std::istream &decoded, std::ostream &encoded) override;

    /**
     * Base64 decode a stream.
     *
     * @param encoded Stream holding the contents to decode.
     * @param decoded Stream to store the decoded contents.
     *
     * @return True if the input stream was successfully decoded.
     */
    bool decode_internal(std::istream &encoded, std::ostream &decoded) override;

private:
    static constexpr const std::size_t s_decoded_chunk_size = 3;
    static constexpr const std::size_t s_encoded_chunk_size = 4;

    std::array<std::ios::char_type, (64 * s_decoded_chunk_size) << 10> m_decoded;
    std::array<std::ios::char_type, (64 * s_encoded_chunk_size) << 10> m_encoded;
};

} // namespace fly::coders
