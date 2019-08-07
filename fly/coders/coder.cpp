#include "fly/coders/coder.h"

#include "fly/coders/bit_stream.h"

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

} // namespace

//==============================================================================
bool Coder::EncodeString(
    const std::string &decoded,
    std::string &encoded) noexcept
{
    std::istringstream input(decoded, s_inputMode);
    std::stringstream output(s_bidrectionalMode);
    {
        BitStreamWriter stream(output);

        if (!EncodeInternal(input, stream))
        {
            return false;
        }
    }

    encoded = output.str();
    return true;
}

//==============================================================================
bool Coder::EncodeFile(
    const std::filesystem::path &decoded,
    const std::filesystem::path &encoded) noexcept
{
    std::ifstream input(decoded, s_inputMode);
    std::fstream output(encoded, s_bidrectionalMode);

    if (input && output)
    {
        BitStreamWriter stream(output);
        return EncodeInternal(input, stream);
    }

    return false;
}

//==============================================================================
bool Coder::DecodeString(
    const std::string &encoded,
    std::string &decoded) noexcept
{
    std::istringstream input(encoded, s_inputMode);
    std::ostringstream output(s_outputMode);

    BitStreamReader stream(input);

    if (DecodeInternal(stream, output))
    {
        decoded = output.str();
        return true;
    }

    return false;
}

//==============================================================================
bool Coder::DecodeFile(
    const std::filesystem::path &encoded,
    const std::filesystem::path &decoded) noexcept
{
    std::ifstream input(encoded, s_inputMode);
    std::ofstream output(decoded, s_outputMode);

    if (input && output)
    {
        BitStreamReader stream(input);
        return DecodeInternal(stream, output);
    }

    return false;
}

} // namespace fly
