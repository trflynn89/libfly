#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_lexer.hpp"
#include "fly/types/string/string_literal.hpp"

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

namespace fly {

template <typename StringType>
class BasicString;

} // namespace fly

namespace fly::detail {

/**
 * Structure to encapsulate positional and formatting options (which constitute a replacement field)
 * based strongly upon:
 *
 *     https://en.cppreference.com/w/cpp/utility/format/format
 *     https://en.cppreference.com/w/cpp/utility/format/formatter#Standard_format_specification
 *
 * A replacement field has the following format:
 *
 *     1. An introductory "{" character.
 *     2. An optional non-negative position.
 *     3. An optional colon ":" following by formatting options.
 *     4. A final "}" character.
 *
 * The postition option specifies the index of the format parameter whose value is to be used for
 * this replacement field. If not specified, the format parameters are used in order. The position
 * option must be specified in all replacements fields or not in any replacement field; mixing of
 * manual and automatic indexing is an error.
 *
 * Formatting options have the following format, where every field is optional:
 *
 *     1. An optional fill character (which may be any ASCII character other than "{" or "}"),
 *        followed by an alignment option. The alignment option may be one of:
 *
 *        "<" - Forces the field to be aligned to the start of the available space.
 *        ">" - Forces the field to be aligned to the end of the available space.
 *        "^" - Forces the field to be centered within the available space.
 *
 *     2. A sign indicator. The sign character may be one of:
 *
 *        "+" - A sign should be used for both non-negative and negative numbers.
 *        "-" - A sign should be used for negative numbers only.
 *        " " - A sign should be used for negative numbers and a leading space should be used
 *              for non-negative numbers.
 *
 *     3. An alternate form indicator (a literal "#" character). If present, the following alternate
 *        forms will be used:
 *
 *        Integral types - If binary, octal, or hexadecimal presentation types are used, the
 *        alternate form inserts 0b, 0, or 0x prefixes, respectively, before the value.
 *
 *        Floating point types - A decimal point character will always be inserted even if no digits
 *        follow it.
 *
 *     4. A zero-padding indicator (a literal "0" character). If present, the value is padded with
 *        leading zeros. This option is ignored if an alignment option is also specified.
 *
 *     5. A width value. A width is either a positive decimal number or a nested replacement field
 *        (*). If present, it specifies the minimum field width.
 *
 *     6. A precision value. A precision is a decimal (".") followed by a non-negative decimal
 *        number or a nested replacement field (*). If present, it specifies the precision or
 *        maximum field size. It may only be used for the following value types:
 *
 *        String types - precision specifies the maxiumum number of characters to be used.
 *
 *        Floating point types - precision specifies the formatting precision.
 *
 *     7. A locale-specific form indicator (a literal "L" character). It may only be used for the
 *        following value types:
 *
 *        Integral types - locale-specific form inserts appropriate digit group separator
 *        characters.
 *
 *        Floating point types - locale-specific form inserts appropriate digit group and radix
 *        separator characters.
 *
 *        Boolean types - locale-specific form uses the appropriate string for textual
 *        representation.
 *
 *     8. A presentation type. The type determines how the value should be presented, where the
 *        allowed presentation type varies by value type:
 *
 *        Character types - Valid presentations: none, "c", b", "B", "o", "d", "x", "X".
 *
 *        String types - Valid presentations: none, "s".
 *
 *        Pointer types - Valid presentations: none, "p".
 *
 *        Integral types - Valid presentations: none, "c", b", "B", "o", "d", "x", "X".
 *
 *        Floating point types - Valid presentations: none, "a", A", "e", "E", "f", "F", "g", "G".
 *
 *        Boolean types - Valid presentations: none, "c", s", b", "B", "o", "d", "x", "X".
 *
 *        Strong enumeration types - If an overload of operator<< is defined, valid presentations
 *        are: none, "s". Else, valid presentations are: none, "c", b", "B", "o", "d", "x", "X".
 *
 *        Other (generic) types - Valid presentations: none. An overload of operator<< must be
 *        defined for generic types.
 *
 *        For details on each presentation type, see the above links.
 *
 * (*) Nested replacement fields are a subset of the full replacement field, and may be of the
 * form "{}" or "{n}", where n is an optional non-negative position. The corresponding format
 * parameter must be an integral type. Its value has the same restrictions as the formatting option
 * it is used for.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename CharType>
struct BasicFormatSpecifier
{
    BasicFormatSpecifier() = default;

    BasicFormatSpecifier(BasicFormatSpecifier &&) = default;
    BasicFormatSpecifier &operator=(BasicFormatSpecifier &&) = default;

    enum class Alignment : std::uint8_t
    {
        Default,
        Left,
        Right,
        Center,
    };

    enum class Sign : std::uint8_t
    {
        Default,
        Always,
        NegativeOnly,
        NegativeOnlyWithPositivePadding,
    };

    // For runtime convenience, this enumeration is valued such that binary, octal, decimal, and
    // hexadecimal presentation types correspond to their base (2, 8, 10, and 16, respectively).
    enum class Type : std::uint8_t
    {
        None = 20,
        Character = 21,
        String = 22,
        Pointer = 23,
        Binary = 2,
        Octal = 8,
        Decimal = 10,
        Hex = 16,
        HexFloat = 24,
        Scientific = 25,
        Fixed = 26,
        General = 27,
    };

    enum class Case : std::uint8_t
    {
        Lower,
        Upper,
    };

    /**
     * Parse the formatting options for a standard replacement field.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     * @param parameter_type The type of format parameter corresponding to the replacement field.
     *
     * @return The parsed specifier.
     */
    template <typename FormatString>
    constexpr void parse(FormatString &format, typename FormatString::ParameterType parameter_type);

    /**
     * Infer a presentation type for a replacement field based on the corresponding format
     * parameter's type.
     *
     * TODO: This should be private. Once the standard fly::Formatters are updated to inherit from
     * this structure, they may infer their presentation type.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     * @param parameter_type The type of format parameter corresponding to the replacement field.
     */
    template <typename FormatString>
    constexpr void infer_type(typename FormatString::ParameterType parameter_type);

    /**
     * The width formatting option may either be a number or a nested replacement field. If a
     * numeric value was specified, return that value. If a nested replacement field was specified,
     * return the value of the format parameter at the position indicated by the nested replacement
     * field.
     *
     * @tparam FormatContext The formatting context type.
     *
     * @param context The context holding the formatting state.
     * @param fallback The value to return if neither a number or replacement field was specified.
     *
     * @return The replacement field's resolved width.
     */
    template <typename FormatContext>
    std::size_t width(FormatContext &context, std::size_t fallback) const;

    /**
     * The precision formatting option may either be a number or a nested replacement field. If a
     * numeric value was specified, return that value. If a nested replacement field was specified,
     * return the value of the format parameter at the position indicated by the nested replacement
     * field.
     *
     * @tparam FormatContext The formatting context type.
     *
     * @param context The context holding the formatting state.
     * @param fallback The value to return if neither a number or replacement field was specified.
     *
     * @return The replacement field's resolved precision.
     */
    template <typename FormatContext>
    std::size_t precision(FormatContext &context, std::size_t fallback) const;

    /**
     * Compare two replacement field instances for equality.
     *
     * @return True if the two instances are equal.
     */
    template <typename T>
    friend bool operator==(
        const BasicFormatSpecifier<T> &specifier1,
        const BasicFormatSpecifier<T> &specifier2);

    std::size_t m_position {0};

    std::optional<CharType> m_fill {std::nullopt};
    Alignment m_alignment {Alignment::Default};

    Sign m_sign {Sign::Default};
    bool m_alternate_form {false};
    bool m_zero_padding {false};

    std::optional<std::size_t> m_width {std::nullopt};
    std::optional<std::size_t> m_width_position {std::nullopt};

    std::optional<std::size_t> m_precision {std::nullopt};
    std::optional<std::size_t> m_precision_position {std::nullopt};

    bool m_locale_specific_form {false};

    Type m_type {Type::None};
    Case m_case {Case::Lower};

    std::size_t m_size {0};

private:
    BasicFormatSpecifier(const BasicFormatSpecifier &) = delete;
    BasicFormatSpecifier &operator=(const BasicFormatSpecifier &) = delete;

    /**
     * Parse the optional fill and alignment arguments of the replacement field.
     *
     * It is an error if the fill character is an opening brace, a closing brace, or any non-ASCII
     * symbol.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     */
    template <typename FormatString>
    constexpr void parse_fill_and_alignment(FormatString &format);

    /**
     * Parse the optional sign argument of the replacement field.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     */
    template <typename FormatString>
    constexpr void parse_sign(FormatString &format);

    /**
     * Parse the optional alternate form and zero-padding arguments of the replacement field.
     *
     * If the zero-padding argument is specified, and an alignment argument was previously
     * specified, the zero-padding argument is dropped.
     *
     * It is an error if alternate form was specified, but the presentation type is not a
     * non-decimal numeric type.
     *
     * It is an error if zero-padding was specified, but the presentation type is not a numeric
     * type.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     */
    template <typename FormatString>
    constexpr void parse_alternate_form_and_zero_padding(FormatString &format);

    /**
     * Parse the width argument of the replacement field.
     *
     * It is an error if the width is not a positive (non-zero) value or a nested replacement field.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     */
    template <typename FormatString>
    constexpr void parse_width(FormatString &format);

    /**
     * Parse the precision argument of the replacement field.
     *
     * It is an error the precision was specified, but the type of the corresponding format
     * parameter is not a string or floating point type.
     *
     * It is an error if a decimal was parsed and was not followed by a non-negative precision or a
     * replacement field.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     */
    template <typename FormatString>
    constexpr void parse_precision(FormatString &format);

    /**
     * Parse the optional locale-specific form of the replacement field.
     *
     * It is an error the locale-specific form was specified, but the type of the corresponding
     * format parameter is not an integral, floating point, or boolean type.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     */
    template <typename FormatString>
    constexpr void parse_locale_specific_form(FormatString &format);

    /**
     * Parse the optional presentation type argument of the replacement field. If not specified,
     * infer a default type based on the corresponding format parameter's type.
     *
     * It is an error if the presentation type is not allowed for the corresponding format parameter
     * type.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     * @param parameter_type The type of format parameter corresponding to the replacement field.
     */
    template <typename FormatString>
    constexpr void
    parse_type(FormatString &format, typename FormatString::ParameterType parameter_type);

    /**
     * After parsing a single replacement field, validate all options that were parsed. Raise an
     * error if the field is invalid.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     * @param parameter_type The type of format parameter corresponding to the replacement field.
     */
    template <typename FormatString>
    constexpr void
    validate(FormatString &format, typename FormatString::ParameterType parameter_type);

    /**
     * Helper to validate the presentation type of a single replacement field. Raise an error if the
     * type is invalid.
     *
     * @tparam FormatString The type of the format string containing the replacement field.
     *
     * @param format The format string containing the replacement field.
     * @param parameter_type The type of format parameter corresponding to the replacement field.
     */
    template <typename FormatString>
    constexpr void
    validate_type(FormatString &format, typename FormatString::ParameterType parameter_type);

    /**
     * Retrieve the value of a format parameter at the specified position if its type is applicable
     * to either the width or precision formatting options.
     *
     * @tparam FormatContext The formatting context type.
     *
     * @param context The context holding the formatting state.
     * @param position The format parameter position to resolve.
     *
     * @return The resolved value.
     */
    template <typename FormatContext>
    static std::optional<std::size_t> resolve(FormatContext &context, std::size_t position);

    /**
     * Search for the presentation type which maps to a character, if any.
     *
     * @param ch The character to search for.
     *
     * @return If found, the mapped presentation type. Otherwise, an uninitialized value.
     */
    static constexpr std::optional<Type> type_of(CharType ch);

    /**
     * @return True if the presentation type is a numeric type.
     */
    constexpr bool is_numeric() const;

    static constexpr std::array<std::pair<CharType, Type>, 17> s_type_map {{
        {FLY_CHR(CharType, 'c'), Type::Character},
        {FLY_CHR(CharType, 's'), Type::String},
        {FLY_CHR(CharType, 'p'), Type::Pointer},
        {FLY_CHR(CharType, 'b'), Type::Binary},
        {FLY_CHR(CharType, 'B'), Type::Binary},
        {FLY_CHR(CharType, 'o'), Type::Octal},
        {FLY_CHR(CharType, 'd'), Type::Decimal},
        {FLY_CHR(CharType, 'x'), Type::Hex},
        {FLY_CHR(CharType, 'X'), Type::Hex},
        {FLY_CHR(CharType, 'a'), Type::HexFloat},
        {FLY_CHR(CharType, 'A'), Type::HexFloat},
        {FLY_CHR(CharType, 'e'), Type::Scientific},
        {FLY_CHR(CharType, 'E'), Type::Scientific},
        {FLY_CHR(CharType, 'f'), Type::Fixed},
        {FLY_CHR(CharType, 'F'), Type::Fixed},
        {FLY_CHR(CharType, 'g'), Type::General},
        {FLY_CHR(CharType, 'G'), Type::General},
    }};

    static constexpr const auto s_left_brace = FLY_CHR(CharType, '{');
    static constexpr const auto s_right_brace = FLY_CHR(CharType, '}');
    static constexpr const auto s_less_than_sign = FLY_CHR(CharType, '<');
    static constexpr const auto s_greater_than_sign = FLY_CHR(CharType, '>');
    static constexpr const auto s_caret = FLY_CHR(CharType, '^');
    static constexpr const auto s_plus_sign = FLY_CHR(CharType, '+');
    static constexpr const auto s_minus_sign = FLY_CHR(CharType, '-');
    static constexpr const auto s_space = FLY_CHR(CharType, ' ');
    static constexpr const auto s_number_sign = FLY_CHR(CharType, '#');
    static constexpr const auto s_zero = FLY_CHR(CharType, '0');
    static constexpr const auto s_letter_l = FLY_CHR(CharType, 'L');
    static constexpr const auto s_decimal = FLY_CHR(CharType, '.');
};

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
template <typename StringType, typename... ParameterTypes>
class BasicFormatString
{
    using traits = BasicStringTraits<StringType>;
    using lexer = fly::BasicStringLexer<StringType>;
    using char_type = typename traits::char_type;
    using view_type = typename traits::view_type;

    using FormatSpecifier = BasicFormatSpecifier<char_type>;

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
    FLY_CONSTEVAL BasicFormatString(const char_type (&format)[N]) noexcept;

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
    friend BasicFormatSpecifier<char_type>;

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

    static constexpr const auto s_left_brace = FLY_CHR(char_type, '{');
    static constexpr const auto s_right_brace = FLY_CHR(char_type, '}');
    static constexpr const auto s_colon = FLY_CHR(char_type, ':');

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
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse(
    FormatString &format,
    typename FormatString::ParameterType parameter_type)
{
    parse_fill_and_alignment(format);
    parse_sign(format);
    parse_alternate_form_and_zero_padding(format);
    parse_width(format);
    parse_precision(format);
    parse_locale_specific_form(format);
    parse_type(format, parameter_type);

    validate(format, parameter_type);
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse_fill_and_alignment(FormatString &format)
{
    if (auto next = format.m_lexer.peek(1); next)
    {
        if ((*next == s_less_than_sign) || (*next == s_greater_than_sign) || (*next == s_caret))
        {
            m_fill = format.m_lexer.consume().value();
        }
    }

    if (format.m_lexer.consume_if(s_less_than_sign))
    {
        m_alignment = Alignment::Left;
    }
    else if (format.m_lexer.consume_if(s_greater_than_sign))
    {
        m_alignment = Alignment::Right;
    }
    else if (format.m_lexer.consume_if(s_caret))
    {
        m_alignment = Alignment::Center;
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse_sign(FormatString &format)
{
    if (format.m_lexer.consume_if(s_plus_sign))
    {
        m_sign = Sign::Always;
    }
    else if (format.m_lexer.consume_if(s_minus_sign))
    {
        m_sign = Sign::NegativeOnly;
    }
    else if (format.m_lexer.consume_if(s_space))
    {
        m_sign = Sign::NegativeOnlyWithPositivePadding;
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void
BasicFormatSpecifier<CharType>::parse_alternate_form_and_zero_padding(FormatString &format)
{
    if (format.m_lexer.consume_if(s_number_sign))
    {
        m_alternate_form = true;
    }

    if (format.m_lexer.consume_if(s_zero) && (m_alignment == Alignment::Default))
    {
        m_zero_padding = true;
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse_width(FormatString &format)
{
    if (auto width = format.m_lexer.consume_number(); width)
    {
        m_width = static_cast<std::size_t>(*width);
    }
    else if (format.m_lexer.consume_if(s_left_brace))
    {
        if (auto nested = format.parse_specifier(FormatString::SpecifierType::Nested); nested)
        {
            m_width_position = nested->m_position;
        }
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse_precision(FormatString &format)
{
    if (format.m_lexer.consume_if(s_decimal))
    {
        if (auto precision = format.m_lexer.consume_number(); precision)
        {
            m_precision = static_cast<std::size_t>(*precision);
        }
        else if (format.m_lexer.consume_if(s_left_brace))
        {
            if (auto nested = format.parse_specifier(FormatString::SpecifierType::Nested); nested)
            {
                m_precision_position = nested->m_position;
            }
        }
        else
        {
            format.on_error(
                "Expected a non-negative precision or nested replacement field after decimal");
        }
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse_locale_specific_form(FormatString &format)
{
    if (format.m_lexer.consume_if(s_letter_l))
    {
        m_locale_specific_form = true;
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::parse_type(
    FormatString &format,
    typename FormatString::ParameterType parameter_type)
{
    using classifier = BasicStringClassifier<std::basic_string<CharType>>;

    if (auto ch = format.m_lexer.peek(); ch)
    {
        if (auto type = type_of(ch.value()); type)
        {
            m_type = type.value();
            format.m_lexer.consume();

            if (classifier::is_upper(ch.value()))
            {
                m_case = Case::Upper;
            }
        }
    }

    if (m_type == Type::None)
    {
        infer_type<FormatString>(parameter_type);
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void
BasicFormatSpecifier<CharType>::infer_type(typename FormatString::ParameterType parameter_type)
{
    if (parameter_type == FormatString::ParameterType::Character)
    {
        m_type = Type::Character;
    }
    else if (parameter_type == FormatString::ParameterType::String)
    {
        m_type = Type::String;
    }
    else if (parameter_type == FormatString::ParameterType::Pointer)
    {
        m_type = Type::Pointer;
    }
    else if (parameter_type == FormatString::ParameterType::Integral)
    {
        m_type = Type::Decimal;
    }
    else if (parameter_type == FormatString::ParameterType::FloatingPoint)
    {
        m_type = Type::General;
    }
    else if (parameter_type == FormatString::ParameterType::Boolean)
    {
        m_type = Type::String;
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::validate(
    FormatString &format,
    typename FormatString::ParameterType parameter_type)
{
    // Validate the fill character.
    if (m_fill && ((m_fill == s_left_brace) || (m_fill == s_right_brace)))
    {
        format.on_error("Characters { and } are not allowed as fill characters");
    }
    else if (m_fill && (static_cast<std::make_unsigned_t<CharType>>(*m_fill) >= 0x80))
    {
        format.on_error("Non-ascii characters are not allowed as fill characters");
    }

    // Validate the sign.
    if ((m_sign != Sign::Default) && !is_numeric())
    {
        format.on_error("Sign may only be used with numeric presentation types");
    }

    // Validate the alternate form.
    if (m_alternate_form && (!is_numeric() || (m_type == Type::Decimal)))
    {
        format.on_error(
            "Alternate form may only be used with non-decimal numeric presentation types");
    }

    // Validate the zero-padding option.
    if (m_zero_padding && !is_numeric())
    {
        format.on_error("Zero-padding may only be used with numeric presentation types");
    }

    // Validate the width value.
    if (m_width && (*m_width == 0))
    {
        format.on_error("Width must be a positive (non-zero) value");
    }
    else if (m_width_position)
    {
        const auto nested_type = format.parameter_type(*m_width_position);

        if (nested_type != FormatString::ParameterType::Integral)
        {
            format.on_error("Position of width parameter must be an integral type");
        }
    }

    // Validate the precision value.
    if (m_precision || m_precision_position)
    {
        if ((parameter_type != FormatString::ParameterType::String) &&
            (parameter_type != FormatString::ParameterType::FloatingPoint))
        {
            format.on_error("Precision may only be used for string and floating point types");
        }
        else if (m_precision_position)
        {
            const auto nested_type = format.parameter_type(*m_precision_position);

            if (nested_type != FormatString::ParameterType::Integral)
            {
                format.on_error("Position of precision parameter must be an integral type");
            }
        }
    }

    // Validate the locale-specifc form.
    if (m_locale_specific_form &&
        ((parameter_type != FormatString::ParameterType::Integral) &&
         (parameter_type != FormatString::ParameterType::FloatingPoint) &&
         (parameter_type != FormatString::ParameterType::Boolean)))
    {
        format.on_error("Locale-specific form may only be used for numeric and boolean types");
    }

    // Validate the presentation type.
    if (m_type != Type::None)
    {
        validate_type(format, parameter_type);
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatString>
constexpr void BasicFormatSpecifier<CharType>::validate_type(
    FormatString &format,
    typename FormatString::ParameterType parameter_type)
{
    if (parameter_type == FormatString::ParameterType::Character)
    {
        if ((m_type != Type::Character) && (m_type != Type::Binary) && (m_type != Type::Octal) &&
            (m_type != Type::Decimal) && (m_type != Type::Hex))
        {
            format.on_error("Character types must be formatted with {} or {:cbBodxX}");
        }
    }
    else if (parameter_type == FormatString::ParameterType::String)
    {
        if (m_type != Type::String)
        {
            format.on_error("String types must be formatted with {} or {:s}");
        }
    }
    else if (parameter_type == FormatString::ParameterType::Pointer)
    {
        if (m_type != Type::Pointer)
        {
            format.on_error("Pointer types must be formatted with {} or {:p}");
        }
    }
    else if (parameter_type == FormatString::ParameterType::Integral)
    {
        if ((m_type != Type::Character) && (m_type != Type::Binary) && (m_type != Type::Octal) &&
            (m_type != Type::Decimal) && (m_type != Type::Hex))
        {
            format.on_error("Integral types must be formatted with {} or one of {:cbBodxX}");
        }
    }
    else if (parameter_type == FormatString::ParameterType::FloatingPoint)
    {
        if ((m_type != Type::HexFloat) && (m_type != Type::Scientific) && (m_type != Type::Fixed) &&
            (m_type != Type::General))
        {
            format.on_error("Floating point types must be formatted with {} or one of {:aAeEfFgG}");
        }
    }
    else if (parameter_type == FormatString::ParameterType::Boolean)
    {
        if ((m_type != Type::Character) && (m_type != Type::String) && (m_type != Type::Binary) &&
            (m_type != Type::Octal) && (m_type != Type::Decimal) && (m_type != Type::Hex))
        {
            format.on_error("Boolean types must be formatted with {} or one of {:csbBodxX}");
        }
    }
}

//==================================================================================================
template <typename CharType>
template <typename FormatContext>
inline std::size_t
BasicFormatSpecifier<CharType>::width(FormatContext &context, std::size_t fallback) const
{
    if (m_width_position)
    {
        return resolve(context, *m_width_position).value_or(fallback);
    }

    return m_width.value_or(fallback);
}

//==================================================================================================
template <typename CharType>
template <typename FormatContext>
inline std::size_t
BasicFormatSpecifier<CharType>::precision(FormatContext &context, std::size_t fallback) const
{
    if (m_precision_position)
    {
        return resolve(context, *m_precision_position).value_or(fallback);
    }

    return m_precision.value_or(fallback);
}

//==================================================================================================
template <typename CharType>
template <typename FormatContext>
inline std::optional<std::size_t>
BasicFormatSpecifier<CharType>::resolve(FormatContext &context, std::size_t position)
{
    return context.arg(position).visit(
        [](auto value) -> std::optional<std::size_t>
        {
            using T = std::remove_cvref_t<decltype(value)>;
            std::optional<std::size_t> resolved;

            if constexpr (std::is_unsigned_v<T>)
            {
                resolved = static_cast<std::size_t>(value);
            }
            else if constexpr (std::is_integral_v<T>)
            {
                if (value >= 0)
                {
                    resolved = static_cast<std::size_t>(value);
                }
            }

            return resolved;
        });
}

//==================================================================================================
template <typename CharType>
constexpr auto BasicFormatSpecifier<CharType>::type_of(CharType ch) -> std::optional<Type>
{
    auto it = std::find_if(
        s_type_map.begin(),
        s_type_map.end(),
        [&ch](const auto &item)
        {
            return item.first == ch;
        });

    if (it == s_type_map.end())
    {
        return std::nullopt;
    }

    return it->second;
}

//==================================================================================================
template <typename CharType>
constexpr bool BasicFormatSpecifier<CharType>::is_numeric() const
{
    switch (m_type)
    {
        case Type::Binary:
        case Type::Octal:
        case Type::Decimal:
        case Type::Hex:
        case Type::HexFloat:
        case Type::Scientific:
        case Type::Fixed:
        case Type::General:
            return true;

        default:
            return false;
    }
}

//==================================================================================================
template <typename T>
bool operator==(
    const BasicFormatSpecifier<T> &specifier1,
    const BasicFormatSpecifier<T> &specifier2)
{
    return (specifier1.m_position == specifier2.m_position) &&
        (specifier1.m_fill == specifier2.m_fill) &&
        (specifier1.m_alignment == specifier2.m_alignment) &&
        (specifier1.m_sign == specifier2.m_sign) &&
        (specifier1.m_alternate_form == specifier2.m_alternate_form) &&
        (specifier1.m_zero_padding == specifier2.m_zero_padding) &&
        (specifier1.m_width == specifier2.m_width) &&
        (specifier1.m_width_position == specifier2.m_width_position) &&
        (specifier1.m_precision == specifier2.m_precision) &&
        (specifier1.m_precision_position == specifier2.m_precision_position) &&
        (specifier1.m_locale_specific_form == specifier2.m_locale_specific_form) &&
        (specifier1.m_type == specifier2.m_type) && (specifier1.m_case == specifier2.m_case);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <std::size_t N>
FLY_CONSTEVAL BasicFormatString<StringType, ParameterTypes...>::BasicFormatString(
    const char_type (&format)[N]) noexcept :
    m_lexer(format)
{
    std::optional<char_type> ch;

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
template <typename StringType, typename... ParameterTypes>
constexpr auto BasicFormatString<StringType, ParameterTypes...>::view() const -> view_type
{
    return m_lexer.view();
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr bool BasicFormatString<StringType, ParameterTypes...>::has_error() const
{
    return !m_error.empty();
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
std::string BasicFormatString<StringType, ParameterTypes...>::error() const
{
    return std::string(m_error);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
auto BasicFormatString<StringType, ParameterTypes...>::next_specifier()
    -> std::optional<FormatSpecifier>
{
    if (m_specifier_index >= m_specifier_count)
    {
        return std::nullopt;
    }

    return std::move(m_specifiers[m_specifier_index++]);
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr auto
BasicFormatString<StringType, ParameterTypes...>::parse_specifier(SpecifierType specifier_type)
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
template <typename StringType, typename... ParameterTypes>
constexpr std::size_t BasicFormatString<StringType, ParameterTypes...>::parse_position()
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
template <typename StringType, typename... ParameterTypes>
template <size_t N>
constexpr auto BasicFormatString<StringType, ParameterTypes...>::parameter_type(size_t index)
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
template <typename StringType, typename... ParameterTypes>
void BasicFormatString<StringType, ParameterTypes...>::on_error(const char *error)
{
    if (!has_error())
    {
        m_error = error;
    }
}

} // namespace fly::detail
