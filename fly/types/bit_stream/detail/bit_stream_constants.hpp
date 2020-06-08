#pragma once

#include "fly/types/bit_stream/bit_stream_types.hpp"

#include <limits>

namespace fly::detail {

constexpr const byte_type s_magic = 0x1a;
constexpr const byte_type s_magic_mask = 0x1f;
constexpr const byte_type s_magic_shift = 0x03;

static_assert(s_magic <= s_magic_mask, "Magic header has exceeded 5 bits");

constexpr const byte_type s_remainder_mask = 0x07;
constexpr const byte_type s_remainder_shift = 0x00;

constexpr const byte_type s_byte_type_size = sizeof(byte_type);
constexpr const byte_type s_buffer_type_size = sizeof(buffer_type);

constexpr const byte_type s_bits_per_word = std::numeric_limits<word_type>::digits;
constexpr const byte_type s_bits_per_byte = std::numeric_limits<byte_type>::digits;

constexpr const byte_type s_most_significant_bit_position = s_buffer_type_size * s_bits_per_byte;

} // namespace fly::detail
