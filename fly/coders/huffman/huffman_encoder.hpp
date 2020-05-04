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
     * @param HuffmanConfig Reference to Huffman configuration.
     */
    HuffmanEncoder(const std::shared_ptr<HuffmanConfig> &) noexcept;

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
     * @param istream Stream holding the contents to encode.
     * @param BitStreamWriter Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    bool EncodeBinary(std::istream &, BitStreamWriter &) noexcept override;

private:
    /**
     * Read the stream into a buffer, up to a static maximum size, storing bytes
     * in the chunk buffer.
     *
     * @param istream Stream holding the contents to read.
     *
     * @return uint32_t The number of bytes that were read.
     */
    std::uint32_t readStream(std::istream &) const noexcept;

    /**
     * Create a Huffman tree from the current chunk buffer.
     *
     * @param uint32_t The number of bytes the chunk buffer holds.
     */
    void createTree(std::uint32_t) noexcept;

    /**
     * Create a list of Huffman codes from the generated Huffman tree. The list
     * of codes will be in canonical form.
     */
    void createCodes() noexcept;

    /**
     * Insert a new Huffman code into the list of already sorted codes.
     *
     * @param HuffmanCode The Huffman code to insert.
     */
    void insertCode(HuffmanCode &&) noexcept;

    /**
     * Length-limit the generated Huffman codes to a static maximum size, using
     * a method described in Charles Bloom's blog, which is based around the
     * Kraft–McMillan inequality:
     *
     * https://cbloomrants.blogspot.com/2010/07/07-03-10-length-limitted-huffman-codes.html
     */
    void limitCodeLengths() noexcept;

    /**
     * Convert the generated list of standard Huffman codes into canonical form.
     * It is assumed that the codes are already sorted in accordance with
     * canonical form.
     */
    void convertToCanonicalForm() noexcept;

    /**
     * Encode the header to the output stream.
     *
     * @param BitStreamWriter Stream to store the encoded header.
     */
    void encodeHeader(BitStreamWriter &) const noexcept;

    /**
     * Encode the generated Huffman codes to the output stream.
     *
     * @param BitStreamWriter Stream to store the encoded codes.
     */
    void encodeCodes(BitStreamWriter &) const noexcept;

    /**
     * Encode symbols from the current chunk buffer with the generated list of
     * Huffman codes. The list of codes is effectively destroyed as its elements
     * are moved to a map for faster lookups.
     *
     * @param uint32_t The number of bytes the chunk buffer holds.
     * @param BitStreamWriter Stream to store the encoded symbols.
     */
    void encodeSymbols(std::uint32_t, BitStreamWriter &) noexcept;

    // Configuration.
    const std::uint32_t m_chunkSize;
    const length_type m_maxCodeLength;

    std::unique_ptr<symbol_type[]> m_chunkBuffer;

    // Sized to fit 8-bit ASCII symbols.
    std::array<HuffmanCode, 1 << 8> m_huffmanCodes;
    std::uint16_t m_huffmanCodesSize;

    // Sized to fit a complete Huffman tree. With 8-bit symbols, a complete tree
    // will have a height of 9, and 2^9 - 1 = 511 nodes (round to 512).
    std::array<HuffmanNode, 1 << 9> m_huffmanTree;
};

} // namespace fly
