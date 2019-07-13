#include "fly/coders/huffman_coder.h"

#include "fly/coders/bit_stream.h"
#include "fly/coders/huffman_types.h"

#include <algorithm>
#include <limits>
#include <stack>

namespace fly {

namespace {

    /**
     * Create and insert a new Huffman code into a list of already sorted codes.
     * The codes are sorted first by code length, then by code value.
     *
     * @tparam Args Variadic template arguments to construct a Huffman code.
     *
     * @param vector List of sorted Huffman codes to insert a new code into.
     * @param Args Variadic list of arguments to construct a Huffman code with.
     */
    template <typename... Args>
    void SortedInsert(std::vector<HuffmanCode> &codes, Args &&... args)
    {
        const HuffmanCode code(args...);

        const auto it = std::upper_bound(
            codes.begin(),
            codes.end(),
            code,
            [](const HuffmanCode &a, const HuffmanCode &b) {
                if (a.m_length == b.m_length)
                {
                    return a.m_symbol < b.m_symbol;
                }

                return a.m_length < b.m_length;
            });

        codes.insert(it, code);
    }

    constexpr const std::uint8_t s_huffmanVersion = 1;

    constexpr const symbol_type s_maxSymbols =
        std::numeric_limits<symbol_type>::max();

    constexpr const std::uint8_t s_huffmanTreeSize = 128;

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

    std::istream::char_type data[s_chunkSize];
    std::uint32_t bytes = 0;

    while ((bytes = readStream(input, data)) > 0)
    {
        HuffmanNode tree[s_huffmanTreeSize];
        std::vector<HuffmanCode> codes;

        if (!createTree(data, bytes, tree))
        {
            return false;
        }

        codes = std::move(createCodes(tree));
        convertToCanonicalForm(codes);

        if (!encodeCodes(codes, output))
        {
            return false;
        }
        else if (!encodeSymbols(data, bytes, codes, output))
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

    while (!input.FullyConsumed())
    {
        HuffmanNode tree[s_huffmanTreeSize];
        std::vector<HuffmanCode> codes;

        if (!decodeCodes(input, codes))
        {
            return false;
        }
        else if (!createTree(codes, tree))
        {
            return false;
        }
        else if (!decodeSymbols(input, chunkSize, tree, output))
        {
            return false;
        }
    }

    return true;
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

    auto lengthToRead =
        std::min(static_cast<std::uint32_t>(length), s_chunkSize);

    if (lengthToRead > 0)
    {
        input.read(data, lengthToRead);
        return input.gcount();
    }

    return 0;
}

//==============================================================================
bool HuffmanCoder::createTree(
    const std::istream::char_type *input,
    std::uint32_t inputSize,
    HuffmanNode *root) const noexcept
{
    // Create a frequency map of each input symbol.
    frequency_type counts[s_maxSymbols] = {0};

    for (std::uint32_t i = 0; i < inputSize; ++i)
    {
        ++counts[static_cast<symbol_type>(input[i])];
    }

    // Create a priority queue of HuffmanNode, sorted such that the least common
    // symbol is always on top.
    HuffmanNodeQueue queue;
    std::uint8_t index = 0;

    auto next_node = [&root, &index]() -> HuffmanNode * {
        if (++index < s_huffmanTreeSize)
        {
            return &root[index];
        }

        return nullptr;
    };

    for (symbol_type symbol = 0; symbol < s_maxSymbols; ++symbol)
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
    *root = *node;

    return true;
}

//==============================================================================
bool HuffmanCoder::createTree(
    const std::vector<HuffmanCode> &codes,
    HuffmanNode *root) const noexcept
{
    std::uint8_t index = 0;

    auto next_node = [&root, &index]() -> HuffmanNode * {
        if (++index < s_huffmanTreeSize)
        {
            return &root[index];
        }

        return nullptr;
    };

    for (const HuffmanCode &code : codes)
    {
        HuffmanNode *node = root;

        // Follow the path defined by the Huffman code, creating intermediate
        // nodes along the way as needed.
        for (std::uint8_t shift = code.m_length; shift != 0; --shift)
        {
            // Convert the current node to an intermediate node.
            if ((node->m_left == nullptr) || (node->m_right == nullptr))
            {
                node->BecomeIntermediate(next_node(), next_node());

                if ((node->m_left == nullptr) || (node->m_right == nullptr))
                {
                    return false;
                }
            }

            const bool right = (code.m_code >> (shift - 1)) & 0x1;
            node = right ? node->m_right : node->m_left;
        }

        // Store the symbol at the end of the path
        node->BecomeSymbol(code.m_symbol, 0);
    }

    return true;
}

//==============================================================================
std::vector<HuffmanCode> HuffmanCoder::createCodes(HuffmanNode *root) const
    noexcept
{
    std::vector<HuffmanCode> codes;

    std::stack<const HuffmanNode *> pending;
    std::stack<const HuffmanNode *> path;

    // Symbol frequency is no longer needed. Use that field to form codes.
    root->m_frequency = 0;

    const HuffmanNode *node = nullptr;
    pending.push(root);

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
                SortedInsert(codes, node->m_symbol, code, length);
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

    return codes;
}

//==============================================================================
void HuffmanCoder::convertToCanonicalForm(std::vector<HuffmanCode> &codes) const
    noexcept
{
    // First code is always set to zero. Its length does not change.
    codes[0].m_code = 0;

    if (codes.size() == 1)
    {
        // Single-node Huffman trees occur when the input stream contains only
        // one unique symbol. Set its length to one so a single bit is encoded
        // for each occurrence of that symbol.
        codes[0].m_length = 1;
    }

    for (std::uint8_t i = 1; i < codes.size(); ++i)
    {
        // Subsequent codes are one greater than the previous code, but also
        // bit-shifted left enough times to maintain the same code length.
        codes[i].m_code = codes[i - 1].m_code + 1;

        const std::uint8_t shift = codes[i].m_length - codes[i - 1].m_length;
        codes[i].m_code <<= shift;
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
bool HuffmanCoder::encodeCodes(
    const std::vector<HuffmanCode> &codes,
    BitStreamWriter &output) const noexcept
{
    // At the least, encode that there were zero Huffman codes of length zero.
    std::vector<code_type> counts(1);

    for (const HuffmanCode &code : codes)
    {
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
    for (const HuffmanCode &code : codes)
    {
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
    std::vector<HuffmanCode> &codes) const noexcept
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
            if (!codes.empty())
            {
                const HuffmanCode &last = codes.back();
                code = last.m_code + 1;

                const std::uint8_t shift = length - last.m_length;
                code <<= shift;
            }

            codes.emplace_back(
                static_cast<symbol_type>(symbol),
                code,
                static_cast<code_type>(length));
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::encodeSymbols(
    const std::istream::char_type *input,
    std::uint32_t inputSize,
    std::vector<HuffmanCode> &codes,
    BitStreamWriter &output) const noexcept
{
    HuffmanCode symbols[s_maxSymbols];

    for (HuffmanCode &code : codes)
    {
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
    std::uint32_t chunkSize,
    const HuffmanNode *root,
    std::ostream &output) const noexcept
{
    std::ostream::char_type data[chunkSize];
    std::uint32_t bytes = 0;

    const HuffmanNode *node = root;
    bool right;

    while (input.ReadBit(right))
    {
        node = right ? node->m_right : node->m_left;

        if (!node->m_left)
        {
            data[bytes] = static_cast<std::ostream::char_type>(node->m_symbol);

            if (++bytes == chunkSize)
            {
                break;
            }

            node = root;
        }
    }

    if (bytes > 0)
    {
        output.write(data, bytes);
    }

    return (bytes == chunkSize) || input.FullyConsumed();
}

} // namespace fly
