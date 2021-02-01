#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <string>

namespace fly::detail {

/**
 * Helper class to provide safe alernatives to the STL's <cctype> methods.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename StringType>
class BasicStringClassifier
{
    using traits = detail::BasicStringTraits<StringType>;
    using size_type = typename traits::size_type;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using int_type = typename traits::int_type;

public:
    /**
     * Determine the length of any string-like value. Accepts std::basic_string and
     * std::basic_string_view specializations.
     *
     * @tparam T The string-like type.
     *
     * @param value The string-like value.
     *
     * @return The length of the string-like value.
     */
    template <typename T, enable_if<detail::is_like_supported_string<T>> = 0>
    static constexpr size_type size(T &&value);

    /**
     * Determine the length of a character array value, excluding the null terminator (if present).
     *
     * @param value The character array.
     *
     * @return The length of the character array.
     */
    template <std::size_t N>
    static constexpr size_type size(const char_type (&value)[N]);

    /**
     * Checks if the given character is an alphabetic character as classified by the default C
     * locale.
     *
     * The STL's std::isalpha and std::iswalpha require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is an alphabetic character.
     */
    static constexpr bool is_alpha(char_type ch);

    /**
     * Checks if the given character is an upper-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std::isupper and std::iswupper require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is an alphabetic character.
     */
    static constexpr bool is_upper(char_type ch);

    /**
     * Checks if the given character is a lower-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std::islower and std::iswlower require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is an alphabetic character.
     */
    static constexpr bool is_lower(char_type ch);

    /**
     * Converts the given character to an upper-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std:tosupper and std::towupper require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to convert.
     *
     * @return The converted character.
     */
    static constexpr char_type to_upper(char_type ch);

    /**
     * Converts the given character to a lower-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std:toslower and std::towlower require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to convert.
     *
     * @return The converted character.
     */
    static constexpr char_type to_lower(char_type ch);

    /**
     * Checks if the given character is a decimal digit character.
     *
     * The STL's std::isdigit and std::iswdigit require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is a decimal digit character.
     */
    static constexpr bool is_digit(char_type ch);

    /**
     * Checks if the given character is a hexadecimal digit character.
     *
     * The STL's std::isxdigit and std::iswxdigit require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is a hexadecimal digit character.
     */
    static constexpr bool is_x_digit(char_type ch);

private:
    /**
     * Remove the 0x20 bit from the given character, effectively converting the a-z range of
     * characters to the A-Z range.
     *
     * @param ch The character to modify.
     *
     * @return The modified character.
     */
    static constexpr char_type unify_az_characters(char_type ch);

    static constexpr const auto s_null_terminator = FLY_CHR(char_type, '\0');
    static constexpr const auto s_zero = FLY_CHR(char_type, '0');
    static constexpr const auto s_upper_a = FLY_CHR(char_type, 'A');
    static constexpr const auto s_upper_z = FLY_CHR(char_type, 'Z');
    static constexpr const auto s_upper_f = FLY_CHR(char_type, 'F');
    static constexpr const auto s_lower_a = FLY_CHR(char_type, 'a');
    static constexpr const auto s_lower_z = FLY_CHR(char_type, 'z');

    static constexpr const auto s_case_bit = static_cast<int_type>(0x20);
    static constexpr const auto s_case_mask = static_cast<int_type>(~s_case_bit);
};

//==================================================================================================
template <typename StringType>
template <typename T, enable_if<detail::is_like_supported_string<T>>>
constexpr inline auto BasicStringClassifier<StringType>::size(T &&value) -> size_type
{
    if constexpr (any_same_v<T, StringType, view_type>)
    {
        return value.size();
    }
    else
    {
        return std::char_traits<char_type>::length(std::forward<T>(value));
    }
}

//==================================================================================================
template <typename StringType>
template <std::size_t N>
constexpr inline auto BasicStringClassifier<StringType>::size(const char_type (&value)[N])
    -> size_type
{
    static_assert(N > 0, "Character arrays must have non-zero size");
    return N - ((value[N - 1] == s_null_terminator) ? 1 : 0);
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicStringClassifier<StringType>::is_alpha(char_type ch)
{
    return is_upper(unify_az_characters(ch));
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicStringClassifier<StringType>::is_upper(char_type ch)
{
    return (ch >= s_upper_a) && (ch <= s_upper_z);
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicStringClassifier<StringType>::is_lower(char_type ch)
{
    return (ch >= s_lower_a) && (ch <= s_lower_z);
}

//==================================================================================================
template <typename StringType>
constexpr inline auto BasicStringClassifier<StringType>::to_upper(char_type ch) -> char_type
{
    if (is_lower(ch))
    {
        ch = static_cast<char_type>(static_cast<int_type>(ch) & s_case_mask);
    }

    return ch;
}

//==================================================================================================
template <typename StringType>
constexpr inline auto BasicStringClassifier<StringType>::to_lower(char_type ch) -> char_type
{
    if (is_upper(ch))
    {
        ch = static_cast<char_type>(static_cast<int_type>(ch) | s_case_bit);
    }

    return ch;
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicStringClassifier<StringType>::is_digit(char_type ch)
{
    return (ch ^ s_zero) < 10;
}

//==================================================================================================
template <typename StringType>
constexpr inline bool BasicStringClassifier<StringType>::is_x_digit(char_type ch)
{
    const auto alpha = unify_az_characters(ch);
    return is_digit(ch) || ((alpha >= s_upper_a) && (alpha <= s_upper_f));
}

//==================================================================================================
template <typename StringType>
constexpr inline auto BasicStringClassifier<StringType>::unify_az_characters(char_type ch)
    -> char_type
{
    return static_cast<char_type>(static_cast<int_type>(ch) & s_case_mask);
}

} // namespace fly::detail
