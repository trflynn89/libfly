#include "fly/coders/huffman/huffman_encoder.h"

#include "fly/coders/bit_stream.h"
#include "fly/coders/huffman/huffman_config.h"
#include "fly/literals.h"
#include "fly/logger/logger.h"

#include <algorithm>
#include <limits>
#include <stack>
#include <vector>

namespace fly {

namespace {

    constexpr const std::uint8_t s_huffmanVersion = 1;

} // namespace

//==============================================================================
HuffmanEncoder::HuffmanEncoder(
    const std::shared_ptr<HuffmanConfig> &spConfig) noexcept :
    m_chunkSizeKB(spConfig->EncoderChunkSizeKB()),
    m_maxCodeLength(spConfig->EncoderMaxCodeLength())
{
}

//==============================================================================
bool HuffmanEncoder::EncodeInternal(
    std::istream &input,
    BitStreamWriter &output) noexcept
{
    if (m_maxCodeLength >= std::numeric_limits<code_type>::digits)
    {
        LOGW(
            "Maximum Huffman code length %u is too large for code_type",
            static_cast<std::uint32_t>(m_maxCodeLength));
        return false;
    }

    encodeHeader(output);

    m_chunkBuffer = std::make_unique<symbol_type[]>(m_chunkSizeKB << 10);
    std::uint32_t chunkSize = 0;

    while ((chunkSize = readStream(input)) > 0)
    {
        createTree(chunkSize);
        createCodes();

        encodeCodes(output);
        encodeSymbols(chunkSize, output);
    }

    return output.Finish();
}

//==============================================================================
std::uint32_t HuffmanEncoder::readStream(std::istream &input) const noexcept
{
    const std::ios::pos_type start = input.tellg();
    input.seekg(0, std::ios::end);

    const std::ios::pos_type length = input.tellg() - start;
    input.seekg(start, std::ios::beg);

    std::uint32_t bytesRead = 0;

    if (length > 0)
    {
        input.read(
            reinterpret_cast<std::ios::char_type *>(m_chunkBuffer.get()),
            std::min(
                static_cast<std::streamsize>(length),
                static_cast<std::streamsize>(m_chunkSizeKB << 10)));

        bytesRead = static_cast<std::uint32_t>(input.gcount());
    }

    return bytesRead;
}

//==============================================================================
void HuffmanEncoder::createTree(std::uint32_t chunkSize) noexcept
{
    // Lambda to retrieve the next available HuffmanNode
    std::uint16_t index = 0;

    // Create a frequency map of each input symbol.
    std::array<frequency_type, 1 << 8> counts {};

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
        if (counts[symbol] > 0)
        {
            HuffmanNode *node = &m_huffmanTree[++index];
            node->BecomeSymbol(symbol, counts[symbol]);
            queue.push(node);
        }
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

        HuffmanNode *node = &m_huffmanTree[++index];
        node->BecomeIntermediate(left, right);
        queue.push(node);
    }

    HuffmanNode *node = queue.top();
    m_huffmanTree[0] = std::move(*node);
}

//==============================================================================
void HuffmanEncoder::createCodes() noexcept
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

                insertCode(HuffmanCode(node->m_symbol, code, length));
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

    if (maxCodeLength > m_maxCodeLength)
    {
        limitCodeLengths();
    }

    convertToCanonicalForm();
}

//==============================================================================
void HuffmanEncoder::insertCode(HuffmanCode &&code) noexcept
{
    std::uint16_t pos = m_huffmanCodesSize++;

    for (; (pos > 0) && (code < m_huffmanCodes[pos - 1]); --pos)
    {
        m_huffmanCodes[pos] = std::move(m_huffmanCodes[pos - 1]);
    }

    m_huffmanCodes[pos] = std::move(code);
}

//==============================================================================
void HuffmanEncoder::limitCodeLengths() noexcept
{
    auto computeKraft = [this](const HuffmanCode &code) -> code_type {
        return 1_u16 << (m_maxCodeLength - code.m_length);
    };

    const code_type maxAllowedKraft = (1_u16 << m_maxCodeLength) - 1;
    code_type kraft = 0;

    // Limit all Huffman codes to not be larger than the maximum code length.
    // Compute the Kraft number, which will no longer satisfy the Kraft–McMillan
    // inequality.
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        HuffmanCode &code = m_huffmanCodes[i];

        code.m_length = std::min(code.m_length, m_maxCodeLength);
        kraft += computeKraft(code);
    }

    // The code lengths must now be corrected to satisfy the Kraft–McMillan
    // inequality. Starting from the largest code, increase the code lengths
    // until the inequality is satisfied again.
    for (std::uint16_t i = m_huffmanCodesSize;
         (i-- > 0) && (kraft > maxAllowedKraft);)
    {
        HuffmanCode &code = m_huffmanCodes[i];

        while (code.m_length < m_maxCodeLength)
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
void HuffmanEncoder::convertToCanonicalForm() noexcept
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
        const HuffmanCode &previous = m_huffmanCodes[i - 1];
        HuffmanCode &code = m_huffmanCodes[i];

        // Subsequent codes are one greater than the previous code, but also
        // bit-shifted left enough times to maintain the same code length.
        code.m_code = previous.m_code + 1;
        code.m_code <<= code.m_length - previous.m_length;
    }
}

//==============================================================================
void HuffmanEncoder::encodeHeader(BitStreamWriter &output) const noexcept
{
    // Encode the Huffman coder version.
    output.WriteByte(static_cast<byte_type>(s_huffmanVersion));

    // Encode the chunk size.
    output.WriteWord(static_cast<word_type>(m_chunkSizeKB));

    // Encode the maximum Huffman code length.
    output.WriteByte(static_cast<byte_type>(m_maxCodeLength));
}

//==============================================================================
void HuffmanEncoder::encodeCodes(BitStreamWriter &output) const noexcept
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
    output.WriteByte(static_cast<byte_type>(counts.size()));

    // Encode the code length counts.
    for (const std::uint16_t &length : counts)
    {
        output.WriteWord(static_cast<word_type>(length));
    }

    // Encode the symbols.
    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        const HuffmanCode &code = m_huffmanCodes[i];
        output.WriteByte(static_cast<byte_type>(code.m_symbol));
    }
}

//==============================================================================
void HuffmanEncoder::encodeSymbols(
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

        output.WriteBits(code.m_code, length);
    }
}

} // namespace fly
