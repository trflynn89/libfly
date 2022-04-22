#include "fly/coders/coder_config.hpp"

namespace fly::coders {

//==================================================================================================
std::uint32_t CoderConfig::huffman_encoder_chunk_size() const
{
    auto const encoder_chunk_size_kb =
        get_value<std::uint16_t>("encoder_chunk_size_kb", m_default_huffman_encoder_chunk_size_kb);

    return static_cast<std::uint32_t>(encoder_chunk_size_kb) << 10;
}

//==================================================================================================
length_type CoderConfig::huffman_encoder_max_code_length() const
{
    return get_value<length_type>(
        "encoder_max_code_length",
        m_default_huffman_encoder_max_code_length);
}

} // namespace fly::coders
