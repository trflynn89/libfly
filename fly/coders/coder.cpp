#include "fly/coders/coder.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/bit_stream/bit_stream_reader.hpp"
#include "fly/types/bit_stream/bit_stream_writer.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

namespace fly {

namespace {

    constexpr const std::ios::openmode s_input_mode = std::ios::in | std::ios::binary;

    constexpr const std::ios::openmode s_output_mode =
        std::ios::out | std::ios::binary | std::ios::trunc;

    template <typename SizeType>
    void log_encoder_stats(
        std::chrono::time_point<std::chrono::system_clock> start,
        SizeType decoded_size,
        SizeType encoded_size) noexcept
    {
        const auto end = std::chrono::system_clock::now();
        const auto ratio = (static_cast<double>(decoded_size) - encoded_size) / decoded_size;

        LOGD(
            "Encoded %u bytes to %u bytes (%f%%) in %f seconds",
            decoded_size,
            encoded_size,
            ratio * 100.0,
            std::chrono::duration<double>(end - start).count());
    }

    template <typename SizeType>
    void log_decoder_stats(
        std::chrono::time_point<std::chrono::system_clock> start,
        SizeType encoded_size,
        SizeType decoded_size) noexcept
    {
        const auto end = std::chrono::system_clock::now();

        LOGD(
            "Decoded %u bytes to %u bytes in %f seconds",
            encoded_size,
            decoded_size,
            std::chrono::duration<double>(end - start).count());
    }

} // namespace

//==================================================================================================
bool Encoder::encode_string(const std::string &decoded, std::string &encoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;

    std::istringstream input(decoded, s_input_mode);
    std::ostringstream output(s_output_mode);

    if (input && output)
    {
        successful = encode_internal(input, output);
    }

    if (successful)
    {
        encoded = output.str();
        log_encoder_stats(start, decoded.length(), encoded.length());
    }

    return successful;
}

//==================================================================================================
bool Encoder::encode_file(
    const std::filesystem::path &decoded,
    const std::filesystem::path &encoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;
    {
        std::ifstream input(decoded, s_input_mode);
        std::ofstream output(encoded, s_output_mode);

        if (input && output)
        {
            successful = encode_internal(input, output);
        }
    }

    if (successful)
    {
        log_encoder_stats(
            start,
            std::filesystem::file_size(decoded),
            std::filesystem::file_size(encoded));
    }

    return successful;
}

//==================================================================================================
bool BinaryEncoder::encode_internal(std::istream &decoded, std::ostream &encoded) noexcept
{
    BitStreamWriter stream(encoded);
    return encode_binary(decoded, stream);
}

//==================================================================================================
bool Decoder::decode_string(const std::string &encoded, std::string &decoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;

    std::istringstream input(encoded, s_input_mode);
    std::ostringstream output(s_output_mode);

    if (input && output)
    {
        successful = decode_internal(input, output);
    }

    if (successful)
    {
        decoded = output.str();
        log_decoder_stats(start, encoded.length(), decoded.length());
    }

    return successful;
}

//==================================================================================================
bool Decoder::decode_file(
    const std::filesystem::path &encoded,
    const std::filesystem::path &decoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;
    {
        std::ifstream input(encoded, s_input_mode);
        std::ofstream output(decoded, s_output_mode);

        if (input && output)
        {
            successful = decode_internal(input, output);
        }
    }

    if (successful)
    {
        log_decoder_stats(
            start,
            std::filesystem::file_size(encoded),
            std::filesystem::file_size(decoded));
    }

    return successful;
}

//==================================================================================================
bool BinaryDecoder::decode_internal(std::istream &encoded, std::ostream &decoded) noexcept
{
    BitStreamReader stream(encoded);
    return decode_binary(stream, decoded);
}

} // namespace fly
