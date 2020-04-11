#include "fly/coders/huffman/huffman_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==============================================================================
HuffmanConfig::HuffmanConfig() noexcept :
    m_defaultEncoderChunkSizeKB(1_u16 << 10),
    m_defaultEncoderMaxCodeLength(11_u8)
{
}

//==============================================================================
std::uint32_t HuffmanConfig::EncoderChunkSize() const noexcept
{
    auto encoderChunkSizeKB = GetValue<std::uint16_t>(
        "encoder_chunk_size_kb",
        m_defaultEncoderChunkSizeKB);

    return static_cast<std::uint32_t>(encoderChunkSizeKB << 10);
}

//==============================================================================
length_type HuffmanConfig::EncoderMaxCodeLength() const noexcept
{
    return GetValue<length_type>(
        "encoder_max_code_length",
        m_defaultEncoderMaxCodeLength);
}

} // namespace fly
