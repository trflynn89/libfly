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

    // A mapping of ASCII codes to their indices in the Base64 symbol table. Invalid values are -1.
    constexpr const std::array<std::ios::char_type, 256> s_base64_codes = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62,
        -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -2, -1, -1, -1, 0,
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
    constexpr const std::ios::char_type *s_pad = "===";

    template <typename T>
    inline std::ios::char_type encoded_symbol(T index)
    {
        return s_base64_table[static_cast<std::size_t>(index)];
    }

    template <typename T>
    inline std::ios::char_type decoded_symbol(T index)
    {
        return s_base64_codes[static_cast<std::size_t>(index)];
    }

} // namespace

//==================================================================================================
bool Base64Coder::encode_internal(std::istream &decoded, std::ostream &encoded)
{
    do
    {
        decoded.read(m_decoded.data(), static_cast<std::streamsize>(m_decoded.size()));

        const auto bytes = static_cast<std::size_t>(decoded.gcount());
        const auto chunks = bytes / s_decoded_chunk_size;

        const std::ios::char_type *decoded_pos = m_decoded.data();
        std::ios::char_type *encoded_pos = m_encoded.data();

        for (std::size_t i = 0; i < chunks; ++i)
        {
            encode_chunk(decoded_pos, encoded_pos);
            decoded_pos += s_decoded_chunk_size;
            encoded_pos += s_encoded_chunk_size;
        }

        encoded.write(m_encoded.data(), encoded_pos - m_encoded.data());

        // If the input stream was not evenly split into 3-byte chunks, add padding to the remaining
        // chunk.
        if (const auto remainder = bytes % s_decoded_chunk_size; remainder > 0)
        {
            const auto end = static_cast<std::size_t>(decoded_pos - m_decoded.data()) + remainder;
            ::memset(m_decoded.data() + end, '\0', end);

            encode_chunk(decoded_pos, m_encoded.data());

            encoded.write(m_encoded.data(), static_cast<std::streamsize>(remainder + 1));
            encoded.write(s_pad, static_cast<std::streamsize>(s_decoded_chunk_size - remainder));
        }
    } while (decoded);

    return decoded.eof() && encoded.good();
}

//==================================================================================================
bool Base64Coder::decode_internal(std::istream &encoded, std::ostream &decoded)
{
    do
    {
        encoded.read(m_encoded.data(), static_cast<std::streamsize>(m_encoded.size()));

        const auto bytes = static_cast<std::size_t>(encoded.gcount());
        const auto chunks = bytes / s_encoded_chunk_size;

        if ((bytes % s_encoded_chunk_size) != 0)
        {
            decoded.setstate(std::ios::failbit);
            break;
        }

        const std::ios::char_type *encoded_pos = m_encoded.data();
        std::ios::char_type *decoded_pos = m_decoded.data();

        for (std::size_t i = 0; i < chunks; ++i)
        {
            const std::size_t decoded_bytes = decode_chunk(encoded_pos, decoded_pos);

            if (decoded_bytes == 0)
            {
                decoded.setstate(std::ios::failbit);
                break;
            }

            encoded_pos += s_encoded_chunk_size;
            decoded_pos += decoded_bytes;
        }

        decoded.write(m_decoded.data(), decoded_pos - m_decoded.data());
    } while (encoded);

    return encoded.eof() && decoded.good();
}

//==================================================================================================
void Base64Coder::encode_chunk(const std::ios::char_type *decoded, std::ios::char_type *encoded)
    const
{
    // First 6 bits of the first symbol.
    encoded[0] = encoded_symbol((decoded[0] & 0xfc) >> 2);

    // Last 2 bits of the first symbol, first 4 bits of the second symbol.
    encoded[1] = encoded_symbol(((decoded[0] & 0x03) << 4) | ((decoded[1] & 0xf0) >> 4));

    // Last 4 bits of the second symbol, first 2 bits of the third symbol.
    encoded[2] = encoded_symbol(((decoded[1] & 0x0f) << 2) | ((decoded[2] & 0xc0) >> 6));

    // Last 6 bits of the third symbol.
    encoded[3] = encoded_symbol(decoded[2] & 0x3f);
}

//==================================================================================================
std::size_t
Base64Coder::decode_chunk(const std::ios::char_type *encoded, std::ios::char_type *decoded) const
{
    const auto code0 = decoded_symbol(encoded[0]);
    const auto code1 = decoded_symbol(encoded[1]);
    const auto code2 = decoded_symbol(encoded[2]);
    const auto code3 = decoded_symbol(encoded[3]);

    if ((code0 == -1) || (code1 == -1) || (code2 == -1) || (code3 == -1))
    {
        return 0;
    }

    // All 6 bits of the first code, first 2 bits of the second code.
    decoded[0] = static_cast<std::ios::char_type>((code0 << 2) | ((code1 >> 4) & 0x03));

    // Last 4 bits of the second code, first 4 bits of the third code.
    decoded[1] = static_cast<std::ios::char_type>(((code1 & 0x0f) << 4) | ((code2 & 0xfc) >> 2));

    // Last 2 bits of the third code, all 6 bits of the fourth code.
    decoded[2] = static_cast<std::ios::char_type>(((code2 & 0x03) << 6) | code3);

    return s_decoded_chunk_size -
        ((code0 == -2) ? 4 : ((code1 == -2) ? 3 : ((code2 == -2) ? 2 : ((code3 == -2) ? 1 : 0))));
}

} // namespace fly
