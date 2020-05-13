#pragma once

#include "fly/coders/coder.hpp"

#include <array>
#include <istream>
#include <ostream>

namespace fly {

/**
 * A Base64 encoder and decoder.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version May 3, 2020
 */
class Base64Coder : public Encoder, public Decoder
{
    using DecodedChunk = std::array<std::ios::char_type, 3>;
    using EncodedChunk = std::array<std::ios::char_type, 4>;

protected:
    /**
     * Base64 encode a stream.
     *
     * @param decoded Stream holding the contents to encode.
     * @param encoded Stream to store the encoded contents.
     *
     * @return True if the input stream was successfully encoded.
     */
    bool encode_internal(std::istream &decoded, std::ostream &encoded) noexcept
        override;

    /**
     * Base64 decode a stream.
     *
     * @param encoded Stream holding the contents to decode.
     * @param decoded Stream to store the decoded contents.
     *
     * @return True if the input stream was successfully decoded.
     */
    bool decode_internal(std::istream &encoded, std::ostream &decoded) noexcept
        override;

private:
    /**
     * Encode a chunk of data into Base64 symbols.
     *
     * @param chunk Buffer holding the contents to encode.
     * @param bytes Number of bytes from the input buffer to encode.
     * @param encoded Stream to store the encoded contents.
     */
    void encode_chunk(
        const DecodedChunk &chunk,
        std::size_t bytes,
        std::ostream &encoded) const noexcept;

    /**
     * Decode a chunk of Base64 symbols.
     *
     * @param chunk Buffer holding the contents to decode.
     * @param encoded Stream to store the decoded contents.
     */
    void
    decode_chunk(EncodedChunk &chunk, std::ostream &decoded) const noexcept;

    /**
     * Covert a buffer of Base64 symbols to ASCII codes, validating that each
     * symbol is valid Base64. Coverts the symbols in place.
     *
     * @param chunk Buffer holding the contents to decode.
     *
     * @return The number of Base64 padding symbols in the buffer. If an invalid
     *         symbol is parsed, returns a value as if every symbol was padding.
     */
    std::size_t parse_chunk(EncodedChunk &chunk) const noexcept;
};

} // namespace fly
