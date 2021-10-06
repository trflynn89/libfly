#pragma once

#include "fly/coders/huffman/types.hpp"
#include "fly/config/config.hpp"

#include <chrono>
#include <cstdint>

namespace fly::coders {

/**
 * Class to hold configuration values related to the coder implementations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version July 7, 2019
 */
class CoderConfig : public fly::config::Config
{
public:
    static constexpr const char *identifier = "coder";

    /**
     * @return Huffman encoder chunk size (in bytes).
     */
    std::uint32_t huffman_encoder_chunk_size() const;

    /**
     * @return Maximum Huffman code length (in bits) for encoding.
     */
    length_type huffman_encoder_max_code_length() const;

protected:
    std::uint16_t m_default_huffman_encoder_chunk_size_kb {256};
    length_type m_default_huffman_encoder_max_code_length {11};
};

} // namespace fly::coders
