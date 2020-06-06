#include "fly/coders/base64/base64_coder.hpp"

#include "fly/types/numeric/literals.hpp"

#include <cstdint>
#include <cstring>

namespace fly {

namespace {

    // The Base64 symbol table.
    constexpr const std::array<std::ios::char_type, 64> s_base64_table = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

        '+', '/',
    };

    // A mapping of ASCII codes to their indices in the Base64 symbol table.
    // Invalid values are marked with -1.
    constexpr const std::array<std::ios::char_type, 256> s_base64_codes = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62,
        -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, -1, 0,
        1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,
        23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    };

    // The Base64 padding symbol for 4-byte encoding alignment.
    constexpr const std::ios::char_type s_padding = '=';

} // namespace

//==================================================================================================
bool Base64Coder::encode_internal(std::istream &decoded, std::ostream &encoded) noexcept
{
    DecodedChunk chunk {};

    while (decoded.read(chunk.data(), chunk.size()))
    {
        encode_chunk(chunk, std::tuple_size<EncodedChunk>::value, encoded);
    }

    // If the input stream was not evenly split into 3-byte chunks, add padding
    // to the remaining chunk.
    const std::size_t index = static_cast<size_t>(decoded.gcount());

    if (index > 0)
    {
        ::memset(chunk.data() + index, '\0', chunk.size() - index);
        encode_chunk(chunk, index + 1, encoded);

        for (std::size_t i = index; i < chunk.size(); ++i)
        {
            encoded.put(s_padding);
        }
    }

    return decoded.eof() && encoded.good();
}

//==================================================================================================
bool Base64Coder::decode_internal(std::istream &encoded, std::ostream &decoded) noexcept
{
    EncodedChunk chunk {};

    do
    {
        encoded.read(chunk.data(), chunk.size());

        const std::size_t bytes = static_cast<std::size_t>(encoded.gcount());

        if (bytes == chunk.size())
        {
            decode_chunk(chunk, decoded);
        }
        else if (bytes > 0)
        {
            decoded.setstate(std::ios::failbit);
        }
    } while (encoded);

    return encoded.eof() && decoded.good();
}

//==================================================================================================
void Base64Coder::encode_chunk(const DecodedChunk &chunk, std::size_t bytes, std::ostream &encoded)
    const noexcept
{
    const EncodedChunk data = {
        // Front 6 bits of the first symbol.
        s_base64_table[static_cast<std::size_t>((chunk[0] & 0xfc) >> 2)],

        // Last 2 bits of the first symbol, front 4 bits of the second symbol.
        s_base64_table[static_cast<std::size_t>(
            ((chunk[0] & 0x03) << 4) | ((chunk[1] & 0xf0) >> 4))],

        // Last 4 bits of the second symbol, front 2 bits of the third symbol.
        s_base64_table[static_cast<std::size_t>(
            ((chunk[1] & 0x0f) << 2) | ((chunk[2] & 0xc0) >> 6))],

        // Last 6 bits of the third symbol.
        s_base64_table[static_cast<std::size_t>(chunk[2] & 0x3f)],
    };

    encoded.write(data.data(), static_cast<std::streamsize>(bytes));
}

//==================================================================================================
void Base64Coder::decode_chunk(EncodedChunk &chunk, std::ostream &decoded) const noexcept
{
    const std::size_t bytes = std::tuple_size<DecodedChunk>::value - parse_chunk(chunk);

    if (bytes == 0)
    {
        decoded.setstate(std::ios::failbit);
        return;
    }

    const DecodedChunk data = {
        // All 6 bits of the first code, front 2 bits of the second code.
        static_cast<std::ios::char_type>((chunk[0] << 2) | ((chunk[1] >> 4) & 0x03)),

        // Back 4 bits of the second code, front 4 bits of the third code.
        static_cast<std::ios::char_type>(((chunk[1] & 0x0f) << 4) | ((chunk[2] & 0xfc) >> 2)),

        // Back 2 bits of the third code, all 6 bits of the fourth code.
        static_cast<std::ios::char_type>(((chunk[2] & 0x03) << 6) | chunk[3]),
    };

    decoded.write(data.data(), static_cast<std::streamsize>(bytes));
}

//==================================================================================================
std::size_t Base64Coder::parse_chunk(EncodedChunk &chunk) const noexcept
{
    for (std::size_t i = 0; i < chunk.size(); ++i)
    {
        const std::ios::char_type &symbol = chunk[i];
        const std::ios::char_type code = s_base64_codes[static_cast<std::size_t>(symbol)];

        if (symbol == s_padding)
        {
            return chunk.size() - i;
        }
        else if (code == -1)
        {
            return std::tuple_size<DecodedChunk>::value;
        }
        else
        {
            chunk[i] = code;
        }
    }

    return 0;
}

} // namespace fly
