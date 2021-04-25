#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/lexer.hpp"
#include "fly/types/string/literals.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
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
    using traits = BasicStringTraits<CharType>;
    using lexer = fly::BasicLexer<CharType>;
    using view_type = typename traits::view_type;

    using FormatSpecifier = BasicFormatSpecifier<CharType>;

    enum class ParameterType : std::uint8_t
    {
        Generic,
        Character,
        String,
        Pointer,
        Integral,
        FloatingPoint,
        Boolean,
    };

    enum class SpecifierType : std::uint8_t
    {
        Full,
        Nested,
    };

public:
    /**
     * Constructor. Parse and validate a C-string literal as a format string.
     */
    template <std::size_t N>
    FLY_CONSTEVAL BasicFormatString(const CharType (&format)[N]) noexcept;

    BasicFormatString(BasicFormatString &&) = default;
    BasicFormatString &operator=(BasicFormatString &&) = default;

    /**
     * @return A string view into the format string.
     */
    constexpr view_type view() const;

    /**
     * If the compiler does not support immediate functions, returns whether an error was
     * encountered while parsing the format string.
     *
     * @return True if an error was encountered while parsing.
     */
    constexpr bool has_error() const;

    /**
     * If the compiler does not support immediate functions, returns any error that was encountered
     * while parsing the format string.
     *
     * @return The error (if any) that was encountered while parsing the format string.
     */
    std::string error() const;

    /**
     * @return If available, the next parsed replacement field. Otherwise, an uninitialized value.
     */
    std::optional<FormatSpecifier> next_specifier();

private:
    friend BasicFormatSpecifier<CharType>;

    BasicFormatString(const BasicFormatString &) = delete;
    BasicFormatString &operator=(const BasicFormatString &) = delete;

    /**
     * Upon parsing an un-escaped opening brace, parse a single replacement field in the format
     * string. If valid, the lexer will be advanced to the character after the closing brace.
     *
     * @param specifier_type The type of replacement field to parse (either a full or nested field).
     *
     * @return If successful, the parsed specifier. Otherwise, an uninitialized value.
     */
    constexpr std::optional<FormatSpecifier> parse_specifier(SpecifierType specifier_type);

    /**
     * Parse the optional position argument of the current replacement field. If a position was not
     * found, the position is observed to be the next format parameter in order.
     *
     * It is an error if the format string has a mix of manual and automatic positioning.
     *
     * @return The parsed or observed format parameter position.
     */
    constexpr std::size_t parse_position();

    /**
     * Determine the type of a format parameter. Returns ParameterType::Generic if the type of the
     * format parameter is unknown.
     *
     * @param index The index of the format parameter to inspect.
     *
     * @return If found, the type of the format parameter. Otherwise, an uninitialized value.
     */
    template <size_t N = 0>
    constexpr std::optional<ParameterType> parameter_type(size_t index);

    /**
     * Record an error that was encountered while parsing the format string.
     *
     * If the compiler supports immediate functions, this will raise a compilation error because
     * this method is purposefully non-constexpr. This results in an attempt to invoke a
     * non-constant expression from a constant context, which is erroneous. The error message from
     * the caller should be displayed in the terminal.
     *
     * If the compiler does not support immediate functions, this will store the error message. Only
     * the first error encountered will be stored.
     *
     * @param error A message describing the error that was encountered.
     */
    void on_error(const char *error);

    static constexpr const auto s_left_brace = FLY_CHR(CharType, '{');
    static constexpr const auto s_right_brace = FLY_CHR(CharType, '}');
    static constexpr const auto s_colon = FLY_CHR(CharType, ':');

    lexer m_lexer;

    std::array<FormatSpecifier, 64> m_specifiers;
    std::size_t m_specifier_count {0};
    std::size_t m_specifier_index {0};

    std::size_t m_next_position {0};
    bool m_expect_no_positions_specified {false};
    bool m_expect_all_positions_specified {false};

    std::string_view m_error;
};

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
template <std::size_t N>
FLY_CONSTEVAL BasicFormatString<CharType, ParameterTypes...>::BasicFormatString(
    const CharType (&format)[N]) noexcept :
    m_lexer(format)
{
    std::optional<CharType> ch;

    if constexpr (!(BasicFormatTraits::is_formattable_v<ParameterTypes> && ...))
    {
        on_error("An overloaded operator<< must be defined for all format parameters");
    }

    while (!has_error() && (ch = m_lexer.consume()))
    {
        if (ch == s_left_brace)
        {
            if (m_lexer.consume_if(s_left_brace))
            {
                continue;
            }
            else if (m_specifier_count >= m_specifiers.size())
            {
                on_error("Exceeded maximum allowed number of specifiers");
            }
            else if (auto specifier = parse_specifier(SpecifierType::Full); specifier)
            {
                m_specifiers[m_specifier_count++] = *std::move(specifier);
            }
        }
        else if (ch == s_right_brace)
        {
            if (m_lexer.consume_if(s_right_brace))
            {
                continue;
            }

            on_error("Closing brace } must be esacped");
        }
    }
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
constexpr auto BasicFormatString<CharType, ParameterTypes...>::view() const -> view_type
{
    return m_lexer.view();
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
constexpr bool BasicFormatString<CharType, ParameterTypes...>::has_error() const
{
    return !m_error.empty();
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
std::string BasicFormatString<CharType, ParameterTypes...>::error() const
{
    return std::string(m_error);
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
constexpr auto
BasicFormatString<CharType, ParameterTypes...>::parse_specifier(SpecifierType specifier_type)
    -> std::optional<FormatSpecifier>
{
    // The opening { will have already been consumed, so the starting position is one less.
    const auto starting_position = m_lexer.position() - 1;

    FormatSpecifier specifier {};
    specifier.m_position = parse_position();

    auto param_type = parameter_type(specifier.m_position);

    if (param_type == ParameterType::Generic)
    {
        // TODO: For now, generic format parameters must be formatted with "{}".
        if (!m_lexer.consume_if(s_right_brace))
        {
            on_error("Generic types must be formatted with {}");
            return std::nullopt;
        }
    }
    else
    {
        if (param_type)
        {
            if ((specifier_type == SpecifierType::Full) && m_lexer.consume_if(s_colon))
            {
                specifier.parse(*this, *param_type);

                if (has_error())
                {
                    return std::nullopt;
                }
            }
            else
            {
                specifier.template infer_type<BasicFormatString>(*param_type);
            }
        }

        if (!m_lexer.consume_if(s_right_brace))
        {
            on_error("Detected unclosed format string - must end with }");
            return std::nullopt;
        }
    }

    if (m_expect_no_positions_specified && m_expect_all_positions_specified)
    {
        on_error("Argument position must be provided on all or not on any specifier");
    }
    else if (!param_type)
    {
        on_error("Argument position exceeds number of provided arguments");
    }

    specifier.m_size = m_lexer.position() - starting_position;
    return specifier;
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
constexpr std::size_t BasicFormatString<CharType, ParameterTypes...>::parse_position()
{
    if (auto position = m_lexer.consume_number(); position)
    {
        m_expect_all_positions_specified = true;
        return static_cast<std::size_t>(position.value());
    }
    else
    {
        m_expect_no_positions_specified = true;
        return m_next_position++;
    }
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
template <size_t N>
constexpr auto BasicFormatString<CharType, ParameterTypes...>::parameter_type(size_t index)
    -> std::optional<ParameterType>
{
    if constexpr (N < sizeof...(ParameterTypes))
    {
        if (N != index)
        {
            return parameter_type<N + 1>(index);
        }

        using T = std::decay_t<std::tuple_element_t<N, std::tuple<ParameterTypes...>>>;

        if constexpr (is_supported_character_v<T>)
        {
            return ParameterType::Character;
        }
        else if constexpr (is_like_supported_string_v<T>)
        {
            return ParameterType::String;
        }
        else if constexpr (std::is_pointer_v<T> || std::is_null_pointer_v<T>)
        {
            return ParameterType::Pointer;
        }
        else if constexpr (
            BasicFormatTraits::is_integer_v<T> || BasicFormatTraits::is_default_formatted_enum_v<T>)
        {
            return ParameterType::Integral;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            return ParameterType::FloatingPoint;
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return ParameterType::Boolean;
        }
        else
        {
            return ParameterType::Generic;
        }
    }
    else
    {
        return std::nullopt;
    }
}

//==================================================================================================
template <typename CharType, typename... ParameterTypes>
void BasicFormatString<CharType, ParameterTypes...>::on_error(const char *error)
{
    if (!has_error())
    {
        m_error = error;
    }
}

} // namespace fly::detail
