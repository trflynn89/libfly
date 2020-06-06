#include "fly/coders/huffman/huffman_decoder.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/bit_stream/bit_stream_reader.hpp"
#include "fly/types/numeric/literals.hpp"

#include <vector>

namespace fly {

//==================================================================================================
HuffmanDecoder::HuffmanDecoder() : m_huffman_codes_size(0)
{
}

//==================================================================================================
bool HuffmanDecoder::decode_binary(BitStreamReader &encoded, std::ostream &decoded) noexcept
{
    std::uint32_t chunk_size;
    length_type max_code_length;

    if (!decode_header(encoded, chunk_size, max_code_length))
    {
        LOGW("Error decoding header from stream");
        return false;
    }

    m_chunk_buffer = std::make_unique<symbol_type[]>(chunk_size);
    m_prefix_table = std::make_unique<HuffmanCode[]>(1_zu << max_code_length);

    while (!encoded.fully_consumed())
    {
        length_type local_max_code_length = 0;

        if (!decode_codes(encoded, max_code_length, local_max_code_length))
        {
            LOGW(
                "Error decoding codes from stream (maximum code length = %u)",
                static_cast<std::uint32_t>(max_code_length));
            return false;
        }
        else if (!decode_symbols(encoded, local_max_code_length, chunk_size, decoded))
        {
            LOGW(
                "Error decoding %u symbols from stream (fully consumed = %d)",
                chunk_size,
                encoded.fully_consumed());
            return false;
        }
    }

    return decoded.good();
}

//==================================================================================================
bool HuffmanDecoder::decode_header(
    BitStreamReader &encoded,
    std::uint32_t &chunk_size,
    length_type &max_code_length) const noexcept
{
    // Decode the Huffman coder version.
    byte_type huffman_version;

    if (!encoded.read_byte(huffman_version))
    {
        LOGW("Could not decode Huffman coder version");
        return false;
    }

    switch (huffman_version)
    {
        case 1:
            return decode_header_version1(encoded, chunk_size, max_code_length);

        default:
            LOGW("Decoded invalid Huffman version %u", static_cast<std::uint32_t>(huffman_version));
            break;
    }

    return false;
}

//==================================================================================================
bool HuffmanDecoder::decode_header_version1(
    BitStreamReader &encoded,
    std::uint32_t &chunk_size,
    length_type &max_code_length) const noexcept
{
    // Decode the chunk size.
    word_type encoded_chunk_size_kb;

    if (!encoded.read_word(encoded_chunk_size_kb))
    {
        LOGW("Could not decode chunk size");
        return false;
    }
    else if (encoded_chunk_size_kb == 0)
    {
        LOGW("Decoded invalid chunk size %u", static_cast<std::uint32_t>(encoded_chunk_size_kb));
        return false;
    }

    // Decode the maximum Huffman code length.
    byte_type encoded_max_code_length;

    if (!encoded.read_byte(encoded_max_code_length))
    {
        LOGW("Could not decode maximum code length");
        return false;
    }
    else if (
        (encoded_max_code_length == 0) ||
        (encoded_max_code_length >= std::numeric_limits<code_type>::digits))
    {
        LOGW(
            "Decoded invalid maximum code length %u",
            static_cast<std::uint32_t>(encoded_max_code_length));
        return false;
    }

    chunk_size = static_cast<std::uint32_t>(encoded_chunk_size_kb) << 10;
    max_code_length = static_cast<length_type>(encoded_max_code_length);

    return true;
}

//==================================================================================================
bool HuffmanDecoder::decode_codes(
    BitStreamReader &encoded,
    length_type global_max_code_length,
    length_type &local_max_code_length) noexcept
{
    m_huffman_codes_size = 0;

    // Decode the number of code length counts.
    byte_type counts_size;

    if (!encoded.read_byte(counts_size))
    {
        LOGW("Could not decode number of code length counts");
        return false;
    }
    else if ((counts_size == 0) || (counts_size > (global_max_code_length + 1)))
    {
        LOGW(
            "Decoded invalid number of code length counts %u",
            static_cast<std::uint32_t>(counts_size));
        return false;
    }

    // The first code length is 0, so the actual maximum code length is 1 less
    // than the number of length counts.
    local_max_code_length = counts_size - 1;

    // Decode the code length counts.
    std::vector<std::uint16_t> counts(counts_size);

    for (std::uint16_t &count : counts)
    {
        if (!encoded.read_word(count))
        {
            LOGW("Could not decode code length counts");
            return false;
        }
    }

    // Decode the symbols.
    for (length_type length = 0; length < counts_size; ++length)
    {
        for (std::uint16_t i = 0; i < counts[length]; ++i)
        {
            byte_type symbol;

            if (!encoded.read_byte(symbol))
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
            if (m_huffman_codes_size != 0)
            {
                const HuffmanCode &last = m_huffman_codes[m_huffman_codes_size - 1_u16];

                const length_type shift = length - last.m_length;
                code = (last.m_code + 1) << shift;
            }

            if (m_huffman_codes_size == m_huffman_codes.size())
            {
                LOGW("Exceeded maximum number of codes %u", m_huffman_codes_size);
                return false;
            }

            m_huffman_codes[m_huffman_codes_size++] =
                HuffmanCode(static_cast<symbol_type>(symbol), code, length);
        }
    }

    convert_to_prefix_table(local_max_code_length);
    return true;
}

//==================================================================================================
void HuffmanDecoder::convert_to_prefix_table(length_type max_code_length) noexcept
{
    for (std::uint16_t i = 0; i < m_huffman_codes_size; ++i)
    {
        const HuffmanCode code = std::move(m_huffman_codes[i]);
        const length_type shift = max_code_length - code.m_length;

        for (code_type j = 0; j < (1_u16 << shift); ++j)
        {
            const code_type index = (code.m_code << shift) + j;
            m_prefix_table[index].m_symbol = code.m_symbol;
            m_prefix_table[index].m_length = code.m_length;
        }
    }
}

//==================================================================================================
bool HuffmanDecoder::decode_symbols(
    BitStreamReader &encoded,
    length_type max_code_length,
    std::uint32_t chunk_size,
    std::ostream &decoded) const noexcept
{
    std::uint32_t bytes = 0;
    code_type index;

    while ((bytes < chunk_size) && (encoded.peek_bits(index, max_code_length) != 0))
    {
        const HuffmanCode &code = m_prefix_table[index];

        m_chunk_buffer[bytes++] = code.m_symbol;
        encoded.discard_bits(code.m_length);
    }

    if (bytes > 0)
    {
        decoded.write(
            reinterpret_cast<const std::ios::char_type *>(m_chunk_buffer.get()),
            static_cast<std::streamsize>(bytes));
    }

    return (bytes == chunk_size) || encoded.fully_consumed();
}

} // namespace fly
