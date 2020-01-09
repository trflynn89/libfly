#pragma once

#include "fly/coders/huffman/huffman_types.h"
#include "fly/config/config.h"

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
    std::uint32_t EncoderChunkSize() const noexcept;

    /**
     * @return Maximum Huffman code length (in bits) for encoding.
     */
    length_type EncoderMaxCodeLength() const noexcept;

protected:
    std::uint16_t m_defaultEncoderChunkSizeKB;
    length_type m_defaultEncoderMaxCodeLength;
};

} // namespace fly
