#include "fly/coders/coder.hpp"

#include "fly/logger/logger.hpp"
#include "fly/types/bit_stream/bit_stream_reader.hpp"
#include "fly/types/bit_stream/bit_stream_writer.hpp"

#include <chrono>
#include <fstream>
#include <sstream>

namespace fly {

namespace {

    constexpr const std::ios::openmode s_inputMode =
        std::ios::in | std::ios::binary;

    constexpr const std::ios::openmode s_outputMode =
        std::ios::out | std::ios::binary | std::ios::trunc;

    template <typename SizeType>
    void logEncoderStats(
        std::chrono::time_point<std::chrono::system_clock> start,
        SizeType decodedSize,
        SizeType encodedSize) noexcept
    {
        const auto end = std::chrono::system_clock::now();
        const auto ratio =
            (static_cast<double>(decodedSize) - encodedSize) / decodedSize;

        LOGD(
            "Encoded %u bytes to %u bytes (%f%%) in %f seconds",
            decodedSize,
            encodedSize,
            ratio * 100.0,
            std::chrono::duration<double>(end - start).count());
    }

    template <typename SizeType>
    void logDecoderStats(
        std::chrono::time_point<std::chrono::system_clock> start,
        SizeType encodedSize,
        SizeType decodedSize) noexcept
    {
        const auto end = std::chrono::system_clock::now();

        LOGD(
            "Decoded %u bytes to %u bytes in %f seconds",
            encodedSize,
            decodedSize,
            std::chrono::duration<double>(end - start).count());
    }

} // namespace

//==============================================================================
bool Encoder::EncodeString(
    const std::string &decoded,
    std::string &encoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;

    std::istringstream input(decoded, s_inputMode);
    std::ostringstream output(s_outputMode);

    if (input && output)
    {
        successful = EncodeInternal(input, output);
    }

    if (successful)
    {
        encoded = output.str();
        logEncoderStats(start, decoded.length(), encoded.length());
    }

    return successful;
}

//==============================================================================
bool Encoder::EncodeFile(
    const std::filesystem::path &decoded,
    const std::filesystem::path &encoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;
    {
        std::ifstream input(decoded, s_inputMode);
        std::ofstream output(encoded, s_outputMode);

        if (input && output)
        {
            successful = EncodeInternal(input, output);
        }
    }

    if (successful)
    {
        logEncoderStats(
            start,
            std::filesystem::file_size(decoded),
            std::filesystem::file_size(encoded));
    }

    return successful;
}

//==============================================================================
bool BinaryEncoder::EncodeInternal(
    std::istream &input,
    std::ostream &output) noexcept
{
    BitStreamWriter stream(output);
    return EncodeBinary(input, stream);
}

//==============================================================================
bool Decoder::DecodeString(
    const std::string &encoded,
    std::string &decoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;

    std::istringstream input(encoded, s_inputMode);
    std::ostringstream output(s_outputMode);

    if (input && output)
    {
        successful = DecodeInternal(input, output);
    }

    if (successful)
    {
        decoded = output.str();
        logDecoderStats(start, encoded.length(), decoded.length());
    }

    return successful;
}

//==============================================================================
bool Decoder::DecodeFile(
    const std::filesystem::path &encoded,
    const std::filesystem::path &decoded) noexcept
{
    const auto start = std::chrono::system_clock::now();
    bool successful = false;
    {
        std::ifstream input(encoded, s_inputMode);
        std::ofstream output(decoded, s_outputMode);

        if (input && output)
        {
            successful = DecodeInternal(input, output);
        }
    }

    if (successful)
    {
        logDecoderStats(
            start,
            std::filesystem::file_size(encoded),
            std::filesystem::file_size(decoded));
    }

    return successful;
}

//==============================================================================
bool BinaryDecoder::DecodeInternal(
    std::istream &input,
    std::ostream &output) noexcept
{
    BitStreamReader stream(input);
    return DecodeBinary(stream, output);
}

} // namespace fly
