#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_formatter_types.hpp"
#include "fly/types/string/detail/string_streamer.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/detail/string_unicode.hpp"
#include "fly/types/string/string_literal.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <optional>
#include <system_error>
#include <type_traits>

namespace fly::detail {

/**
 * Helper trait to classify a type as an integer, excluding boolean types.
 */
template <typename T>
using is_format_integral =
    std::conjunction<std::is_integral<T>, std::negation<std::is_same<T, bool>>>;

/**
 * Class to format parameters according to a provided format string.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename StringType, typename... ParameterTypes>
class BasicStringFormatter
{
    using traits = BasicStringTraits<StringType>;
    using classifier = BasicStringClassifier<StringType>;
    using stream_modifiers = BasicStreamModifiers<StringType>;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;
    using streamed_char_type = typename traits::streamed_char_type;
    using ostringstream_type = typename traits::ostringstream_type;

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
     * @param parameters The variadic list of format parameters to be formatted.
     */
    BasicStringFormatter(ParameterTypes &&...parameters) noexcept;

    /**
     * Format the provided format string with the format parameters, returning the result as a
     * string.
     *
     * @param fmt The string to format.
     *
     * @return The formatted result.
     */
    StringType format(FormatString &&fmt);

private:
    /**
     * Format a single replacement field with the provided generic value.
     *
     * Currently, rather than supporting a set of std::formatter specializations, the generic value
     * will be converted to a string via the streaming operator. The resulting string will then be
     * formatted using the string formatting overload.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <
        typename T,
        fly::disable_if_any<detail::is_like_supported_string<T>, std::is_arithmetic<T>> = 0>
    void format_value(FormatSpecifier &&specifier, const T &value);

    /**
     * Format a single replacement field with the provided string-like value.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<detail::is_like_supported_string<T>> = 0>
    void format_value(FormatSpecifier &&specifier, const T &value);

    /**
     * Format a single replacement field with the provided non-boolean integral value.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<is_format_integral<T>> = 0>
    void format_value(FormatSpecifier &&specifier, T value);

    /**
     * Format a single replacement field with the provided unsigned, non-boolean integral value.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param is_negative Whether the original value was negative.
     */
    template <typename T, fly::enable_if<is_format_integral<T>> = 0>
    void format_value(FormatSpecifier &&specifier, T value, bool is_negative);

    /**
     * Format a single replacement field with the provided floating point value.
     *
     * Currently, major compilers do not support std::to_chars for floating point values. Until they
     * do, this implementation uses an IO stream to format the value.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<std::is_floating_point<T>> = 0>
    void format_value(FormatSpecifier &&specifier, const T &value);

    /**
     * Format a single replacement field with the provided boolean value.
     *
     * @tparam T The type of the value to format.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     */
    template <typename T, fly::enable_if<std::is_same<T, bool>> = 0>
    void format_value(FormatSpecifier &&specifier, T value);

    /**
     * Append a string-like value to the buffer, with an optional maximum string length.
     *
     * If the string-like value's character type is the same as the format string, the value is
     * inserted directly. Otherwise, it is first transcoded to the appropriate Unicode encoding.
     *
     * @tparam T The type of the value to append.
     *
     * @param value The value to append.
     * @param max_width The maximum number of characters from the value to append.
     */
    template <typename T, fly::enable_if<detail::is_like_supported_string<T>> = 0>
    void append_string(const T &value, std::size_t max_width = StringType::npos);

    /**
     * Append the string representation of a base-N integral value to the buffer, where N is the
     * provided integer base.
     *
     * Internally, std::to_chars is used for the conversion. Since std::to_chars only supports
     * char-based strings, this method behaves differently depending on the type of the format
     * string. If the format string is char-based, the conversion is applied directly to the buffer.
     * Otherwise, an intermediate char-based buffer is used for the conversion, then that buffer is
     * transcoded to the appropriate Unicode encoding, incurring extra string copying.
     *
     * @tparam T The type of the value to append.
     *
     * @param value The value to convert.
     * @param base The base to use.
     *
     * @return The number of base-N digits converted.
     */
    template <typename T, fly::enable_if<is_format_integral<T>> = 0>
    std::size_t append_number(T value, int base);

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
    template <typename T = std::size_t>
    T resolve_size(
        const std::optional<typename FormatSpecifier::SizeOrPosition> &size_or_position,
        T fallback = 0) const;

    /**
     * Count the number of base-N digits in a value, where N is the provided integer base.
     *
     * @param value The value to count digits in.
     * @param base The base to use.
     *
     * @return The number of base-N digits in the value.
     */
    template <typename T, fly::enable_if<is_format_integral<T>> = 0>
    std::size_t count_digits(T value, int base);

    static constexpr const auto s_left_brace = FLY_CHR(char_type, '{');
    static constexpr const auto s_right_brace = FLY_CHR(char_type, '}');
    static constexpr const auto s_plus_sign = FLY_CHR(char_type, '+');
    static constexpr const auto s_minus_sign = FLY_CHR(char_type, '-');
    static constexpr const auto s_space = FLY_CHR(char_type, ' ');
    static constexpr const auto s_zero = FLY_CHR(char_type, '0');
    static constexpr const auto s_lower_b = FLY_CHR(char_type, 'b');
    static constexpr const auto s_upper_b = FLY_CHR(char_type, 'B');
    static constexpr const auto s_lower_x = FLY_CHR(char_type, 'x');
    static constexpr const auto s_upper_x = FLY_CHR(char_type, 'X');

    static constexpr const char_type *s_true = FLY_STR(char_type, "true");
    static constexpr const char_type *s_false = FLY_STR(char_type, "false");

    static inline thread_local ostringstream_type s_stream;

    const FormatParameters m_parameters;
    StringType m_buffer;
};

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
BasicStringFormatter<StringType, ParameterTypes...>::BasicStringFormatter(
    ParameterTypes &&...parameters) noexcept :
    m_parameters(std::forward<ParameterTypes>(parameters)...)
{
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
StringType BasicStringFormatter<StringType, ParameterTypes...>::format(FormatString &&fmt)
{
    auto formatter = [this](auto &&specifier, const auto &value)
    {
        this->format_value(std::move(specifier), value);
    };

    const view_type view = fmt.view();
    m_buffer.reserve(view.size() * 2);

    for (std::size_t pos = 0; pos < view.size();)
    {
        switch (const auto &ch = view[pos])
        {
            case s_left_brace:
                if (view[pos + 1] == s_left_brace)
                {
                    m_buffer.push_back(ch);
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
                m_buffer.push_back(ch);
                pos += 2;
                break;

            default:
                m_buffer.push_back(ch);
                ++pos;
                break;
        }
    }

    return std::move(m_buffer);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <
    typename T,
    fly::disable_if_any<detail::is_like_supported_string<T>, std::is_arithmetic<T>>>
inline void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    const T &value)
{
    s_stream << value;
    format_value(std::move(specifier), s_stream.str());
    s_stream.str({});
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<detail::is_like_supported_string<T>>>
inline void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    const T &value)
{
    using string_like_type = detail::is_like_supported_string_t<T>;

    const std::size_t min_width = resolve_size(specifier.m_width);
    const std::size_t max_width = resolve_size(specifier.m_precision, StringType::npos);

    const std::size_t actual_size = BasicStringClassifier<string_like_type>::size(value);
    const std::size_t value_size = std::min(max_width, actual_size);

    const std::size_t padding_size = std::max(value_size, min_width) - value_size;
    const auto padding_char = specifier.m_fill ? *specifier.m_fill : s_space;

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
        case FormatSpecifier::Alignment::Default:
            append_string(value, max_width);
            m_buffer.append(padding_size, padding_char);
            break;

        case FormatSpecifier::Alignment::Right:
            m_buffer.append(padding_size, padding_char);
            append_string(value, max_width);
            break;

        case FormatSpecifier::Alignment::Center:
        {
            const std::size_t left_padding = padding_size / 2;
            const std::size_t right_padding =
                (padding_size % 2 == 0) ? left_padding : left_padding + 1;

            m_buffer.append(left_padding, padding_char);
            append_string(value, max_width);
            m_buffer.append(right_padding, padding_char);
            break;
        }
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<is_format_integral<T>>>
inline void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    T value)
{
    if constexpr (std::is_signed_v<T>)
    {
        using unsigned_type = std::make_unsigned_t<std::remove_cvref_t<T>>;

        // Compute the absolute value of the integer. Benchmarks showed this is exactly as fast as
        // std::abs, but this also tracks whether the original value was negative without branches.
        const T sign = value >> std::numeric_limits<T>::digits;
        value ^= sign;
        value += sign & 1;

        format_value(std::move(specifier), static_cast<unsigned_type>(value), sign);
    }
    else
    {
        format_value(std::move(specifier), value, false);
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<is_format_integral<T>>>
void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    T value,
    bool is_negative)
{
    const std::size_t original_size = m_buffer.size();
    std::size_t prefix_size = 0;
    std::size_t value_size = 0;

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        // TODO: Validate the value fits into char_type / convert Unicode encoding.
        m_buffer.push_back(static_cast<char_type>(value));
        value_size = 1;
    }
    else
    {
        const bool is_upper_case = specifier.m_case == FormatSpecifier::Case::Upper;
        const int base = static_cast<int>(specifier.m_type);

        if (is_negative)
        {
            m_buffer.push_back(s_minus_sign);
        }
        else if (specifier.m_sign == FormatSpecifier::Sign::Always)
        {
            m_buffer.push_back(s_plus_sign);
        }
        else if (specifier.m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding)
        {
            m_buffer.push_back(s_space);
        }

        if (specifier.m_alternate_form)
        {
            m_buffer.push_back(s_zero);

            if (specifier.m_type == FormatSpecifier::Type::Binary)
            {
                m_buffer.push_back(is_upper_case ? s_upper_b : s_lower_b);
            }
            else if (specifier.m_type == FormatSpecifier::Type::Hex)
            {
                m_buffer.push_back(is_upper_case ? s_upper_x : s_lower_x);
            }
        }

        prefix_size = m_buffer.size() - original_size;
        value_size = append_number(value, base) + prefix_size;

        if ((specifier.m_type == FormatSpecifier::Type::Hex) && is_upper_case)
        {
            for (std::size_t i = original_size + prefix_size; i < m_buffer.size(); ++i)
            {
                m_buffer[i] = classifier::to_upper(m_buffer[i]);
            }
        }
    }

    const std::size_t width = resolve_size(specifier.m_width);
    const std::size_t padding_size = std::max(value_size, width) - value_size;
    const char_type padding_char = specifier.m_fill ? *specifier.m_fill : s_space;

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
            m_buffer.append(padding_size, padding_char);
            break;

        case FormatSpecifier::Alignment::Right:
            m_buffer.insert(original_size, padding_size, padding_char);
            break;

        case FormatSpecifier::Alignment::Center:
        {
            const std::size_t left_padding = padding_size / 2;
            const std::size_t right_padding =
                (padding_size % 2 == 0) ? left_padding : left_padding + 1;

            m_buffer.insert(original_size, left_padding, padding_char);
            m_buffer.append(right_padding, padding_char);
            break;
        }

        case FormatSpecifier::Alignment::Default:
            if (specifier.m_zero_padding)
            {
                m_buffer.insert(original_size + prefix_size, padding_size, s_zero);
            }
            else
            {
                m_buffer.insert(original_size, padding_size, padding_char);
            }
            break;
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<std::is_floating_point<T>>>
inline void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    const T &value)
{
    stream_modifiers modifiers(s_stream);

    if (specifier.m_fill)
    {
        modifiers.fill(static_cast<streamed_char_type>(*specifier.m_fill));
    }

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
            modifiers.setf(std::ios_base::left);
            break;

        case FormatSpecifier::Alignment::Right:
            modifiers.setf(std::ios_base::right);
            break;

        case FormatSpecifier::Alignment::Center: // TODO: Implement center-alignment.
        case FormatSpecifier::Alignment::Default:
            modifiers.setf(specifier.is_numeric() ? std::ios_base::right : std::ios_base::left);
            break;
    }

    switch (specifier.m_sign)
    {
        case FormatSpecifier::Sign::Always:
            modifiers.setf(std::ios_base::showpos);
            break;

        case FormatSpecifier::Sign::NegativeOnlyWithPositivePadding:
            modifiers.template locale<PositivePaddingFacet<streamed_char_type>>();
            modifiers.setf(std::ios_base::showpos);
            break;

        default:
            break;
    }

    if (specifier.m_alternate_form)
    {
        modifiers.setf(std::ios_base::showpoint);
    }

    if (specifier.m_zero_padding)
    {
        modifiers.setf(std::ios_base::internal, std::ios_base::adjustfield);
        modifiers.fill(static_cast<streamed_char_type>(s_zero));
    }

    modifiers.width(resolve_size<std::streamsize>(specifier.m_width));
    modifiers.precision(resolve_size<std::streamsize>(specifier.m_precision, 6));

    switch (specifier.m_type)
    {
        case FormatSpecifier::Type::HexFloat:
            modifiers.setf(std::ios_base::fixed | std::ios_base::scientific);
            break;

        case FormatSpecifier::Type::Scientific:
            modifiers.setf(std::ios_base::scientific, std::ios::floatfield);
            break;

        case FormatSpecifier::Type::Fixed:
            // Only Apple's Clang seems to respect std::uppercase with std::fixed values. To
            // ensure consistency, format these values as general types.
            if (!std::isnan(value) && !std::isinf(value))
            {
                modifiers.setf(std::ios_base::fixed, std::ios::floatfield);
            }
            break;

        default:
            break;
    }

    if (specifier.m_case == FormatSpecifier::Case::Upper)
    {
        modifiers.setf(std::ios_base::uppercase);
    }

    s_stream << value;
    append_string(s_stream.str());
    s_stream.str({});
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<std::is_same<T, bool>>>
inline void BasicStringFormatter<StringType, ParameterTypes...>::format_value(
    FormatSpecifier &&specifier,
    T value)
{
    if (specifier.m_type == FormatSpecifier::Type::String)
    {
        format_value(std::move(specifier), value ? s_true : s_false);
    }
    else
    {
        format_value(std::move(specifier), static_cast<unsigned>(value));
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<detail::is_like_supported_string<T>>>
void BasicStringFormatter<StringType, ParameterTypes...>::append_string(
    const T &value,
    std::size_t max_width)
{
    using string_like_type = detail::is_like_supported_string_t<T>;
    using view_like_type = std::basic_string_view<typename string_like_type::value_type>;

    if constexpr (std::is_same_v<StringType, string_like_type>)
    {
        m_buffer.append(value, 0, max_width);
    }
    else
    {
        using unicode = BasicStringUnicode<string_like_type>;
        view_like_type view(value);

        auto it = view.cbegin();
        const auto end = view.cend();

        if (auto converted = unicode::template convert_encoding<StringType>(it, end); converted)
        {
            m_buffer.append(*std::move(converted), 0, max_width);
        }
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<is_format_integral<T>>>
std::size_t BasicStringFormatter<StringType, ParameterTypes...>::append_number(T value, int base)
{
    const std::size_t digits = count_digits(value, base);

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        m_buffer.resize(m_buffer.size() + digits);

        char *end = m_buffer.data() + m_buffer.size();
        char *begin = end - digits;

        std::to_chars(begin, end, value, base);
    }
    else
    {
        static thread_local std::array<char, std::numeric_limits<std::uintmax_t>::digits> s_buffer;
        using unicode = BasicStringUnicode<std::string>;

        char *begin = s_buffer.data();
        char *end = begin + s_buffer.size();

        const auto result = std::to_chars(begin, end, value, base);

        // TODO: Support something like "convert_encoding_into" to avoid a string copy.
        auto converted = unicode::template convert_encoding<StringType>(begin, result.ptr);
        m_buffer.append(*converted);
    }

    return digits;
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T>
inline T BasicStringFormatter<StringType, ParameterTypes...>::resolve_size(
    const std::optional<typename FormatSpecifier::SizeOrPosition> &size_or_position,
    T fallback) const
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

    return fallback;
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, fly::enable_if<is_format_integral<T>>>
inline std::size_t
BasicStringFormatter<StringType, ParameterTypes...>::count_digits(T value, int base)
{
    std::size_t digits = 0;

    do
    {
        ++digits;
    } while ((value /= static_cast<T>(base)) != 0);

    return digits;
}

} // namespace fly::detail
