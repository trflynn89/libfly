#pragma once

#include "fly/coders/coder.hpp"
#include "fly/coders/huffman/huffman_types.hpp"

#include <array>
#include <istream>
#include <memory>

namespace fly {

class BitStreamWriter;
class HuffmanConfig;

/**
 * Implementation of the Encoder interface for Huffman coding. Forms length-
 * limted, canonical Huffman codes to encode symbols.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class HuffmanEncoder : public BinaryEncoder
{
public:
    /**
     * Constructor.
     *
     * @param config Reference to Huffman configuration.
     */
    explicit HuffmanEncoder(
        const std::shared_ptr<HuffmanConfig> &config) noexcept;

protected:
    /**
     * Huffman encode a stream.
     *
     * If the input stream is large, in order to limit memory usage, the stream
     * is encoded in chunks. Each chunk is treated as its own input stream, and
     * the encoding sequence is repeated for each chunk.
     *
     * The first bytes of the output stream are reserved as a header. Currently,
     * the header contains: the incurred BitStream header, the version of the
     * Huffman coder used to encode the stream, the maximum chunk length used to
     * to split large streams (in kilobytes), and the maximum allowed Huffman
     * code length:
     *
     *     |      8 bits      |  8 bits |      16 bits      |      8 bits     |
     *     --------------------------------------------------------------------
     *     | BitStream header | Version | Chunk length (KB) | Max code length |
     *
     * The sequence to encode a stream is:
     *
     *     1. Create a Huffman tree from the input stream.
     *     2. Generate standard Huffman codes from the Huffman tree.
     *     3. Length-limit the standard Huffman codes.
     *     4. Convert the length-limited codes to canonical Huffman codes.
     *     5. Encode the canonical codes.
     *     6. Encode the input stream using the canonical codes.
     *
     * This sequence involves iterating over the entire input stream twice (to
     * create the Huffman tree and to encode the stream).
     *
     * The coder does not assume the Huffman codes are retained between calls.
     * Thus, the codes are encoded before the input stream (step 5) so that they
     * may be learned during decoding.
     *
     * Length-limiting is performed on the generated Huffman codes to improve
     * decoder performance. Worst-case, a Huffman code could have the same
     * length as the maximum number of symbols. Limiting the length of Huffman
     * codes awards a significant decoder performance improvement, while only
     * incurring a small cost in compression ratio.
     *
     * Canonical form is used for its property of generally being describable in
     * fewer bits than standard form. When in canonical form, the Huffman codes
     * are sorted by code length. With this sorting, the count of the number of
     * symbols for each code length is computed: (N0, N1, N2, ..., Nn), where
     * N<n> is the number of symbols of code length <n>. Call the length of this
     * list NN.
     *
     * The encoding of canonical Huffman codes then becomes:
     *
     *      NN,N0,N1,N2,...,Nn,S0,S1,S2,...,Sn
     *
     * Where S<n> is all symbols of code length <n>.
     *
     * Encoding the input stream (step 6) consists of reading each symbol from
     * the input stream and outputting that symbol's canonical Huffman code.
     *
     * @param decoded Stream holding the contents to encode.
     * @param encoded Stream to store the encoded contents.
     *
     * @return True if the input stream was successfully encoded.
     */
    bool encode_binary(std::istream &decoded, BitStreamWriter &encoded) noexcept
        override;

private:
    /**
     * Read the stream into a buffer, up to a static maximum size, storing bytes
     * in the chunk buffer.
     *
     * @param decoded Stream holding the contents to encode.
     *
     * @return The number of bytes that were read.
     */
    std::uint32_t read_stream(std::istream &decoded) const noexcept;

    /**
     * Create a Huffman tree from the current chunk buffer.
     *
     * @param chunk_size The number of bytes the chunk buffer holds.
     */
    void create_tree(std::uint32_t chunk_size) noexcept;

    /**
     * Create a list of Huffman codes from the generated Huffman tree. The list
     * of codes will be in canonical form.
     */
    void create_codes() noexcept;

    /**
     * Insert a new Huffman code into the list of already sorted codes.
     *
     * @param code The Huffman code to insert.
     */
    void insert_code(HuffmanCode &&code) noexcept;

    /**
     * Length-limit the generated Huffman codes to a static maximum size, using
     * a method described in Charles Bloom's blog, which is based around the
     * Kraft–McMillan inequality:
     *
     * https://cbloomrants.blogspot.com/2010/07/07-03-10-length-limitted-huffman-codes.html
     */
    void limit_code_lengths() noexcept;

    /**
     * Convert the generated list of standard Huffman codes into canonical form.
     * It is assumed that the codes are already sorted in accordance with
     * canonical form.
     */
    void convert_to_canonical_form() noexcept;

    /**
     * Encode the header to the output stream.
     *
     * @param encoded Stream to store the encoded header.
     */
    void encode_header(BitStreamWriter &encoded) const noexcept;

    /**
     * Encode the generated Huffman codes to the output stream.
     *
     * @param encoded Stream to store the encoded codes.
     */
    void encode_codes(BitStreamWriter &encoded) const noexcept;

    /**
     * Encode symbols from the current chunk buffer with the generated list of
     * Huffman codes. The list of codes is effectively destroyed as its elements
     * are moved to a map for faster lookups.
     *
     * @param chunk_size The number of bytes the chunk buffer holds.
     * @param encoded Stream to store the encoded symbols.
     */
    void
    encode_symbols(std::uint32_t chunk_size, BitStreamWriter &encoded) noexcept;

    // Configuration.
    const std::uint32_t m_chunk_size;
    const length_type m_max_code_length;

    std::unique_ptr<symbol_type[]> m_chunk_buffer;

    // Sized to fit 8-bit ASCII symbols.
    std::array<HuffmanCode, 1 << 8> m_huffman_codes;
    std::uint16_t m_huffman_codes_size;

    // Sized to fit a complete Huffman tree. With 8-bit symbols, a complete tree
    // will have a height of 9, and 2^9 - 1 = 511 nodes (round to 512).
    std::array<HuffmanNode, 1 << 9> m_huffman_tree;
};

} // namespace fly
