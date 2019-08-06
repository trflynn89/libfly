#pragma once

#include "fly/coders/coder.h"
#include "fly/coders/huffman_types.h"

#include <array>
#include <istream>
#include <ostream>
#include <vector>

namespace fly {

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
     * If the input stream is large, in order to limit memory usage, the stream
     * is encoded in chunks. Each chunk is treated as its own input stream, and
     * the encoding sequence is repeated for each chunk.
     *
     * The first bytes of the output stream are reserved as a header. Currently,
     * the header contains: the incurred BitStream header, the version of the
     * Huffman coder used to encode the stream, and the maximum chunk length
     * used to split large streams (in kilobytes):
     *
     *     |      8 bits      |        8 bits        |      16 bits      |
     *     ---------------------------------------------------------------
     *     | BitStream header | HuffmanCoder version | Chunk length (KB) |
     *
     * The sequence to encode a stream is:
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
     * Because large input streams are encoded in chunks, they must also be
     * decoded in chunks. The input stream is decoded until either the end of
     * the stream or the chunk size is reached. The decoding sequence is then
     * repeated for each chunk.
     *
     * The sequence to decode a stream is:
     *
     *     1. Decode the canonical Huffman codes from the stream.
     *     2. Convert the canonical codes to a prefix table.
     *     3. Decode the input stream using the table.
     *
     * Prefix tables (step 2) function via the property that no Huffman code is
     * a prefix of any other code. Thus, a table can be formed as an array,
     * whose indices are integers where the most-significant bits are Huffman
     * codes.
     *
     * Decoding a symbol the input stream (step 3) consists of peeking N bits
     * from the input stream, where N is maximum length of the decoded Huffman
     * codes. These bits are the index into the prefix table; a single lookup is
     * performed to find the corresponding Huffman code. The actual length of
     * the code is discarded from the input stream.
     *
     * @param BitStreamReader Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool DecodeInternal(BitStreamReader &, std::ostream &) noexcept override;

private:
    /**
     * Reset internal data structures between encoding or decoding chunks.
     */
    void reset() noexcept;

    /**
     * Read the stream into a buffer, up to a static maximum size.
     *
     * @param istream Stream holding the contents to read.
     * @param char_type Buffer to store the contents of the stream.
     *
     * @return uint32_t The number of bytes that were read.
     */
    std::uint32_t readStream(std::istream &, std::istream::char_type *) const
        noexcept;

    /**
     * Create a Huffman tree from the given input stream.
     *
     * @param char_type Buffer holding the contents to parse.
     * @param uint32_t The number of bytes the buffer holds.
     *
     * @return bool True if the Huffman tree was successfully created.
     */
    bool createTree(const std::istream::char_type *, std::uint32_t) noexcept;

    /**
     * Create a Huffman tree from the decoded list of Huffman codes.
     *
     * @return bool True if the Huffman tree was successfully created.
     */
    bool createTree() noexcept;

    /**
     * Create a list of Huffman codes from the generated Huffman tree. The list
     * of codes will be in canonical form.
     *
     * @return bool True if the Huffman codes were successfully created.
     */
    bool createCodes() noexcept;

    /**
     * Insert a new Huffman code into the list of already sorted codes.
     *
     * @param HuffmanCode The Huffman code to insert.
     *
     * @return bool True if the Huffman code could be inserted.
     */
    bool insertCode(HuffmanCode &&) noexcept;

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
     *
     * @return bool True if the header was successfully encoded.
     */
    bool encodeHeader(BitStreamWriter &) const noexcept;

    /**
     * Decode the version of the encoder used to encode the stream, and invoke
     * the header decoder associated with that version.
     *
     * @param BitStreamReader Stream storing the encoded header.
     * @param uint32_t Location to store the maximum chunk size (in bytes).
     *
     * @return bool True if the header was successfully decoded.
     */
    bool decodeHeader(BitStreamReader &, std::uint32_t &) const noexcept;

    /**
     * Decode version 1 of the header. Extract the maximum chunk length the
     * encoder used.
     *
     * @param BitStreamWriter Stream to store the encoded header.
     * @param uint32_t Location to store the maximum chunk size (in bytes).
     *
     * @return bool True if the header was successfully encoded.
     */
    bool decodeHeaderVersion1(BitStreamReader &, std::uint32_t &) const
        noexcept;

    /**
     * Encode the generated Huffman codes to the output stream.
     *
     * @param BitStreamWriter Stream to store the encoded codes.
     *
     * @return bool True if the Huffman codes were successfully encoded.
     */
    bool encodeCodes(BitStreamWriter &) const noexcept;

    /**
     * Decode Huffman codes from an encoded input stream. The list of codes will
     * be stored as a prefix table.
     *
     * @param BitStreamReader Stream storing the encoded codes.
     *
     * @return bool True if the Huffman codes were successfully decoded.
     */
    bool decodeCodes(BitStreamReader &) noexcept;

    /**
     * Convert the decoded list of Huffman codes into a prefix table.
     *
     * @return bool True if the prefix table was successfully created.
     */
    void convertToPrefixTable(std::uint8_t) noexcept;

    /**
     * Encode symbols from an input stream with the generated list of Huffman
     * codes. The list of codes is effectively destroyed as its elements are
     * moved to a map for faster lookups.
     *
     * @param char_type Buffer holding the symbols to parse.
     * @param uint32_t The number of bytes the buffer holds.
     * @param BitStreamWriter Stream to store the encoded symbols.
     *
     * @return bool True if the input stream was successfully encoded.
     */
    bool encodeSymbols(
        const std::istream::char_type *,
        std::uint32_t,
        BitStreamWriter &) noexcept;

    /**
     * Decode symbols from an encoded input stream with a Huffman tree.
     *
     * @param BitStreamReader Stream holding the symbols to decode.
     * @param char_type Intermediate buffer to hold the decoded symbols.
     * @param uint32_t The number of bytes the intermediate buffer can hold.
     * @param ostream Stream to store the decoded symbols.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool decodeSymbols(
        BitStreamReader &,
        std::ostream::char_type *,
        std::uint32_t,
        std::ostream &) noexcept;

    std::array<HuffmanNode, 256> m_huffmanTree;
    std::array<HuffmanCode, 256> m_huffmanCodes;
    std::uint16_t m_huffmanCodesSize;
};

} // namespace fly
