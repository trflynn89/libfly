#pragma once

#include "fly/types/string/concepts.hpp"
#include "fly/types/string/detail/format_parameter_type.hpp"
#include "fly/types/string/detail/traits.hpp"
#include "fly/types/string/lexer.hpp"

#include <cstddef>
#include <string_view>

namespace fly::string::detail {

/**
 * Provides access to the format string parsing state consisting of the format string being parsed,
 * and the format parameter types and indices.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 25, 2021
 */
template <fly::StandardCharacter CharType>
class BasicFormatParseContext
{
    using traits = fly::detail::BasicStringTraits<CharType>;
    using view_type = typename traits::view_type;

public:
    /**
     * Constructor.
     *
     * @param format The format string to be parsed.
     * @param parameters Pointer to a list of parameter types to be formatted.
     * @param parameters_size Size of the parameter types list.
     */
    template <std::size_t N>
    constexpr explicit BasicFormatParseContext(
        CharType const (&format)[N],
        ParameterType const *parameters,
        std::size_t parameters_size) noexcept;

    /**
     * Parse the optional position argument of the current replacement field. If a position was not
     * found, the position is observed to be the next format parameter in order.
     *
     * It is an error if the format string has a mix of manual and automatic positioning.
     *
     * @return The parsed or observed format parameter position.
     */
    constexpr std::size_t next_position();

    /**
     * Retrieve the type of the format parameter at the provided index.
     *
     * @param index The index of the format parameter.
     *
     * @return If the index exists, the format parameter type. Otherwise, an uninitialized value.
     */
    constexpr std::optional<ParameterType> parameter_type(std::size_t position);

    /**
     * @return The lexer for parsing the format string.
     */
    constexpr fly::BasicLexer<CharType> &lexer();

    /**
     * @return A string view into the format string.
     */
    constexpr view_type view() const;

    /**
     * Record an error that was encountered while parsing the format string.
     *
     * If invoked from a constant-evaluation context, this will raise a compilation error because
     * this method is purposefully non-constexpr. This results in an attempt to invoke a
     * non-constant expression from a constant context, which is erroneous. The error message from
     * the caller should be displayed in the terminal.
     *
     * If not invoked from a constant-evaluation context, this will store the error message.
     *
     * @param error A message describing the error that was encountered.
     */
    void on_error(char const *error);

    /**
     * If an error was stored from a non-constant-evaluated context, returns whether an error was
     * encountered while parsing the format string.
     *
     * @return True if an error was encountered while parsing.
     */
    constexpr bool has_error() const;

    /**
     * If an error was stored from a non-constant-evaluated context, returns the last error that was
     * encountered while parsing the format string.
     *
     * @return The error (if any) that was encountered while parsing the format string.
     */
    std::string error() const;

private:
    BasicFormatParseContext(BasicFormatParseContext &&) = delete;
    BasicFormatParseContext &operator=(BasicFormatParseContext &&) = delete;

    BasicFormatParseContext(BasicFormatParseContext const &) = delete;
    BasicFormatParseContext &operator=(BasicFormatParseContext const &) = delete;

    fly::BasicLexer<CharType> m_lexer;

    ParameterType const *m_parameters;
    std::size_t m_parameters_size;

    std::size_t m_next_position {0};
    bool m_expect_no_positions_specified {false};
    bool m_expect_all_positions_specified {false};

    std::string_view m_error;
};

//==================================================================================================
template <fly::StandardCharacter CharType>
template <std::size_t N>
constexpr BasicFormatParseContext<CharType>::BasicFormatParseContext(
    CharType const (&format)[N],
    ParameterType const *parameters,
    std::size_t parameters_size) noexcept :
    m_lexer(format),
    m_parameters(parameters),
    m_parameters_size(parameters_size)
{
}

//==================================================================================================
template <fly::StandardCharacter CharType>
constexpr std::size_t BasicFormatParseContext<CharType>::next_position()
{
    std::size_t position = 0;

    if (auto specified_position = m_lexer.consume_number(); specified_position)
    {
        m_expect_all_positions_specified = true;
        position = static_cast<std::size_t>(*specified_position);
    }
    else
    {
        m_expect_no_positions_specified = true;
        position = m_next_position++;
    }

    if (m_expect_all_positions_specified && m_expect_no_positions_specified)
    {
        on_error("Argument position must be provided on all or not on any specifier");
    }

    return position;
}

//==================================================================================================
template <fly::StandardCharacter CharType>
constexpr std::optional<ParameterType>
BasicFormatParseContext<CharType>::parameter_type(std::size_t position)
{
    if (position >= m_parameters_size)
    {
        on_error("Argument position exceeds number of provided arguments");
        return std::nullopt;
    }

    return m_parameters[position];
}

//==================================================================================================
template <fly::StandardCharacter CharType>
constexpr fly::BasicLexer<CharType> &BasicFormatParseContext<CharType>::lexer()
{
    return m_lexer;
}

//==================================================================================================
template <fly::StandardCharacter CharType>
constexpr auto BasicFormatParseContext<CharType>::view() const -> view_type
{
    return m_lexer.view();
}

//==================================================================================================
template <fly::StandardCharacter CharType>
void BasicFormatParseContext<CharType>::on_error(char const *error)
{
    m_error = error;
}

//==================================================================================================
template <fly::StandardCharacter CharType>
constexpr bool BasicFormatParseContext<CharType>::has_error() const
{
    return !m_error.empty();
}

//==================================================================================================
template <fly::StandardCharacter CharType>
std::string BasicFormatParseContext<CharType>::error() const
{
    return std::string(m_error.data(), m_error.size());
}

} // namespace fly::string::detail
