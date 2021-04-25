#pragma once

#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <cstdint>
#include <optional>

namespace fly {

template <typename CharType>
class BasicLexer;

using Lexer = BasicLexer<char>;
using WLexer = BasicLexer<wchar_t>;
using Lexer8 = BasicLexer<char8_t>;
using Lexer16 = BasicLexer<char16_t>;
using Lexer32 = BasicLexer<char32_t>;

/**
 * Helper class to perform lexical analysis of a C-string literal. All methods are constant
 * expressions, allowing for string analysis at compile time.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename CharType>
class BasicLexer
{
    using string_type = std::basic_string<CharType>;

    using traits = detail::BasicStringTraits<string_type>;
    using classifier = detail::BasicStringClassifier<string_type>;
    using view_type = typename traits::view_type;

public:
    /**
     * Constructor. Stores a view into a C-string literal.
     *
     * @tparam N The size of the C-string literal.
     *
     * @param literals Reference to a C-string literal of size N.
     */
    template <std::size_t N>
    constexpr explicit BasicLexer(const CharType (&literals)[N]) noexcept;

    /**
     * Constructor. Stores an existing view into a string.
     *
     * @param view The existing view into the string.
     */
    constexpr explicit BasicLexer(view_type view) noexcept;

    /**
     * @return A string view into the C-string literal.
     */
    constexpr view_type view() const;

    /**
     * @return The lexer's current position into the C-string literal.
     */
    constexpr std::size_t position() const;

    /**
     * If a character is available at the current position (or some offset from the current
     * position) in the C-string literal, return that character.
     *
     * @param offset The offset from the current position to peek.
     *
     * @return If available, the character at the provided offset. Otherwise, an uninitialized
     *         value.
     */
    constexpr std::optional<CharType> peek(std::size_t offset = 0);

    /**
     * If a character is available at the current position in the C-string literal, return that
     * character and advance the current position to the next character.
     *
     * @return If available, the current character. Otherwise, an uninitialized value.
     */
    constexpr std::optional<CharType> consume();

    /**
     * If a character is available at the current position in the C-string literal, and if that
     * character is equivalent to the provided character, advance the current position to the next
     * character.
     *
     * @param ch The character to test with.
     *
     * @return Whether the current character was available and matched the provided character.
     */
    constexpr bool consume_if(CharType ch);

    /**
     * Beginning with the current position, retrieve characters from the C-string literal and
     * advance the current position to the next character until a character is either not available
     * or not a decimal digit. Convert the retrieved characters to an unsigned number.
     *
     * @return If consumed, the parsed decimal number. Otherwise, an uninitialized value.
     */
    constexpr std::optional<std::uintmax_t> consume_number();

    /**
     * Beginning with the current position, retrieve characters from the C-string literal and
     * advance the current position to the next character until a character is either not available
     * or not a hexadecimal digit. Convert the retrieved characters to an unsigned number.
     *
     * @return If consumed, the parsed hexadecimal number. Otherwise, an uninitialized value.
     */
    constexpr std::optional<std::uintmax_t> consume_hex_number();

private:
    /**
     * If a character is available at the current position in the C-string literal, and if that
     * character satisfies the provided condition, advance the current position to the next
     * character.
     *
     * @tparam Condition A callable conditional for testing a character.
     *
     * @param condition The condition to test with.
     *
     * @return Whether the current character was available and satisfied the provided condition.
     */
    template <typename Condition>
    constexpr std::optional<CharType> consume_if(Condition condition);

    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
    static constexpr const auto s_upper_a = FLY_CHR(CharType, 'A');
    static constexpr const auto s_upper_f = FLY_CHR(CharType, 'F');
    static constexpr const auto s_lower_a = FLY_CHR(CharType, 'a');
    static constexpr const auto s_lower_f = FLY_CHR(CharType, 'f');

    const std::size_t m_size;
    const view_type m_view;

    std::size_t m_index {0};
};

//==================================================================================================
template <typename CharType>
template <std::size_t N>
constexpr BasicLexer<CharType>::BasicLexer(const CharType (&literals)[N]) noexcept :
    m_size(classifier::size(literals)),
    m_view(literals, m_size)
{
}

//==================================================================================================
template <typename CharType>
constexpr BasicLexer<CharType>::BasicLexer(view_type view) noexcept :
    m_size(view.size()),
    m_view(std::move(view))
{
}

//==================================================================================================
template <typename CharType>
constexpr auto BasicLexer<CharType>::view() const -> view_type
{
    return m_view;
}

//==================================================================================================
template <typename CharType>
constexpr std::size_t BasicLexer<CharType>::position() const
{
    return m_index;
}

//==================================================================================================
template <typename CharType>
constexpr std::optional<CharType> BasicLexer<CharType>::peek(std::size_t offset)
{
    if ((m_index + offset) >= m_size)
    {
        return std::nullopt;
    }

    return *(m_view.data() + m_index + offset);
}

//==================================================================================================
template <typename CharType>
constexpr std::optional<CharType> BasicLexer<CharType>::consume()
{
    if (m_index >= m_size)
    {
        return std::nullopt;
    }

    return *(m_view.data() + m_index++);
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicLexer<CharType>::consume_if(CharType ch)
{
    if (auto next = peek(); next && (next.value() == ch))
    {
        ++m_index;
        return true;
    }

    return false;
}

//==================================================================================================
template <typename CharType>
constexpr std::optional<std::uintmax_t> BasicLexer<CharType>::consume_number()
{
    bool parsed_number = false;
    std::uintmax_t number = 0;

    while (auto ch = consume_if(classifier::is_digit))
    {
        parsed_number = true;

        number *= 10;
        number += static_cast<std::uintmax_t>(ch.value() - s_zero);
    }

    return parsed_number ? std::optional<std::uintmax_t>(number) : std::nullopt;
}

//==================================================================================================
template <typename CharType>
constexpr std::optional<std::uintmax_t> BasicLexer<CharType>::consume_hex_number()
{
    bool parsed_number = false;
    std::uintmax_t number = 0;

    while (auto ch = consume_if(classifier::is_x_digit))
    {
        parsed_number = true;
        number *= 16;

        if ((ch.value() >= s_upper_a) && (ch.value() <= s_upper_f))
        {
            number += static_cast<std::uintmax_t>(ch.value()) - s_upper_a + 0xA;
        }
        else if ((ch.value() >= s_lower_a) && (ch.value() <= s_lower_f))
        {
            number += static_cast<std::uintmax_t>(ch.value()) - s_lower_a + 0xa;
        }
        else
        {
            number += static_cast<std::uintmax_t>(ch.value()) - s_zero;
        }
    }

    return parsed_number ? std::optional<std::uintmax_t>(number) : std::nullopt;
}

//==================================================================================================
template <typename CharType>
template <typename Condition>
constexpr std::optional<CharType> BasicLexer<CharType>::consume_if(Condition condition)
{
    if (auto next = peek(); next && condition(next.value()))
    {
        return consume();
    }

    return std::nullopt;
}

} // namespace fly
