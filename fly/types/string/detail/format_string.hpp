#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/detail/format_parameter_type.hpp"
#include "fly/types/string/detail/format_parse_context.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/string_traits.hpp"
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
template <typename CharType, typename... ParameterTypes>
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
    constexpr const FormatParseContext &context();

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
     * @return If successful, the parsed replacement field. Otherwise, an uninitialized value.
     */
    constexpr std::optional<FormatSpecifier> parse_specifier();

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
template <typename CharType, typename... ParameterTypes>
template <std::size_t N>
FLY_CONSTEVAL BasicFormatString<CharType, ParameterTypes...>::BasicFormatString(
    const CharType (&format)[N]) noexcept :
    m_context(format, s_parameters.data(), s_parameters.size())
{
    std::optional<CharType> ch;

    if constexpr (!(BasicFormatTraits::is_formattable_v<ParameterTypes> && ...))
    {
        m_context.on_error("An overloaded operator<< must be defined for all format parameters");
    }

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
            else if (auto specifier = parse_specifier(); specifier)
            {
                m_specifiers[m_specifier_count++] = *std::move(specifier);
            }
        }
        else if (ch == s_right_brace)
        {
            if (m_context.lexer().consume_if(s_right_brace))
            {
                continue;
            }

            m_context.on_error("Closing brace } must be esacped");
        }
    }
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
constexpr const BasicFormatParseContext<CharType> &
BasicFormatString<CharType, ParameterTypes...>::context()
{
    return m_context;
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
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
template <typename CharType, typename... ParameterTypes>
constexpr auto BasicFormatString<CharType, ParameterTypes...>::parse_specifier()
    -> std::optional<FormatSpecifier>
{
    // The opening { will have already been consumed, so the starting position is one less.
    const auto starting_position = m_context.lexer().position() - 1;

    FormatSpecifier specifier {};
    specifier.m_position = m_context.next_position();

    // TODO: For now, user-defined format parameters must be formatted with "{}".
    if (auto parameter_type = m_context.parameter_type(specifier.m_position);
        parameter_type && parameter_type != ParameterType::UserDefined)
    {
        if (m_context.lexer().consume_if(s_colon))
        {
            specifier.parse(m_context, *parameter_type);

            if (m_context.has_error())
            {
                return std::nullopt;
            }
        }
        else
        {
            specifier.infer_type(*parameter_type);
        }
    }

    if (!m_context.lexer().consume_if(s_right_brace))
    {
        m_context.on_error("Detected unclosed format string - must end with }");
        return std::nullopt;
    }

    specifier.m_size = m_context.lexer().position() - starting_position;
    return specifier;
}

} // namespace fly::detail
