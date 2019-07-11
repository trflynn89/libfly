#pragma once

#include "fly/coders/coder.h"

#include <istream>
#include <ostream>
#include <vector>

namespace fly {

struct HuffmanCode;
struct HuffmanNode;
class BitStreamReader;
class BitStreamWriter;

/**
 * Implementation of the Coder interface for Huffman coding. Forms canonical
 * Huffman codes to encode and decode symbols.
 *
 * For reading:
 *
 *     https://en.wikipedia.org/wiki/Huffman_coding
 *     https://en.wikipedia.org/wiki/Canonical_Huffman_code
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
class HuffmanCoder : public Coder
{
protected:
    /**
     * Huffman encode a stream.
     *
     * The steps to encode a stream are:
     *
     *     1. Create a Huffman tree from the input stream.
     *     2. Generate standard Huffman codes from the Huffman tree.
     *     3. Convert the standard codes to canonical Huffman codes.
     *     4. Encode the canonical codes.
     *     5. Encode the input stream using the canonical codes.
     *
     * This sequence involves iterating over the entire input stream twice (to
     * create the Huffman tree and to encode the stream).
     *
     * If the input stream is large, in order to limit memory usage, the stream
     * is encoded in chunks. Each chunk is treated as its own input stream, and
     * the encoding sequence is repeated for each chunk.
     *
     * The coder does not assume the Huffman codes are retained between calls.
     * Thus, the codes are encoded before the input stream (step 4) so that they
     * may be learned during decoding. Canonical form is used for its property
     * of generally being describable in fewer bits than standard form.
     *
     * When in canonical form, the Huffman codes are sorted by code length. With
     * this sorting, the count of the number of symbols for each code length is
     * computed: (N0, N1, N2, ..., Nn), where N<n> is the number of symbols of
     * code length <n>. Call the length of this list NN.
     *
     * The encoding of canonical Huffman codes then becomes:
     *
     *      NN,N0,N1,N2,...,Nn,S0,S1,S2,...,Sn
     *
     * Where S<n> is all symbols of code length <n>.
     *
     * Encoding the input stream (step 5) consists of reading each symbol from
     * the input stream and outputting that symbol's canonical Huffman code.
     *
     * @param istream Stream holding the contents to encode.
     * @param BitStreamWriter Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    bool EncodeInternal(std::istream &, BitStreamWriter &) noexcept override;

    /**
     * Huffman decode a stream.
     *
     * The steps to decode a stream are:
     *
     *     1. Decode the canonical Huffman codes from the stream.
     *     2. Create a partial Huffman tree from the canonical codes.
     *     3. Decode the input stream using the Huffman tree.
     *
     * A complete Huffman tree would also contain symbol frequencies. This
     * information is not required to decode the stream, so it is not encoded.
     *
     * Because large input streams are encoded in chunks, they must also be
     * decoded in chunks. The input stream is decoded until either the end of
     * the stream or the chunk size is reached. The decoding sequence is then
     * repeated for each chunk.
     *
     * @param BitStreamReader Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool DecodeInternal(BitStreamReader &, std::ostream &) noexcept override;

private:
    /**
     * Read the stream into a buffer, up to a static maximum size.
     *
     * @param istream Stream holding the contents to read.
     * @param char_type Buffer to store the contents of the stream.
     *
     * @return size_t The number of bytes that were read.
     */
    std::size_t readStream(std::istream &, std::istream::char_type *) const
        noexcept;

    /**
     * Create a Huffman tree from the given input stream.
     *
     * @param char_type Buffer holding the contents to parse.
     * @param size_t The number of bytes the buffer holds.
     * @param HuffmanNode Pointer to the Huffman tree to fill.
     *
     * @return bool True if the Huffman tree was successfully created.
     */
    bool createTree(const std::istream::char_type *, std::size_t, HuffmanNode *)
        const noexcept;

    /**
     * Create a Huffman tree from the given list of Huffman codes.
     *
     * @param vector List of Huffman codes to parse.
     * @param HuffmanNode Pointer to the Huffman tree to fill.
     *
     * @return bool True if the Huffman tree was successfully created.
     */
    bool createTree(const std::vector<HuffmanCode> &, HuffmanNode *) const
        noexcept;

    /**
     * Create a list of Huffman codes from the given Huffman tree. The returned
     * list of codes are sorted in accordance with canonical form.
     *
     * @param HuffmanNode Pointer to the root node of the Huffman tree.
     *
     * @return vector List of created Huffman codes.
     */
    std::vector<HuffmanCode> createCodes(HuffmanNode *) const noexcept;

    /**
     * Convert a list of standard Huffman codes into canonical Huffman codes. It
     * is assumed that the codes are already sorted in accordance with canonical
     * form.
     *
     * @return vector List of standard Huffman codes to convert.
     */
    void convertToCanonicalForm(std::vector<HuffmanCode> &) const noexcept;

    /**
     * Encode the Huffman codes to the output stream.
     *
     * @param vector List of Huffman codes to encode.
     * @param BitStreamWriter Stream to store the encoded codes.
     *
     * @return bool True if the Huffman codes were successfully encoded.
     */
    bool encodeCodes(const std::vector<HuffmanCode> &, BitStreamWriter &) const
        noexcept;

    /**
     * Decode Huffman codes from an encoded input stream.
     *
     * @param BitStreamReader Stream storing the encoded codes.
     * @param vector List to place the decoded codes into.
     *
     * @return bool True if the Huffman codes were successfully decoded.
     */
    bool decodeCodes(BitStreamReader &, std::vector<HuffmanCode> &) const
        noexcept;

    /**
     * Encode symbols from an input stream with a list of Huffman codes. The
     * list of codes is effectively destroyed as its elements are moved to a map
     * for faster lookups.
     *
     * @param char_type Buffer holding the symbols to parse.
     * @param size_t The number of bytes the buffer holds.
     * @param vector List of Huffman codes to encode with.
     * @param BitStreamWriter Stream to store the encoded symbols.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    bool encodeSymbols(
        const std::istream::char_type *,
        std::size_t,
        std::vector<HuffmanCode> &,
        BitStreamWriter &) const noexcept;

    /**
     * Decode symbols from an encoded input stream with a Huffman tree.
     *
     * @param HuffmanNode Pointer to the Huffman tree to decode with.
     * @param BitStreamReader Stream holding the symbols to decode.
     * @param ostream Stream to store the decoded symbols.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool
    decodeSymbols(const HuffmanNode *, BitStreamReader &, std::ostream &) const
        noexcept;
};

} // namespace fly
