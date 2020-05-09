#pragma once

#include "fly/coders/huffman/huffman_types.hpp"
#include "fly/config/config.hpp"

#include <chrono>
#include <cstdint>

namespace fly {

/**
 * Class to hold configuration values related to the Huffman coder.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class HuffmanConfig : public Config
{
public:
    static constexpr const char *identifier = "huffman";

    /**
     * Constructor.
     */
    HuffmanConfig() noexcept;

    /**
     * @return Huffman encoder chunk size (in bytes).
     */
    std::uint32_t encoder_chunk_size() const noexcept;

    /**
     * @return Maximum Huffman code length (in bits) for encoding.
     */
    length_type encoder_max_code_length() const noexcept;

protected:
    std::uint16_t m_default_encoder_chunk_size_kb;
    length_type m_default_encoder_max_code_length;
};

} // namespace fly
