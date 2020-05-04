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
     * @param istream Stream holding the contents to encode.
     * @param ostream Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    bool EncodeInternal(std::istream &, std::ostream &) noexcept override;

    /**
     * Base64 decode a stream.
     *
     * @param istream Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool DecodeInternal(std::istream &, std::ostream &) noexcept override;

private:
    /**
     * Encode a chunk of data into Base64 symbols.
     *
     * @param DecodedChunk Buffer holding the contents to encode.
     * @param size_t Number of bytes from the input buffer to encode.
     * @param ostream Stream to store the encoded contents.
     */
    void encodeChunk(const DecodedChunk &, std::size_t, std::ostream &)
        const noexcept;

    /**
     * Decode a chunk of Base64 symbols.
     *
     * @param EncodedChunk Buffer holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     */
    void decodeChunk(EncodedChunk &, std::ostream &) const noexcept;

    /**
     * Covert a buffer of Base64 symbols to ASCII codes, validating that each
     * symbol is valid Base64. Coverts the symbols in place.
     *
     * @param EncodedChunk Buffer holding the contents to decode.
     *
     * @return size_t The number of Base64 padding symbols in the buffer. If an
     *                invalid symbol is parsed, returns a value as if every
     *                symbol were padding.
     */
    std::size_t parseChunk(EncodedChunk &) const noexcept;
};

} // namespace fly
