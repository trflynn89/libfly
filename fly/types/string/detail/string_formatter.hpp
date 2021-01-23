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
template <typename StringType>
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

    template <typename... ParameterTypes>
    using FormatString = BasicFormatString<StringType, std::type_identity_t<ParameterTypes>...>;

    template <typename... ParameterTypes>
    using FormatParameters = BasicFormatParameters<StringType, ParameterTypes...>;

public:
    /**
     * Format a string with a set of format parameters, inserting the formatted string into a
     * stream. Based strongly upon: https://en.cppreference.com/w/cpp/utility/format/format.
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
     * The format string type is implicitly constructed from a C-string literal. Callers should only
     * invoke this method accordingly:
     *
     *     format("Format {:d}", 1);
     *
     * On compilers that support immediate functions (consteval), the format string is validated at
     * compile time against the types of the format parameters. If the format string is invalid, a
     * compile error with a diagnostic message will be raised. On other compilers, the error message
     * will returned rather than a formatted string.
     *
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted string into.
     * @param fmt The string to format.
     * @param parameters The variadic list of format parameters to be formatted.
     *
     * @return The same stream object.
     */
    template <typename... ParameterTypes>
    static ostream_type &format(
        ostream_type &stream,
        FormatString<ParameterTypes...> &&fmt,
        ParameterTypes &&...parameters);

private:
    /**
     * Iterate over the format string and format each replacement field in the format string.
     *
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted string into.
     * @param fmt The string to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <typename... ParameterTypes>
    static void format_internal(
        ostream_type &stream,
        FormatString<ParameterTypes...> &&fmt,
        FormatParameters<ParameterTypes...> &&parameters);

    /**
     * Format a single replacement field with the provided value.
     *
     * @tparam T The type of the value to format.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted value into.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <typename T, typename... ParameterTypes>
    static void format_value(
        ostream_type &stream,
        FormatSpecifier &&specifier,
        const T &value,
        const FormatParameters<ParameterTypes...> &parameters);

    /**
     * Format a single replacement field with the provided boolean value.
     *
     * @tparam T The type of the value to format.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted value into.
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <typename T, fly::enable_if<std::is_same<T, bool>> = 0, typename... ParameterTypes>
    static void format_value_for_type(
        ostream_type &stream,
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value,
        const FormatParameters<ParameterTypes...> &parameters);

    /**
     * Format a single replacement field with the provided non-boolean integral value.
     *
     * @tparam T The type of the value to format.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted value into.
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <typename T, fly::enable_if<is_format_integral<T>> = 0, typename... ParameterTypes>
    static void format_value_for_type(
        ostream_type &stream,
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value,
        const FormatParameters<ParameterTypes...> &parameters);

    /**
     * Format a single replacement field with the provided floating point value.
     *
     * @tparam T The type of the value to format.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted value into.
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <typename T, fly::enable_if<std::is_floating_point<T>> = 0, typename... ParameterTypes>
    static void format_value_for_type(
        ostream_type &stream,
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value,
        const FormatParameters<ParameterTypes...> &parameters);

    /**
     * Format a single replacement field with the provided string-like value.
     *
     * @tparam T The type of the value to format.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted value into.
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <
        typename T,
        fly::enable_if<detail::is_like_supported_string<T>> = 0,
        typename... ParameterTypes>
    static void format_value_for_type(
        ostream_type &stream,
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value,
        const FormatParameters<ParameterTypes...> &parameters);

    /**
     * Format a single replacement field with the provided generic value.
     *
     * @tparam T The type of the value to format.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param stream The stream to insert the formatted value into.
     * @param modifiers The active stream manipulator container.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param parameters The packed list of format parameters to be formatted.
     */
    template <
        typename T,
        fly::disable_if_any<
            std::is_integral<T>,
            std::is_floating_point<T>,
            detail::is_like_supported_string<T>> = 0,
        typename... ParameterTypes>
    static void format_value_for_type(
        ostream_type &stream,
        stream_modifiers &&modifiers,
        FormatSpecifier &&specifier,
        const T &value,
        const FormatParameters<ParameterTypes...> &parameters);

    /**
     * The width and precision formatting options may either be a number or a nested replacement
     * field. If a numeric value was specified, return that value. If a nested replacement field was
     * specified, return the value of the format parameter at the position indicated by the nested
     * replacement field.
     *
     * @tparam T The type to cast the returned value to.
     * @tparam ParameterTypes Variadic format parameter types.
     *
     * @param parameters The packed list of format parameters to be formatted.
     * @param size_or_position The numeric value or format parameter position.
     *
     * @return If either option was specified, and the corresponding value is non-negative, returns
     *         that value. Otherwise, an uninitialized value.
     */
    template <typename T = std::streamsize, typename... ParameterTypes>
    static std::optional<T> resolve_size(
        const FormatParameters<ParameterTypes...> &parameters,
        const std::optional<typename FormatSpecifier::SizeOrPosition> &size_or_position);

    static constexpr const auto s_left_brace = FLY_CHR(char_type, '{');
    static constexpr const auto s_right_brace = FLY_CHR(char_type, '}');
    static constexpr const auto s_zero = FLY_CHR(streamed_char_type, '0');
};

//==================================================================================================
template <typename StringType>
template <typename... ParameterTypes>
auto BasicStringFormatter<StringType>::format(
    ostream_type &stream,
    FormatString<ParameterTypes...> &&fmt,
    ParameterTypes &&...parameters) -> ostream_type &
{
    if (fmt.has_error())
    {
        streamer::stream_value(stream, "Ignored invalid formatter: ");
        streamer::stream_value(stream, fmt.error());
    }
    else
    {
        FormatParameters<ParameterTypes...> packed_parameters(
            std::forward<ParameterTypes>(parameters)...);

        format_internal<ParameterTypes...>(stream, std::move(fmt), std::move(packed_parameters));
    }

    return stream;
}

//==================================================================================================
template <typename StringType>
template <typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_internal(
    ostream_type &stream,
    FormatString<ParameterTypes...> &&fmt,
    FormatParameters<ParameterTypes...> &&parameters)
{
    auto formatter = [&stream, &parameters](auto &&specifier, const auto &value)
    {
        format_value(stream, std::move(specifier), value, parameters);
    };

    const view_type view = fmt.view();

    for (std::size_t pos = 0; pos < view.size();)
    {
        switch (const auto &ch = view[pos])
        {
            case s_left_brace:
                if (view[pos + 1] == s_left_brace)
                {
                    streamer::stream_value(stream, ch);
                    pos += 2;
                }
                else
                {
                    auto specifier = *std::move(fmt.next_specifier());
                    pos += specifier.m_size;

                    parameters.visit(std::move(specifier), formatter);
                }
                break;

            case s_right_brace:
                streamer::stream_value(stream, ch);
                pos += 2;
                break;

            default:
                streamer::stream_value(stream, ch);
                ++pos;
                break;
        }
    }
}

//==================================================================================================
template <typename StringType>
template <typename T, typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_value(
    ostream_type &stream,
    FormatSpecifier &&specifier,
    const T &value,
    const FormatParameters<ParameterTypes...> &parameters)
{
    stream_modifiers modifiers(stream);

    if (specifier.m_fill)
    {
        stream.fill(static_cast<streamed_char_type>(*specifier.m_fill));
    }

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
            stream.setf(std::ios_base::left);
            break;

        case FormatSpecifier::Alignment::Right:
            stream.setf(std::ios_base::right);
            break;

        case FormatSpecifier::Alignment::Center: // TODO: Implement center-alignment.
        case FormatSpecifier::Alignment::Default:
            stream.setf(specifier.is_numeric() ? std::ios_base::right : std::ios_base::left);
            break;
    }

    switch (specifier.m_sign)
    {
        case FormatSpecifier::Sign::Always:
            stream.setf(std::ios_base::showpos);
            break;

        case FormatSpecifier::Sign::NegativeOnlyWithPositivePadding:
            modifiers.template locale<PositivePaddingFacet<streamed_char_type>>();
            stream.setf(std::ios_base::showpos);
            break;

        default:
            break;
    }

    if (specifier.m_alternate_form)
    {
        stream.setf(std::ios_base::showbase);
        stream.setf(std::ios_base::showpoint);
    }

    if (specifier.m_zero_padding)
    {
        stream.setf(std::ios_base::internal, std::ios_base::adjustfield);
        stream.fill(s_zero);
    }

    if (const auto width = resolve_size(parameters, specifier.m_width); width && (*width > 0))
    {
        stream.width(*width);
    }

    if (specifier.m_case == FormatSpecifier::Case::Upper)
    {
        stream.setf(std::ios_base::uppercase);
    }

    format_value_for_type(stream, std::move(modifiers), std::move(specifier), value, parameters);
}

//==================================================================================================
template <typename StringType>
template <typename T, fly::enable_if<std::is_same<T, bool>>, typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_value_for_type(
    ostream_type &stream,
    stream_modifiers &&modifiers,
    FormatSpecifier &&specifier,
    const T &value,
    const FormatParameters<ParameterTypes...> &)
{
    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::String:
            stream.setf(std::ios_base::boolalpha);
            break;

        case FormatSpecifier::Type::Binary:
            modifiers.template locale<BinaryFacet<streamed_char_type>>();
            break;

        case FormatSpecifier::Type::Octal:
            stream.setf(std::ios_base::oct);
            break;

        case FormatSpecifier::Type::Hex:
            stream.setf(std::ios_base::hex);
            break;

        default:
            break;
    }

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        // TODO: Validate the value fits into streamed_char_type / convert Unicode encoding.
        streamer::template stream_value<decltype(value), streamed_char_type>(stream, value);
    }
    else
    {
        streamer::stream_value(stream, value);
    }
}

//==================================================================================================
template <typename StringType>
template <typename T, fly::enable_if<is_format_integral<T>>, typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_value_for_type(
    ostream_type &stream,
    stream_modifiers &&modifiers,
    FormatSpecifier &&specifier,
    const T &value,
    const FormatParameters<ParameterTypes...> &)
{
    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::Binary:
            modifiers.template locale<BinaryFacet<streamed_char_type>>();
            break;

        case FormatSpecifier::Type::Octal:
            stream.setf(std::ios_base::oct);
            break;

        case FormatSpecifier::Type::Hex:
            stream.setf(std::ios_base::hex);
            break;

        default:
            break;
    }

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        // TODO: Validate the value fits into streamed_char_type / convert Unicode encoding.
        streamer::template stream_value<decltype(value), streamed_char_type>(stream, value);
    }
    else
    {
        using integral_type =
            std::conditional_t<std::numeric_limits<T>::is_signed, std::intmax_t, std::uintmax_t>;
        streamer::template stream_value<decltype(value), integral_type>(stream, value);
    }
}

//==================================================================================================
template <typename StringType>
template <typename T, fly::enable_if<std::is_floating_point<T>>, typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_value_for_type(
    ostream_type &stream,
    stream_modifiers &&,
    FormatSpecifier &&specifier,
    const T &value,
    const FormatParameters<ParameterTypes...> &parameters)
{
    if (const auto precision = resolve_size(parameters, specifier.m_precision); precision)
    {
        stream.precision(*precision);
    }

    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::HexFloat:
            stream.setf(std::ios_base::fixed | std::ios_base::scientific);
            break;

        case FormatSpecifier::Type::Scientific:
            stream.setf(std::ios_base::scientific);
            break;

        case FormatSpecifier::Type::Fixed:
            // Only Apple's Clang seems to respect std::uppercase with std::fixed values. To ensure
            // consistency, format these values as general types.
            if (!std::isnan(value) && !std::isinf(value))
            {
                stream.setf(std::ios_base::fixed);
            }
            break;

        default:
            break;
    }

    streamer::stream_value(stream, value);
}

//==================================================================================================
template <typename StringType>
template <
    typename T,
    fly::enable_if<detail::is_like_supported_string<T>>,
    typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_value_for_type(
    ostream_type &stream,
    stream_modifiers &&,
    FormatSpecifier &&specifier,
    const T &value,
    const FormatParameters<ParameterTypes...> &parameters)
{
    // There isn't a standard manipulator to limit the number of characters from the string that are
    // written to the stream. Instead, inform the streamer to limit the streamed length.
    if (const auto max_string_length = resolve_size<std::size_t>(parameters, specifier.m_precision);
        max_string_length)
    {
        streamer::stream_string(stream, value, *max_string_length);
    }
    else
    {
        streamer::stream_string(stream, value, std::nullopt);
    }
}

//==================================================================================================
template <typename StringType>
template <
    typename T,
    fly::disable_if_any<
        std::is_integral<T>,
        std::is_floating_point<T>,
        detail::is_like_supported_string<T>>,
    typename... ParameterTypes>
void BasicStringFormatter<StringType>::format_value_for_type(
    ostream_type &stream,
    stream_modifiers &&,
    FormatSpecifier &&,
    const T &value,
    const FormatParameters<ParameterTypes...> &)
{
    streamer::stream_value(stream, value);
}

//==================================================================================================
template <typename StringType>
template <typename T, typename... ParameterTypes>
inline std::optional<T> BasicStringFormatter<StringType>::resolve_size(
    const FormatParameters<ParameterTypes...> &parameters,
    const std::optional<typename FormatSpecifier::SizeOrPosition> &size_or_position)
{
    if (size_or_position)
    {
        if (size_or_position->is_size())
        {
            return static_cast<T>(size_or_position->value());
        }
        else
        {
            const auto value = parameters.template get<std::streamsize>(size_or_position->value());

            if (value && (*value >= 0))
            {
                return static_cast<T>(*value);
            }
        }
    }

    return std::nullopt;
}

} // namespace fly::detail
