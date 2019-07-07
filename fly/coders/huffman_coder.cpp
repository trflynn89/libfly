#include "fly/coders/huffman_coder.h"

#include "fly/coders/bit_stream.h"
#include "fly/coders/huffman_types.h"

#include <algorithm>
#include <limits>
#include <queue>
#include <stack>

namespace fly {

namespace {

    /**
     * Comparator for HuffmanNode for use in HuffmanNodeQueue.
     */
    struct HuffmanNodeComparator
    {
        bool operator()(
            const std::unique_ptr<HuffmanNode> &left,
            const std::unique_ptr<HuffmanNode> &right)
        {
            // Reverse std::less so the least frequent symbol is on top.
            return left->m_frequency > right->m_frequency;
        }
    };

    /**
     * Implementation of a std::priority_queue for HuffmanNode. This is needed
     * because std::priority_queue::top() returns a const reference to the top
     * value, which prevents using a std::unique_ptr as the value type (because
     * the implication is that the value cannot be moved). To avoid this
     * constraint, provides a method to internally pop-and-move the top value.
     */
    class HuffmanNodeQueue :
        public std::priority_queue<
            std::unique_ptr<HuffmanNode>,
            std::vector<std::unique_ptr<HuffmanNode>>,
            HuffmanNodeComparator>
    {
    public:
        value_type PopTop()
        {
            std::pop_heap(this->c.begin(), this->c.end(), this->comp);

            value_type top = std::move(this->c.back());
            this->c.pop_back();

            return top;
        }
    };

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

} // namespace

//==============================================================================
bool HuffmanCoder::EncodeInternal(
    std::istream &input,
    BitStreamWriter &output) noexcept
{
    const stream_buffer_type data = readStream(input);
    std::vector<HuffmanCode> codes;
    {
        const std::unique_ptr<HuffmanNode> root = createTree(data);

        codes = std::move(createCodes(root));
        convertToCanonicalForm(codes);
    }

    return encodeCodes(codes, output) && encodeStream(codes, data, output);
}

//==============================================================================
bool HuffmanCoder::DecodeInternal(
    BitStreamReader &input,
    std::ostream &output) noexcept
{
    std::vector<HuffmanCode> codes;

    if (decodeCodes(input, codes))
    {
        const std::unique_ptr<HuffmanNode> root = createTree(codes);
        return decodeStream(root, input, output);
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
std::unique_ptr<HuffmanNode>
HuffmanCoder::createTree(const stream_buffer_type &input) const noexcept
{
    if (input.empty())
    {
        return nullptr;
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

    for (symbol_type symbol = 0; symbol < s_maxSymbols; ++symbol)
    {
        if (counts[symbol] == 0)
        {
            continue;
        }

        auto node = std::make_unique<HuffmanNode>(symbol, counts[symbol]);
        queue.push(std::move(node));
    }

    // Convert the priority queue to a Huffman tree. Remove the two least common
    // symbols, combining their frequencies into a new node, and insert the new
    // node back into the priority queue. Continue until only the root remains.
    while (queue.size() > 1)
    {
        auto left = std::move(queue.PopTop());
        auto right = std::move(queue.PopTop());

        auto node =
            std::make_unique<HuffmanNode>(std::move(left), std::move(right));
        queue.push(std::move(node));
    }

    return std::move(queue.PopTop());
}

//==============================================================================
std::unique_ptr<HuffmanNode>
HuffmanCoder::createTree(const std::vector<HuffmanCode> &codes) const noexcept
{
    auto root = std::make_unique<HuffmanNode>();

    for (const HuffmanCode &code : codes)
    {
        auto *node = root.get();

        // Follow the path defined by the Huffman code, creating intermediate
        // nodes along the way as needed.
        for (code_type shift = code.m_length; shift != 0; --shift)
        {
            if (node->IsSymbol())
            {
                // Convert the current node to an intermediate node
                node->m_left = std::make_unique<HuffmanNode>();
                node->m_right = std::make_unique<HuffmanNode>();
            }

            const bool left = (((code.m_code >> (shift - 1)) & 0x1) == 0);
            node = left ? node->m_left.get() : node->m_right.get();
        }

        // Store the symbol at the end of the path
        node->m_symbol = code.m_symbol;
    }

    return root;
}

//==============================================================================
std::vector<HuffmanCode>
HuffmanCoder::createCodes(const std::unique_ptr<HuffmanNode> &root) const
    noexcept
{
    std::vector<HuffmanCode> codes;

    std::stack<HuffmanNode *> pending;
    std::stack<HuffmanNode *> path;

    // Symbol frequency is no longer needed, so use that field to form codes. Of
    // course, this is only valid if the frequency storage is large enough.
    static_assert(sizeof(frequency_type) >= sizeof(code_type));
    HuffmanNode *node = root.get();

    if (node != nullptr)
    {
        node->m_frequency = 0;
        pending.push(node);
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
            if (node->IsSymbol())
            {
                const auto code = static_cast<code_type>(node->m_frequency);
                const auto length = static_cast<code_type>(path.size());
                SortedInsert(codes, node->m_symbol, code, length);
            }
            else
            {
                node->m_left->m_frequency = node->m_frequency << 1;
                pending.push(node->m_left.get());

                node->m_right->m_frequency = (node->m_frequency << 1) + 1;
                pending.push(node->m_right.get());
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
bool HuffmanCoder::encodeStream(
    std::vector<HuffmanCode> &codes,
    const stream_buffer_type &input,
    BitStreamWriter &output) const noexcept
{
    // Convert the list of Huffman codes to a map keyed by symbol.
    HuffmanCode symbols[s_maxSymbols];

    for (HuffmanCode &code : codes)
    {
        symbols[code.m_symbol] = std::move(code);
    }

    for (const stream_buffer_type::value_type &ch : input)
    {
        const HuffmanCode &code = symbols[static_cast<symbol_type>(ch)];

        if (code.m_length == 0)
        {
            // Single-node Huffman trees occur when the input stream contains
            // only one unique symbol. Write out a single bit for that case.
            if (!output.WriteBit(0))
            {
                return false;
            }
        }
        else
        {
            for (code_type shift = code.m_length; shift != 0; --shift)
            {
                const bool bit = (((code.m_code >> (shift - 1)) & 0x1) == 1);

                if (!output.WriteBit(bit))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

//==============================================================================
bool HuffmanCoder::decodeStream(
    const std::unique_ptr<HuffmanNode> &root,
    BitStreamReader &input,
    std::ostream &output) const noexcept
{
    static constexpr const std::size_t bufferSize = 1 << 20;

    HuffmanNode *node = root.get();
    bool right;

    stream_buffer_type::value_type data[bufferSize];
    std::size_t bytes = 0;

    while ((node != nullptr) && input.ReadBit(right))
    {
        if (node->m_right && node->m_left)
        {
            node = (right ? node->m_right.get() : node->m_left.get());
        }

        if (!node->m_left || !node->m_right)
        {
            data[bytes] =
                static_cast<stream_buffer_type::value_type>(node->m_symbol);

            if (++bytes == bufferSize)
            {
                output.write(data, bytes);
                bytes = 0;
            }

            node = root.get();
        }
    }

    if (bytes > 0)
    {
        output.write(data, bytes);
    }

    return input.ReachedEndOfFile();
}

} // namespace fly
