#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_streamer.hpp"

#include <cstdint>
#include <string>
#include <type_traits>

namespace fly::detail {

/**
 * Traits for basic properties of standard std::basic_string<> specializations.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version March 23, 2019
 */
template <typename StringType>
struct BasicStringTraits
{
    /**
     * Aliases for STL types that use std::basic_string<> specializations as a
     * template type.
     */
    using size_type = typename StringType::size_type;
    using char_type = typename StringType::value_type;

    using iterator = typename StringType::iterator;
    using const_iterator = typename StringType::const_iterator;

    using codepoint_type = std::uint32_t;

    using streamer_type = BasicStringStreamer<StringType>;

    using istream_type = typename streamer_type::istream_type;
    using ostream_type = typename streamer_type::ostream_type;

    using fstream_type = typename streamer_type::fstream_type;
    using ifstream_type = typename streamer_type::ifstream_type;
    using ofstream_type = typename streamer_type::ofstream_type;

    using stringstream_type = typename streamer_type::stringstream_type;
    using istringstream_type = typename streamer_type::istringstream_type;
    using ostringstream_type = typename streamer_type::ostringstream_type;

    static_assert(
        std::is_same_v<StringType, std::string> || std::is_same_v<StringType, std::wstring> ||
            std::is_same_v<StringType, std::u16string> ||
            std::is_same_v<StringType, std::u32string>,
        "StringType must be a standard std::basic_string<> specialization");

    /**
     * Define a trait for testing if type T is a string-like type analogous to
     * StringType.
     */
    template <typename T>
    using is_string_like = std::bool_constant<
        std::is_same_v<char_type *, std::decay_t<T>> ||
        std::is_same_v<char_type const *, std::decay_t<T>> ||
        std::is_same_v<StringType, std::decay_t<T>>>;

    template <typename T>
    inline static constexpr bool is_string_like_v = is_string_like<T>::value;

    /**
     * Define a trait for testing if the STL has defined the std::stoi family
     * of functions for StringType.
     */
    using has_stoi_family = std::bool_constant<
        std::is_same_v<StringType, std::string> || std::is_same_v<StringType, std::wstring>>;

    inline static constexpr bool has_stoi_family_v = has_stoi_family::value;

    /**
     * Define a trait for whether operator<< is defined for a type on the stream
     * type used for StringType.
     */
    template <typename T>
    using OstreamDeclaration = decltype(std::declval<ostream_type &>() << std::declval<T>());

    using OstreamTraits = DeclarationTraits<OstreamDeclaration>;
};

} // namespace fly::detail
