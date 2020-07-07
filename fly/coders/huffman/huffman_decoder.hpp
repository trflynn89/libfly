#pragma once

#include "fly/coders/coder.hpp"
#include "fly/coders/huffman/huffman_types.hpp"

#include <array>
#include <memory>
#include <ostream>

namespace fly {

class BitStreamReader;

/**
 * Implementation of the Decoder interface for Huffman coding.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class HuffmanDecoder : public BinaryDecoder
{
public:
    /**
     * Constructor.
     */
    HuffmanDecoder() noexcept;

    /**
     * Compute the Kraft–McMillan constant of the decoded Huffman codes. Primarily meant for unit
     * testing.
     *
     * @return The Kraft–McMillan constant.
     */
    code_type compute_kraft_mcmillan_constant() const;

protected:
    /**
     * Huffman decode a stream.
     *
     * Because large input streams are encoded in chunks, they must also be decoded in chunks. The
     * input stream is decoded until either the end of the stream or the chunk size is reached. The
     * decoding sequence is then repeated for each chunk.
     *
     * The sequence to decode a stream is:
     *
     *     1. Decode the canonical Huffman codes from the stream.
     *     2. Convert the canonical codes to a prefix table.
     *     3. Decode the input stream using the table.
     *
     * Prefix tables (step 2) function via the property that no Huffman code is a prefix of any
     * other code. Thus, a table can be formed as an array, whose indices are integers where the
     * most-significant bits are Huffman codes.
     *
     * Decoding a symbol from the input stream (step 3) consists of peeking N bits from the input
     * stream, where N is maximum length of the decoded Huffman codes. These bits are the index into
     * the prefix table; a single lookup is performed to find the corresponding Huffman code. The
     * actual length of the code is then discarded from the input stream.
     *
     * @param encoded Stream holding the contents to decode.
     * @param decoded Stream to store the decoded contents.
     *
     * @return True if the input stream was successfully decoded.
     */
    bool decode_binary(BitStreamReader &encoded, std::ostream &decoded) override;

private:
    /**
     * Decode the version of the encoder used to encode the stream, and invoke the header decoder
     * associated with that version.
     *
     * @param encoded Stream storing the encoded header.
     * @param chunk_size Location to store the maximum chunk size (in bytes).
     *
     * @return True if the header was successfully decoded.
     */
    bool decode_header(BitStreamReader &encoded, std::uint32_t &chunk_size);

    /**
     * Decode version 1 of the header. Extract the maximum chunk length and the global maximum
     * Huffman code length the encoder used.
     *
     * @param encoded Stream storing the encoded header.
     * @param chunk_size Location to store the maximum chunk size (in bytes).
     *
     * @return True if the header was successfully encoded.
     */
    bool decode_header_version1(BitStreamReader &encoded, std::uint32_t &chunk_size);

    /**
     * Decode Huffman codes from an encoded input stream. The list of codes will be stored as a
     * prefix table.
     *
     * @param encoded Stream storing the encoded codes.
     * @param max_code_length Location to store the local maximum Huffman code length.
     *
     * @return True if the Huffman codes were successfully decoded.
     */
    bool decode_codes(BitStreamReader &encoded, length_type &max_code_length);

    /**
     * Convert the decoded list of Huffman codes into a prefix table.
     *
     * @param max_code_length The maximum length of the decoded Huffman codes.
     */
    void convert_to_prefix_table(length_type max_code_length);

    /**
     * Decode symbols from an encoded input stream with a Huffman tree. Store decoded data into a
     * chunk buffer until the decoded chunk size is reached, or the end of the encoded input stream
     * is reached. Then flush those bytes to the real output stream.
     *
     * @param encoded Stream holding the symbols to decode.
     * @param max_code_length The maximum length of the decoded Huffman codes.
     * @param chunk_size The number of bytes the chunk buffer can hold.
     * @param decoded Stream to store the decoded symbols.
     *
     * @return True if the input stream was successfully decoded.
     */
    bool decode_symbols(
        BitStreamReader &encoded,
        length_type max_code_length,
        std::uint32_t chunk_size,
        std::ostream &decoded) const;

    std::unique_ptr<symbol_type[]> m_chunk_buffer;

    // Sized to fit 8-bit ASCII symbols.
    std::array<HuffmanCode, 1 << 8> m_huffman_codes;
    std::uint16_t m_huffman_codes_size;
    length_type m_max_code_length;

    // Will be sized to fit the global maximum Huffman code length used by the encoder. The size
    // will be 2^L, were L is the maximum code length.
    std::unique_ptr<HuffmanCode[]> m_prefix_table;
};

} // namespace fly
