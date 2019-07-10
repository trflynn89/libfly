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

} // namespace

//==============================================================================
bool HuffmanCoder::EncodeInternal(
    std::istream &input,
    BitStreamWriter &output) noexcept
{
    const stream_buffer_type data = readStream(input);
    HuffmanNode tree[s_huffmanTreeSize];

    if (createTree(data, tree))
    {
        std::vector<HuffmanCode> codes;

        codes = std::move(createCodes(tree));
        convertToCanonicalForm(codes);

        if (encodeCodes(codes, output))
        {
            return encodeSymbols(codes, data, output);
        }
    }

    return false;
}

//==============================================================================
bool HuffmanCoder::DecodeInternal(
    BitStreamReader &input,
    std::ostream &output) noexcept
{
    std::vector<HuffmanCode> codes;

    if (decodeCodes(input, codes))
    {
        HuffmanNode tree[s_huffmanTreeSize];

        if (createTree(codes, tree))
        {
            return decodeSymbols(tree, input, output);
        }
    }

    return false;
}

//==============================================================================
HuffmanCoder::stream_buffer_type
HuffmanCoder::readStream(std::istream &input) const noexcept
{
    input.seekg(0, std::ios::end);
    const std::istream::pos_type length = input.tellg();
    input.seekg(0, std::ios::beg);

    stream_buffer_type data;

    if (length > 0)
    {
        data.resize(static_cast<stream_buffer_type::size_type>(length));
        input.read(data.data(), length);
    }

    return data;
}

//==============================================================================
bool HuffmanCoder::createTree(
    const stream_buffer_type &input,
    HuffmanNode *root) const noexcept
{
    if (input.empty())
    {
        return false;
    }

    // Create a frequency map of each input symbol.
    frequency_type counts[s_maxSymbols] = {0};

    for (const stream_buffer_type::value_type &ch : input)
    {
        ++counts[static_cast<symbol_type>(ch)];
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

        node->m_symbol = symbol;
        node->m_frequency = counts[symbol];

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

        node->m_frequency = left->m_frequency + right->m_frequency;
        node->m_left = left;
        node->m_right = right;

        queue.push(node);
    }

    HuffmanNode *node = queue.top();
    root->m_symbol = node->m_symbol;
    root->m_frequency = node->m_frequency;
    root->m_left = node->m_left;
    root->m_right = node->m_right;

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
        // entries along the way as needed.
        for (code_type shift = code.m_length; shift != 0; --shift)
        {
            if ((node->m_left == nullptr) || (node->m_right == nullptr))
            {
                // Convert the current node to an intermediate node.
                node->m_left = next_node();
                node->m_right = next_node();

                if ((node->m_left == nullptr) || (node->m_right == nullptr))
                {
                    return false;
                }
            }

            const bool left = (((code.m_code >> (shift - 1)) & 0x1) == 0);
            node = left ? node->m_left : node->m_right;
        }

        // Store the symbol at the end of the path
        node->m_symbol = code.m_symbol;
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
    const HuffmanNode *node = nullptr;

    if (root != nullptr)
    {
        // Symbol frequency is no longer needed. Use that field to form codes.
        root->m_frequency = 0;
        pending.push(root);
    }

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
    if (!codes.empty())
    {
        // First code is always set to zero. Its length does not change.
        codes[0].m_code = 0;

        if (codes.size() == 1)
        {
            // Single-node Huffman trees occur when the input stream contains
            // only one unique symbol. Set its length to one so a single bit is
            // encoded for each occurrence of that symbol.
            codes[0].m_length = 1;
        }
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
    std::vector<HuffmanCode> &codes,
    const stream_buffer_type &input,
    BitStreamWriter &output) const noexcept
{
    HuffmanCode symbols[s_maxSymbols];

    for (HuffmanCode &code : codes)
    {
        symbols[code.m_symbol] = std::move(code);
    }

    for (const stream_buffer_type::value_type &ch : input)
    {
        const HuffmanCode &code = symbols[static_cast<symbol_type>(ch)];

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
    static constexpr const std::size_t bufferSize = 1 << 20;

    const HuffmanNode *node = root;
    bool right;

    stream_buffer_type::value_type data[bufferSize];
    std::size_t bytes = 0;

    while (input.ReadBit(right))
    {
        node = right ? node->m_right : node->m_left;

        if (!node->m_left)
        {
            data[bytes] =
                static_cast<stream_buffer_type::value_type>(node->m_symbol);

            if (++bytes == bufferSize)
            {
                output.write(data, bytes);
                bytes = 0;
            }

            node = root;
        }
    }

    if (bytes > 0)
    {
        output.write(data, bytes);
    }

    return input.ReachedEndOfFile();
}

} // namespace fly
