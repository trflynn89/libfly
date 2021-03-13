#include "fly/coders/base64/base64_coder.hpp"

#include <cstdint>
#include <cstring>

namespace fly::coders {

namespace {

    // The Base64 symbol table.
    constexpr const std::array<std::ios::char_type, 64> s_base64_symbols = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',

        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',

        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',

        '+', '/',
    };

    // A mapping of ASCII codes to their indices in the Base64 symbol table. Invalid values are -1,
    // and the padding symbol is -2.
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

    /**
     * Encode a chunk of data into Base64 symbols.
     *
     * @param decoded Buffer holding the contents to encode.
     * @param encoded Buffer to store the encoded contents.
     */
    void encode_chunk(const std::ios::char_type *decoded, std::ios::char_type *encoded)
    {
        const auto &ch0 = decoded[0];
        const auto &ch1 = decoded[1];
        const auto &ch2 = decoded[2];

        // First 6 bits of the first symbol.
        encoded[0] = s_base64_symbols[static_cast<std::size_t>((ch0 & 0xfc) >> 2)];

        // Last 2 bits of the first symbol, first 4 bits of the second symbol.
        encoded[1] =
            s_base64_symbols[static_cast<std::size_t>(((ch0 & 0x03) << 4) | ((ch1 & 0xf0) >> 4))];

        // Last 4 bits of the second symbol, first 2 bits of the third symbol.
        encoded[2] =
            s_base64_symbols[static_cast<std::size_t>(((ch1 & 0x0f) << 2) | ((ch2 & 0xc0) >> 6))];

        // Last 6 bits of the third symbol.
        encoded[3] = s_base64_symbols[static_cast<std::size_t>(ch2 & 0x3f)];
    }

    /**
     * Decode a chunk of Base64 symbols, conditionally allowing padding symbols.
     *
     * @tparam AllowPadding Provide std::true_type to allow padding symbols.
     *
     * @param encoded Buffer holding the contents to decode.
     * @param decoded Buffer to store the decoded contents.
     *
     * @return If padding symbols are allowed, returns the number of non-padding symbols decoded. If
     *         padding symbols are not allowed, returns true if all symbols were valid, non-padding
     *         symbols.
     */
    template <typename AllowPadding>
    std::conditional_t<AllowPadding::value, std::size_t, bool>
    decode_chunk(const std::ios::char_type *encoded, std::ios::char_type *decoded)
    {
        const auto code0 = s_base64_codes[static_cast<std::size_t>(encoded[0])];
        const auto code1 = s_base64_codes[static_cast<std::size_t>(encoded[1])];
        const auto code2 = s_base64_codes[static_cast<std::size_t>(encoded[2])];
        const auto code3 = s_base64_codes[static_cast<std::size_t>(encoded[3])];

        // All 6 bits of the first code, first 2 bits of the second code.
        decoded[0] = static_cast<std::ios::char_type>((code0 << 2) | ((code1 >> 4) & 0x03));

        // Last 4 bits of the second code, first 4 bits of the third code.
        decoded[1] =
            static_cast<std::ios::char_type>(((code1 & 0x0f) << 4) | ((code2 & 0xfc) >> 2));

        // Last 2 bits of the third code, all 6 bits of the fourth code.
        decoded[2] = static_cast<std::ios::char_type>(((code2 & 0x03) << 6) | code3);

        if constexpr (std::is_same_v<AllowPadding, std::true_type>)
        {
            if ((code0 == -1) || (code1 == -1) || (code2 == -1) || (code3 == -1))
            {
                // Fail if any of the codes were invalid.
                return 0;
            }
            else if ((code0 == -2) || (code1 == -2) || ((code2 == -2) && (code3 != -2)))
            {
                // Fail if either of the first two coders were padding, or the third code was
                // padding but the fourth was not.
                return 0;
            }

            return (code2 == -2) ? 1 : ((code3 == -2) ? 2 : 3);
        }
        else
        {
            return !((code0 | code1 | code2 | code3) & 0x80);
        }
    }

} // namespace

//==================================================================================================
bool Base64Coder::encode_internal(std::istream &decoded, std::ostream &encoded)
{
    do
    {
        decoded.read(m_decoded.data(), static_cast<std::streamsize>(m_decoded.size()));
        const auto bytes = static_cast<std::size_t>(decoded.gcount());

        const std::ios::char_type *decoding = m_decoded.data();
        std::ios::char_type *encoding = m_encoded.data();

        for (std::size_t i = bytes / s_decoded_chunk_size; i > 0; --i)
        {
            encode_chunk(decoding, encoding);
            decoding += s_decoded_chunk_size;
            encoding += s_encoded_chunk_size;
        }

        encoded.write(m_encoded.data(), encoding - m_encoded.data());

        // If the input stream was not evenly split into 3-byte chunks, add padding to the remaining
        // chunk.
        if (const auto remainder = bytes % s_decoded_chunk_size; remainder > 0)
        {
            const auto end = static_cast<std::size_t>(decoding - m_decoded.data()) + remainder;
            ::memset(m_decoded.data() + end, '\0', end);

            encode_chunk(decoding, m_encoded.data());

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

        if ((bytes % s_encoded_chunk_size) != 0)
        {
            decoded.setstate(std::ios::failbit);
            break;
        }
        else if (bytes == 0)
        {
            break;
        }

        const std::ios::char_type *encoding = m_encoded.data();
        std::ios::char_type *decoding = m_decoded.data();

        // For performance, the decoding loop is potentially broken up depending on whether this is
        // the last read from the encoded stream. The goal is to keep the decoding loop as simple
        // as possible; the decode_chunk method is specialized to disallow padding symbols within
        // this loop. So on the last read from the encoded stream, break out of this loop one
        // iteration early to then allow padding symbols.
        for (std::size_t i = bytes / s_encoded_chunk_size - encoded.eof(); i > 0; --i)
        {
            if (!decode_chunk<std::false_type>(encoding, decoding))
            {
                decoded.setstate(std::ios::failbit);
                break;
            }

            encoding += s_encoded_chunk_size;
            decoding += s_decoded_chunk_size;
        }

        if (encoded.eof())
        {
            const std::size_t bytes_read = decode_chunk<std::true_type>(encoding, decoding);
            decoding += bytes_read;

            if (bytes_read == 0)
            {
                decoded.setstate(std::ios::failbit);
                break;
            }
        }

        decoded.write(m_decoded.data(), decoding - m_decoded.data());
    } while (encoded);

    return encoded.eof() && decoded.good();
}

} // namespace fly::coders
