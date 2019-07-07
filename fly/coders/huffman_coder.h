#pragma once

#include "fly/coders/coder.h"

#include <istream>
#include <memory>
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
     *     1. Reserve a buffer up-front to store all data from the input stream.
     *     2. Create a Huffman tree from the input stream.
     *     3. Generate standard Huffman codes from the Huffman tree.
     *     4. Convert the standard codes to canonical Huffman codes.
     *     5. Encode the canonical codes.
     *     6. Encode the input stream using the canonical codes.
     *
     * Step 1 is a large speed optimization. On a 100MB file, compiled with -02,
     * creating the Huffman tree alone took upwards of 5 seconds when reading
     * the input stream one symbol at a time. Reading the entire stream at once,
     * steps 1 and 2 together is reduced to milliseconds. Of course, this means
     * acceptable input size is limited by system memory; step 1 should be
     * changed to read the stream in large chunks.
     *
     * This sequence involves iterating over the entire input stream twice (to
     * create the Huffman tree and to encode the stream).
     *
     * The coder does not assume the Huffman codes are retained between by the
     * caller. Thus, the codes are encoded before the input stream (step 6) so
     * that they may be learned during decoding. Canonical form is used for its
     * property of generally being describable in fewer bits than standard form.
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
     * @param BitStreamReader Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool DecodeInternal(BitStreamReader &, std::ostream &) noexcept override;

private:
    /**
     * Read the entire stream into a vector buffer.
     *
     * @param istream Stream holding the contents to read.
     *
     * @return vector Vector holding the entire contents of the input stream.
     */
    std::vector<std::istream::char_type> readStream(std::istream &) const
        noexcept;

    /**
     * Create a Huffman tree from the given input stream.
     *
     * @param vector Buffer holding the contents to parse.
     *
     * @return unique_ptr Scoped pointer to the root node of the Huffman tree.
     */
    std::unique_ptr<HuffmanNode>
    createTree(const std::vector<std::istream::char_type> &) const noexcept;

    /**
     * Create a Huffman tree from the given list of Huffman codes. The created
     * Huffman tree will not contain valid frequencies because that information
     * is not stored in Huffman codes; it is not needed for decoding.
     *
     * @param vector List of Huffman codes to parse.
     *
     * @return unique_ptr Scoped pointer to the root node of the Huffman tree.
     */
    std::unique_ptr<HuffmanNode>
    createTree(const std::vector<HuffmanCode> &) const noexcept;

    /**
     * Create a list of Huffman codes from the given Huffman tree. The returned
     * list of codes are sorted in accordance with canonical form.
     *
     * @param unique_ptr Scoped pointer to the root node of the Huffman tree.
     *
     * @return vector List of created Huffman codes.
     */
    std::vector<HuffmanCode>
    createCodes(const std::unique_ptr<HuffmanNode> &) const noexcept;

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
     * Encode an input stream with a list of Huffman codes. The list of codes
     * is effectively destroyed as its elements are moved to a map for easier
     * lookup.
     *
     * @param vector List of Huffman codes to encode with.
     * @param vector Buffer holding the contents to parse.
     * @param BitStreamWriter Stream to store the encoded contents.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    bool encodeStream(
        std::vector<HuffmanCode> &,
        const std::vector<std::istream::char_type> &,
        BitStreamWriter &) const noexcept;

    /**
     * Decode an encoded input stream with a Huffman tree.
     *
     * @param unique_ptr Scoped pointer to the root node of the Huffman tree.
     * @param BitStreamReader Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool decodeStream(
        const std::unique_ptr<HuffmanNode> &,
        BitStreamReader &,
        std::ostream &) const noexcept;
};

} // namespace fly
