#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_streamer_traits.hpp"

#include <cstdint>
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
 * Define a trait for testing if StringType is like a supported std::basic_string specialization. A
 * type is "like" a std::basic_string specialization if it is that specialization itself or a
 * C-string equalivent.
 *
 * For types that are like a supported std::basic_string specialization, this trait also defines a
 * type alias to that specialization. Other types are aliased to void and should not be used.
 */
template <typename StringType>
// NOLINTNEXTLINE(readability-identifier-naming)
struct is_like_supported_string
{
    using type = std::conditional_t<
        fly::any_same_v<StringType, char *, char const *, std::string>,
        std::string,
        std::conditional_t<
            fly::any_same_v<StringType, wchar_t *, wchar_t const *, std::wstring>,
            std::wstring,
            std::conditional_t<
                fly::any_same_v<StringType, char8_t *, char8_t const *, std::u8string>,
                std::u8string,
                std::conditional_t<
                    fly::any_same_v<StringType, char16_t *, char16_t const *, std::u16string>,
                    std::u16string,
                    std::conditional_t<
                        fly::any_same_v<StringType, char32_t *, char32_t const *, std::u32string>,
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
 * Traits for basic properties of standard std::basic_string specializations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 23, 2019
 */
template <typename StringType>
struct BasicStringTraits
{
    static_assert(
        is_supported_string_v<StringType>,
        "StringType must be a standard std::basic_string specialization");

    /**
     * Aliases for STL types that use std::basic_string specializations as a template type.
     */
    using size_type = typename StringType::size_type;
    using char_type = typename StringType::value_type;

    using iterator = typename StringType::iterator;
    using const_iterator = typename StringType::const_iterator;

    using codepoint_type = std::uint32_t;

    using streamer_traits = BasicStringStreamerTraits<StringType>;
    using streamed_type = typename streamer_traits::streamed_type;

    using istream_type = typename streamer_traits::istream_type;
    using ostream_type = typename streamer_traits::ostream_type;

    using fstream_type = typename streamer_traits::fstream_type;
    using ifstream_type = typename streamer_traits::ifstream_type;
    using ofstream_type = typename streamer_traits::ofstream_type;

    using stringstream_type = typename streamer_traits::stringstream_type;
    using istringstream_type = typename streamer_traits::istringstream_type;
    using ostringstream_type = typename streamer_traits::ostringstream_type;

    /**
     * Define a trait for testing if type T is a string-like type analogous to StringType.
     */
    template <typename T>
    using is_string_like = std::is_same<is_like_supported_string_t<T>, StringType>;

    template <typename T>
    inline static constexpr bool is_string_like_v = is_string_like<T>::value;

    /**
     * Define a trait for testing if the STL has defined the std::stoi family of functions for
     * StringType.
     */
    using has_stoi_family = any_same<StringType, std::string, std::wstring>;

    inline static constexpr bool has_stoi_family_v = has_stoi_family::value;

    /**
     * Define a trait for whether operator<< is defined for a type on the stream type used for
     * StringType.
     */
    template <typename T>
    using OstreamDeclaration = decltype(std::declval<ostream_type &>() << std::declval<T>());

    using OstreamTraits = DeclarationTraits<OstreamDeclaration>;
};

} // namespace fly::detail
