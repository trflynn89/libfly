#pragma once

#include "fly/concepts/concepts.hpp"

#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Concept that is satisfied when the given type is like a supported std::basic_string
 * specialization. A type is "like" a std::basic_string specialization if it is that specialization
 * itself, a C-string equalivent, or a std::basic_string_view specialization.
 */
template <typename T, typename CharType>
concept IsLikeStandardString = fly::SameAsAny<
    std::decay_t<T>, // Decay to perform array-to-pointer conversion (e.g. char[] to char*).
    CharType *,
    CharType const *,
    std::basic_string<CharType>,
    std::basic_string_view<CharType>>;

/**
 * Concept that is satisfied when the given type is like a supported std::basic_string
 * specialization. A type is "like" a std::basic_string specialization if it is that specialization
 * itself, a C-string equalivent, or a std::basic_string_view specialization.
 */
template <typename T>
struct StandardStringType
{
    using string_type = std::conditional_t<
        IsLikeStandardString<T, char>,
        std::string,
        std::conditional_t<
            IsLikeStandardString<T, wchar_t>,
            std::wstring,
            std::conditional_t<
                IsLikeStandardString<T, char8_t>,
                std::u8string,
                std::conditional_t<
                    IsLikeStandardString<T, char16_t>,
                    std::u16string,
                    std::conditional_t<
                        IsLikeStandardString<T, char32_t>,
                        std::u32string,
                        std::false_type>>>>>;

    using char_type = std::conditional_t<
        fly::SameAs<string_type, std::false_type>,
        std::false_type,
        typename string_type::value_type>;
};

} // namespace fly::detail
