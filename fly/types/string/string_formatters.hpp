#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_format_specifier.hpp"
#include "fly/types/string/detail/string_stream_util.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/detail/string_unicode.hpp"

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
template <typename T, typename CharType = char, typename SFINAE = bool>
struct Formatter;

template <typename T, typename CharType>
struct Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_generic<T>>>
{
    /**
     * Format a single replacement field with the provided generic value.
     *
     * Currently, rather than supporting a set of std::formatter specializations, the generic value
     * will be converted to a string via the streaming operator. The resulting string will then be
     * formatted using the string formatting overload.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(const T &value, FormatContext &context);
};

template <typename T, typename CharType>
struct Formatter<T, CharType, fly::enable_if<detail::is_like_supported_string<T>>>
{
    /**
     * Format a single replacement field with the provided string-like value.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(const T &value, FormatContext &context);

    /**
     * Append a string-like value to the buffer.
     *
     * If the string-like value's character type is the same as the format string, the value is
     * inserted directly. Otherwise, it is first transcoded to the appropriate Unicode encoding. If
     * transcoding fails, the value is dropped.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to append.
     * @param value_size The size of the value to append.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void append_string(const T &value, std::size_t value_size, FormatContext &context);

private:
    using string_type = std::basic_string<CharType>;
    using string_like_type = detail::is_like_supported_string_t<T>;
    using view_like_type = std::basic_string_view<typename string_like_type::value_type>;
    using specifier = detail::BasicFormatSpecifier<CharType>;

    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
};

template <typename T, typename CharType>
struct Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_pointer<T>>>
{
    /**
     * Format a single replacement field with the provided pointer value.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(T value, FormatContext &context);

private:
    using specifier = detail::BasicFormatSpecifier<CharType>;
};

template <typename T, typename CharType>
struct Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>
{
    /**
     * Format a single replacement field with the provided non-boolean integral value.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(T value, FormatContext &context);

private:
    using string_type = std::basic_string<CharType>;
    using specifier = detail::BasicFormatSpecifier<CharType>;

    /**
     * Format a single replacement field with the provided unsigned, non-boolean integral value.
     *
     * @tparam U The type of the value to format.
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param is_negative Whether the original value was negative.
     * @param context The context holding the formatting state.
     */
    template <typename U, typename FormatContext>
    void format(U value, bool is_negative, FormatContext &context);

    /**
     * Format a single replacement field as a character with the provided unsigned, non-boolean
     * integral value.
     *
     * If the value does not fit into the bounds of CharType, it is dropped.
     *
     * @tparam U The type of the value to format.
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param is_negative Whether the original value was negative.
     * @param context The context holding the formatting state.
     */
    template <typename U, typename FormatContext>
    void format_as_character(U value, bool is_negative, FormatContext &context);

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
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to append.
     * @param base The base of the value.
     * @param context The context holding the formatting state.
     *
     * @return The number of base-N digits converted.
     */
    template <typename U, typename FormatContext>
    void append_number(U value, int base, FormatContext &context);

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
    static std::size_t count_digits(U value, int base);

    static constexpr const auto s_plus_sign = FLY_CHR(CharType, '+');
    static constexpr const auto s_minus_sign = FLY_CHR(CharType, '-');
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
    static constexpr const auto s_lower_b = FLY_CHR(CharType, 'b');
    static constexpr const auto s_upper_b = FLY_CHR(CharType, 'B');
    static constexpr const auto s_lower_x = FLY_CHR(CharType, 'x');
    static constexpr const auto s_upper_x = FLY_CHR(CharType, 'X');
};

template <typename T, typename CharType>
struct Formatter<T, CharType, fly::enable_if<std::is_floating_point<T>>>
{
    /**
     * Format a single replacement field with the provided floating point value.
     *
     * Currently, major compilers do not support std::to_chars for floating point values. Until they
     * do, this implementation uses an IO stream to format the value.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(const T &value, FormatContext &context);

private:
    using string_type = std::basic_string<CharType>;
    using stream_modifiers = detail::BasicStreamModifiers<string_type>;
    using specifier = detail::BasicFormatSpecifier<CharType>;

    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
};

template <typename T, typename CharType>
struct Formatter<T, CharType, fly::enable_if<std::is_same<T, bool>>>
{
    /**
     * Format a single replacement field with the provided boolean value.
     *
     * @tparam FormatParameter The type of the formatting context.
     *
     * @param value The value to format.
     * @param context The context holding the formatting state.
     */
    template <typename FormatContext>
    void format(T value, FormatContext &context);

private:
    using string_type = std::basic_string<CharType>;
    using specifier = detail::BasicFormatSpecifier<CharType>;

    static constexpr const CharType *s_true = FLY_STR(CharType, "true");
    static constexpr const CharType *s_false = FLY_STR(CharType, "false");
};

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_generic<T>>>::format(
    const T &value,
    FormatContext &context)
{
    if constexpr (detail::OstreamTraits::is_declared_v<T>)
    {
        static thread_local std::stringstream s_stream;

        s_stream << value;
        const auto formatted = s_stream.str();
        s_stream.str({});

        Formatter<std::string, CharType>().append_string(formatted, formatted.size(), context);
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
void Formatter<T, CharType, fly::enable_if<detail::is_like_supported_string<T>>>::format(
    const T &value,
    FormatContext &context)
{
    const std::size_t min_width = context.spec().width(context, 0);
    const std::size_t max_width = context.spec().precision(context, string_type::npos);

    const std::size_t actual_size = detail::BasicStringClassifier<string_like_type>::size(value);
    const std::size_t value_size = std::min(max_width, actual_size);

    const std::size_t padding_size = std::max(value_size, min_width) - value_size;
    const auto padding_char = context.spec().m_fill.value_or(s_space);

    auto append_padding = [&context, padding_char](std::size_t count) mutable
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *context.out()++ = padding_char;
        }
    };

    switch (context.spec().m_alignment)
    {
        case specifier::Alignment::Left:
        case specifier::Alignment::Default:
            append_string(value, value_size, context);
            append_padding(padding_size);
            break;

        case specifier::Alignment::Right:
            append_padding(padding_size);
            append_string(value, value_size, context);
            break;

        case specifier::Alignment::Center:
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

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
void Formatter<T, CharType, fly::enable_if<detail::is_like_supported_string<T>>>::append_string(
    const T &value,
    std::size_t value_size,
    FormatContext &context)
{
    view_like_type view;

    if constexpr (std::is_array_v<T> || std::is_pointer_v<T>)
    {
        view = view_like_type(value, value_size);
    }
    else
    {
        view = view_like_type(value).substr(0, value_size);
    }

    if constexpr (std::is_same_v<string_type, string_like_type>)
    {
        for (const auto &ch : view)
        {
            *context.out()++ = ch;
        }
    }
    else
    {
        using unicode = detail::BasicStringUnicode<string_like_type>;

        if (auto converted = unicode::template convert_encoding<string_type>(view); converted)
        {
            for (const auto &ch : *converted)
            {
                *context.out()++ = ch;
            }
        }
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_pointer<T>>>::format(
    T value,
    FormatContext &context)
{
    context.spec().m_alternate_form = true;
    context.spec().m_type = specifier::Type::Hex;

    Formatter<std::uintptr_t, CharType>().format(
        reinterpret_cast<std::uintptr_t>(static_cast<const void *>(value)),
        context);
}

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::format(
    T value,
    FormatContext &context)
{
    if constexpr (std::is_signed_v<T>)
    {
        using U = std::make_unsigned_t<std::remove_cvref_t<T>>;

        // Compute the absolute value of the integer. Benchmarks showed this is exactly as fast as
        // std::abs, but this also tracks whether the original value was negative without branches.
        const T sign = value >> std::numeric_limits<T>::digits;
        const U unsigned_value = static_cast<U>(static_cast<U>(value ^ sign) + (sign & 1));

        format(unsigned_value, static_cast<bool>(sign), context);
    }
    else
    {
        format(value, false, context);
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename U, typename FormatContext>
void Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::format(
    U value,
    bool is_negative,
    FormatContext &context)
{
    static_assert(std::is_unsigned_v<U>);

    if (context.spec().m_type == specifier::Type::Character)
    {
        format_as_character(value, is_negative, context);
        return;
    }

    std::size_t prefix_size = 0;

    if (is_negative || (context.spec().m_sign == specifier::Sign::Always) ||
        (context.spec().m_sign == specifier::Sign::NegativeOnlyWithPositivePadding))
    {
        ++prefix_size;
    }
    if (context.spec().m_alternate_form)
    {
        ++prefix_size;

        if ((context.spec().m_type == specifier::Type::Binary) ||
            (context.spec().m_type == specifier::Type::Hex))
        {
            ++prefix_size;
        }
    }

    const int base = static_cast<int>(context.spec().m_type);
    const std::size_t value_size = count_digits(value, base) + prefix_size;

    const std::size_t width = context.spec().width(context, 0);
    const std::size_t padding_size = std::max(value_size, width) - value_size;
    const auto padding_char = context.spec().m_fill.value_or(s_space);

    auto append_prefix = [is_negative, &context]()
    {
        if (is_negative)
        {
            *context.out()++ = s_minus_sign;
        }
        else if (context.spec().m_sign == specifier::Sign::Always)
        {
            *context.out()++ = s_plus_sign;
        }
        else if (context.spec().m_sign == specifier::Sign::NegativeOnlyWithPositivePadding)
        {
            *context.out()++ = s_space;
        }

        if (context.spec().m_alternate_form)
        {
            const bool is_upper_case = context.spec().m_case == specifier::Case::Upper;
            *context.out()++ = s_zero;

            if (context.spec().m_type == specifier::Type::Binary)
            {
                *context.out()++ = is_upper_case ? s_upper_b : s_lower_b;
            }
            else if (context.spec().m_type == specifier::Type::Hex)
            {
                *context.out()++ = is_upper_case ? s_upper_x : s_lower_x;
            }
        }
    };

    auto append_padding = [&context](std::size_t count, CharType pad) mutable
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *context.out()++ = pad;
        }
    };

    switch (context.spec().m_alignment)
    {
        case specifier::Alignment::Left:
            append_prefix();
            append_number(value, base, context);
            append_padding(padding_size, padding_char);
            break;

        case specifier::Alignment::Right:
            append_padding(padding_size, padding_char);
            append_prefix();
            append_number(value, base, context);
            break;

        case specifier::Alignment::Center:
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

        case specifier::Alignment::Default:
            if (context.spec().m_zero_padding)
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

//==================================================================================================
template <typename T, typename CharType>
template <typename U, typename FormatContext>
void Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::
    format_as_character(U value, bool is_negative, FormatContext &context)
{
    static_assert(std::is_unsigned_v<U>);

    if (is_negative || (value > static_cast<U>(std::numeric_limits<CharType>::max())))
    {
        return;
    }

    const std::size_t width = context.spec().width(context, 0);
    const std::size_t padding_size = width > 1 ? width - 1 : 0;
    const auto padding_char = context.spec().m_fill.value_or(s_space);

    auto append_padding = [&context, padding_char](std::size_t count) mutable
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *context.out()++ = padding_char;
        }
    };

    switch (context.spec().m_alignment)
    {
        case specifier::Alignment::Left:
            *context.out()++ = static_cast<CharType>(value);
            append_padding(padding_size);
            break;

        case specifier::Alignment::Right:
            append_padding(padding_size);
            *context.out()++ = static_cast<CharType>(value);
            break;

        case specifier::Alignment::Center:
        {
            const std::size_t left_padding = padding_size / 2;
            const std::size_t right_padding =
                (padding_size % 2 == 0) ? left_padding : left_padding + 1;

            append_padding(left_padding);
            *context.out()++ = static_cast<CharType>(value);
            append_padding(right_padding);
            break;
        }

        case specifier::Alignment::Default:
            append_padding(padding_size);
            *context.out()++ = static_cast<CharType>(value);
            break;
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename U, typename FormatContext>
void Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::
    append_number(U value, int base, FormatContext &context)
{
    static thread_local std::array<char, std::numeric_limits<std::uintmax_t>::digits> s_buffer;

    char *begin = s_buffer.data();
    char *end = begin + s_buffer.size();

    const auto result = std::to_chars(begin, end, value, base);

    if ((context.spec().m_type == specifier::Type::Hex) &&
        (context.spec().m_case == specifier::Case::Upper))
    {
        for (char *it = begin; it != result.ptr; ++it)
        {
            *it = detail::BasicStringClassifier<std::string>::to_upper(*it);
        }
    }

    if constexpr (std::is_same_v<string_type, std::string>)
    {
        for (char *it = begin; it != result.ptr; ++it)
        {
            *context.out()++ = *it;
        }
    }
    else
    {
        using unicode = detail::BasicStringUnicode<std::string>;
        std::string_view view(begin, static_cast<std::size_t>(std::distance(begin, result.ptr)));

        unicode::template convert_encoding_into<string_type>(view, context.out());
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename U>
inline std::size_t
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::count_digits(
    U value,
    int base)
{
    std::size_t digits = 0;

    do
    {
        ++digits;
    } while ((value /= static_cast<U>(base)) != 0);

    return digits;
}

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
void Formatter<T, CharType, fly::enable_if<std::is_floating_point<T>>>::format(
    const T &value,
    FormatContext &context)
{
    static thread_local std::stringstream s_stream;
    stream_modifiers modifiers(s_stream);

    if (context.spec().m_alignment == specifier::Alignment::Default)
    {
        context.spec().m_alignment = specifier::Alignment::Right;
    }

    switch (context.spec().m_sign)
    {
        case specifier::Sign::Always:
            modifiers.setf(std::ios_base::showpos);
            break;

        case specifier::Sign::NegativeOnlyWithPositivePadding:
            modifiers.template locale<detail::PositivePaddingFacet<char>>();
            modifiers.setf(std::ios_base::showpos);
            break;

        default:
            break;
    }

    if (context.spec().m_alternate_form)
    {
        modifiers.setf(std::ios_base::showpoint);
    }

    if (context.spec().m_zero_padding)
    {
        modifiers.setf(std::ios_base::internal, std::ios_base::adjustfield);
        modifiers.fill(static_cast<char>(s_zero));
        modifiers.width(static_cast<std::streamsize>(context.spec().width(context, 0)));
    }

    modifiers.precision(static_cast<std::streamsize>(context.spec().precision(context, 6)));
    context.spec().m_precision = std::nullopt;
    context.spec().m_precision_position = std::nullopt;

    switch (context.spec().m_type)
    {
        case specifier::Type::HexFloat:
            modifiers.setf(std::ios_base::fixed | std::ios_base::scientific);
            break;

        case specifier::Type::Scientific:
            modifiers.setf(std::ios_base::scientific, std::ios::floatfield);
            break;

        case specifier::Type::Fixed:
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

    if (context.spec().m_case == specifier::Case::Upper)
    {
        modifiers.setf(std::ios_base::uppercase);
    }

    s_stream << value;
    Formatter<std::string, CharType>().format(s_stream.str(), context);
    s_stream.str({});
}

//==================================================================================================
template <typename T, typename CharType>
template <typename FormatContext>
inline void Formatter<T, CharType, fly::enable_if<std::is_same<T, bool>>>::format(
    T value,
    FormatContext &context)
{
    if (context.spec().m_type == specifier::Type::String)
    {
        Formatter<string_type, CharType>().format(value ? s_true : s_false, context);
    }
    else
    {
        Formatter<unsigned, CharType>().format(static_cast<unsigned>(value), context);
    }
}

} // namespace fly
