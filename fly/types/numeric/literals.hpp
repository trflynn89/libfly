#pragma once

#include "fly/fly.hpp"
#include "fly/types/numeric/detail/literal_parser.hpp"

#include <cstddef>
#include <cstdint>

namespace fly {

/**
 * Type-safe, fixed-width integer literal suffixes not provided by the STL.
 *
 * The expression that precedes the literal suffix is parsed and validated at compile time.
 * Compilation will fail if any of the following error conditions are met:
 *
 *     1. The expression preceding the literal suffix is invalid. All standard integer literals are
 *        accepted.
 *     2. The value represented by the preceding expression does not fit in the type specified by
 *        the suffix.
 *     3. A character in the preceding expression does not match the corresponding base (e.g. 0b2 is
 *        an invalid expression).
 *
 * The inline namespaces are to allow importing literal suffixes in the fly namespace with explicit
 * control, without polluting the global namespace. Callers may import the literal suffixes in this
 * file in any of the following ways:
 *
 *     // Import only the literal suffixes declared in this file. This is recommended.
 *     using namespace fly::literals::numeric_literals;
 *
 *     // Import all literal suffixes declared in the fly namespace. Okay, but not recommended.
 *     using namespace fly::literals;
 *
 *     // Import a specific literal suffix declared in this file. Okay, but can be verbose.
 *     using fly::literals::numeric_literals::operator"" _i8;
 *
 *     // Import all symbols from namespace fly. Okay, but strongly discouraged.
 *     using namespace fly;
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version December 15, 2019
 */
inline namespace literals {
    inline namespace numeric_literals {

        template <char... Literals>
        FLY_CONSTEVAL inline std::int8_t operator"" _i8()
        {
            return fly::detail::literal<std::int8_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::int16_t operator"" _i16()
        {
            return fly::detail::literal<std::int16_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::int32_t operator"" _i32()
        {
            return fly::detail::literal<std::int32_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::int64_t operator"" _i64()
        {
            return fly::detail::literal<std::int64_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::uint8_t operator"" _u8()
        {
            return fly::detail::literal<std::uint8_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::uint16_t operator"" _u16()
        {
            return fly::detail::literal<std::uint16_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::uint32_t operator"" _u32()
        {
            return fly::detail::literal<std::uint32_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::uint64_t operator"" _u64()
        {
            return fly::detail::literal<std::uint64_t, Literals...>();
        }

        template <char... Literals>
        FLY_CONSTEVAL inline std::size_t operator"" _zu()
        {
            return fly::detail::literal<std::size_t, Literals...>();
        }

    } // namespace numeric_literals
} // namespace literals

} // namespace fly
