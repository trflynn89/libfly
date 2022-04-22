#pragma once

#include "fly/types/bit_stream/types.hpp"

#include <limits>

namespace fly::detail {

constexpr byte_type const s_magic = 0x1a;
constexpr byte_type const s_magic_mask = 0x1f;
constexpr byte_type const s_magic_shift = 0x03;

static_assert(s_magic <= s_magic_mask, "Magic header has exceeded 5 bits");

constexpr byte_type const s_remainder_mask = 0x07;
constexpr byte_type const s_remainder_shift = 0x00;

constexpr byte_type const s_byte_type_size = sizeof(byte_type);
constexpr byte_type const s_buffer_type_size = sizeof(buffer_type);

constexpr byte_type const s_bits_per_word = std::numeric_limits<word_type>::digits;
constexpr byte_type const s_bits_per_byte = std::numeric_limits<byte_type>::digits;

constexpr byte_type const s_most_significant_bit_position = s_buffer_type_size * s_bits_per_byte;

} // namespace fly::detail
