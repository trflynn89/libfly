#include "fly/coders/huffman/huffman_config.hpp"

#include "fly/types/numeric/literals.hpp"

namespace fly {

//==================================================================================================
HuffmanConfig::HuffmanConfig() noexcept :
    m_default_encoder_chunk_size_kb(1_u16 << 10),
    m_default_encoder_max_code_length(11_u8)
{
}

//==================================================================================================
std::uint32_t HuffmanConfig::encoder_chunk_size() const noexcept
{
    auto encoder_chunk_size_kb =
        get_value<std::uint16_t>("encoder_chunk_size_kb", m_default_encoder_chunk_size_kb);

    return static_cast<std::uint32_t>(encoder_chunk_size_kb << 10);
}

//==================================================================================================
length_type HuffmanConfig::encoder_max_code_length() const noexcept
{
    return get_value<length_type>("encoder_max_code_length", m_default_encoder_max_code_length);
}

} // namespace fly
