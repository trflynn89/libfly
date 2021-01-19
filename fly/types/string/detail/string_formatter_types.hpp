#pragma once

#include "fly/fly.hpp"
#include "fly/types/string/detail/string_classifier.hpp"
#include "fly/types/string/detail/string_lexer.hpp"
#include "fly/types/string/detail/string_traits.hpp"
#include "fly/types/string/string_literal.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>

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
 *        Other (general) types - Valid presentations: none, "s". If an overload of operator<< is
 *        defined, the value is formatted using that operator. Otherwise, the memory location of the
 *        value is used.
 *
 *        For details on each presentation type, see the above links.
 *
 *     (*) Nested replacement fields are a subset of the full replacement field, and may be of the
 *         form "{}" or "{n}", where n is an optional non-negative position. The corresponding
 *         format parameter must be an integral type. Its value has the same restrictions as the
 *         formatting option it is used for.
 *
 * The above includes some differences from the standardized formatting specificaton. Namely, there
 * is not a compatible implementation of the std::formatter specializations. Instead, the streaming
 * operator (operator<<) is used to format general types.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename CharType>
struct BasicFormatSpecifier
{
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

    enum class Type : std::uint8_t
    {
        None,
        Character,
        String,
        Pointer,
        Binary,
        Octal,
        Decimal,
        Hex,
        HexFloat,
        Scientific,
        Fixed,
        General,
    };

    enum class Case : std::uint8_t
    {
        Lower,
        Upper,
    };

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
    using classifier = BasicStringClassifier<StringType>;
    using lexer = BasicStringLexer<StringType>;
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
    BasicFormatString(const BasicFormatString &) = delete;
    BasicFormatString &operator=(const BasicFormatString &) = delete;

    /**
     * Helper trait to determine if a type is either streamable or string-like.
     */
    template <typename T>
    using is_formatable_type = std::disjunction<
        typename traits::OstreamTraits::template is_declared<T>,
        detail::is_like_supported_string<T>,
        detail::is_supported_character<T>>;

    template <typename T>
    static inline constexpr bool is_formatable_type_v = is_formatable_type<T>::value;

    /**
     * Helper trait to classify a type as an integer, excluding character and boolean types.
     */
    template <typename T>
    using is_integer = std::conjunction<
        std::is_integral<T>,
        std::negation<is_supported_character<T>>,
        std::negation<std::is_same<T, bool>>>;

    template <typename T>
    static inline constexpr bool is_integer_v = is_integer<T>::value;

    /**
     * Upon parsing an un-escaped opening brace, parse a single replacement field in the format
     * string. If valid, the lexer will be advanced to the character after the closing brace.
     *
     * @param type The type of replacement field to parse (either a full or nested field).
     *
     * @return The parsed specifier.
     */
    constexpr std::optional<FormatSpecifier> parse_specifier(SpecifierType type);

    /**
     * Parse the optional position argument of the current replacement field. If a position was not
     * found, the position is observed to be the next format parameter in order.
     *
     * It is an error if the format string has a mix of manual and automatic positioning.
     *
     * @param specifier The specifier instance to store the position into.
     */
    constexpr void parse_position(FormatSpecifier &specifier);

    /**
     * Parse the optional fill and alignment arguments of the current replacement field.
     *
     * It is an error if the fill character is an opening brace, a closing brace, or any non-ASCII
     * symbol.
     *
     * @param specifier The specifier instance to store the fill and alignment into.
     */
    constexpr void parse_fill_and_alignment(FormatSpecifier &specifier);

    /**
     * Parse the optional sign argument of the current replacement field.
     *
     * It is an error if a sign was specified, but the presentation type is not a numeric type.
     *
     * @param specifier The specifier instance to store the sign into.
     */
    constexpr void parse_sign(FormatSpecifier &specifier);

    /**
     * Parse the optional alternate form and zero-padding arguments of the current replacement
     * field.
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
     * @param specifier The specifier instance to store the alternate form and padding into.
     */
    constexpr void parse_alternate_form_and_zero_padding(FormatSpecifier &specifier);

    /**
     * Parse the width argument of the current replacement field.
     *
     * It is an error if the width is not a positive (non-zero) value or a nested replacement field.
     *
     * @param specifier The specifier instance to store the width and precision into.
     */
    constexpr void parse_width(FormatSpecifier &specifier);

    /**
     * Parse the precision argument of the current replacement field.
     *
     * It is an error the precision was specified, but the type of the corresponding format
     * parameter is not a string or floating point type.
     *
     * It is an error if a decimal was parsed and was not followed by a non-negative precision or a
     * replacement field.
     *
     * @param specifier The specifier instance to store the width and precision into.
     */
    constexpr void parse_precision(FormatSpecifier &specifier);

    /**
     * Parse the optional locale-specific form of the current replacement field.
     *
     * It is an error the locale-specific form was specified, but the type of the corresponding
     * format parameter is not an integral, floating point, or boolean type.
     *
     * @param specifier The specifier instance to store the locale-specific form.
     */
    constexpr void parse_locale_specific_form(FormatSpecifier &specifier);

    /**
     * Parse the optional presentation type argument of the current replacement field. If not
     * specified, infer a default type based on the corresponding format parameter's type.
     *
     * It is an error if the presentation type is not allowed for the corresponding format parameter
     * type.
     *
     * @param specifier The specifier instance to store the presentation type into.
     */
    constexpr void parse_type(FormatSpecifier &specifier);

    /**
     * Infer a presentation type for a replacement field based on the corresponding format
     * parameter's type.
     *
     * @param specifier The specifier instance to store the presentation type into.
     */
    constexpr void infer_type(FormatSpecifier &specifier);

    /**
     * After parsing a single replacement field, validate all options that were parsed. Raise an
     * error if the field is invalid.
     *
     * @param specifier The specifier instance to validate.
     *
     * @return True if the specifier instance is valid.
     */
    constexpr bool validate_specifier(const FormatSpecifier &specifier);

    /**
     * Helper to validate the presentation type of a single replacement field. Raise an error if the
     * type is invalid.
     *
     * @param type Type of the format parameter corresponding to the provided specifier.
     * @param specifier The specifier instance to validate.
     *
     * @return True if the presentation type is valid.
     */
    constexpr void validate_type(ParameterType type, const FormatSpecifier &specifier);

    /**
     * Determine the type of a format parameter. Returns ParameterType::Generic if the given index
     * was not found, or if the type of the format parameter is unknown.
     *
     * @param index The index of the format parameter to inspect.
     *
     * @return The type of the format parameter.
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
    static constexpr const auto s_less_than_sign = FLY_CHR(char_type, '<');
    static constexpr const auto s_greater_than_sign = FLY_CHR(char_type, '>');
    static constexpr const auto s_caret = FLY_CHR(char_type, '^');
    static constexpr const auto s_plus_sign = FLY_CHR(char_type, '+');
    static constexpr const auto s_minus_sign = FLY_CHR(char_type, '-');
    static constexpr const auto s_space = FLY_CHR(char_type, ' ');
    static constexpr const auto s_number_sign = FLY_CHR(char_type, '#');
    static constexpr const auto s_zero = FLY_CHR(char_type, '0');
    static constexpr const auto s_letter_l = FLY_CHR(char_type, 'L');
    static constexpr const auto s_decimal = FLY_CHR(char_type, '.');

    lexer m_lexer;

    std::array<FormatSpecifier, 64> m_specifiers;
    std::size_t m_specifier_count {0};
    std::size_t m_specifier_index {0};

    std::size_t m_next_position {0};
    bool m_expect_no_positions_specified {false};
    bool m_expect_all_positions_specified {false};

    std::string_view m_error;
};

/**
 * A container to hold references to variadic format parameters without copying any of them.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version January 3, 2021
 */
template <typename StringType, typename... ParameterTypes>
class BasicFormatParameters
{
    using FormatParameters = std::tuple<std::type_identity_t<ParameterTypes>...>;
    using FormatSpecifier = BasicFormatSpecifier<typename StringType::value_type>;

public:
    /**
     * Constructor. Forward references to the provided format parameters to a tuple.
     *
     * @param parameters The format parameters to store.
     */
    explicit BasicFormatParameters(ParameterTypes &&...parameters);

    /**
     * Visitor to provide runtime access to the stored parameters based on a replacement field's
     * position. If the provided position is found, invokes the provided callback with the
     * replacement field and a reference to the found format parameter.
     *
     * @tparam Callback Type of the callback to invoke.
     *
     * @param specifier The replacement field corresponding to the parameter to search for.
     * @param callback The callback to invoke if the parameter is found.
     */
    template <typename Callback, size_t N = 0>
    void visit(FormatSpecifier &&specifier, Callback callback) const;

    /**
     * Visitor to provide runtime access to the stored parameter at the provided index. If the index
     * is found, and is convertible to the desired type, returns a copy of the found format
     * parameter.
     *
     * This is only allowed for integral format parameters. Attempting to copy other format types is
     * forbidden.
     *
     * @tparam T Desired type of the format parameter.
     *
     * @param index The index of the parameter to search for.
     *
     * @return If successful, a copy of the format parameter. Otherwise, an uninitialized value.
     */
    template <typename T, size_t N = 0>
    std::optional<T> get(std::size_t index) const;

private:
    BasicFormatParameters(const BasicFormatParameters &) = delete;
    BasicFormatParameters &operator=(const BasicFormatParameters &) = delete;

    const FormatParameters m_parameters;

    static constexpr const std::size_t s_parameter_count = sizeof...(ParameterTypes);
};

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
        (specifier1.m_precision == specifier2.m_precision) &&
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

    if constexpr (!(is_formatable_type_v<ParameterTypes> && ...))
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
constexpr auto BasicFormatString<StringType, ParameterTypes...>::parse_specifier(SpecifierType type)
    -> std::optional<FormatSpecifier>
{
    // The opening { will have already been consumed, so the starting position is one less.
    const auto starting_position = m_lexer.position() - 1;

    FormatSpecifier specifier {};
    parse_position(specifier);

    if ((type == SpecifierType::Full) && m_lexer.consume_if(s_colon))
    {
        parse_fill_and_alignment(specifier);
        parse_sign(specifier);
        parse_alternate_form_and_zero_padding(specifier);
        parse_width(specifier);
        parse_precision(specifier);
        parse_locale_specific_form(specifier);
        parse_type(specifier);
    }
    else
    {
        infer_type(specifier);
    }

    if (!m_lexer.consume_if(s_right_brace))
    {
        on_error("Detected unclosed format string - must end with }");
        return std::nullopt;
    }
    else if (!validate_specifier(specifier))
    {
        return std::nullopt;
    }

    specifier.m_size = m_lexer.position() - starting_position;
    return specifier;
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::parse_position(FormatSpecifier &specifier)
{
    if (auto position = m_lexer.consume_number(); position)
    {
        specifier.m_position = position.value();
        m_expect_no_positions_specified = true;
    }
    else
    {
        specifier.m_position = m_next_position++;
        m_expect_all_positions_specified = true;
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void BasicFormatString<StringType, ParameterTypes...>::parse_fill_and_alignment(
    FormatSpecifier &specifier)
{
    if (auto next = m_lexer.peek(1); next)
    {
        if ((*next == s_less_than_sign) || (*next == s_greater_than_sign) || (*next == s_caret))
        {
            specifier.m_fill = m_lexer.consume().value();
        }
    }

    if (m_lexer.consume_if(s_less_than_sign))
    {
        specifier.m_alignment = FormatSpecifier::Alignment::Left;
    }
    else if (m_lexer.consume_if(s_greater_than_sign))
    {
        specifier.m_alignment = FormatSpecifier::Alignment::Right;
    }
    else if (m_lexer.consume_if(s_caret))
    {
        specifier.m_alignment = FormatSpecifier::Alignment::Center;
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::parse_sign(FormatSpecifier &specifier)
{
    if (m_lexer.consume_if(s_plus_sign))
    {
        specifier.m_sign = FormatSpecifier::Sign::Always;
    }
    else if (m_lexer.consume_if(s_minus_sign))
    {
        specifier.m_sign = FormatSpecifier::Sign::NegativeOnly;
    }
    else if (m_lexer.consume_if(s_space))
    {
        specifier.m_sign = FormatSpecifier::Sign::NegativeOnlyWithPositivePadding;
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::parse_alternate_form_and_zero_padding(
    FormatSpecifier &specifier)
{
    if (m_lexer.consume_if(s_number_sign))
    {
        specifier.m_alternate_form = true;
    }

    if (m_lexer.consume_if(s_zero) &&
        (specifier.m_alignment == FormatSpecifier::Alignment::Default))
    {
        specifier.m_zero_padding = true;
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::parse_width(FormatSpecifier &specifier)
{
    if (auto width = m_lexer.consume_number(); width)
    {
        specifier.m_width = width.value();
    }
    else if (m_lexer.consume_if(s_left_brace))
    {
        if (auto nested_specifier = parse_specifier(SpecifierType::Nested); nested_specifier)
        {
            specifier.m_width_position = nested_specifier->m_position;
        }
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::parse_precision(FormatSpecifier &specifier)
{
    if (m_lexer.consume_if(s_decimal))
    {
        if (auto precision = m_lexer.consume_number(); precision)
        {
            specifier.m_precision = precision.value();
        }
        else if (m_lexer.consume_if(s_left_brace))
        {
            if (auto nested_specifier = parse_specifier(SpecifierType::Nested); nested_specifier)
            {
                specifier.m_precision_position = nested_specifier->m_position;
            }
        }
        else
        {
            on_error("Expected a non-negative precision or nested replacement field after decimal");
        }
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void BasicFormatString<StringType, ParameterTypes...>::parse_locale_specific_form(
    FormatSpecifier &specifier)
{
    if (m_lexer.consume_if(s_letter_l))
    {
        specifier.m_locale_specific_form = true;
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::parse_type(FormatSpecifier &specifier)
{
    if (auto ch = m_lexer.peek(); ch)
    {
        if (auto type = FormatSpecifier::type_of(ch.value()); type)
        {
            specifier.m_type = type.value();
            m_lexer.consume();

            if (classifier::is_upper(ch.value()))
            {
                specifier.m_case = FormatSpecifier::Case::Upper;
            }
        }
    }

    if (specifier.m_type == FormatSpecifier::Type::None)
    {
        infer_type(specifier);
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void
BasicFormatString<StringType, ParameterTypes...>::infer_type(FormatSpecifier &specifier)
{
    if (auto type = parameter_type(specifier.m_position); type)
    {
        switch (type.value())
        {
            case ParameterType::Generic:
                break;
            case ParameterType::Character:
                specifier.m_type = FormatSpecifier::Type::Character;
                break;
            case ParameterType::String:
                specifier.m_type = FormatSpecifier::Type::String;
                break;
            case ParameterType::Pointer:
                specifier.m_type = FormatSpecifier::Type::Pointer;
                break;
            case ParameterType::Integral:
                specifier.m_type = FormatSpecifier::Type::Decimal;
                break;
            case ParameterType::FloatingPoint:
                specifier.m_type = FormatSpecifier::Type::General;
                break;
            case ParameterType::Boolean:
                specifier.m_type = FormatSpecifier::Type::String;
                break;
        }
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr bool BasicFormatString<StringType, ParameterTypes...>::validate_specifier(
    const FormatSpecifier &specifier)
{
    std::optional<ParameterType> type = parameter_type(specifier.m_position);

    // Validate the position option.
    if (m_expect_no_positions_specified && m_expect_all_positions_specified)
    {
        on_error("Argument position must be provided on all or not on any specifier");
    }
    else if (!type)
    {
        on_error("Argument position exceeds number of provided arguments");
    }

    // Validate the fill character.
    if (specifier.m_fill &&
        ((specifier.m_fill == s_left_brace) || (specifier.m_fill == s_right_brace)))
    {
        on_error("Characters { and } are not allowed as fill characters");
    }
    else if (
        specifier.m_fill &&
        (static_cast<std::make_unsigned_t<char_type>>(*specifier.m_fill) >= 0x80))
    {
        on_error("Non-ascii characters are not allowed as fill characters");
    }

    // Validate the sign.
    if ((specifier.m_sign != FormatSpecifier::Sign::Default) && !specifier.is_numeric())
    {
        on_error("Sign may only be used with numeric presentation types");
    }

    // Validate the alternate form.
    if (specifier.m_alternate_form &&
        (!specifier.is_numeric() || (specifier.m_type == FormatSpecifier::Type::Decimal)))
    {
        on_error("Alternate form may only be used with non-decimal numeric presentation types");
    }

    // Validate the zero-padding option.
    if (specifier.m_zero_padding && !specifier.is_numeric())
    {
        on_error("Zero-padding may only be used with numeric presentation types");
    }

    // Validate the width value.
    if (specifier.m_width && (specifier.m_width.value() == 0))
    {
        on_error("Width must be a positive (non-zero) value");
    }
    else if (specifier.m_width_position)
    {
        std::optional<ParameterType> nested_type = parameter_type(*specifier.m_width_position);

        if (nested_type != ParameterType::Integral)
        {
            on_error("Position of width parameter must be an integral type");
        }
    }

    // Validate the precision value.
    if ((specifier.m_precision) &&
        ((type != ParameterType::String) && (type != ParameterType::FloatingPoint)))
    {
        on_error("Precision may only be used for string and floating point types");
    }
    else if (specifier.m_precision_position)
    {
        std::optional<ParameterType> nested_type = parameter_type(*specifier.m_precision_position);

        if (nested_type != ParameterType::Integral)
        {
            on_error("Position of precision parameter must be an integral type");
        }
    }

    // Validate the locale-specifc form.
    if (specifier.m_locale_specific_form &&
        ((type != ParameterType::Integral) && (type != ParameterType::FloatingPoint) &&
         (type != ParameterType::Boolean)))
    {
        on_error("Locale-specific form may only be used for numeric and boolean types");
    }

    // Validate the presentation type.
    validate_type(*type, specifier);

    return !has_error();
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
constexpr void BasicFormatString<StringType, ParameterTypes...>::validate_type(
    ParameterType type,
    const FormatSpecifier &specifier)
{
    switch (type)
    {
        case ParameterType::Generic:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::String))
            {
                on_error("Generic types must be formatted with {} or {:s}");
            }
            break;

        case ParameterType::Character:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::Character) &&
                (specifier.m_type != FormatSpecifier::Type::Binary) &&
                (specifier.m_type != FormatSpecifier::Type::Octal) &&
                (specifier.m_type != FormatSpecifier::Type::Decimal) &&
                (specifier.m_type != FormatSpecifier::Type::Hex))
            {
                on_error("Character types must be formatted with {} or {:cbBodxX}");
            }
            break;

        case ParameterType::String:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::String))
            {
                on_error("String types must be formatted with {} or {:s}");
            }
            break;

        case ParameterType::Pointer:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::Pointer))
            {
                on_error("Pointer types must be formatted with {} or {:p}");
            }
            break;

        case ParameterType::Integral:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::Character) &&
                (specifier.m_type != FormatSpecifier::Type::Binary) &&
                (specifier.m_type != FormatSpecifier::Type::Octal) &&
                (specifier.m_type != FormatSpecifier::Type::Decimal) &&
                (specifier.m_type != FormatSpecifier::Type::Hex))
            {
                on_error("Integral types must be formatted with {} or one of {:cbBodxX}");
            }
            break;

        case ParameterType::FloatingPoint:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::HexFloat) &&
                (specifier.m_type != FormatSpecifier::Type::Scientific) &&
                (specifier.m_type != FormatSpecifier::Type::Fixed) &&
                (specifier.m_type != FormatSpecifier::Type::General))
            {
                on_error("Floating point types must be formatted with {} or one of {:aAeEfFgG}");
            }
            break;

        case ParameterType::Boolean:
            if ((specifier.m_type != FormatSpecifier::Type::None) &&
                (specifier.m_type != FormatSpecifier::Type::Character) &&
                (specifier.m_type != FormatSpecifier::Type::String) &&
                (specifier.m_type != FormatSpecifier::Type::Binary) &&
                (specifier.m_type != FormatSpecifier::Type::Octal) &&
                (specifier.m_type != FormatSpecifier::Type::Decimal) &&
                (specifier.m_type != FormatSpecifier::Type::Hex))
            {
                on_error("Boolean types must be formatted with {} or one of {:csbBodxX}");
            }
            break;
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
        else if constexpr (is_integer_v<T>)
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

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
BasicFormatParameters<StringType, ParameterTypes...>::BasicFormatParameters(
    ParameterTypes &&...parameters) :
    m_parameters {std::forward<ParameterTypes>(parameters)...}
{
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename Callback, size_t N>
void BasicFormatParameters<StringType, ParameterTypes...>::visit(
    FormatSpecifier &&specifier,
    Callback callback) const
{
    if constexpr (N < s_parameter_count)
    {
        if (N == specifier.m_position)
        {
            std::invoke(std::move(callback), std::move(specifier), std::get<N>(m_parameters));
            return;
        }

        visit<Callback, N + 1>(std::move(specifier), std::move(callback));
    }
}

//==================================================================================================
template <typename StringType, typename... ParameterTypes>
template <typename T, size_t N>
std::optional<T> BasicFormatParameters<StringType, ParameterTypes...>::get(std::size_t index) const
{
    if constexpr (N < s_parameter_count)
    {
        if (N == index)
        {
            using P = std::remove_cvref_t<std::tuple_element_t<N, FormatParameters>>;

            if constexpr (std::is_integral_v<P> && std::is_convertible_v<P, T>)
            {
                return static_cast<T>(std::get<N>(m_parameters));
            }
        }

        return get<T, N + 1>(index);
    }
    else
    {
        return std::nullopt;
    }
}

} // namespace fly::detail
