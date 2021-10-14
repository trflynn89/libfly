#include "fly/coders/huffman/huffman_encoder.hpp"

#include "fly/coders/coder_config.hpp"
#include "fly/logger/logger.hpp"
#include "fly/types/bit_stream/bit_stream_writer.hpp"
#include "fly/types/numeric/literals.hpp"

#include <algorithm>
#include <limits>
#include <stack>
#include <vector>

using namespace fly::literals::numeric_literals;

namespace fly::coders {

namespace {

    constexpr const std::uint8_t s_huffman_version = 1;

} // namespace

//==================================================================================================
HuffmanEncoder::HuffmanEncoder(const std::shared_ptr<CoderConfig> &config) noexcept :
    m_chunk_size(config->huffman_encoder_chunk_size()),
    m_max_code_length(config->huffman_encoder_max_code_length()),
    m_huffman_codes_size(0)
{
}

//==================================================================================================
bool HuffmanEncoder::encode_binary(std::istream &decoded, fly::BitStreamWriter &encoded)
{
    if (m_max_code_length >= std::numeric_limits<code_type>::digits)
    {
        LOGW("Maximum Huffman code length {} is too large for code_type", m_max_code_length);
        return false;
    }

    encode_header(encoded);

    m_chunk_buffer = std::make_unique<symbol_type[]>(m_chunk_size);
    std::uint32_t chunk_size = 0;

    while ((chunk_size = read_stream(decoded)) > 0)
    {
        create_tree(chunk_size);
        create_codes();

        encode_codes(encoded);
        encode_symbols(chunk_size, encoded);
    }

    return encoded.finish();
}

//==================================================================================================
std::uint32_t HuffmanEncoder::read_stream(std::istream &decoded) const
{
    decoded.read(
        reinterpret_cast<std::ios::char_type *>(m_chunk_buffer.get()),
        static_cast<std::streamsize>(m_chunk_size));

    return static_cast<std::uint32_t>(decoded.gcount());
}

//==================================================================================================
void HuffmanEncoder::create_tree(std::uint32_t chunk_size)
{
    std::uint16_t index = 0;

    // Create a frequency map of each input symbol.
    std::array<frequency_type, 1 << 8> counts {};

    for (std::uint32_t i = 0; i < chunk_size; ++i)
    {
        ++counts[m_chunk_buffer[i]];
    }

    // Create a priority queue of HuffmanNode, sorted such that the least common symbol is on top.
    HuffmanNodeQueue queue;
    symbol_type symbol = 0;

    do
    {
        if (counts[symbol] > 0)
        {
            HuffmanNode *node = &m_huffman_tree[++index];
            node->become_symbol(symbol, counts[symbol]);
            queue.push(node);
        }
    } while (++symbol != 0);

    // Convert the priority queue to a Huffman tree. Remove the two least common symbols, combining
    // their frequencies into a new node, and insert the new node back into the priority queue.
    // Continue until only the root remains.
    while (queue.size() > 1)
    {
        auto *left = queue.top();
        queue.pop();

        auto *right = queue.top();
        queue.pop();

        HuffmanNode *node = &m_huffman_tree[++index];
        node->become_intermediate(left, right);
        queue.push(node);
    }

    HuffmanNode *node = queue.top();
    m_huffman_tree[0] = std::move(*node);
}

//==================================================================================================
void HuffmanEncoder::create_codes()
{
    std::stack<const HuffmanNode *> pending;
    std::stack<const HuffmanNode *> path;

    length_type max_code_length = 0;
    m_huffman_codes_size = 0;

    // Symbol frequency is no longer needed. Use that field to form codes.
    m_huffman_tree[0].m_frequency = 0;

    const HuffmanNode *node = nullptr;
    pending.push(&m_huffman_tree[0]);

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
                max_code_length = std::max(max_code_length, length);

                insert_code(HuffmanCode(node->m_symbol, code, length));
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

    if (max_code_length > m_max_code_length)
    {
        limit_code_lengths();
    }

    convert_to_canonical_form();
}

//==================================================================================================
void HuffmanEncoder::insert_code(HuffmanCode &&code)
{
    std::uint16_t pos = m_huffman_codes_size++;

    for (; (pos > 0) && (code < m_huffman_codes[pos - 1_u16]); --pos)
    {
        m_huffman_codes[pos] = std::move(m_huffman_codes[pos - 1_u16]);
    }

    m_huffman_codes[pos] = std::move(code);
}

//==================================================================================================
void HuffmanEncoder::limit_code_lengths()
{
    auto compute_kraft = [this](const HuffmanCode &code) -> code_type {
        return 1_u16 << (m_max_code_length - code.m_length);
    };

    const code_type max_allowed_kraft = (1_u16 << m_max_code_length) - 1;
    code_type kraft = 0;

    // Limit all Huffman codes to not be larger than the maximum code length. Compute the Kraft
    // number, which will no longer satisfy the Kraft-McMillan inequality.
    for (std::uint16_t i = 0; i < m_huffman_codes_size; ++i)
    {
        HuffmanCode &code = m_huffman_codes[i];

        code.m_length = std::min(code.m_length, m_max_code_length);
        kraft += compute_kraft(code);
    }

    // The code lengths must now be corrected to satisfy the Kraft-McMillan inequality. Starting
    // from the largest code, increase the code lengths until the inequality is satisfied again.
    for (std::uint16_t i = m_huffman_codes_size; (i-- > 0) && (kraft > max_allowed_kraft);)
    {
        HuffmanCode &code = m_huffman_codes[i];

        while (code.m_length < m_max_code_length)
        {
            ++code.m_length;
            kraft -= compute_kraft(code);
        }
    }

    // The Kraft-McMillan inequality is now satisfied, but possibly overly so. Starting from the
    // shortest code, decrease code lengths just until the inequality would no longer be satisfied.
    for (std::uint16_t i = 0; i < m_huffman_codes_size; ++i)
    {
        HuffmanCode &code = m_huffman_codes[i];
        code_type candidate;

        while ((candidate = kraft + compute_kraft(code)) <= max_allowed_kraft)
        {
            kraft = candidate;
            --code.m_length;
        }
    }
}

//==================================================================================================
void HuffmanEncoder::convert_to_canonical_form()
{
    // First code is always set to zero. Its length does not change.
    m_huffman_codes[0].m_code = 0;

    if (m_huffman_codes_size == 1)
    {
        // Single-node Huffman trees occur when the input stream contains only one unique symbol.
        // Set its length to one so a single bit is encoded for each occurrence of that symbol.
        m_huffman_codes[0].m_length = 1;
    }

    for (std::uint16_t i = 1; i < m_huffman_codes_size; ++i)
    {
        const HuffmanCode &previous = m_huffman_codes[i - 1_u16];
        HuffmanCode &code = m_huffman_codes[i];

        // Subsequent codes are one greater than the previous code, but also bit-shifted left enough
        // times to maintain the same code length.
        code.m_code = previous.m_code + 1;
        code.m_code <<= code.m_length - previous.m_length;
    }
}

//==================================================================================================
void HuffmanEncoder::encode_header(fly::BitStreamWriter &encoded) const
{
    // Encode the Huffman coder version.
    encoded.write_byte(static_cast<byte_type>(s_huffman_version));

    // Encode the chunk size.
    encoded.write_word(static_cast<word_type>(m_chunk_size >> 10));

    // Encode the maximum Huffman code length.
    encoded.write_byte(static_cast<byte_type>(m_max_code_length));
}

//==================================================================================================
void HuffmanEncoder::encode_codes(fly::BitStreamWriter &encoded) const
{
    // At the least, encode that there were zero Huffman codes of length zero.
    std::vector<std::uint16_t> counts(1);

    for (std::uint16_t i = 0; i < m_huffman_codes_size; ++i)
    {
        const HuffmanCode &code = m_huffman_codes[i];

        if (counts.size() <= code.m_length)
        {
            counts.resize(code.m_length + 1_u8);
        }

        ++counts[code.m_length];
    }

    // Encode the number of code length counts.
    encoded.write_byte(static_cast<byte_type>(counts.size()));

    // Encode the code length counts.
    for (const std::uint16_t &length : counts)
    {
        encoded.write_word(static_cast<word_type>(length));
    }

    // Encode the symbols.
    for (std::uint16_t i = 0; i < m_huffman_codes_size; ++i)
    {
        const HuffmanCode &code = m_huffman_codes[i];
        encoded.write_byte(static_cast<byte_type>(code.m_symbol));
    }
}

//==================================================================================================
void HuffmanEncoder::encode_symbols(std::uint32_t chunk_size, fly::BitStreamWriter &encoded)
{
    decltype(m_huffman_codes) symbols;

    for (std::uint16_t i = 0; i < m_huffman_codes_size; ++i)
    {
        HuffmanCode code = std::move(m_huffman_codes[i]);
        symbols[code.m_symbol] = std::move(code);
    }

    for (std::uint32_t i = 0; i < chunk_size; ++i)
    {
        const HuffmanCode &code = symbols[m_chunk_buffer[i]];
        const auto length = static_cast<byte_type>(code.m_length);

        encoded.write_bits(code.m_code, length);
    }
}

} // namespace fly::coders
