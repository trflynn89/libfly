#pragma once

#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

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
    using char_type = typename traits::char_type;
    using int_type = typename traits::int_type;

public:
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
    static bool is_alpha(char_type ch);

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
    static bool is_digit(char_type ch);

private:
    static constexpr const char_type s_zero = FLY_CHR(char_type, '0');
    static constexpr const char_type s_upper_a = FLY_CHR(char_type, 'A');
    static constexpr const char_type s_upper_z = FLY_CHR(char_type, 'Z');

    static constexpr const int_type s_case_mask = static_cast<int_type>(~0x20);
};

//==================================================================================================
template <typename StringType>
inline bool BasicStringClassifier<StringType>::is_alpha(char_type ch)
{
    // Remove the 0x20 bit, converting the a-z range of characters to the A-Z range.
    ch &= s_case_mask;

    return (ch >= s_upper_a) && (ch <= s_upper_z);
}

//==================================================================================================
template <typename StringType>
inline bool BasicStringClassifier<StringType>::is_digit(char_type ch)
{
    return (ch ^ s_zero) < 10;
}

} // namespace fly::detail
