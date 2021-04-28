#pragma once

#include "fly/traits/traits.hpp"

#include <cstdint>
#include <ostream>
#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Define a trait for testing if StringType is a supported std::basic_string specialization.
 */
template <typename StringType>
using is_supported_string =
    any_same<StringType, std::string, std::wstring, std::u8string, std::u16string, std::u32string>;

template <typename StringType>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool is_supported_string_v = is_supported_string<StringType>::value;

/**
 * Define a trait for testing if CharType is a supported std::basic_string specialization's
 * character type.
 */
template <typename CharType>
using is_supported_character = any_same<CharType, char, wchar_t, char8_t, char16_t, char32_t>;

template <typename CharType>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool is_supported_character_v = is_supported_character<CharType>::value;

/**
 * Define a trait for testing if StringType is like a supported std::basic_string specialization. A
 * type is "like" a std::basic_string specialization if it is that specialization itself, a C-string
 * equalivent, or a std::basic_string_view specialization.
 *
 * For types that are like a supported std::basic_string specialization, this trait also defines a
 * type alias to that specialization. Other types are aliased to void and should not be used.
 */
template <typename StringType>
// NOLINTNEXTLINE(readability-identifier-naming)
struct is_like_supported_string
{
private:
    template <typename T, typename CharType>
    inline static constexpr bool is_like_string_impl = fly::any_same_v<
        T,
        CharType *,
        CharType const *,
        std::basic_string<CharType>,
        std::basic_string_view<CharType>>;

public:
    using type = std::conditional_t<
        is_like_string_impl<StringType, char>,
        std::string,
        std::conditional_t<
            is_like_string_impl<StringType, wchar_t>,
            std::wstring,
            std::conditional_t<
                is_like_string_impl<StringType, char8_t>,
                std::u8string,
                std::conditional_t<
                    is_like_string_impl<StringType, char16_t>,
                    std::u16string,
                    std::conditional_t<
                        is_like_string_impl<StringType, char32_t>,
                        std::u32string,
                        void>>>>>;

    inline static constexpr bool value = std::negation_v<std::is_same<type, void>>;
};

template <typename StringType>
// NOLINTNEXTLINE(readability-identifier-naming)
using is_like_supported_string_t = typename is_like_supported_string<StringType>::type;

template <typename StringType>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool is_like_supported_string_v = is_like_supported_string<StringType>::value;

/**
 * Define a trait for whether operator<< is defined for a type.
 */
template <typename T>
using OstreamDeclaration = decltype(std::declval<std::ostream &>() << std::declval<T>());

using OstreamTraits = DeclarationTraits<OstreamDeclaration>;

/**
 * Traits for basic properties of standard std::basic_string specializations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 23, 2019
 */
template <typename CharType>
struct BasicStringTraits
{
    static_assert(is_supported_character_v<CharType>, "CharType must be a standard character type");

    using string_type = std::basic_string<CharType>;
    using size_type = typename string_type::size_type;
    using char_type = CharType;
    using view_type = std::basic_string_view<char_type>;
    using int_type = typename std::char_traits<char_type>::int_type;
    using codepoint_type = std::uint32_t;

    /**
     * Define a trait for testing if type T is a string-like type analogous to string_type.
     */
    template <typename T>
    using is_string_like = std::is_same<is_like_supported_string_t<T>, string_type>;

    template <typename T>
    inline static constexpr bool is_string_like_v = is_string_like<T>::value;
};

/**
 * Traits for basic properties of format parameters.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version February 5, 2021
 */
struct BasicFormatTraits
{
    /**
     * Trait to determine if a type is either streamable or string-like.
     */
    template <typename T, typename U = std::remove_cvref_t<T>>
    using is_formattable = std::disjunction<
        OstreamTraits::is_declared<T>,
        detail::is_like_supported_string<T>,
        detail::is_supported_character<T>,
        std::is_enum<U>>;

    template <typename T>
    static inline constexpr bool is_formattable_v = is_formattable<T>::value;

    /**
     * Trait to classify a type as a pointer (excluding C-string types).
     */
    template <typename T, typename U = std::remove_cvref_t<T>>
    using is_pointer = std::conjunction<
        std::disjunction<std::is_pointer<U>, std::is_null_pointer<U>>,
        std::negation<detail::is_like_supported_string<T>>>;

    template <typename T>
    static inline constexpr bool is_pointer_v = is_pointer<T>::value;

    /**
     * Trait to classify a type as an integer, excluding boolean types.
     */
    template <typename T, typename U = std::remove_cvref_t<T>>
    using is_integral = std::conjunction<std::is_integral<U>, std::negation<std::is_same<U, bool>>>;

    template <typename T>
    static inline constexpr bool is_integral_v = is_integral<T>::value;

    /**
     * Trait to classify a type as an integer, excluding character and boolean types.
     */
    template <typename T>
    using is_integer = std::conjunction<is_integral<T>, std::negation<is_supported_character<T>>>;

    template <typename T>
    static inline constexpr bool is_integer_v = is_integer<T>::value;

    /**
     * Trait to classify an enumeration type as default-formatted (i.e. the user has not defined a
     * custom operator<< for this type).
     */
    template <typename T, typename U = std::remove_cvref_t<T>>
    using is_default_formatted_enum =
        std::conjunction<std::is_enum<U>, std::negation<OstreamTraits::is_declared<T>>>;

    template <typename T>
    static inline constexpr bool is_default_formatted_enum_v = is_default_formatted_enum<T>::value;

    /**
     * Trait to classify a type as a user-defined type.
     */
    template <typename T, typename U = std::remove_cvref_t<T>>
    using is_user_defined = std::negation<std::disjunction<
        detail::is_like_supported_string<T>,
        is_pointer<T>,
        is_integral<T>,
        std::is_floating_point<T>,
        std::is_same<T, bool>,
        is_default_formatted_enum<T>>>;

    template <typename T>
    static inline constexpr bool is_user_defined_v = is_user_defined<T>::value;
};

} // namespace fly::detail
