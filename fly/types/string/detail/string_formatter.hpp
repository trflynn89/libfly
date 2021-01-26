#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_formatter_types.hpp"
#include "fly/types/string/detail/string_streamer.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <iomanip>
#include <limits>
#include <type_traits>

namespace fly::detail {

/**
 * Helper trait to classify a type as an integer, excluding boolean types.
 */
template <typename T>
using is_format_integral =
    std::conjunction<std::is_integral<T>, std::negation<std::is_same<T, bool>>>;

template <typename T>
// NOLINTNEXTLINE(readability-identifier-naming)
static inline constexpr bool is_format_integral_v = is_format_integral<T>::value;

/**
 * Helper class to format and stream generic values into a std::basic_string's output stream type.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename StringType, typename... ParameterTypes>
class BasicStringFormatter
{
    using traits = BasicStringTraits<StringType>;
    using streamer = BasicStringStreamer<StringType>;
    using stream_modifiers = BasicStreamModifiers<StringType>;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using ostream_type = typename traits::ostream_type;
    using streamed_char_type = typename streamer::streamed_char_type;

    using FormatSpecifier = BasicFormatSpecifier<char_type>;
    using FormatString = BasicFormatString<StringType, ParameterTypes...>;
    using FormatParameters = BasicFormatParameters<StringType, ParameterTypes...>;

public:
    /**
     * Constructor. Create a string formatter for the provided format parameters.
     *
     * A format string consists of:
     *
     *     1. Any character other than "{" or "}", which are copied unchanged to the output.
     *     2. Escape sequences "{{" and "}}", which are replaced with "{" and "}" in the output.
     *     3. Replacement fields.
     *
     * Replacement fields may be of the form:
     *
     *     1. An introductory "{" character.
     *     2. An optional non-negative position.
     *     3. An optional colon ":" following by formatting options.
     *     4. A final "}" character.
     *
     * For a detailed description of replacement fields, and how this implementation differs from
     * std::format, see fly::detail::BasicFormatSpecifier.
     *
     * The main difference is the means by which generic format parameters may be formatted into a
     * string. In this implementation, any type for which an operator<< overload is defined will be
     * formatted using that overload. Other types will result in an error.
     *
     * On compilers that support immediate functions (consteval), the format string is validated at
     * compile time against the types of the format parameters. If the format string is invalid, a
     * compile error with a diagnostic message will be raised. On other compilers, the error message
     * will returned rather than a formatted string.
     *
     * @param stream The stream to insert the formatted string into.
     * @param parameters The variadic list of format parameters to be formatted.
     */
    BasicStringFormatter(ostream_type &stream, ParameterTypes &&...parameters) noexcept;

    /**
     * Format the provided format string with the format parameters, inserting the formatted string
     * into thea stream.
     *
     * @param fmt The string to format.
     */
    void format(FormatString &&fmt);

private:
    /**
     * Format a single replacement field with the provided value.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T>
    void format_value(FormatSpecifier &&specifier, const T &value);

    /**
     * Format a single replacement field with the provided boolean value.
     *
     * @tparam T The type of the value to format.
     *
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<std::is_same<T, bool>> = 0>
    void format_value_for_type(
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value);

    /**
     * Format a single replacement field with the provided non-boolean integral value.
     *
     * @tparam T The type of the value to format.
     *
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<is_format_integral<T>> = 0>
    void format_value_for_type(
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value);

    /**
     * Format a single replacement field with the provided floating point value.
     *
     * @tparam T The type of the value to format.
     *
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<std::is_floating_point<T>> = 0>
    void format_value_for_type(
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value);

    /**
     * Format a single replacement field with the provided string-like value.
     *
     * @tparam T The type of the value to format.
     *
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<detail::is_like_supported_string<T>> = 0>
    void format_value_for_type(
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value);

    /**
     * Format a single replacement field with the provided generic value.
     *
     * @tparam T The type of the value to format.
     *
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <
        typename T,
        fly::disable_if_any<
            std::is_integral<T>,
            std::is_floating_point<T>,
            detail::is_like_supported_string<T>> = 0>
    void format_value_for_type(
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value);

    /**
     * The width and precision formatting options may either be a number or a nested replacement
     * field. If a numeric value was specified, return that value. If a nested replacement field was
     * specified, return the value of the format parameter at the position indicated by the nested
     * replacement field.
     *
     * @tparam T The type to cast the returned value to.
     *
     * @param size_or_position The numeric value or format parameter position.
     *
     * @return If either option was specified, and the corresponding value is non-negative, returns
     *         that value. Otherwise, an uninitialized value.
     */
    template <typename T = std::streamsize>
    std::optional<T> resolve_size(
        const std::optional<typename FormatSpecifier::SizeOrPosition> &size_or_position) const;

    static constexpr const auto s_left_brace = FLY_CHR(char_type, '{');
    static constexpr const auto s_right_brace = FLY_CHR(char_type, '}');
    static constexpr const auto s_zero = FLY_CHR(streamed_char_type, '0');

    ostream_type &m_stream;
    const FormatParameters m_parameters;
};

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
BasicStringFormatter<StringType, ParameterTypes...>::BasicStringFormatter(
    ostream_type &stream,
    ParameterTypes &&...parameters) noexcept :
    m_stream(stream),
    m_parameters(std::forward<ParameterTypes>(parameters)...)
{
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
void BasicStringFormatter<StringType, ParameterTypes...>::format(FormatString &&fmt)
{
    auto formatter = [this](auto &&specifier, const auto &value)
    {
        this->format_value(std::move(specifier), value);
    };

    const view_type view = fmt.view();

    for (std::size_t pos = 0; pos < view.size();)
    {
        switch (const auto &ch = view[pos])
        {
            case s_left_brace:
                if (view[pos + 1] == s_left_brace)
                {
                    streamer::stream_value(m_stream, ch);
                    pos += 2;
                }
                else
                {
                    auto specifier = *std::move(fmt.next_specifier());
                    pos += specifier.m_size;

                    m_parameters.visit(std::move(specifier), formatter);
                }
                break;

            case s_right_brace:
                streamer::stream_value(m_stream, ch);
                pos += 2;
                break;

            default:
                streamer::stream_value(m_stream, ch);
                ++pos;
                break;
        }
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    const T &value)
{
    stream_modifiers modifiers(m_stream);

    if (specifier.m_fill)
    {
        m_stream.fill(static_cast<streamed_char_type>(*specifier.m_fill));
    }

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
            m_stream.setf(std::ios_base::left);
            break;

        case FormatSpecifier::Alignment::Right:
            m_stream.setf(std::ios_base::right);
            break;

        case FormatSpecifier::Alignment::Center: // TODO: Implement center-alignment.
        case FormatSpecifier::Alignment::Default:
            m_stream.setf(specifier.is_numeric() ? std::ios_base::right : std::ios_base::left);
            break;
    }

    switch (specifier.m_sign)
    {
        case FormatSpecifier::Sign::Always:
            m_stream.setf(std::ios_base::showpos);
            break;

        case FormatSpecifier::Sign::NegativeOnlyWithPositivePadding:
            modifiers.template locale<PositivePaddingFacet<streamed_char_type>>();
            m_stream.setf(std::ios_base::showpos);
            break;

        default:
            break;
    }

    if (specifier.m_alternate_form)
    {
        m_stream.setf(std::ios_base::showbase);
        m_stream.setf(std::ios_base::showpoint);
    }

    if (specifier.m_zero_padding)
    {
        m_stream.setf(std::ios_base::internal, std::ios_base::adjustfield);
        m_stream.fill(s_zero);
    }

    if (const auto width = resolve_size(specifier.m_width); width && (*width > 0))
    {
        m_stream.width(*width);
    }

    if (specifier.m_case == FormatSpecifier::Case::Upper)
    {
        m_stream.setf(std::ios_base::uppercase);
    }

    format_value_for_type(std::move(modifiers), std::move(specifier), value);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<std::is_same<T, bool>>>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value_for_type(
    stream_modifiers &&modifiers,
    FormatSpecifier &&specifier,
    const T &value)
{
    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::String:
            m_stream.setf(std::ios_base::boolalpha);
            break;

        case FormatSpecifier::Type::Binary:
            modifiers.template locale<BinaryFacet<streamed_char_type>>();
            break;

        case FormatSpecifier::Type::Octal:
            m_stream.setf(std::ios_base::oct);
            break;

        case FormatSpecifier::Type::Hex:
            m_stream.setf(std::ios_base::hex);
            break;

        default:
            break;
    }

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        // TODO: Validate the value fits into streamed_char_type / convert Unicode encoding.
        streamer::template stream_value<decltype(value), streamed_char_type>(m_stream, value);
    }
    else
    {
        streamer::stream_value(m_stream, value);
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<is_format_integral<T>>>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value_for_type(
    stream_modifiers &&modifiers,
    FormatSpecifier &&specifier,
    const T &value)
{
    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::Binary:
            modifiers.template locale<BinaryFacet<streamed_char_type>>();
            break;

        case FormatSpecifier::Type::Octal:
            m_stream.setf(std::ios_base::oct);
            break;

        case FormatSpecifier::Type::Hex:
            m_stream.setf(std::ios_base::hex);
            break;

        default:
            break;
    }

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        // TODO: Validate the value fits into streamed_char_type / convert Unicode encoding.
        streamer::template stream_value<decltype(value), streamed_char_type>(m_stream, value);
    }
    else
    {
        using integral_type =
            std::conditional_t<std::numeric_limits<T>::is_signed, std::intmax_t, std::uintmax_t>;
        streamer::template stream_value<decltype(value), integral_type>(m_stream, value);
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<std::is_floating_point<T>>>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value_for_type(
    stream_modifiers &&,
    FormatSpecifier &&specifier,
    const T &value)
{
    if (const auto precision = resolve_size(specifier.m_precision); precision)
    {
        m_stream.precision(*precision);
    }

    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::HexFloat:
            m_stream.setf(std::ios_base::fixed | std::ios_base::scientific);
            break;

        case FormatSpecifier::Type::Scientific:
            m_stream.setf(std::ios_base::scientific);
            break;

        case FormatSpecifier::Type::Fixed:
            // Only Apple's Clang seems to respect std::uppercase with std::fixed values. To ensure
            // consistency, format these values as general types.
            if (!std::isnan(value) && !std::isinf(value))
            {
                m_stream.setf(std::ios_base::fixed);
            }
            break;

        default:
            break;
    }

    streamer::stream_value(m_stream, value);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<detail::is_like_supported_string<T>>>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value_for_type(
    stream_modifiers &&,
    FormatSpecifier &&specifier,
    const T &value)
{
    // There isn't a standard manipulator to limit the number of characters from the string that are
    // written to the stream. Instead, inform the streamer to limit the streamed length.
    if (const auto max_string_length = resolve_size<std::size_t>(specifier.m_precision);
        max_string_length)
    {
        streamer::stream_string(m_stream, value, *max_string_length);
    }
    else
    {
        streamer::stream_string(m_stream, value, std::nullopt);
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <
    typename T,
    fly::disable_if_any<
        std::is_integral<T>,
        std::is_floating_point<T>,
        detail::is_like_supported_string<T>>>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value_for_type(
    stream_modifiers &&,
    FormatSpecifier &&,
    const T &value)
{
    streamer::stream_value(m_stream, value);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T>
inline std::optional<T> BasicStringFormatter<StringType, ParameterTypes...>::resolve_size(
    const std::optional<typename FormatSpecifier::SizeOrPosition> &size_or_position) const
{
    if (size_or_position)
    {
        if (size_or_position->is_size())
        {
            return static_cast<T>(size_or_position->value());
        }

        const auto value = m_parameters.template get<std::streamsize>(size_or_position->value());

        if (value && (*value >= 0))
        {
            return static_cast<T>(*value);
        }
    }

    return std::nullopt;
}

} // namespace fly::detail
