#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/concepts.hpp"
#include "fly/types/string/detail/format_parameter_type.hpp"
#include "fly/types/string/detail/format_parse_context.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/traits.hpp"
#include "fly/types/string/lexer.hpp"
#include "fly/types/string/literals.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace fly::detail {

/**
 * A container to hold and parse a format string at compile time.
 *
 * This class depends on C++20 immediate functions (consteval), which are not yet supported by all
 * compilers. With compilers that do support immediate functions, if a format string is invalid
 * (either due to syntax or the foratting parameter types), a compile error will be raised with a
 * brief message indicating the error. For other compilers, that error message will be stored in the
 * instance and callers should check if an error was encountered.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <fly::StandardCharacter CharType, typename... ParameterTypes>
class BasicFormatString
{
    using FormatParseContext = BasicFormatParseContext<CharType>;
    using FormatSpecifier = BasicFormatSpecifier<CharType>;

public:
    /**
     * Constructor. Parse and validate a C-string literal as a format string.
     */
    template <std::size_t N>
    FLY_CONSTEVAL BasicFormatString(const CharType (&format)[N]) noexcept;

    BasicFormatString(BasicFormatString &&) = default;
    BasicFormatString &operator=(BasicFormatString &&) = default;

    /**
     * @return A reference to the format parsing context.
     */
    constexpr FormatParseContext &context();

    /**
     * @return If available, the next parsed replacement field. Otherwise, an uninitialized value.
     */
    std::optional<FormatSpecifier> next_specifier();

private:
    BasicFormatString(const BasicFormatString &) = delete;
    BasicFormatString &operator=(const BasicFormatString &) = delete;

    /**
     * Upon parsing an un-escaped opening brace, parse a single replacement field in the format
     * string. If valid, the format parsing context will be advanced to the character after the
     * closing brace.
     *
     * @return The parsed replacement field.
     */
    constexpr FormatSpecifier parse_specifier();

    /**
     * Parse a replacement field for a user-defined type.
     *
     * @param specifier The replacement field being parsed.
     */
    constexpr void parse_user_defined_specifier(FormatSpecifier &specifier);

    /**
     * Parse a replacement field for a standard type.
     *
     * @param specifier The replacement field being parsed.
     */
    constexpr void parse_standard_specifier(FormatSpecifier &specifier);

    static constexpr const auto s_left_brace = FLY_CHR(CharType, '{');
    static constexpr const auto s_right_brace = FLY_CHR(CharType, '}');
    static constexpr const auto s_colon = FLY_CHR(CharType, ':');

    static constexpr std::array<ParameterType, sizeof...(ParameterTypes)> s_parameters {
        infer_parameter_type<ParameterTypes>()...};

    FormatParseContext m_context;

    std::array<FormatSpecifier, 64> m_specifiers;
    std::size_t m_specifier_count {0};
    std::size_t m_specifier_index {0};
};

//==================================================================================================
template <fly::StandardCharacter CharType, typename... ParameterTypes>
template <std::size_t N>
FLY_CONSTEVAL BasicFormatString<CharType, ParameterTypes...>::BasicFormatString(
    const CharType (&format)[N]) noexcept :
    m_context(format, s_parameters.data(), s_parameters.size())
{
    std::optional<CharType> ch;

    while (!m_context.has_error() && (ch = m_context.lexer().consume()))
    {
        if (ch == s_left_brace)
        {
            if (m_context.lexer().consume_if(s_left_brace))
            {
                continue;
            }
            else if (m_specifier_count >= m_specifiers.size())
            {
                m_context.on_error("Exceeded maximum allowed number of specifiers");
            }
            else
            {
                m_specifiers[m_specifier_count++] = parse_specifier();
            }
        }
        else if (ch == s_right_brace)
        {
            if (m_context.lexer().consume_if(s_right_brace))
            {
                continue;
            }

            m_context.on_error("Closing brace } must be escaped");
        }
    }
}

//==================================================================================================
template <fly::StandardCharacter CharType, typename... ParameterTypes>
constexpr BasicFormatParseContext<CharType> &
BasicFormatString<CharType, ParameterTypes...>::context()
{
    return m_context;
}

//==================================================================================================
template <fly::StandardCharacter CharType, typename... ParameterTypes>
auto BasicFormatString<CharType, ParameterTypes...>::next_specifier()
    -> std::optional<FormatSpecifier>
{
    if (m_specifier_index >= m_specifier_count)
    {
        return std::nullopt;
    }

    return std::move(m_specifiers[m_specifier_index++]);
}

//==================================================================================================
template <fly::StandardCharacter CharType, typename... ParameterTypes>
constexpr auto BasicFormatString<CharType, ParameterTypes...>::parse_specifier() -> FormatSpecifier
{
    // The opening { will have already been consumed, so the starting position is one less.
    const auto starting_position = m_context.lexer().position() - 1;

    FormatSpecifier specifier(m_context);
    specifier.m_parse_index = m_context.lexer().position();

    if (m_context.parameter_type(specifier.m_position) == ParameterType::UserDefined)
    {
        parse_user_defined_specifier(specifier);
    }
    else
    {
        parse_standard_specifier(specifier);
    }

    specifier.m_size = m_context.lexer().position() - starting_position;
    return specifier;
}

//==================================================================================================
template <fly::StandardCharacter CharType, typename... ParameterTypes>
constexpr void BasicFormatString<CharType, ParameterTypes...>::parse_user_defined_specifier(
    FormatSpecifier &specifier)
{
    // Replacement fields for user-defined types are parsed at runtime.
    //
    // TODO: Formatters that inherit from standard formatters might be parsed at compile time.
    // TODO: Allow nested format specifiers.
    std::size_t expected_close_brace_count = 1;
    std::size_t nested_specifier_count = 0;

    while (expected_close_brace_count != 0)
    {
        if (auto ch = m_context.lexer().consume(); ch)
        {
            if (ch == s_right_brace)
            {
                --expected_close_brace_count;
            }
            else if (ch == s_left_brace)
            {
                ++expected_close_brace_count;
                ++nested_specifier_count;
            }
            else if (ch == s_colon)
            {
                specifier.m_parse_index = m_context.lexer().position();
            }
        }
        else
        {
            m_context.on_error("Detected unclosed replacement field - must end with }");
            expected_close_brace_count = 0;
        }
    }

    if (nested_specifier_count != 0)
    {
        m_context.on_error("Nested replacement fields are not allowed in user-defined formatters");
    }
}

//==================================================================================================
template <fly::StandardCharacter CharType, typename... ParameterTypes>
constexpr void
BasicFormatString<CharType, ParameterTypes...>::parse_standard_specifier(FormatSpecifier &specifier)
{
    if (m_context.lexer().consume_if(s_colon))
    {
        specifier.m_parse_index = m_context.lexer().position();
        specifier.parse(m_context);
    }

    if (!m_context.has_error() && !m_context.lexer().consume_if(s_right_brace))
    {
        m_context.on_error("Detected unclosed replacement field - must end with }");
    }
}

} // namespace fly::detail
