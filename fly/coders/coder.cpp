#include "fly/coders/coder.h"

#include "fly/coders/bit_stream.h"

#include <fstream>
#include <sstream>

namespace fly {

namespace {

    constexpr const std::ios::openmode s_binaryIOMode =
        std::ios::in | std::ios::out | std::ios::binary;

} // namespace

//==============================================================================
bool Coder::EncodeString(const std::string &raw, std::string &encoded) noexcept
{
    std::istringstream input(raw);
    std::stringstream output(s_binaryIOMode);
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
    const std::filesystem::path &raw,
    const std::filesystem::path &encoded) noexcept
{
    std::ifstream input(raw);
    std::fstream output(encoded, s_binaryIOMode);

    if (input && output)
    {
        BitStreamWriter stream(output);
        return EncodeInternal(input, stream);
    }

    return false;
}

//==============================================================================
bool Coder::DecodeString(const std::string &raw, std::string &decoded) noexcept
{
    std::istringstream input(raw);
    std::ostringstream output;

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
    const std::filesystem::path &raw,
    const std::filesystem::path &decoded) noexcept
{
    std::ifstream input(raw);
    std::fstream output(decoded);

    if (input && output)
    {
        BitStreamReader stream(input);
        return DecodeInternal(stream, output);
    }

    return false;
}

} // namespace fly
