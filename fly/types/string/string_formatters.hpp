#pragma once

#include "fly/traits/traits.hpp"
#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_formatter_types.hpp"
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
 *     template <typename OutputIterator>
 *     void format(FormatSpecifier specifier, const T &value, OutputIterator output);
 *
 * Where the OutputIterator is a back-insert iterator into which the formatted value should be
 * written.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version April 4, 2021
 */
template <typename T, typename CharType = char, typename SFINAE = bool>
class Formatter;

template <typename T, typename CharType>
class Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_generic<T>>>
{
    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided generic value.
     *
     * Currently, rather than supporting a set of std::formatter specializations, the generic value
     * will be converted to a string via the streaming operator. The resulting string will then be
     * formatted using the string formatting overload.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier, const T &value, OutputIterator output);
};

template <typename T, typename CharType>
class Formatter<T, CharType, fly::enable_if<detail::is_like_supported_string<T>>>
{
    using string_type = std::basic_string<CharType>;
    using string_like_type = detail::is_like_supported_string_t<T>;
    using view_like_type = std::basic_string_view<typename string_like_type::value_type>;

    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided string-like value.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier specifier, const T &value, OutputIterator output);

    /**
     * Append a string-like value to the buffer.
     *
     * If the string-like value's character type is the same as the format string, the value is
     * inserted directly. Otherwise, it is first transcoded to the appropriate Unicode encoding. If
     * transcoding fails, the value is dropped.
     *
     * @tparam T The type of the value to append.
     *
     * @param value The value to append.
     * @param value_size The size of the value to append.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void append_string(const T &value, std::size_t value_size, OutputIterator output);

private:
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
};

template <typename T, typename CharType>
class Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_pointer<T>>>
{
    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided pointer value.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier specifier, const T &value, OutputIterator output);
};

template <typename T, typename CharType>
class Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>
{
    using string_type = std::basic_string<CharType>;

    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided non-boolean integral value.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier specifier, T value, OutputIterator output);

private:
    /**
     * Format a single replacement field with the provided unsigned, non-boolean integral value.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param is_negative Whether the original value was negative.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename U, typename OutputIterator>
    void format(FormatSpecifier specifier, U value, bool is_negative, OutputIterator output);

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
     * @param specifier The replacement field to format.
     * @param value The value to convert.
     * @param base The base of the value.
     * @param output The output iterator to write the formatted value into.
     *
     * @return The number of base-N digits converted.
     */
    template <typename U, typename OutputIterator>
    void append_number(const FormatSpecifier &specifier, U value, int base, OutputIterator output);

    /**
     * Count the number of base-N digits in a value, where N is the provided integer base.
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
class Formatter<T, CharType, fly::enable_if<std::is_floating_point<T>>>
{
    using string_type = std::basic_string<CharType>;
    using stream_modifiers = detail::BasicStreamModifiers<string_type>;

    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided floating point value.
     *
     * Currently, major compilers do not support std::to_chars for floating point values. Until they
     * do, this implementation uses an IO stream to format the value.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier specifier, T value, OutputIterator output);

private:
    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
};

template <typename T, typename CharType>
class Formatter<T, CharType, fly::enable_if<std::is_same<T, bool>>>
{
    using string_type = std::basic_string<CharType>;

    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided boolean value.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier specifier, T value, OutputIterator output);

private:
    static constexpr const CharType *s_true = FLY_STR(CharType, "true");
    static constexpr const CharType *s_false = FLY_STR(CharType, "false");
};

template <typename T, typename CharType>
class Formatter<
    T,
    CharType,
    fly::enable_if<detail::BasicFormatTraits::is_default_formatted_enum<T>>>
{
    using FormatSpecifier = detail::BasicFormatSpecifier<CharType>;

public:
    /**
     * Format a single replacement field with the provided enumeration.
     *
     * @tparam OutputIterator The type of the output iterator to write the formatted value into.
     *
     * @param specifier The replacement field to format.
     * @param value The value to format.
     * @param output The output iterator to write the formatted value into.
     */
    template <typename OutputIterator>
    void format(FormatSpecifier specifier, T value, OutputIterator output);
};

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_generic<T>>>::format(
    FormatSpecifier,
    const T &value,
    OutputIterator output)
{
    if constexpr (detail::OstreamTraits::is_declared_v<T>)
    {
        static thread_local std::stringstream s_stream;

        s_stream << value;
        const auto formatted = s_stream.str();
        s_stream.str({});

        Formatter<std::string, CharType>().append_string(formatted, formatted.size(), output);
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
void Formatter<T, CharType, fly::enable_if<detail::is_like_supported_string<T>>>::format(
    FormatSpecifier specifier,
    const T &value,
    OutputIterator output)
{
    const std::size_t min_width = specifier.m_width.value_or(0);
    const std::size_t max_width = specifier.m_precision.value_or(string_type::npos);

    const std::size_t actual_size = detail::BasicStringClassifier<string_like_type>::size(value);
    const std::size_t value_size = std::min(max_width, actual_size);

    const std::size_t padding_size = std::max(value_size, min_width) - value_size;
    const auto padding_char = specifier.m_fill.value_or(s_space);

    auto append_padding = [&output, padding_char](std::size_t count)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *output++ = padding_char;
        }
    };

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
        case FormatSpecifier::Alignment::Default:
            append_string(value, value_size, output);
            append_padding(padding_size);
            break;

        case FormatSpecifier::Alignment::Right:
            append_padding(padding_size);
            append_string(value, value_size, output);
            break;

        case FormatSpecifier::Alignment::Center:
        {
            const std::size_t left_padding = padding_size / 2;
            const std::size_t right_padding =
                (padding_size % 2 == 0) ? left_padding : left_padding + 1;

            append_padding(left_padding);
            append_string(value, value_size, output);
            append_padding(right_padding);
            break;
        }
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
void Formatter<T, CharType, fly::enable_if<detail::is_like_supported_string<T>>>::append_string(
    const T &value,
    std::size_t value_size,
    OutputIterator output)
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
            *output++ = ch;
        }
    }
    else
    {
        using unicode = detail::BasicStringUnicode<string_like_type>;

        if (auto converted = unicode::template convert_encoding<string_type>(view); converted)
        {
            for (const auto &ch : *converted)
            {
                *output++ = ch;
            }
        }
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_pointer<T>>>::format(
    FormatSpecifier specifier,
    const T &value,
    OutputIterator output)
{
    specifier.m_alternate_form = true;
    specifier.m_type = FormatSpecifier::Type::Hex;

    Formatter<std::uintptr_t, CharType>().format(
        std::move(specifier),
        reinterpret_cast<std::uintptr_t>(static_cast<const void *>(value)),
        output);
}

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::format(
    FormatSpecifier specifier,
    T value,
    OutputIterator output)
{
    if constexpr (std::is_signed_v<T>)
    {
        using U = std::make_unsigned_t<std::remove_cvref_t<T>>;

        // Compute the absolute value of the integer. Benchmarks showed this is exactly as fast
        // as std::abs, but this also tracks whether the original value was negative without
        // branches.
        const T sign = value >> std::numeric_limits<T>::digits;
        const U unsigned_value = static_cast<U>(static_cast<U>(value ^ sign) + (sign & 1));

        format(std::move(specifier), unsigned_value, static_cast<bool>(sign), output);
    }
    else
    {
        format(std::move(specifier), value, false, output);
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename U, typename OutputIterator>
void Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::format(
    FormatSpecifier specifier,
    U value,
    bool is_negative,
    OutputIterator output)
{
    static_assert(std::is_unsigned_v<U>);

    std::size_t prefix_size = 0;
    std::size_t value_size = 0;

    if (specifier.m_type == FormatSpecifier::Type::Character)
    {
        static constexpr U s_max = static_cast<U>(std::numeric_limits<CharType>::max());
        value_size = is_negative || (value > s_max) ? 0 : 1;
    }
    else
    {
        if (is_negative || (specifier.m_sign == FormatSpecifier::Sign::Always) ||
            (specifier.m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding))
        {
            ++prefix_size;
        }

        if (specifier.m_alternate_form)
        {
            ++prefix_size;

            if ((specifier.m_type == FormatSpecifier::Type::Binary) ||
                (specifier.m_type == FormatSpecifier::Type::Hex))
            {
                ++prefix_size;
            }
        }

        const int base = static_cast<int>(specifier.m_type);
        value_size = count_digits(value, base) + prefix_size;
    }

    const std::size_t width = specifier.m_width.value_or(0);
    const std::size_t padding_size = std::max(value_size, width) - value_size;
    const auto padding_char = specifier.m_fill.value_or(s_space);

    auto append_prefix = [&specifier, is_negative, &output]()
    {
        if (specifier.m_type == FormatSpecifier::Type::Character)
        {
            return;
        }

        if (is_negative)
        {
            *output++ = s_minus_sign;
        }
        else if (specifier.m_sign == FormatSpecifier::Sign::Always)
        {
            *output++ = s_plus_sign;
        }
        else if (specifier.m_sign == FormatSpecifier::Sign::NegativeOnlyWithPositivePadding)
        {
            *output++ = s_space;
        }

        if (specifier.m_alternate_form)
        {
            const bool is_upper_case = specifier.m_case == FormatSpecifier::Case::Upper;
            *output++ = s_zero;

            if (specifier.m_type == FormatSpecifier::Type::Binary)
            {
                *output++ = is_upper_case ? s_upper_b : s_lower_b;
            }
            else if (specifier.m_type == FormatSpecifier::Type::Hex)
            {
                *output++ = is_upper_case ? s_upper_x : s_lower_x;
            }
        }
    };

    auto append_value = [this, &specifier, value, value_size, &output]()
    {
        if (specifier.m_type == FormatSpecifier::Type::Character)
        {
            if (value_size != 0)
            {
                *output++ = static_cast<CharType>(value);
            }
        }
        else
        {
            const int base = static_cast<int>(specifier.m_type);
            append_number(specifier, value, base, output);
        }
    };

    auto append_padding = [&output](std::size_t count, CharType pad)
    {
        for (std::size_t i = 0; i < count; ++i)
        {
            *output++ = pad;
        }
    };

    switch (specifier.m_alignment)
    {
        case FormatSpecifier::Alignment::Left:
            append_prefix();
            append_value();
            append_padding(padding_size, padding_char);
            break;

        case FormatSpecifier::Alignment::Right:
            append_padding(padding_size, padding_char);
            append_prefix();
            append_value();
            break;

        case FormatSpecifier::Alignment::Center:
        {
            const std::size_t left_padding = padding_size / 2;
            const std::size_t right_padding =
                (padding_size % 2 == 0) ? left_padding : left_padding + 1;

            append_padding(left_padding, padding_char);
            append_prefix();
            append_value();
            append_padding(right_padding, padding_char);
            break;
        }

        case FormatSpecifier::Alignment::Default:
            if (specifier.m_zero_padding)
            {
                append_prefix();
                append_padding(padding_size, s_zero);
                append_value();
            }
            else
            {
                append_padding(padding_size, padding_char);
                append_prefix();
                append_value();
            }
            break;
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename U, typename OutputIterator>
void Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_integral<T>>>::
    append_number(const FormatSpecifier &specifier, U value, int base, OutputIterator output)
{
    static thread_local std::array<char, std::numeric_limits<std::uintmax_t>::digits> s_buffer;

    char *begin = s_buffer.data();
    char *end = begin + s_buffer.size();

    const auto result = std::to_chars(begin, end, value, base);

    if ((specifier.m_type == FormatSpecifier::Type::Hex) &&
        (specifier.m_case == FormatSpecifier::Case::Upper))
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
            *output++ = *it;
        }
    }
    else
    {
        using unicode = detail::BasicStringUnicode<std::string>;
        std::string_view view(begin, static_cast<std::size_t>(std::distance(begin, result.ptr)));

        unicode::template convert_encoding_into<string_type>(view, output);
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
template <typename OutputIterator>
void Formatter<T, CharType, fly::enable_if<std::is_floating_point<T>>>::format(
    FormatSpecifier specifier,
    T value,
    OutputIterator output)
{
    static thread_local std::stringstream s_stream;
    stream_modifiers modifiers(s_stream);

    if (specifier.m_alignment == FormatSpecifier::Alignment::Default)
    {
        specifier.m_alignment = FormatSpecifier::Alignment::Right;
    }

    switch (specifier.m_sign)
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

    if (specifier.m_alternate_form)
    {
        modifiers.setf(std::ios_base::showpoint);
    }

    if (specifier.m_zero_padding)
    {
        modifiers.setf(std::ios_base::internal, std::ios_base::adjustfield);
        modifiers.fill(static_cast<char>(s_zero));
        modifiers.width(static_cast<std::streamsize>(specifier.m_width.value_or(0)));
    }

    modifiers.precision(static_cast<std::streamsize>(specifier.m_precision.value_or(6)));
    specifier.m_precision = std::nullopt;

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
    Formatter<std::string, CharType>().format(std::move(specifier), s_stream.str(), output);
    s_stream.str({});
}

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
inline void Formatter<T, CharType, fly::enable_if<std::is_same<T, bool>>>::format(
    FormatSpecifier specifier,
    T value,
    OutputIterator output)
{
    if (specifier.m_type == FormatSpecifier::Type::String)
    {
        Formatter<string_type, CharType>().format(
            std::move(specifier),
            value ? s_true : s_false,
            output);
    }
    else
    {
        Formatter<unsigned, CharType>().format(
            std::move(specifier),
            static_cast<unsigned>(value),
            output);
    }
}

//==================================================================================================
template <typename T, typename CharType>
template <typename OutputIterator>
inline void
Formatter<T, CharType, fly::enable_if<detail::BasicFormatTraits::is_default_formatted_enum<T>>>::
    format(FormatSpecifier specifier, T value, OutputIterator output)
{
    Formatter<std::underlying_type_t<T>, CharType>().format(
        std::move(specifier),
        static_cast<std::underlying_type_t<T>>(value),
        output);
}

} // namespace fly
