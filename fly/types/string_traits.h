#pragma once

#include <string>
#include <type_traits>

namespace fly {

/**
 * Traits for basic properties of standard std::basic_string<> specializations.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version March 23, 2019
 */
template <typename StringType>
struct BasicStringTraits
{
    using BaseType = std::decay_t<StringType>;

    static_assert(
        std::is_same_v<BaseType, std::string> ||
            std::is_same_v<BaseType, std::wstring> ||
            std::is_same_v<BaseType, std::u16string> ||
            std::is_same_v<BaseType, std::u32string>,
        "StringType must be a standard std::basic_string<> specialization");

    using CharType = typename BaseType::value_type;

    /**
     * Define a trait for testing if type T is a string-like type analogous to
     * StringType.
     */
    template <typename T>
    using is_string_like = std::bool_constant<
        std::is_same_v<CharType *, std::decay_t<T>> ||
        std::is_same_v<CharType const *, std::decay_t<T>> ||
        std::is_same_v<BaseType, std::decay_t<T>>>;

    template <typename T>
    inline static constexpr bool is_string_like_v = is_string_like<T>::value;

    /**
     * Define a trait for testing if the STL has defined the std::stoi family
     * of functions for StringType.
     */
    using has_stoi_family = std::bool_constant<
        std::is_same_v<BaseType, std::string> ||
        std::is_same_v<BaseType, std::wstring>>;

    inline static constexpr bool has_stoi_family_v = has_stoi_family::value;
};

} // namespace fly
