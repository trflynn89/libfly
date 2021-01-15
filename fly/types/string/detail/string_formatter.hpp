#pragma once

#include "fly/types/string/detail/string_formatter_types.hpp"
#include "fly/types/string/detail/string_streamer.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <iomanip>
#include <limits>
#include <type_traits>

namespace fly::detail {

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
    using size_type = typename traits::size_type;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using ostream_type = typename traits::ostream_type;
    using streamed_type = typename traits::streamed_type;
    using streamed_char_type = typename streamer::streamed_char_type;

    using FormatSpecifier = BasicFormatSpecifier<char_type>;

    template <typename... ParameterTypes>
    using FormatString = BasicFormatString<StringType, std::type_identity_t<ParameterTypes>...>;

    template <typename... ParameterTypes>
    using FormatParameters =
        BasicFormatParameters<StringType, std::type_identity_t<ParameterTypes>...>;

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
     * @param stream The stream to insert the formatted string into.
     * @param fmt The string to format.
     * @param parameters The variadic list of format parameters to be formatted.
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
     *
     * @param stream The stream to insert the formatted value into.
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T>
    static void format_value(ostream_type &stream, FormatSpecifier &&specifier, const T &value);

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
    auto formatter = [&stream](auto &&specifier, const auto &value)
    {
        format_value(stream, std::move(specifier), value);
    };

    for (auto it = fmt.begin(); it != fmt.end(); ++it)
    {
        if (*it == s_left_brace)
        {
            if (*(it + 1) == s_left_brace)
            {
                streamer::stream_value(stream, *it++);
                continue;
            }

            while (*(++it) != s_right_brace)
            {
            }

            if (auto specifier = fmt.next_specifier(); specifier)
            {
                parameters.visit(*std::move(specifier), formatter);
            }
        }
        else if (*it == s_right_brace && (*(it + 1) == s_right_brace))
        {
            streamer::stream_value(stream, *it++);
        }
        else
        {
            streamer::stream_value(stream, *it);
        }
    }
}

//==================================================================================================
template <typename StringType>
template <typename T>
void BasicStringFormatter<StringType>::format_value(
    ostream_type &stream,
    FormatSpecifier &&specifier,
    const T &value)
{
    using U = std::remove_cvref_t<T>;

    BasicStreamModifiers<StringType> modifiers(stream);

    modifiers.manip(std::setfill(static_cast<streamed_char_type>(specifier.m_fill)));
    modifiers.manip(specifier.m_alternate_form ? std::showbase : std::noshowbase);
    modifiers.manip(specifier.m_alternate_form ? std::showpoint : std::noshowpoint);
    modifiers.manip(
        (specifier.m_case == FormatSpecifier::Case::Upper) ? std::uppercase : std::nouppercase);
    modifiers.manip(
        (specifier.m_type == FormatSpecifier::Type::String) ? std::boolalpha : std::noboolalpha);

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
            modifiers.manip(std::left);
            break;
        case FormatSpecifier::Alignment::Right:
            modifiers.manip(std::right);
            break;
        case FormatSpecifier::Alignment::Center: // TODO: Implement center-alignment.
        case FormatSpecifier::Alignment::Default:
            modifiers.manip(specifier.is_numeric() ? std::right : std::left);
            break;
    }

    switch (specifier.m_sign)
    {
        case FormatSpecifier::Sign::Always:
            modifiers.manip(std::showpos);
            break;
        case FormatSpecifier::Sign::Default:
        case FormatSpecifier::Sign::NegativeOnly:
            modifiers.manip(std::noshowpos);
            break;
        case FormatSpecifier::Sign::NegativeOnlyWithPositivePadding:
            modifiers.template locale<PositivePaddingFacet<streamed_char_type>>();
            modifiers.manip(std::showpos);
            break;
    }

    if (specifier.m_zero_padding)
    {
        modifiers.manip(std::setfill(s_zero));
        modifiers.manip(std::internal);
    }

    if (specifier.m_width)
    {
        modifiers.manip(std::setw(static_cast<int>(*specifier.m_width)));
    }

    if (specifier.m_precision)
    {
        if constexpr (detail::is_like_supported_string_v<U>)
        {
            using traits_type = BasicStringTraits<detail::is_like_supported_string_t<U>>;

            // Neither std::setw nor std::setprecision will limit the number of characters from the
            // string that are written to the stream. Instead, stream a substring view if needed.
            if (typename traits_type::view_type view(value); *specifier.m_precision < view.size())
            {
                streamer::stream_value(stream, view.substr(0, *specifier.m_precision));
                return;
            }
        }
        else
        {
            modifiers.manip(std::setprecision(static_cast<int>(*specifier.m_precision)));
        }
    }

    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::Binary:
            modifiers.template locale<BinaryFacet<streamed_char_type>>();
            break;
        case FormatSpecifier::Type::Octal:
            modifiers.manip(std::oct);
            break;
        case FormatSpecifier::Type::Decimal:
            modifiers.manip(std::dec);
            break;
        case FormatSpecifier::Type::Hex:
            modifiers.manip(std::hex);
            break;
        case FormatSpecifier::Type::HexFloat:
            modifiers.manip(std::hexfloat);
            break;
        case FormatSpecifier::Type::Scientific:
            modifiers.manip(std::scientific);
            break;
        case FormatSpecifier::Type::Fixed:
            // Note: this branch will only ever be entered with floating point types, but the
            // compiler will generate the below code for other types, so it must be protected.
            if constexpr (std::is_floating_point_v<U>)
            {
                // Only Apple's Clang seems to respect std::uppercase with std::fixed values. To
                // ensure consistency, format these values as general types.
                if (!std::isnan(value) && !std::isinf(value))
                {
                    modifiers.manip(std::fixed);
                }
            }
            break;
        default:
            break;
    }

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        // TODO: Validate the value fits into streamed_char_type / convert Unicode encoding.
        streamer::template stream_value<decltype(value), streamed_char_type>(stream, value);
    }
    else if (specifier.is_integral())
    {
        // Note: this branch will only ever be entered with integral types, but the compiler will
        // generate the below code for other types, so it must be protected.
        if constexpr (std::is_integral_v<U>)
        {
            using integral_type = std::
                conditional_t<std::numeric_limits<U>::is_signed, std::intmax_t, std::uintmax_t>;
            streamer::template stream_value<decltype(value), integral_type>(stream, value);
        }
    }
    else
    {
        streamer::stream_value(stream, value);
    }
}

} // namespace fly::detail
