#include "fly/coders/huffman_coder.h"

#include "fly/coders/bit_stream.h"
#include "fly/fly.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <stack>
#include <vector>

namespace fly {

namespace {

    constexpr const std::uint8_t s_huffmanVersion = 1;

    constexpr const std::uint16_t s_chunkSizeKB = 1 << 10;
    constexpr const std::uint32_t s_chunkSize = s_chunkSizeKB << 10;

    constexpr const length_type s_maxCodeLength = 11;

    static_assert(std::numeric_limits<code_type>::digits >= s_maxCodeLength,
        "Maximum Huffman code length is too large for code_type");

} // namespace

//==============================================================================
bool HuffmanCoder::EncodeInternal(
    std::istream &input,
    BitStreamWriter &output) noexcept
{
    if (!encodeHeader(output))
    {
        return false;
    }

    m_chunkBuffer = std::make_unique<symbol_type[]>(s_chunkSize);
    std::uint32_t chunkSize = 0;

    while ((chunkSize = readStream(input)) > 0)
    {
        if (!createTree(chunkSize))
        {
            return false;
        }
        else if (!createCodes())
        {
            return false;
        }
        else if (!encodeCodes(output))
        {
            return false;
        }
        else if (!encodeSymbols(chunkSize, output))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::DecodeInternal(
    BitStreamReader &input,
    std::ostream &output) noexcept
{
    std::uint32_t chunkSize;
    length_type maxCodeLength;

    if (!decodeHeader(input, chunkSize, maxCodeLength))
    {
        return false;
    }

    m_chunkBuffer = std::make_unique<symbol_type[]>(chunkSize);
    m_prefixTable = std::make_unique<HuffmanCode[]>(U64(1) << maxCodeLength);

    while (!input.FullyConsumed())
    {
        if (!decodeCodes(input, maxCodeLength))
        {
            return false;
        }
        else if (!decodeSymbols(input, maxCodeLength, chunkSize, output))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
std::uint32_t HuffmanCoder::readStream(std::istream &input) const noexcept
{
    const std::istream::pos_type start = input.tellg();
    input.seekg(0, std::ios::end);

    const std::istream::pos_type length = input.tellg() - start;
    input.seekg(start, std::ios::beg);

    if (length > 0)
    {
        input.read(
            reinterpret_cast<std::ios::char_type *>(m_chunkBuffer.get()),
            std::min(static_cast<std::uint32_t>(length), s_chunkSize));

        return static_cast<std::uint32_t>(input.gcount());
    }

    return 0;
}

//==============================================================================
bool HuffmanCoder::createTree(std::uint32_t chunkSize) noexcept
{
    // Lambda to retrieve the next available HuffmanNode
    std::uint16_t index = 0;

    auto next_node = [this, &index]() -> HuffmanNode * {
        if (++index < m_huffmanTree.size())
        {
            return &m_huffmanTree[index];
        }

        return nullptr;
    };

    // Create a frequency map of each input symbol.
    std::array<frequency_type, 256> counts {};

    for (std::uint32_t i = 0; i < chunkSize; ++i)
    {
        ++counts[m_chunkBuffer[i]];
    }

    // Create a priority queue of HuffmanNode, sorted such that the least common
    // symbol is always on top.
    HuffmanNodeQueue queue;
    symbol_type symbol = 0;

    do
    {
        if (counts[symbol] == 0)
        {
            continue;
        }

        HuffmanNode *node = next_node();
        if (node == nullptr)
        {
            return false;
        }

        node->BecomeSymbol(symbol, counts[symbol]);
        queue.push(node);
    } while (++symbol != 0);

    // Convert the priority queue to a Huffman tree. Remove the two least common
    // symbols, combining their frequencies into a new node, and insert the new
    // node back into the priority queue. Continue until only the root remains.
    while (queue.size() > 1)
    {
        auto *left = queue.top();
        queue.pop();

        auto *right = queue.top();
        queue.pop();

        HuffmanNode *node = next_node();
        if (node == nullptr)
        {
            return false;
        }

        node->BecomeIntermediate(left, right);
        queue.push(node);
    }

    HuffmanNode *node = queue.top();
    m_huffmanTree[0] = std::move(*node);

    return true;
}

//==============================================================================
bool HuffmanCoder::createCodes() noexcept
{
    std::stack<const HuffmanNode *> pending;
    std::stack<const HuffmanNode *> path;

    length_type maxCodeLength = 0;
    m_huffmanCodesSize = 0;

    // Symbol frequency is no longer needed. Use that field to form codes.
    m_huffmanTree[0].m_frequency = 0;

    const HuffmanNode *node = nullptr;
    pending.push(&m_huffmanTree[0]);

    while (!pending.empty())
    {
        node = pending.top();

        if (!path.empty() && (node == path.top()))
        {
            pending.pop();
            path.pop();
        }
        else
        {
            if ((node->m_left == nullptr) || (node->m_right == nullptr))
            {
                const auto code = static_cast<code_type>(node->m_frequency);
                const auto length = static_cast<length_type>(path.size());
                maxCodeLength = std::max(maxCodeLength, length);

                if (!insertCode(HuffmanCode(node->m_symbol, code, length)))
                {
                    return false;
                }
            }
            else
            {
                node->m_left->m_frequency = node->m_frequency << 1;
                pending.push(node->m_left);

                node->m_right->m_frequency = (node->m_frequency << 1) + 1;
                pending.push(node->m_right);
            }

            path.push(node);
        }
    }

    if (maxCodeLength > s_maxCodeLength)
    {
        limitCodeLengths();
    }

    convertToCanonicalForm();
    return true;
}

//==============================================================================
bool HuffmanCoder::insertCode(HuffmanCode &&code) noexcept
{
    if (m_huffmanCodesSize == m_huffmanCodes.size())
    {
        return false;
    }

    std::uint16_t pos = m_huffmanCodesSize++;

    for (; (pos > 0) && (code < m_huffmanCodes[pos - 1]); --pos)
    {
        m_huffmanCodes[pos] = std::move(m_huffmanCodes[pos - 1]);
    }

    m_huffmanCodes[pos] = std::move(code);
    return true;
}

//==============================================================================
void HuffmanCoder::limitCodeLengths() noexcept
{
    auto computeKraft = [](const HuffmanCode &code) -> code_type {
        return 1 << (s_maxCodeLength - code.m_length);
    };

    constexpr const code_type maxAllowedKraft = (1 << s_maxCodeLength) - 1;
    code_type kraft = 0;

    // Limit all Huffman codes to not be larger than the maximum code length.
    // Compute the Kraft number, which will no longer satisfy the Kraft–McMillan
    // inequality.
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        HuffmanCode &code = m_huffmanCodes[i];

        code.m_length = std::min(code.m_length, s_maxCodeLength);
        kraft += computeKraft(code);
    }

    // The code lengths must now be corrected to satisfy the Kraft–McMillan
    // inequality. Starting from the largest code, increase the code lengths
    // until the inequality is satisfied again.
    for (std::uint16_t i = m_huffmanCodesSize;
         (i-- > 0) && (kraft > maxAllowedKraft);)
    {
        HuffmanCode &code = m_huffmanCodes[i];

        while (code.m_length < s_maxCodeLength)
        {
            ++code.m_length;
            kraft -= computeKraft(code);
        }
    }

    // The Kraft–McMillan inequality is now satisfied, but possibly overly so.
    // Starting from the shortest code, decrease code lengths just until the
    // inequality would no longer be satisfied.
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        HuffmanCode &code = m_huffmanCodes[i];
        code_type candidate;

        while ((candidate = kraft + computeKraft(code)) <= maxAllowedKraft)
        {
            kraft = candidate;
            --code.m_length;
        }
    }
}

//==============================================================================
void HuffmanCoder::convertToCanonicalForm() noexcept
{
    // First code is always set to zero. Its length does not change.
    m_huffmanCodes[0].m_code = 0;

    if (m_huffmanCodesSize == 1)
    {
        // Single-node Huffman trees occur when the input stream contains only
        // one unique symbol. Set its length to one so a single bit is encoded
        // for each occurrence of that symbol.
        m_huffmanCodes[0].m_length = 1;
    }

    for (std::uint16_t i = 1; i < m_huffmanCodesSize; ++i)
    {
        // Subsequent codes are one greater than the previous code, but also
        // bit-shifted left enough times to maintain the same code length.
        m_huffmanCodes[i].m_code = m_huffmanCodes[i - 1].m_code + 1;

        const length_type shift =
            m_huffmanCodes[i].m_length - m_huffmanCodes[i - 1].m_length;
        m_huffmanCodes[i].m_code <<= shift;
    }
}

//==============================================================================
bool HuffmanCoder::encodeHeader(BitStreamWriter &output) const noexcept
{
    // Encode the Huffman coder version.
    if (!output.WriteByte(static_cast<byte_type>(s_huffmanVersion)))
    {
        return false;
    }

    // Encode the chunk size.
    if (!output.WriteWord(static_cast<word_type>(s_chunkSizeKB)))
    {
        return false;
    }

    // Encode the maximum Huffman code length.
    if (!output.WriteByte(static_cast<byte_type>(s_maxCodeLength)))
    {
        return false;
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeHeader(
    BitStreamReader &input,
    std::uint32_t &chunkSize,
    length_type &maxCodeLength) const noexcept
{
    // Decode the Huffman coder version.
    byte_type huffmanVersion;

    if (!input.ReadByte(huffmanVersion))
    {
        return false;
    }

    switch (huffmanVersion)
    {
        case 1:
            return decodeHeaderVersion1(input, chunkSize, maxCodeLength);

        default:
            break;
    }

    return false;
}

//==============================================================================
bool HuffmanCoder::decodeHeaderVersion1(
    BitStreamReader &input,
    std::uint32_t &chunkSize,
    length_type &maxCodeLength) const noexcept
{
    // Decode the chunk size.
    word_type encodedChunkSizeKB;
    if (!input.ReadWord(encodedChunkSizeKB))
    {
        return false;
    }

    // Decode the maximum Huffman code length.
    byte_type encodedMaxCodeLength;
    if (!input.ReadByte(encodedMaxCodeLength))
    {
        return false;
    }

    chunkSize = static_cast<std::uint32_t>(encodedChunkSizeKB) << 10;
    maxCodeLength = static_cast<length_type>(encodedMaxCodeLength);

    return true;
}

//==============================================================================
bool HuffmanCoder::encodeCodes(BitStreamWriter &output) const noexcept
{
    // At the least, encode that there were zero Huffman codes of length zero.
    std::vector<std::uint16_t> counts(1);

    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        const HuffmanCode &code = m_huffmanCodes[i];

        if (counts.size() <= code.m_length)
        {
            counts.resize(code.m_length + 1);
        }

        ++counts[code.m_length];
    }

    // Encode the number of code length counts.
    if (!output.WriteByte(static_cast<byte_type>(counts.size())))
    {
        return false;
    }

    // Encode the code length counts.
    for (const std::uint16_t &length : counts)
    {
        if (!output.WriteWord(static_cast<word_type>(length)))
        {
            return false;
        }
    }

    // Encode the symbols.
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        const HuffmanCode &code = m_huffmanCodes[i];

        if (!output.WriteByte(static_cast<byte_type>(code.m_symbol)))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeCodes(
    BitStreamReader &input,
    length_type &maxCodeLength) noexcept
{
    m_huffmanCodesSize = 0;

    // Decode the number of code length counts. This number must be at least 1.
    byte_type countsSize;
    if (!input.ReadByte(countsSize) || (countsSize == 0))
    {
        return false;
    }

    // The first code length is 0, so the actual maximum code length is 1 less
    // than the number of length counts.
    maxCodeLength = countsSize - 1;

    // Decode the code length counts.
    std::vector<std::uint16_t> counts(countsSize);

    for (std::uint16_t &count : counts)
    {
        if (!input.ReadWord(count))
        {
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
                return false;
            }

            // First code is always set to zero.
            code_type code = 0;

            // Subsequent codes are one greater than the previous code, but also
            // bit-shifted left enough to maintain the right code length.
            if (m_huffmanCodesSize != 0)
            {
                const HuffmanCode &last =
                    m_huffmanCodes[m_huffmanCodesSize - 1];

                const length_type shift = length - last.m_length;
                code = (last.m_code + 1) << shift;
            }

            if (m_huffmanCodesSize == m_huffmanCodes.size())
            {
                return false;
            }

            m_huffmanCodes[m_huffmanCodesSize++] =
                HuffmanCode(static_cast<symbol_type>(symbol), code, length);
        }
    }

    convertToPrefixTable(maxCodeLength);
    return true;
}

//==============================================================================
void HuffmanCoder::convertToPrefixTable(length_type maxCodeLength) noexcept
{
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        const HuffmanCode code = std::move(m_huffmanCodes[i]);
        const length_type shift = maxCodeLength - code.m_length;

        for (code_type j = 0; j < (1 << shift); ++j)
        {
            const code_type index = (code.m_code << shift) + j;
            m_prefixTable[index].m_symbol = code.m_symbol;
            m_prefixTable[index].m_length = code.m_length;
        }
    }
}

//==============================================================================
bool HuffmanCoder::encodeSymbols(
    std::uint32_t inputSize,
    BitStreamWriter &output) noexcept
{
    decltype(m_huffmanCodes) symbols;

    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        HuffmanCode code = std::move(m_huffmanCodes[i]);
        symbols[code.m_symbol] = std::move(code);
    }

    for (std::uint32_t i = 0; i < inputSize; ++i)
    {
        const HuffmanCode &code = symbols[m_chunkBuffer[i]];
        const auto length = static_cast<byte_type>(code.m_length);

        if (!output.WriteBits(code.m_code, length))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeSymbols(
    BitStreamReader &input,
    length_type maxCodeLength,
    std::uint32_t chunkSize,
    std::ostream &output) const noexcept
{
    std::uint32_t bytes = 0;
    code_type index;

    while ((bytes < chunkSize) && (input.PeekBits(maxCodeLength, index) != 0))
    {
        const HuffmanCode &code = m_prefixTable[index];

        m_chunkBuffer[bytes++] = code.m_symbol;
        input.DiscardBits(code.m_length);
    }

    if (bytes > 0)
    {
        output.write(
            reinterpret_cast<const std::ios::char_type *>(m_chunkBuffer.get()),
            bytes);
    }

    return (bytes == chunkSize) || input.FullyConsumed();
}

} // namespace fly
