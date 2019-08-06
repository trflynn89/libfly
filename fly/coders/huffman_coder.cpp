#include "fly/coders/huffman_coder.h"

#include "fly/coders/bit_stream.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <stack>

namespace fly {

namespace {

    constexpr const std::uint8_t s_huffmanVersion = 1;

    constexpr const std::uint16_t s_chunkSizeKB = 1 << 10;
    constexpr const std::uint32_t s_chunkSize = s_chunkSizeKB << 10;

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

    auto data = std::make_unique<std::istream::char_type[]>(s_chunkSize);
    std::uint32_t bytes = 0;

    while ((bytes = readStream(input, data.get())) > 0)
    {
        reset();

        if (!createTree(data.get(), bytes))
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
        else if (!encodeSymbols(data.get(), bytes, output))
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

    if (!decodeHeader(input, chunkSize))
    {
        return false;
    }

    auto data = std::make_unique<std::ostream::char_type[]>(chunkSize);

    while (!input.FullyConsumed())
    {
        reset();

        if (!decodeCodes(input))
        {
            return false;
        }
        else if (!decodeSymbols(input, data.get(), chunkSize, output))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
void HuffmanCoder::reset() noexcept
{
    decltype(m_huffmanTree) tree;
    m_huffmanTree.swap(tree);

    m_huffmanCodesSize = 0;
}

//==============================================================================
std::uint32_t HuffmanCoder::readStream(
    std::istream &input,
    std::istream::char_type *data) const noexcept
{
    const std::istream::pos_type start = input.tellg();
    input.seekg(0, std::ios::end);

    const std::istream::pos_type length = input.tellg() - start;
    input.seekg(start, std::ios::beg);

    if (length > 0)
    {
        const auto lengthToRead =
            std::min(static_cast<std::uint32_t>(length), s_chunkSize);
        input.read(data, lengthToRead);

        return static_cast<std::uint32_t>(input.gcount());
    }

    return 0;
}

//==============================================================================
bool HuffmanCoder::createTree(
    const std::istream::char_type *input,
    std::uint32_t inputSize) noexcept
{
    static constexpr auto maxSymbols = std::numeric_limits<symbol_type>::max();

    // Create a frequency map of each input symbol.
    frequency_type counts[maxSymbols] = {0};

    for (std::uint32_t i = 0; i < inputSize; ++i)
    {
        ++counts[static_cast<symbol_type>(input[i])];
    }

    // Create a priority queue of HuffmanNode, sorted such that the least common
    // symbol is always on top.
    HuffmanNodeQueue queue;
    std::uint16_t index = 0;

    auto next_node = [this, &index]() -> HuffmanNode * {
        if (++index < m_huffmanTree.size())
        {
            return &m_huffmanTree[index];
        }

        return nullptr;
    };

    for (symbol_type symbol = 0; symbol < maxSymbols; ++symbol)
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
    }

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
            if ((node->m_left == nullptr) && (node->m_right == nullptr))
            {
                const auto code = static_cast<code_type>(node->m_frequency);
                const auto length = static_cast<code_type>(path.size());

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

        const std::uint8_t shift =
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
    if (!output.WriteByte(static_cast<byte_type>((s_chunkSizeKB >> 8) & 0xff)))
    {
        return false;
    }
    else if (!output.WriteByte(static_cast<byte_type>(s_chunkSizeKB & 0xff)))
    {
        return false;
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeHeader(
    BitStreamReader &input,
    std::uint32_t &chunkSize) const noexcept
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
            return decodeHeaderVersion1(input, chunkSize);

        default:
            break;
    }

    return false;
}

//==============================================================================
bool HuffmanCoder::decodeHeaderVersion1(
    BitStreamReader &input,
    std::uint32_t &chunkSize) const noexcept
{
    // Decode the chunk size.
    byte_type chunkSizeHigh;
    byte_type chunkSizeLow;

    if (!input.ReadByte(chunkSizeHigh) || !input.ReadByte(chunkSizeLow))
    {
        return false;
    }

    chunkSize = (static_cast<std::uint32_t>(chunkSizeHigh << 8)) | chunkSizeLow;
    chunkSize <<= 10;

    return true;
}

//==============================================================================
bool HuffmanCoder::encodeCodes(BitStreamWriter &output) const noexcept
{
    // At the least, encode that there were zero Huffman codes of length zero.
    std::vector<code_type> counts(1);

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
    const auto countsSize = static_cast<byte_type>(counts.size());
    if (!output.WriteByte(countsSize))
    {
        return false;
    }

    // Encode the code length counts.
    for (const code_type &length : counts)
    {
        if (!output.WriteByte(static_cast<byte_type>(length)))
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
bool HuffmanCoder::decodeCodes(BitStreamReader &input) noexcept
{
    // Decode the number of code length counts. This number must be at least 1.
    byte_type countsSize;
    if (!input.ReadByte(countsSize) || (countsSize == 0))
    {
        return false;
    }

    // Decode the code length counts.
    std::vector<code_type> counts(countsSize);

    for (code_type i = 0; i < countsSize; ++i)
    {
        byte_type length;
        if (!input.ReadByte(length))
        {
            return false;
        }

        counts[i] = static_cast<code_type>(length);
    }

    // Decode the symbols.
    for (std::uint8_t length = 0; length < countsSize; ++length)
    {
        for (code_type i = 0; i < counts[length]; ++i)
        {
            byte_type symbol;
            if (!input.ReadByte(symbol))
            {
                return false;
            }

            // First code is always set to zero.
            code_type code = 0;

            // Subsequent codes are one greater than the previous code, but also
            // bit-shifted left enough times to maintain the right code length.
            if (m_huffmanCodesSize != 0)
            {
                const HuffmanCode &last =
                    m_huffmanCodes[m_huffmanCodesSize - 1];
                code = last.m_code + 1;

                const std::uint8_t shift = length - last.m_length;
                code <<= shift;
            }

            if (m_huffmanCodesSize == m_huffmanCodes.size())
            {
                return false;
            }

            m_huffmanCodes[m_huffmanCodesSize++] = HuffmanCode(
                static_cast<symbol_type>(symbol),
                code,
                static_cast<code_type>(length));
        }
    }

    convertToPrefixTable(countsSize - 1);
    return true;
}

//==============================================================================
void HuffmanCoder::convertToPrefixTable(std::uint8_t maxCodeLength) noexcept
{
    decltype(m_huffmanCodes) codes;

    for (std::uint16_t i = 0; i < m_huffmanCodesSize; ++i)
    {
        HuffmanCode code = std::move(m_huffmanCodes[i]);
        std::uint8_t shift = maxCodeLength - code.m_length;

        for (std::uint16_t j = 0; j < (1 << shift); ++j)
        {
            std::uint16_t index = (code.m_code << shift) + j;
            codes[index].m_symbol = code.m_symbol;
            codes[index].m_length = code.m_length;
        }
    }

    m_huffmanCodesSize = maxCodeLength;
    m_huffmanCodes = std::move(codes);
}

//==============================================================================
bool HuffmanCoder::encodeSymbols(
    const std::istream::char_type *input,
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
        const auto &huffmanCode = symbols[static_cast<symbol_type>(input[i])];
        const auto code = static_cast<byte_type>(huffmanCode.m_code);
        const auto length = static_cast<byte_type>(huffmanCode.m_length);

        if (!output.WriteBits(code, length))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeSymbols(
    BitStreamReader &input,
    std::ostream::char_type *data,
    std::uint32_t chunkSize,
    std::ostream &output) const noexcept
{
    const byte_type maxCodeLength = static_cast<byte_type>(m_huffmanCodesSize);
    std::uint32_t bytes = 0;
    byte_type index;

    while ((bytes < chunkSize) && input.PeekBits(maxCodeLength, index))
    {
        const HuffmanCode &code = m_huffmanCodes[index];

        data[bytes++] = static_cast<std::ostream::char_type>(code.m_symbol);
        input.DiscardBits(code.m_length);
    }

    if (bytes > 0)
    {
        output.write(data, bytes);
    }

    return (bytes == chunkSize) || input.FullyConsumed();
}

} // namespace fly
