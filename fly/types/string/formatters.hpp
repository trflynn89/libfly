#pragma once

#include "fly/concepts/concepts.hpp"
#include "fly/fly.hpp"
#include "fly/types/string/concepts.hpp"
#include "fly/types/string/detail/classifier.hpp"
#include "fly/types/string/detail/format_specifier.hpp"
#include "fly/types/string/detail/stream_util.hpp"
#include "fly/types/string/detail/unicode.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>
#include <system_error>
#include <type_traits>

namespace fly {

/**
 * Class to define formatting rules for a given type. Enabled specializations must define the
 * following member template function:
 *
 *     template <typename FormatContext>
 *     void format(const T &value, FormatContext &context);
 *
 * Where the FormatContext is a structure holding the formatting state.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename T, StandardCharacter CharType = char>
struct Formatter;

//==================================================================================================
template <FormattableString T, StandardCharacter CharType>
struct Formatter<T, CharType> : public detail::BasicFormatSpecifier<CharType>
{
    FLY_DEFINE_FORMATTER(CharType, detail::ParameterType::String)

    /**
     * Format a single replacement field with the provided string-like value.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(const T &value, FormatContext &context)
    {
        const std::size_t min_width = FormatSpecifier::width(context, 0);
        const std::size_t max_width = FormatSpecifier::precision(context, string_type::npos);

        const std::size_t actual_size = detail::BasicClassifier<CharType>::size(value);
        const std::size_t value_size = std::min(max_width, actual_size);

        const std::size_t padding_size = std::max(value_size, min_width) - value_size;
        const auto padding_char = m_fill.value_or(s_space);

        auto append_padding = [&context, padding_char](std::size_t count)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                *context.out()++ = padding_char;
            }
        };

        switch (m_alignment)
        {
            case FormatSpecifier::Alignment::Left:
            case FormatSpecifier::Alignment::Default:
                append_string(value, value_size, context);
                append_padding(padding_size);
                break;

            case FormatSpecifier::Alignment::Right:
                append_padding(padding_size);
                append_string(value, value_size, context);
                break;

            case FormatSpecifier::Alignment::Center:
            {
                const std::size_t left_padding = padding_size / 2;
                const std::size_t right_padding =
                    (padding_size % 2 == 0) ? left_padding : left_padding + 1;

                append_padding(left_padding);
                append_string(value, value_size, context);
                append_padding(right_padding);
                break;
            }
        }
    }

    /**
     * Append a string-like value to the buffer.
     *
     * If the string-like value's character type is the same as the format string, the value is
     * inserted directly. Otherwise, it is first transcoded to the appropriate Unicode encoding. If
     * transcoding fails, the value is dropped.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to append.
     * @param value_size The size of the value to append.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    static void append_string(const T &value, std::size_t value_size, FormatContext &context)
    {
        using standard_character_type = StandardCharacterType<T>;
        using standard_view_type = std::basic_string_view<standard_character_type>;

        standard_view_type view;

        if constexpr (std::is_array_v<T> || std::is_pointer_v<T>)
        {
            view = standard_view_type(value, value_size);
        }
        else
        {
            view = standard_view_type(value).substr(0, value_size);
        }

        if constexpr (fly::SameAs<CharType, standard_character_type>)
        {
            for (const auto &ch : view)
            {
                *context.out()++ = ch;
            }
        }
        else
        {
            using unicode = detail::BasicUnicode<standard_character_type>;

            if (auto converted = unicode::template convert_encoding<string_type>(view); converted)
            {
                for (const auto &ch : *converted)
                {
                    *context.out()++ = ch;
                }
            }
        }
    }

private:
    using string_type = std::basic_string<CharType>;

    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
};

//==================================================================================================
template <FormattablePointer T, StandardCharacter CharType>
struct Formatter<T, CharType> : public detail::BasicFormatSpecifier<CharType>
{
    FLY_DEFINE_FORMATTER(CharType, detail::ParameterType::Pointer)

    /**
     * Format a single replacement field with the provided pointer value.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    inline void format(T value, FormatContext &context)
    {
        m_alternate_form = true;
        m_type = FormatSpecifier::Type::Hex;

        Formatter<std::uintptr_t, CharType> formatter(*this);
        formatter.format(
            reinterpret_cast<std::uintptr_t>(static_cast<const void *>(value)),
            context);
    }
};

//==================================================================================================
template <FormattableIntegral T, StandardCharacter CharType>
struct Formatter<T, CharType> : public detail::BasicFormatSpecifier<CharType>
{
    FLY_DEFINE_FORMATTER(CharType, detail::ParameterType::Integral)

    /**
     * Format a single replacement field with the provided non-boolean integral value.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    inline void format(T value, FormatContext &context)
    {
        if constexpr (std::is_signed_v<T>)
        {
            using U = std::make_unsigned_t<std::remove_cvref_t<T>>;

            // Compute the absolute value of the integer. Benchmarks showed this is exactly as fast
            // as std::abs, but this also tracks whether the original value was negative without
            // branches.
            const T sign = value >> std::numeric_limits<T>::digits;
            const U unsigned_value = static_cast<U>(static_cast<U>(value ^ sign) + (sign & 1));

            format(unsigned_value, static_cast<bool>(sign), context);
        }
        else
        {
            format(value, false, context);
        }
    }

private:
    using string_type = std::basic_string<CharType>;

    /**
     * Format a single replacement field with the provided unsigned, non-boolean integral value.
     *
     * @tparam U The type of the value to format.
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param is_negative Whether the original value was negative.
     * @param context The context holding the formatting state.
     */
    template <typename U, typename FormatContext>
    void format(U value, bool is_negative, FormatContext &context)
    {
        static_assert(std::is_unsigned_v<U>);

        if (m_type == FormatSpecifier::Type::Character)
        {
            format_as_character(value, is_negative, context);
            return;
        }

        std::size_t prefix_size = 0;

        if (is_negative || (m_sign == FormatSpecifier::Sign::Always) ||
            (m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding))
        {
            ++prefix_size;
        }
        if (m_alternate_form)
        {
            ++prefix_size;

            if ((m_type == FormatSpecifier::Type::Binary) || (m_type == FormatSpecifier::Type::Hex))
            {
                ++prefix_size;
            }
        }

        const int base = static_cast<int>(m_type);
        const std::size_t value_size = count_digits(value, base) + prefix_size;

        const std::size_t width = FormatSpecifier::width(context, 0);
        const std::size_t padding_size = std::max(value_size, width) - value_size;
        const auto padding_char = m_fill.value_or(s_space);

        auto append_prefix = [this, is_negative, &context]()
        {
            if (is_negative)
            {
                *context.out()++ = s_minus_sign;
            }
            else if (m_sign == FormatSpecifier::Sign::Always)
            {
                *context.out()++ = s_plus_sign;
            }
            else if (m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding)
            {
                *context.out()++ = s_space;
            }

            if (m_alternate_form)
            {
                const bool is_upper_case = m_case == FormatSpecifier::Case::Upper;
                *context.out()++ = s_zero;

                if (m_type == FormatSpecifier::Type::Binary)
                {
                    *context.out()++ = is_upper_case ? s_upper_b : s_lower_b;
                }
                else if (m_type == FormatSpecifier::Type::Hex)
                {
                    *context.out()++ = is_upper_case ? s_upper_x : s_lower_x;
                }
            }
        };

        auto append_padding = [&context](std::size_t count, CharType pad)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                *context.out()++ = pad;
            }
        };

        switch (m_alignment)
        {
            case FormatSpecifier::Alignment::Left:
                append_prefix();
                append_number(value, base, context);
                append_padding(padding_size, padding_char);
                break;

            case FormatSpecifier::Alignment::Right:
                append_padding(padding_size, padding_char);
                append_prefix();
                append_number(value, base, context);
                break;

            case FormatSpecifier::Alignment::Center:
            {
                const std::size_t left_padding = padding_size / 2;
                const std::size_t right_padding =
                    (padding_size % 2 == 0) ? left_padding : left_padding + 1;

                append_padding(left_padding, padding_char);
                append_prefix();
                append_number(value, base, context);
                append_padding(right_padding, padding_char);
                break;
            }

            case FormatSpecifier::Alignment::Default:
                if (m_zero_padding)
                {
                    append_prefix();
                    append_padding(padding_size, s_zero);
                    append_number(value, base, context);
                }
                else
                {
                    append_padding(padding_size, padding_char);
                    append_prefix();
                    append_number(value, base, context);
                }
                break;
        }
    }

    /**
     * Format a single replacement field as a character with the provided unsigned, non-boolean
     * integral value.
     *
     * If the value does not fit into the bounds of CharType, it is dropped.
     *
     * @tparam U The type of the value to format.
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param is_negative Whether the original value was negative.
     * @param context The context holding the formatting state.
     */
    template <typename U, typename FormatContext>
    void format_as_character(U value, bool is_negative, FormatContext &context)
    {
        static_assert(std::is_unsigned_v<U>);

        if (is_negative || (value > static_cast<U>(std::numeric_limits<CharType>::max())))
        {
            return;
        }

        const std::size_t width = FormatSpecifier::width(context, 0);
        const std::size_t padding_size = width > 1 ? width - 1 : 0;
        const auto padding_char = m_fill.value_or(s_space);

        auto append_padding = [&context, padding_char](std::size_t count)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                *context.out()++ = padding_char;
            }
        };

        switch (m_alignment)
        {
            case FormatSpecifier::Alignment::Left:
                *context.out()++ = static_cast<CharType>(value);
                append_padding(padding_size);
                break;

            case FormatSpecifier::Alignment::Right:
                append_padding(padding_size);
                *context.out()++ = static_cast<CharType>(value);
                break;

            case FormatSpecifier::Alignment::Center:
            {
                const std::size_t left_padding = padding_size / 2;
                const std::size_t right_padding =
                    (padding_size % 2 == 0) ? left_padding : left_padding + 1;

                append_padding(left_padding);
                *context.out()++ = static_cast<CharType>(value);
                append_padding(right_padding);
                break;
            }

            case FormatSpecifier::Alignment::Default:
                append_padding(padding_size);
                *context.out()++ = static_cast<CharType>(value);
                break;
        }
    }

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
     * @tparam U The type of the value to append.
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to append.
     * @param base The base of the value.
     * @param context The context holding the formatting state.
     */
    template <typename U, typename FormatContext>
    void append_number(U value, int base, FormatContext &context)
    {
        static thread_local std::array<char, std::numeric_limits<std::uintmax_t>::digits> s_buffer;

        char *begin = s_buffer.data();
        char *end = begin + s_buffer.size();

        const auto result = std::to_chars(begin, end, value, base);

        if ((m_type == FormatSpecifier::Type::Hex) && (m_case == FormatSpecifier::Case::Upper))
        {
            for (char *it = begin; it != result.ptr; ++it)
            {
                *it = detail::BasicClassifier<char>::to_upper(*it);
            }
        }

        if constexpr (fly::SameAs<string_type, std::string>)
        {
            for (const char *it = begin; it != result.ptr; ++it)
            {
                *context.out()++ = *it;
            }
        }
        else
        {
            using unicode = detail::BasicUnicode<char>;

            std::string_view view(
                begin,
                static_cast<std::size_t>(std::distance(begin, result.ptr)));

            unicode::template convert_encoding_into<string_type>(view, context.out());
        }
    }

    /**
     * Count the number of base-N digits in a value, where N is the provided integer base.
     *
     * @tparam U The type of the value.
     *
     * @param value The value to count digits in.
     * @param base The base of the value.
     *
     * @return The number of base-N digits in the value.
     */
    template <typename U>
    static constexpr std::size_t count_digits(U value, int base)
    {
        std::size_t digits = 0;

        do
        {
            ++digits;
        } while ((value /= static_cast<U>(base)) != 0);

        return digits;
    }

    static constexpr const auto s_plus_sign = FLY_CHR(CharType, '+');
    static constexpr const auto s_minus_sign = FLY_CHR(CharType, '-');
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
    static constexpr const auto s_lower_b = FLY_CHR(CharType, 'b');
    static constexpr const auto s_upper_b = FLY_CHR(CharType, 'B');
    static constexpr const auto s_lower_x = FLY_CHR(CharType, 'x');
    static constexpr const auto s_upper_x = FLY_CHR(CharType, 'X');
};

//==================================================================================================
template <FormattableFloatingPoint T, StandardCharacter CharType>
struct Formatter<T, CharType> : public detail::BasicFormatSpecifier<CharType>
{
    FLY_DEFINE_FORMATTER(CharType, detail::ParameterType::FloatingPoint)

#if defined(FLY_COMPILER_SUPPORTS_FP_CHARCONV)
    /**
     * Format a single replacement field with the provided floating-point value.
     *
     * Internally, std::to_chars is used for the conversion. Since std::to_chars only supports
     * char-based strings, this method behaves differently depending on the type of the format
     * string. If the format string is char-based, the conversion is applied directly to the buffer.
     * Otherwise, an intermediate char-based buffer is used for the conversion, then that buffer is
     * transcoded to the appropriate Unicode encoding, incurring extra string copying.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(T value, FormatContext &context)
    {
        const bool is_negative = std::signbit(value);
        value = std::abs(value);

        std::size_t prefix_size = 0;

        if (is_negative || (m_sign == FormatSpecifier::Sign::Always) ||
            (m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding))
        {
            ++prefix_size;
        }

        const int precision = static_cast<int>(FormatSpecifier::precision(context, 6));
        const FloatConversionResult result = convert_value(value, precision);

        auto append_prefix = [this, &is_negative, &context]()
        {
            if (is_negative)
            {
                *context.out()++ = s_minus_sign;
            }
            else if (m_sign == FormatSpecifier::Sign::Always)
            {
                *context.out()++ = s_plus_sign;
            }
            else if (m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding)
            {
                *context.out()++ = s_space;
            }
        };

        auto append_padding = [&context](std::size_t count, CharType pad)
        {
            for (std::size_t i = 0; i < count; ++i)
            {
                *context.out()++ = pad;
            }
        };

        auto append_number = [this, &context, &result]()
        {
            if constexpr (fly::SameAs<string_type, std::string>)
            {
                for (auto ch : result.m_digits)
                {
                    *context.out()++ = ch;
                }
                if (result.m_append_decimal)
                {
                    *context.out()++ = '.';
                }
                for (std::size_t i = 0; i < result.m_zeroes_to_append; ++i)
                {
                    *context.out()++ = '0';
                }
                for (auto ch : result.m_exponent)
                {
                    *context.out()++ = ch;
                }
            }
            else
            {
                using unicode = detail::BasicUnicode<char>;

                unicode::template convert_encoding_into<string_type>(
                    result.m_digits,
                    context.out());

                if (result.m_append_decimal)
                {
                    *context.out()++ = FLY_CHR(CharType, '.');
                }
                for (std::size_t i = 0; i < result.m_zeroes_to_append; ++i)
                {
                    *context.out()++ = FLY_CHR(CharType, '0');
                }

                unicode::template convert_encoding_into<string_type>(
                    result.m_exponent,
                    context.out());
            }
        };

        const std::size_t value_size = prefix_size + result.m_digits.size() +
            result.m_exponent.size() + static_cast<std::size_t>(result.m_append_decimal) +
            result.m_zeroes_to_append;
        const std::size_t width = FormatSpecifier::width(context, 0);
        const std::size_t padding_size = std::max(value_size, width) - value_size;
        const auto padding_char = m_fill.value_or(s_space);

        switch (m_alignment)
        {
            case FormatSpecifier::Alignment::Left:
                append_prefix();
                append_number();
                append_padding(padding_size, padding_char);
                break;

            case FormatSpecifier::Alignment::Right:
                append_padding(padding_size, padding_char);
                append_prefix();
                append_number();
                break;

            case FormatSpecifier::Alignment::Center:
            {
                const std::size_t left_padding = padding_size / 2;
                const std::size_t right_padding =
                    (padding_size % 2 == 0) ? left_padding : left_padding + 1;

                append_padding(left_padding, padding_char);
                append_prefix();
                append_number();
                append_padding(right_padding, padding_char);
                break;
            }

            case FormatSpecifier::Alignment::Default:
                if (m_zero_padding)
                {
                    append_prefix();
                    append_padding(padding_size, s_zero);
                    append_number();
                }
                else
                {
                    append_padding(padding_size, padding_char);
                    append_prefix();
                    append_number();
                }
                break;
        }
    }
#else
    /**
     * Format a single replacement field with the provided floating-point value.
     *
     * Internally, a combination of an IO stream and the fly::Formatter<std::basic_string_view>
     * specialization is used to format the value.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(T value, FormatContext &context)
    {
        static thread_local std::stringstream s_stream;
        detail::ScopedStreamModifiers modifiers(s_stream);

        if (m_alignment == FormatSpecifier::Alignment::Default)
        {
            m_alignment = FormatSpecifier::Alignment::Right;
        }

        switch (m_sign)
        {
            case FormatSpecifier::Sign::Always:
                modifiers.setf(std::ios_base::showpos);
                break;

            case FormatSpecifier::Sign::NegativeOnlyWithPositivePadding:
                modifiers.template locale<detail::PositivePaddingFacet<char>>();
                modifiers.setf(std::ios_base::showpos);
                break;

            default:
                break;
        }

        if (m_alternate_form)
        {
            modifiers.setf(std::ios_base::showpoint);
        }

        if (m_zero_padding)
        {
            modifiers.setf(std::ios_base::internal, std::ios_base::adjustfield);
            modifiers.fill(static_cast<char>(s_zero));
            modifiers.width(static_cast<std::streamsize>(FormatSpecifier::width(context, 0)));
        }

        modifiers.precision(static_cast<std::streamsize>(FormatSpecifier::precision(context, 6)));
        m_precision = std::nullopt;
        m_precision_position = std::nullopt;

        switch (m_type)
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

        if (m_case == FormatSpecifier::Case::Upper)
        {
            modifiers.setf(std::ios_base::uppercase);
        }

        s_stream << value;

        Formatter<std::string_view, CharType> formatter(*this);
        formatter.format(s_stream.str(), context);

        s_stream.str({});
    }
#endif

private:
    using string_type = std::basic_string<CharType>;

#if defined(FLY_COMPILER_SUPPORTS_FP_CHARCONV)

    /**
     * Structure to hold the information needed to fully format a floating-point value as a string.
     */
    struct FloatConversionResult
    {
        std::string_view m_digits;
        std::string_view m_exponent;
        bool m_append_decimal {false};
        std::size_t m_zeroes_to_append {0};
    };

    /**
     * Convert a floating-point value to a string.
     *
     * Internally, std::to_chars is used for the conversion, which does not handle all
     * floating-point formatting options, such as alternate form. So rather than creating a
     * fully-formatted string, this method returns a structure holding the information needed to
     * format the value as a string.
     *
     * @param value The value to convert.
     * @param precision The floating-point precision to use.
     *
     * @return A structure holding the information needed to fully format the value as a string.
     */
    FloatConversionResult convert_value(T value, int precision)
    {
        static thread_local std::array<char, std::numeric_limits<T>::digits> s_buffer;

        char *begin = s_buffer.data();
        char *end = begin + s_buffer.size();

        std::chars_format fmt = std::chars_format::general;
        char exponent = '\0';

        switch (m_type)
        {
            case FormatSpecifier::Type::HexFloat:
                fmt = std::chars_format::hex;
                exponent = 'p';
                break;
            case FormatSpecifier::Type::Scientific:
                fmt = std::chars_format::scientific;
                exponent = 'e';
                break;
            case FormatSpecifier::Type::Fixed:
                fmt = std::chars_format::fixed;
                break;
            default:
                exponent = 'e';
                break;
        }

        const auto to_chars_result = std::to_chars(begin, end, value, fmt, precision);

        FloatConversionResult conversion_result;
        conversion_result.m_digits =
            std::string_view(begin, static_cast<std::size_t>(to_chars_result.ptr - begin));

        if (m_alternate_form)
        {
            conversion_result.m_append_decimal = true;

            for (const char *it = begin; it != to_chars_result.ptr; ++it)
            {
                if (*it == '.')
                {
                    conversion_result.m_append_decimal = false;
                }
                else if (*it == exponent)
                {
                    const auto position = static_cast<std::size_t>(it - begin);

                    conversion_result.m_exponent = conversion_result.m_digits.substr(position);
                    conversion_result.m_digits = conversion_result.m_digits.substr(0, position);
                }
            }

            if (m_type == FormatSpecifier::Type::General)
            {
                const auto digits = conversion_result.m_digits.size() -
                    static_cast<std::size_t>(!conversion_result.m_append_decimal);

                if (static_cast<std::size_t>(precision) > digits)
                {
                    conversion_result.m_zeroes_to_append =
                        static_cast<std::size_t>(precision) - digits;
                }
            }
        }

        if (m_case == FormatSpecifier::Case::Upper)
        {
            for (char *it = begin; it != to_chars_result.ptr; ++it)
            {
                *it = detail::BasicClassifier<char>::to_upper(*it);
            }
        }

        return conversion_result;
    }

#endif // FLY_COMPILER_SUPPORTS_FP_CHARCONV

    static constexpr const auto s_plus_sign = FLY_CHR(CharType, '+');
    static constexpr const auto s_minus_sign = FLY_CHR(CharType, '-');
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
};

//==================================================================================================
template <FormattableBoolean T, StandardCharacter CharType>
struct Formatter<T, CharType> : public detail::BasicFormatSpecifier<CharType>
{
    FLY_DEFINE_FORMATTER(CharType, detail::ParameterType::Boolean)

    /**
     * Format a single replacement field with the provided boolean value.
     *
     * @tparam FormatContext The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    inline void format(T value, FormatContext &context)
    {
        if (m_type == FormatSpecifier::Type::String)
        {
            Formatter<std::basic_string_view<CharType>, CharType> formatter(*this);
            formatter.format(value ? s_true : s_false, context);
        }
        else
        {
            Formatter<unsigned, CharType> formatter(*this);
            formatter.format(static_cast<unsigned>(value), context);
        }
    }

private:
    static constexpr const CharType *s_true = FLY_STR(CharType, "true");
    static constexpr const CharType *s_false = FLY_STR(CharType, "false");
};

} // namespace fly
