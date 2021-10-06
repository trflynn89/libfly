#pragma once

#include "fly/types/string/string_concepts.hpp"

#include <cstdint>
#include <string>

namespace fly::detail {

/**
 * Traits for basic properties of standard std::basic_string specializations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 23, 2019
 */
template <fly::StandardCharacter CharType>
struct BasicStringTraits
{
    using string_type = std::basic_string<CharType>;
    using size_type = typename string_type::size_type;
    using char_type = CharType;
    using view_type = std::basic_string_view<char_type>;
    using int_type = typename std::char_traits<char_type>::int_type;
    using codepoint_type = std::uint32_t;
};

} // namespace fly::detail
