#pragma once

#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <cstdint>
#include <optional>

namespace fly::detail {

/**
 * Helper class to perform lexical analysis of a C-string literal. All methods are constant
 * expressions, allowing for string analysis at compile time.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename StringType>
class BasicStringLexer
{
    using traits = BasicStringTraits<StringType>;
    using classifier = BasicStringClassifier<StringType>;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;

public:
    /**
     * Constructor. Stores a view into a C-string literal. This class is not interested in the null
     * terminator, thus if provided, it is excluded from the view.
     *
     * @tparam N The size of the C-string literal.
     *
     * @param literals Reference to a C-string literal of size N.
     */
    template <std::size_t N>
    constexpr explicit BasicStringLexer(const char_type (&literals)[N]) noexcept;

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
    constexpr std::optional<char_type> peek(std::size_t offset = 0);

    /**
     * If a character is available at the current position in the C-string literal, return that
     * character and advance the current position to the next character.
     *
     * @return If available, the current character. Otherwise, an uninitialized value.
     */
    constexpr std::optional<char_type> consume();

    /**
     * If a character is available at the current position in the C-string literal, and if that
     * character is equivalent to the provided character, advance the current position to the next
     * character.
     *
     * @param ch The character to test with.
     *
     * @return Whether the current character was available and matched the provided character.
     */
    constexpr bool consume_if(char_type ch);

    /**
     * Beginning with the current position, retrieve characters from the current position in the
     * C-string literal and advance the current position to the next character until a character is
     * either not available or not a decimal digit. Convert the retrieved characters to an unsigned
     * number.
     *
     * @return If consumed, the parsed unsigned number. Otherwise, an uninitialized value.
     */
    constexpr std::optional<std::size_t> consume_number();

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
    constexpr std::optional<char_type> consume_if(Condition condition);

    static constexpr const auto s_null_terminator = FLY_CHR(char_type, '\0');

    const std::size_t m_size;
    const view_type m_view;

    std::size_t m_index {0};
};

//==================================================================================================
template <typename StringType>
template <std::size_t N>
constexpr BasicStringLexer<StringType>::BasicStringLexer(const char_type (&literals)[N]) noexcept :
    m_size(N - ((literals[N - 1] == s_null_terminator) ? 1 : 0)),
    m_view(literals, m_size)
{
}

//==================================================================================================
template <typename StringType>
constexpr auto BasicStringLexer<StringType>::view() const -> view_type
{
    return m_view;
}

//==================================================================================================
template <typename StringType>
constexpr std::size_t BasicStringLexer<StringType>::position() const
{
    return m_index;
}

//==================================================================================================
template <typename StringType>
constexpr auto BasicStringLexer<StringType>::peek(std::size_t offset) -> std::optional<char_type>
{
    if ((m_index + offset) >= m_size)
    {
        return std::nullopt;
    }

    return *(m_view.data() + m_index + offset);
}

//==================================================================================================
template <typename StringType>
constexpr auto BasicStringLexer<StringType>::consume() -> std::optional<char_type>
{
    if (m_index >= m_size)
    {
        return std::nullopt;
    }

    return *(m_view.data() + m_index++);
}

//==================================================================================================
template <typename StringType>
constexpr bool BasicStringLexer<StringType>::consume_if(char_type ch)
{
    if (auto next = peek(); next && (next.value() == ch))
    {
        ++m_index;
        return true;
    }

    return false;
}

//==================================================================================================
template <typename StringType>
template <typename Condition>
constexpr auto BasicStringLexer<StringType>::consume_if(Condition condition)
    -> std::optional<char_type>
{
    if (auto next = peek(); next && condition(next.value()))
    {
        return consume();
    }

    return std::nullopt;
}

//==================================================================================================
template <typename StringType>
constexpr std::optional<std::size_t> BasicStringLexer<StringType>::consume_number()
{
    bool parsed_number = false;
    std::size_t number = 0;

    while (auto ch = consume_if(classifier::is_digit))
    {
        parsed_number = true;

        number *= 10;
        number += static_cast<std::size_t>(ch.value() - FLY_CHR(char_type, '0'));
    }

    return parsed_number ? std::optional<std::size_t>(number) : std::nullopt;
}

} // namespace fly::detail
