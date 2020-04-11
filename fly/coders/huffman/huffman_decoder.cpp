#include "fly/coders/huffman/huffman_decoder.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/bit_stream/bit_stream_reader.hpp"
#include "fly/types/numeric/literals.hpp"

#include <vector>

namespace fly {

//==============================================================================
bool HuffmanDecoder::DecodeInternal(
    BitStreamReader &input,
    std::ostream &output) noexcept
{
    std::uint32_t chunkSize;
    length_type maxCodeLength;

    if (!decodeHeader(input, chunkSize, maxCodeLength))
    {
        LOGW("Error decoding header from stream");
        return false;
    }

    m_chunkBuffer = std::make_unique<symbol_type[]>(chunkSize);
    m_prefixTable = std::make_unique<HuffmanCode[]>(1_zu << maxCodeLength);

    while (!input.FullyConsumed())
    {
        length_type localMaxCodeLength = 0;

        if (!decodeCodes(input, maxCodeLength, localMaxCodeLength))
        {
            LOGW(
                "Error decoding codes from stream (maximum code length = %u)",
                static_cast<std::uint32_t>(maxCodeLength));
            return false;
        }
        else if (!decodeSymbols(input, localMaxCodeLength, chunkSize, output))
        {
            LOGW(
                "Error decoding %u symbols from stream (fully consumed = %d)",
                chunkSize,
                input.FullyConsumed());
            return false;
        }
    }

    return output.good();
}

//==============================================================================
bool HuffmanDecoder::decodeHeader(
    BitStreamReader &input,
    std::uint32_t &chunkSize,
    length_type &maxCodeLength) const noexcept
{
    // Decode the Huffman coder version.
    byte_type huffmanVersion;

    if (!input.ReadByte(huffmanVersion))
    {
        LOGW("Could not decode Huffman coder version");
        return false;
    }

    switch (huffmanVersion)
    {
        case 1:
            return decodeHeaderVersion1(input, chunkSize, maxCodeLength);

        default:
            LOGW(
                "Decoded invalid Huffman version %u",
                static_cast<std::uint32_t>(huffmanVersion));
            break;
    }

    return false;
}

//==============================================================================
bool HuffmanDecoder::decodeHeaderVersion1(
    BitStreamReader &input,
    std::uint32_t &chunkSize,
    length_type &maxCodeLength) const noexcept
{
    // Decode the chunk size.
    word_type encodedChunkSizeKB;

    if (!input.ReadWord(encodedChunkSizeKB))
    {
        LOGW("Could not decode chunk size");
        return false;
    }
    else if (encodedChunkSizeKB == 0)
    {
        LOGW(
            "Decoded invalid chunk size %u",
            static_cast<std::uint32_t>(encodedChunkSizeKB));
        return false;
    }

    // Decode the maximum Huffman code length.
    byte_type encodedMaxCodeLength;

    if (!input.ReadByte(encodedMaxCodeLength))
    {
        LOGW("Could not decode maximum code length");
        return false;
    }
    else if (
        (encodedMaxCodeLength == 0) ||
        (encodedMaxCodeLength >= std::numeric_limits<code_type>::digits))
    {
        LOGW(
            "Decoded invalid maximum code length %u",
            static_cast<std::uint32_t>(encodedMaxCodeLength));
        return false;
    }

    chunkSize = static_cast<std::uint32_t>(encodedChunkSizeKB) << 10;
    maxCodeLength = static_cast<length_type>(encodedMaxCodeLength);

    return true;
}

//==============================================================================
bool HuffmanDecoder::decodeCodes(
    BitStreamReader &input,
    length_type globalMaxCodeLength,
    length_type &localMaxCodeLength) noexcept
{
    m_huffmanCodesSize = 0;

    // Decode the number of code length counts.
    byte_type countsSize;

    if (!input.ReadByte(countsSize))
    {
        LOGW("Could not decode number of code length counts");
        return false;
    }
    else if ((countsSize == 0) || (countsSize > (globalMaxCodeLength + 1)))
    {
        LOGW(
            "Decoded invalid number of code length counts %u",
            static_cast<std::uint32_t>(countsSize));
        return false;
    }

    // The first code length is 0, so the actual maximum code length is 1 less
    // than the number of length counts.
    localMaxCodeLength = countsSize - 1;

    // Decode the code length counts.
    std::vector<std::uint16_t> counts(countsSize);

    for (std::uint16_t &count : counts)
    {
        if (!input.ReadWord(count))
        {
            LOGW("Could not decode code length counts");
            return false;
        }
    }

    // Decode the symbols.
    for (length_type length = 0; length < countsSize; ++length)
    {
        for (std::uint16_t i = 0; i < counts[length]; ++i)
        {
            byte_type symbol;

            if (!input.ReadByte(symbol))
            {
                LOGW(
                    "Could not decode symbol of length %u bits",
                    static_cast<std::uint32_t>(length));
                return false;
            }

            // First code is always set to zero.
            code_type code = 0;

            // Subsequent codes are one greater than the previous code, but also
            // bit-shifted left enough to maintain the right code length.
            if (m_huffmanCodesSize != 0)
            {
                const HuffmanCode &last =
                    m_huffmanCodes[m_huffmanCodesSize - 1_u16];

                const length_type shift = length - last.m_length;
                code = (last.m_code + 1) << shift;
            }

            if (m_huffmanCodesSize == m_huffmanCodes.size())
            {
                LOGW("Exceeded maximum number of codes %u", m_huffmanCodesSize);
                return false;
            }

            m_huffmanCodes[m_huffmanCodesSize++] =
                HuffmanCode(static_cast<symbol_type>(symbol), code, length);
        }
    }

    convertToPrefixTable(localMaxCodeLength);
    return true;
}

//==============================================================================
void HuffmanDecoder::convertToPrefixTable(length_type maxCodeLength) noexcept
{
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        const HuffmanCode code = std::move(m_huffmanCodes[i]);
        const length_type shift = maxCodeLength - code.m_length;

        for (code_type j = 0; j < (1_u16 << shift); ++j)
        {
            const code_type index = (code.m_code << shift) + j;
            m_prefixTable[index].m_symbol = code.m_symbol;
            m_prefixTable[index].m_length = code.m_length;
        }
    }
}

//==============================================================================
bool HuffmanDecoder::decodeSymbols(
    BitStreamReader &input,
    length_type maxCodeLength,
    std::uint32_t chunkSize,
    std::ostream &output) const noexcept
{
    std::uint32_t bytes = 0;
    code_type index;

    while ((bytes < chunkSize) && (input.PeekBits(index, maxCodeLength) != 0))
    {
        const HuffmanCode &code = m_prefixTable[index];

        m_chunkBuffer[bytes++] = code.m_symbol;
        input.DiscardBits(code.m_length);
    }

    if (bytes > 0)
    {
        output.write(
            reinterpret_cast<const std::ios::char_type *>(m_chunkBuffer.get()),
            static_cast<std::streamsize>(bytes));
    }

    return (bytes == chunkSize) || input.FullyConsumed();
}

} // namespace fly
