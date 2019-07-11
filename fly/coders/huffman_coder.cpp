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

    constexpr const symbol_type s_maxSymbols =
        std::numeric_limits<symbol_type>::max();

    constexpr const symbol_type s_huffmanTreeSize = 128;

    constexpr const std::size_t s_chunkSize = 1 << 20;

} // namespace

//==============================================================================
bool HuffmanCoder::EncodeInternal(
    std::istream &input,
    BitStreamWriter &output) noexcept
{
    std::istream::char_type data[s_chunkSize];
    std::size_t bytes = 0;

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
    std::vector<HuffmanCode> codes;

    while (!input.ReachedEndOfFile())
    {
        HuffmanNode tree[s_huffmanTreeSize];

        if (!decodeCodes(input, codes))
        {
            return false;
        }
        else if (!createTree(codes, tree))
        {
            return false;
        }
        else if (!decodeSymbols(tree, input, output))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
std::size_t HuffmanCoder::readStream(
    std::istream &input,
    std::istream::char_type *data) const noexcept
{
    const std::istream::pos_type start = input.tellg();
    input.seekg(0, std::ios::end);

    const std::istream::pos_type length = input.tellg() - start;
    input.seekg(start, std::ios::beg);

    auto lengthToRead = std::min(static_cast<std::size_t>(length), s_chunkSize);

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
    std::size_t inputSize,
    HuffmanNode *root) const noexcept
{
    // Create a frequency map of each input symbol.
    frequency_type counts[s_maxSymbols] = {0};

    for (std::size_t i = 0; i < inputSize; ++i)
    {
        ++counts[static_cast<symbol_type>(input[i])];
    }

    // Create a priority queue of HuffmanNode, sorted such that the least common
    // symbol is always on top.
    HuffmanNodeQueue queue;
    std::size_t index = 0;

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
    code_type index = 0;

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
        for (code_type shift = code.m_length; shift != 0; --shift)
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

    for (std::size_t i = 1; i < codes.size(); ++i)
    {
        // Subsequent codes are one greater than the previous code, but also
        // bit-shifted left enough times to maintain the same code length.
        codes[i].m_code = codes[i - 1].m_code + 1;

        const std::size_t shift = codes[i].m_length - codes[i - 1].m_length;
        codes[i].m_code <<= shift;
    }
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
        if (!output.WriteByte(length))
        {
            return false;
        }
    }

    // Encode the symbols.
    for (const HuffmanCode &code : codes)
    {
        if (!output.WriteByte(code.m_symbol))
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
    code_type countsSize;
    if (!input.ReadByte(countsSize) || (countsSize == 0))
    {
        return false;
    }

    // Decode the code length counts.
    std::vector<code_type> counts(countsSize);

    for (code_type i = 0; i < countsSize; ++i)
    {
        code_type length;
        if (!input.ReadByte(length))
        {
            return false;
        }

        counts[i] = length;
    }

    // Decode the symbols.
    for (code_type length = 0; length < countsSize; ++length)
    {
        for (code_type i = 0; i < counts[length]; ++i)
        {
            code_type symbol;
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

                const std::size_t shift = length - last.m_length;
                code <<= shift;
            }

            codes.emplace_back(static_cast<symbol_type>(symbol), code, length);
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::encodeSymbols(
    const std::istream::char_type *input,
    std::size_t inputSize,
    std::vector<HuffmanCode> &codes,
    BitStreamWriter &output) const noexcept
{
    HuffmanCode symbols[s_maxSymbols];

    for (HuffmanCode &code : codes)
    {
        symbols[code.m_symbol] = std::move(code);
    }

    for (std::size_t i = 0; i < inputSize; ++i)
    {
        const HuffmanCode &code = symbols[static_cast<symbol_type>(input[i])];

        if (!output.WriteBits(code.m_code, code.m_length))
        {
            return false;
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeSymbols(
    const HuffmanNode *root,
    BitStreamReader &input,
    std::ostream &output) const noexcept
{
    const HuffmanNode *node = root;
    bool right;

    std::ostream::char_type data[s_chunkSize];
    std::size_t bytes = 0;

    while (input.ReadBit(right))
    {
        node = right ? node->m_right : node->m_left;

        if (!node->m_left)
        {
            data[bytes] = static_cast<std::ostream::char_type>(node->m_symbol);

            if (++bytes == s_chunkSize)
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

    return (bytes == s_chunkSize) || input.ReachedEndOfFile();
}

} // namespace fly
