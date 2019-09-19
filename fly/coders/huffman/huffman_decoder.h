#pragma once

#include "fly/coders/coder.h"
#include "fly/coders/huffman/huffman_types.h"

#include <array>
#include <memory>
#include <ostream>

namespace fly {

class BitStreamReader;

/**
 * Implementation of the Decoder interface for Huffman coding.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version July 7, 2019
 */
class HuffmanDecoder : public Decoder
{
protected:
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
     * Decoding a symbol from the input stream (step 3) consists of peeking N
     * bits from the input stream, where N is maximum length of the decoded
     * Huffman codes. These bits are the index into the prefix table; a single
     * lookup is performed to find the corresponding Huffman code. The actual
     * length of the code is then discarded from the input stream.
     *
     * @param BitStreamReader Stream holding the contents to decode.
     * @param ostream Stream to store the decoded contents.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool DecodeInternal(BitStreamReader &, std::ostream &) noexcept override;

private:
    /**
     * Decode the version of the encoder used to encode the stream, and invoke
     * the header decoder associated with that version.
     *
     * @param BitStreamReader Stream storing the encoded header.
     * @param uint32_t Location to store the maximum chunk size (in bytes).
     * @param length_type Location to store the global maximum Huffman code
     *                    length.
     *
     * @return bool True if the header was successfully decoded.
     */
    bool decodeHeader(BitStreamReader &, std::uint32_t &, length_type &) const
        noexcept;

    /**
     * Decode version 1 of the header. Extract the maximum chunk length and the
     * global maximum Huffman code length the encoder used.
     *
     * @param BitStreamWriter Stream to store the encoded header.
     * @param uint32_t Location to store the maximum chunk size (in bytes).
     * @param length_type Location to store the global maximum Huffman code
     *                    length.
     *
     * @return bool True if the header was successfully encoded.
     */
    bool decodeHeaderVersion1(BitStreamReader &, std::uint32_t &, length_type &)
        const noexcept;

    /**
     * Decode Huffman codes from an encoded input stream. The list of codes will
     * be stored as a prefix table.
     *
     * @param BitStreamReader Stream storing the encoded codes.
     * @param length_type The global maximum Huffman code length.
     * @param length_type Location to store the local maximum Huffman code
     *                    length.
     *
     * @return bool True if the Huffman codes were successfully decoded.
     */
    bool decodeCodes(BitStreamReader &, length_type, length_type &) noexcept;

    /**
     * Convert the decoded list of Huffman codes into a prefix table.
     *
     * @param length_type The local maximum length of the decoded Huffman codes.
     *
     * @return bool True if the prefix table was successfully created.
     */
    void convertToPrefixTable(length_type) noexcept;

    /**
     * Decode symbols from an encoded input stream with a Huffman tree. Store
     * decoded data into a chunk buffer until the decoded chunk size is reached,
     * or the end of the encoded input stream is reached. Then flush those bytes
     * to the real output stream.
     *
     * @param BitStreamReader Stream holding the symbols to decode.
     * @param length_type The local maximum length of the decoded Huffman codes.
     * @param uint32_t The number of bytes the chunk buffer can hold.
     * @param ostream Stream to store the decoded symbols.
     *
     * @return bool True if the input stream was successfully decoded.
     */
    bool
    decodeSymbols(BitStreamReader &, length_type, std::uint32_t, std::ostream &)
        const noexcept;

    std::unique_ptr<symbol_type[]> m_chunkBuffer;

    // Sized to fit 8-bit ASCII symbols.
    std::array<HuffmanCode, 1 << 8> m_huffmanCodes;
    std::uint16_t m_huffmanCodesSize;

    // Will be sized to fit the global maximum Huffman code length used by the
    // encoder. The size will be 2^L, were L is the maximum code length.
    std::unique_ptr<HuffmanCode[]> m_prefixTable;

    // Friend class for unit testing.
    friend class HuffmanCoderTest;
};

} // namespace fly
