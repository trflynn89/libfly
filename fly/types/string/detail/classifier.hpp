#pragma once

#include "fly/types/string/detail/string_concepts.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/literals.hpp"

#include <string>

namespace fly::detail {

/**
 * Helper class to provide safe alernatives to the STL's <cctype> methods.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename CharType>
class BasicClassifier
{
    using traits = detail::BasicStringTraits<CharType>;
    using size_type = typename traits::size_type;
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
    template <IsLikeSupportedString T>
    static constexpr size_type size(T &&value);

    /**
     * Determine the length of a character array value, excluding the null terminator (if present).
     *
     * @param value The character array.
     *
     * @return The length of the character array.
     */
    template <std::size_t N>
    static constexpr size_type size(const CharType (&value)[N]);

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
    static constexpr bool is_alpha(CharType ch);

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
    static constexpr bool is_upper(CharType ch);

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
    static constexpr bool is_lower(CharType ch);

    /**
     * Converts the given character to an upper-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std:toupper and std::towupper require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to convert.
     *
     * @return The converted character.
     */
    static constexpr CharType to_upper(CharType ch);

    /**
     * Converts the given character to a lower-case alphabetic character as classified by the
     * default C locale.
     *
     * The STL's std:tolower and std::towlower require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to convert.
     *
     * @return The converted character.
     */
    static constexpr CharType to_lower(CharType ch);

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
    static constexpr bool is_digit(CharType ch);

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
    static constexpr bool is_x_digit(CharType ch);

    /**
     * Checks if the given character is a whitespace character as classified by the default C
     * locale.
     *
     * The STL's std::isspace and std::iswspace require that the provided character fits into an
     * unsigned char and unsigned wchar_t, respectively. Other values result in undefined behavior.
     * This method has no such restriction.
     *
     * @param ch The character to classify.
     *
     * @return True if the character is a whitespace character.
     */
    static constexpr bool is_space(CharType ch);

private:
    /**
     * Remove the 0x20 bit from the given character, effectively converting the a-z range of
     * characters to the A-Z range.
     *
     * @param ch The character to modify.
     *
     * @return The modified character.
     */
    static constexpr CharType unify_az_characters(CharType ch);

    static constexpr const auto s_null_terminator = FLY_CHR(CharType, '\0');
    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
    static constexpr const auto s_upper_a = FLY_CHR(CharType, 'A');
    static constexpr const auto s_upper_z = FLY_CHR(CharType, 'Z');
    static constexpr const auto s_upper_f = FLY_CHR(CharType, 'F');
    static constexpr const auto s_lower_a = FLY_CHR(CharType, 'a');
    static constexpr const auto s_lower_z = FLY_CHR(CharType, 'z');
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
    static constexpr const auto s_form_feed = FLY_CHR(CharType, '\f');
    static constexpr const auto s_line_feed = FLY_CHR(CharType, '\n');
    static constexpr const auto s_carriage_return = FLY_CHR(CharType, '\r');
    static constexpr const auto s_horizontal_tab = FLY_CHR(CharType, '\t');
    static constexpr const auto s_vertical_tab = FLY_CHR(CharType, '\v');

    static constexpr const auto s_case_bit = static_cast<int_type>(0x20);
    static constexpr const auto s_case_mask = static_cast<int_type>(~s_case_bit);
};

//==================================================================================================
template <typename CharType>
template <IsLikeSupportedString T>
constexpr inline auto BasicClassifier<CharType>::size(T &&value) -> size_type
{
    using U = std::remove_cvref_t<T>;

    if constexpr (std::is_array_v<U> || std::is_pointer_v<U>)
    {
        return std::char_traits<CharType>::length(std::forward<T>(value));
    }
    else
    {
        return value.size();
    }
}

//==================================================================================================
template <typename CharType>
template <std::size_t N>
constexpr inline auto BasicClassifier<CharType>::size(const CharType (&value)[N]) -> size_type
{
    static_assert(N > 0, "Character arrays must have non-zero size");
    return N - ((value[N - 1] == s_null_terminator) ? 1 : 0);
}

//==================================================================================================
template <typename CharType>
constexpr inline bool BasicClassifier<CharType>::is_alpha(CharType ch)
{
    return is_upper(unify_az_characters(ch));
}

//==================================================================================================
template <typename CharType>
constexpr inline bool BasicClassifier<CharType>::is_upper(CharType ch)
{
    return (ch >= s_upper_a) && (ch <= s_upper_z);
}

//==================================================================================================
template <typename CharType>
constexpr inline bool BasicClassifier<CharType>::is_lower(CharType ch)
{
    return (ch >= s_lower_a) && (ch <= s_lower_z);
}

//==================================================================================================
template <typename CharType>
constexpr inline CharType BasicClassifier<CharType>::to_upper(CharType ch)
{
    if (is_lower(ch))
    {
        ch = static_cast<CharType>(static_cast<int_type>(ch) & s_case_mask);
    }

    return ch;
}

//==================================================================================================
template <typename CharType>
constexpr inline CharType BasicClassifier<CharType>::to_lower(CharType ch)
{
    if (is_upper(ch))
    {
        ch = static_cast<CharType>(static_cast<int_type>(ch) | s_case_bit);
    }

    return ch;
}

//==================================================================================================
template <typename CharType>
constexpr inline bool BasicClassifier<CharType>::is_digit(CharType ch)
{
    return (ch ^ s_zero) < 10;
}

//==================================================================================================
template <typename CharType>
constexpr inline bool BasicClassifier<CharType>::is_x_digit(CharType ch)
{
    const auto alpha = unify_az_characters(ch);
    return is_digit(ch) || ((alpha >= s_upper_a) && (alpha <= s_upper_f));
}

//==================================================================================================
template <typename CharType>
constexpr inline bool BasicClassifier<CharType>::is_space(CharType ch)
{
    return (ch == s_space) || (ch == s_form_feed) || (ch == s_line_feed) ||
        (ch == s_carriage_return) || (ch == s_horizontal_tab) || (ch == s_vertical_tab);
}

//==================================================================================================
template <typename CharType>
constexpr inline CharType BasicClassifier<CharType>::unify_az_characters(CharType ch)
{
    return static_cast<CharType>(static_cast<int_type>(ch) & s_case_mask);
}

} // namespace fly::detail
