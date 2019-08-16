#include "fly/coders/coder.h"

#include "fly/coders/bit_stream.h"
#include "fly/logger/logger.h"

#include <chrono>
#include <fstream>
#include <sstream>

namespace fly {

namespace {

    constexpr const std::ios::openmode s_inputMode =
        std::ios::in | std::ios::binary;

    constexpr const std::ios::openmode s_outputMode =
        std::ios::out | std::ios::binary | std::ios::trunc;

    constexpr const std::ios::openmode s_bidrectionalMode =
        s_inputMode | s_outputMode;

    void logEncoderStats(
        std::chrono::time_point<std::chrono::system_clock> start,
        std::size_t decodedSize,
        std::size_t encodedSize) noexcept
    {
        const auto end = std::chrono::system_clock::now();
        const auto ratio = static_cast<double>(encodedSize) / decodedSize;

        LOGD(
            "Encoded %u bytes to %u bytes (%f%%) in %f seconds",
            decodedSize,
            encodedSize,
            ratio * 100.0,
            std::chrono::duration<double>(end - start).count());
    }

    void logDecoderStats(
        std::chrono::time_point<std::chrono::system_clock> start,
        std::size_t encodedSize,
        std::size_t decodedSize) noexcept
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
    std::stringstream output(s_bidrectionalMode);

    if (input && output)
    {
        BitStreamWriter stream(output);
        successful = EncodeInternal(input, stream);
    }

    if (successful)
    {
        encoded = output.str();
        logEncoderStats(start, decoded.length(), encoded.length());
    }

    return true;
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
        std::fstream output(encoded, s_bidrectionalMode);

        if (input && output)
        {
            BitStreamWriter stream(output);
            successful = EncodeInternal(input, stream);
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
        BitStreamReader stream(input);
        successful = DecodeInternal(stream, output);
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
            BitStreamReader stream(input);
            successful = DecodeInternal(stream, output);
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

} // namespace fly
