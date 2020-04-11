#pragma once

#include "fly/types/bit_stream/bit_stream_types.hpp"

#include <limits>

namespace fly::detail {

constexpr const byte_type s_magic = 0x1a;
constexpr const byte_type s_magicMask = 0x1f;
constexpr const byte_type s_magicShift = 0x03;

static_assert(s_magic <= s_magicMask, "Magic header has exceeded 5 bits");

constexpr const byte_type s_remainderMask = 0x07;
constexpr const byte_type s_remainderShift = 0x00;

constexpr const byte_type s_byteTypeSize = sizeof(byte_type);
constexpr const byte_type s_bufferTypeSize = sizeof(buffer_type);

constexpr const byte_type s_bitsPerWord =
    std::numeric_limits<word_type>::digits;
constexpr const byte_type s_bitsPerByte =
    std::numeric_limits<byte_type>::digits;

constexpr const byte_type s_mostSignificantBitPosition =
    s_bufferTypeSize * s_bitsPerByte;

} // namespace fly::detail
