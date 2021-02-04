// By default, force FLY_CONSTEVAL to evaluate to 'constexpr' to cause BasicFormatString to store
// errors for runtime analysis. To manually test with 'consteval' for compilers that support it,
// comment out the following line (and note that parsing error tests will not be run).
#define FLY_COMPILER_DISABLE_CONSTEVAL

#include "fly/types/string/detail/string_formatter_types.hpp"

#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string_literal.hpp"

#include "catch2/catch.hpp"

#include <ostream>
#include <string>

using namespace fly::literals::numeric_literals;

namespace {

#define FMT(format) FLY_ARR(char_type, format)

struct GenericType
{
};

[[maybe_unused]] std::ostream &operator<<(std::ostream &stream, const GenericType &)
{
    return stream;
}

[[maybe_unused]] std::wostream &operator<<(std::wostream &stream, const GenericType &)
{
    return stream;
}

enum class DefaultFormattedEnum
{
    One = 1,
};

enum class UserFormattedEnum
{
    One = 1,
};

[[maybe_unused]] std::ostream &operator<<(std::ostream &stream, const UserFormattedEnum &)
{
    return stream;
}

[[maybe_unused]] std::wostream &operator<<(std::wostream &stream, const UserFormattedEnum &)
{
    return stream;
}

class ConstructorCounter
{
public:
    ConstructorCounter()
    {
        ++s_default_constructor_count;
    }

    ConstructorCounter(const ConstructorCounter &)
    {
        ++s_copy_constructor_count;
    }

    ConstructorCounter(ConstructorCounter &&)
    {
        ++s_move_constructor_count;
    }

    static void reset()
    {
        s_default_constructor_count = 0;
        s_copy_constructor_count = 0;
        s_move_constructor_count = 0;
    }

    static std::size_t default_constructor_count()
    {
        return s_default_constructor_count;
    };

    static std::size_t copy_constructor_count()
    {
        return s_copy_constructor_count;
    };

    static std::size_t move_constructor_count()
    {
        return s_move_constructor_count;
    };

private:
    static inline std::size_t s_default_constructor_count {0};
    static inline std::size_t s_copy_constructor_count {0};
    static inline std::size_t s_move_constructor_count {0};
};

template <typename CharType, std::size_t N, typename... Types>
FLY_CONSTEVAL auto make_format(const CharType (&format)[N], Types &&...)
{
    using StringType = std::basic_string<CharType>;
    using FormatString = fly::detail::BasicFormatString<StringType, std::type_identity_t<Types>...>;

    return FormatString(format);
}

template <typename FormatStringType, typename... SpecifierType>
void test_format(FormatStringType &&format, const SpecifierType &...specifiers)
{
    [[maybe_unused]] auto equals = [&format](const auto &specifier)
    {
        auto actual_specifier = format.next_specifier();
        CATCH_REQUIRE(actual_specifier);
        CATCH_CHECK(actual_specifier == specifier);
    };

    CATCH_CHECK_FALSE(format.has_error());
    CATCH_CAPTURE(format.error());
    (equals(specifiers), ...);

    CATCH_CHECK_FALSE(format.next_specifier());
}

#if defined(FLY_COMPILER_DISABLE_CONSTEVAL)

template <typename FormatStringType>
void test_error(FormatStringType &&format, std::string &&error)
{
    CATCH_CHECK(format.has_error());
    CATCH_CHECK(format.error() == error);
}

// Copies of all of the error messages that BasicFormatString might raise. Ideally, these could be
// defined in a common header, but when FLY_CONSTEVAL evaluates to 'consteval', the error message
// raised would not display in the compile error (the variable name or macro name would display).
constexpr const char *s_non_streamable_parameter =
    "An overloaded operator<< must be defined for all format parameters";
constexpr const char *s_unclosed_string = "Detected unclosed format string - must end with }";
constexpr const char *s_unescaped_close = "Closing brace } must be esacped";
constexpr const char *s_too_many_specifiers = "Exceeded maximum allowed number of specifiers";
constexpr const char *s_bad_position = "Argument position exceeds number of provided arguments";
constexpr const char *s_position_mismatch =
    "Argument position must be provided on all or not on any specifier";
constexpr const char *s_bad_fill = "Characters { and } are not allowed as fill characters";
constexpr const char *s_non_ascii_fill = "Non-ascii characters are not allowed as fill characters";
constexpr const char *s_bad_sign = "Sign may only be used with numeric presentation types";
constexpr const char *s_bad_alternate_form =
    "Alternate form may only be used with non-decimal numeric presentation types";
constexpr const char *s_bad_zero_padding =
    "Zero-padding may only be used with numeric presentation types";
constexpr const char *s_bad_width = "Width must be a positive (non-zero) value";
constexpr const char *s_bad_width_position = "Position of width parameter must be an integral type";
constexpr const char *s_missing_precision =
    "Expected a non-negative precision or nested replacement field after decimal";
constexpr const char *s_bad_precision =
    "Precision may only be used for string and floating point types";
constexpr const char *s_bad_precision_position =
    "Position of precision parameter must be an integral type";
constexpr const char *s_bad_locale =
    "Locale-specific form may only be used for numeric and boolean types";
constexpr const char *s_bad_generic = "Generic types must be formatted with {}";
constexpr const char *s_bad_character = "Character types must be formatted with {} or {:cbBodxX}";
constexpr const char *s_bad_string = "String types must be formatted with {} or {:s}";
constexpr const char *s_bad_pointer = "Pointer types must be formatted with {} or {:p}";
constexpr const char *s_bad_integer =
    "Integral types must be formatted with {} or one of {:cbBodxX}";
constexpr const char *s_bad_float =
    "Floating point types must be formatted with {} or one of {:aAeEfFgG}";
constexpr const char *s_bad_bool = "Boolean types must be formatted with {} or one of {:csbBodxX}";

#endif

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "BasicFormatString",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;

    using char_type = typename StringType::value_type;
    using Specifier = fly::detail::BasicFormatSpecifier<char_type>;

    constexpr const GenericType g {};
    constexpr const auto c = FLY_CHR(char_type, 'a');
    constexpr const auto s = FLY_STR(char_type, "a");
    constexpr const auto &a = FLY_ARR(char_type, "a");
    constexpr const int i = 1;
    constexpr const float f = 3.14f;
    constexpr const bool b = true;
    constexpr const DefaultFormattedEnum d = DefaultFormattedEnum::One;
    constexpr const UserFormattedEnum u = UserFormattedEnum::One;

    CATCH_SECTION("No specifiers are parsed from empty string")
    {
        test_format(make_format(FMT("")));
    }

    CATCH_SECTION("No specifiers are parsed from non-empty string without specifiers")
    {
        test_format(make_format(FMT("ab")));
    }

    CATCH_SECTION("Opening braces may be escaped and not parsed as a replacement field")
    {
        test_format(make_format(FMT("{{")));
    }

    CATCH_SECTION("Closing braces may be escaped and not parsed as a replacement field")
    {
        test_format(make_format(FMT("}}")));
    }

    CATCH_SECTION("A single, empty specifier has default values")
    {
        test_format(make_format(FMT("{}"), g), Specifier {});
    }

    CATCH_SECTION("Able to parse the maxiumum allowed number of replacement fields")
    {
        auto format = make_format( // There are 64 format replacement fields here.
            FMT("{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}"
                "{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}"
                "{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}"),
            1);
        CATCH_CHECK_FALSE(format.has_error());

        const std::size_t specifiers_created = format.view().size() / 3;
        std::size_t specifiers_parsed = 0;

        while (format.next_specifier())
        {
            ++specifiers_parsed;
        }

        CATCH_CHECK(specifiers_created == specifiers_parsed);
    }

    CATCH_SECTION("Extra format parameters are ignored")
    {
        test_format(make_format(FMT(""), g, f, b));
        test_format(make_format(FMT("{}"), g, f, b), Specifier {});
    }

    CATCH_SECTION("Format position option can be automatically incremented")
    {
        Specifier specifier1 {};
        Specifier specifier2 {};
        specifier2.m_position = 1;

        test_format(make_format(FMT("{} {}"), g, g), specifier1, specifier2);
    }

    CATCH_SECTION("Format position option can be manually specified")
    {
        Specifier specifier1 {};
        Specifier specifier2 {};
        specifier2.m_position = 1;

        test_format(make_format(FMT("{0}"), g), specifier1);
        test_format(make_format(FMT("{0} {0}"), g), specifier1, specifier1);
        test_format(make_format(FMT("{0} {1}"), g, g), specifier1, specifier2);
        test_format(make_format(FMT("{1} {0}"), g, g), specifier2, specifier1);
    }

    CATCH_SECTION("Fill character may be set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;
        specifier.m_alignment = Specifier::Alignment::Left;

        specifier.m_fill = FLY_CHR(char_type, ' ');
        test_format(make_format(FMT("{: <}"), 1), specifier);

        specifier.m_fill = FLY_CHR(char_type, 'x');
        test_format(make_format(FMT("{:x<}"), 1), specifier);

        specifier.m_fill = FLY_CHR(char_type, 'z');
        test_format(make_format(FMT("{:z<}"), 1), specifier);
    }

    CATCH_SECTION("Alignment option may be set without fill character")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;

        specifier.m_alignment = Specifier::Alignment::Left;
        test_format(make_format(FMT("{:<}"), 1), specifier);

        specifier.m_alignment = Specifier::Alignment::Right;
        test_format(make_format(FMT("{:>}"), 1), specifier);

        specifier.m_alignment = Specifier::Alignment::Center;
        test_format(make_format(FMT("{:^}"), 1), specifier);
    }

    CATCH_SECTION("Alignment option may be set with fill character")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;

        specifier.m_fill = FLY_CHR(char_type, ' ');
        specifier.m_alignment = Specifier::Alignment::Left;
        test_format(make_format(FMT("{: <}"), 1), specifier);

        specifier.m_fill = FLY_CHR(char_type, 'x');
        specifier.m_alignment = Specifier::Alignment::Right;
        test_format(make_format(FMT("{:x>}"), 1), specifier);

        specifier.m_fill = FLY_CHR(char_type, 'z');
        specifier.m_alignment = Specifier::Alignment::Center;
        test_format(make_format(FMT("{:z^}"), 1), specifier);
    }

    CATCH_SECTION("Sign indicator may be set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;

        specifier.m_sign = Specifier::Sign::Always;
        test_format(make_format(FMT("{:+}"), 1), specifier);

        specifier.m_sign = Specifier::Sign::NegativeOnly;
        test_format(make_format(FMT("{:-}"), 1), specifier);

        specifier.m_sign = Specifier::Sign::NegativeOnlyWithPositivePadding;
        test_format(make_format(FMT("{: }"), 1), specifier);
    }

    CATCH_SECTION("Alternate form indicator may be set")
    {
        Specifier specifier {};
        specifier.m_alternate_form = true;

        specifier.m_type = Specifier::Type::Binary;
        test_format(make_format(FMT("{:#b}"), i), specifier);

        specifier.m_type = Specifier::Type::General;
        test_format(make_format(FMT("{:#}"), f), specifier);
    }

    CATCH_SECTION("Zero-padding indicator may be set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;
        specifier.m_zero_padding = true;

        test_format(make_format(FMT("{:0}"), 1), specifier);
    }

    CATCH_SECTION("Zero-padding indicator ignored when alignment option is set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;
        specifier.m_alignment = Specifier::Alignment::Center;
        specifier.m_zero_padding = false;

        test_format(make_format(FMT("{:^0}"), 1), specifier);
    }

    CATCH_SECTION("Width value may be set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;

        specifier.set_width(Specifier::SizeOrPosition::Type::Size, 1);
        test_format(make_format(FMT("{:1}"), 1), specifier);

        specifier.set_width(Specifier::SizeOrPosition::Type::Size, 123);
        test_format(make_format(FMT("{:123}"), 1), specifier);
    }

    CATCH_SECTION("Width position may be set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::String;

        specifier.set_width(Specifier::SizeOrPosition::Type::Position, 1);
        test_format(make_format(FMT("{:{}}"), s, 1), specifier);

        specifier.set_width(Specifier::SizeOrPosition::Type::Position, 1);
        test_format(make_format(FMT("{0:{1}}"), s, 1), specifier);

        specifier.m_position = 1;
        specifier.set_width(Specifier::SizeOrPosition::Type::Position, 0);
        test_format(make_format(FMT("{1:{0}}"), 1, s), specifier);
    }

    CATCH_SECTION("Precision value may be set")
    {
        Specifier specifier {};
        specifier.set_precision(Specifier::SizeOrPosition::Type::Size, 1);

        specifier.m_type = Specifier::Type::String;
        test_format(make_format(FMT("{:.1}"), s), specifier);

        specifier.m_type = Specifier::Type::General;
        test_format(make_format(FMT("{:.1}"), f), specifier);
    }

    CATCH_SECTION("Precision position may be set")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::String;

        specifier.set_precision(Specifier::SizeOrPosition::Type::Position, 1);
        test_format(make_format(FMT("{:.{}}"), s, 1), specifier);

        specifier.set_precision(Specifier::SizeOrPosition::Type::Position, 1);
        test_format(make_format(FMT("{0:.{1}}"), s, 1), specifier);

        specifier.m_position = 1;
        specifier.set_precision(Specifier::SizeOrPosition::Type::Position, 0);
        test_format(make_format(FMT("{1:.{0}}"), 1, s), specifier);
    }

    CATCH_SECTION("Locale-specific form indicator may be set")
    {
        Specifier specifier {};
        specifier.m_locale_specific_form = true;

        specifier.m_type = Specifier::Type::Decimal;
        test_format(make_format(FMT("{:L}"), i), specifier);

        specifier.m_type = Specifier::Type::General;
        test_format(make_format(FMT("{:L}"), f), specifier);

        specifier.m_type = Specifier::Type::String;
        test_format(make_format(FMT("{:L}"), b), specifier);
    }

    CATCH_SECTION("Presentation type may be inferred if not set")
    {
        Specifier specifier {};

        specifier.m_type = Specifier::Type::None;
        test_format(make_format(FMT("{}"), g), specifier);
        test_format(make_format(FMT("{}"), u), specifier);

        specifier.m_type = Specifier::Type::Character;
        test_format(make_format(FMT("{}"), c), specifier);

        specifier.m_type = Specifier::Type::String;
        test_format(make_format(FMT("{}"), s), specifier);
        test_format(make_format(FMT("{}"), a), specifier);

        specifier.m_type = Specifier::Type::Pointer;
        test_format(make_format(FMT("{}"), &i), specifier);

        specifier.m_type = Specifier::Type::Decimal;
        test_format(make_format(FMT("{}"), i), specifier);
        test_format(make_format(FMT("{}"), d), specifier);

        specifier.m_type = Specifier::Type::General;
        test_format(make_format(FMT("{}"), f), specifier);

        specifier.m_type = Specifier::Type::String;
        test_format(make_format(FMT("{}"), b), specifier);
    }

    CATCH_SECTION("Generic types may be formatted without presentation type")
    {
        Specifier specifier {};
        test_format(make_format(FMT("{}"), g), specifier);
        test_format(make_format(FMT("{}"), u), specifier);
    }

    CATCH_SECTION("Presentation type may be set (character)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Character;

        test_format(make_format(FMT("{:c}"), c), specifier);
        test_format(make_format(FMT("{:c}"), i), specifier);
        test_format(make_format(FMT("{:c}"), b), specifier);
        test_format(make_format(FMT("{:c}"), d), specifier);
    }

    CATCH_SECTION("Presentation type may be set (string)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::String;

        test_format(make_format(FMT("{:s}"), s), specifier);
        test_format(make_format(FMT("{:s}"), a), specifier);
        test_format(make_format(FMT("{:s}"), b), specifier);
    }

    CATCH_SECTION("Presentation type may be set (pointer)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Pointer;

        test_format(make_format(FMT("{:p}"), &g), specifier);
        test_format(make_format(FMT("{:p}"), &i), specifier);
        test_format(make_format(FMT("{:p}"), nullptr), specifier);
    }

    CATCH_SECTION("Presentation type may be set (binary)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Binary;

        test_format(make_format(FMT("{:b}"), c), specifier);
        test_format(make_format(FMT("{:b}"), i), specifier);
        test_format(make_format(FMT("{:b}"), b), specifier);
        test_format(make_format(FMT("{:b}"), d), specifier);

        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{:B}"), c), specifier);
        test_format(make_format(FMT("{:B}"), i), specifier);
        test_format(make_format(FMT("{:B}"), b), specifier);
        test_format(make_format(FMT("{:B}"), d), specifier);
    }

    CATCH_SECTION("Presentation type may be set (octal)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Octal;

        test_format(make_format(FMT("{:o}"), c), specifier);
        test_format(make_format(FMT("{:o}"), i), specifier);
        test_format(make_format(FMT("{:o}"), b), specifier);
        test_format(make_format(FMT("{:o}"), d), specifier);
    }

    CATCH_SECTION("Presentation type may be set (decimal)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Decimal;

        test_format(make_format(FMT("{:d}"), c), specifier);
        test_format(make_format(FMT("{:d}"), i), specifier);
        test_format(make_format(FMT("{:d}"), b), specifier);
        test_format(make_format(FMT("{:d}"), d), specifier);
    }

    CATCH_SECTION("Presentation type may be set (hex)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Hex;

        test_format(make_format(FMT("{:x}"), c), specifier);
        test_format(make_format(FMT("{:x}"), i), specifier);
        test_format(make_format(FMT("{:x}"), b), specifier);
        test_format(make_format(FMT("{:x}"), d), specifier);

        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{:X}"), c), specifier);
        test_format(make_format(FMT("{:X}"), i), specifier);
        test_format(make_format(FMT("{:X}"), b), specifier);
        test_format(make_format(FMT("{:X}"), d), specifier);
    }

    CATCH_SECTION("Presentation type may be set (hexfloat)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::HexFloat;
        test_format(make_format(FMT("{:a}"), f), specifier);

        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{:A}"), f), specifier);
    }

    CATCH_SECTION("Presentation type may be set (scientific)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Scientific;
        test_format(make_format(FMT("{:e}"), f), specifier);

        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{:E}"), f), specifier);
    }

    CATCH_SECTION("Presentation type may be set (fixed)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::Fixed;
        test_format(make_format(FMT("{:f}"), f), specifier);

        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{:F}"), f), specifier);
    }

    CATCH_SECTION("Presentation type may be set (general)")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::General;
        test_format(make_format(FMT("{:g}"), f), specifier);

        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{:G}"), f), specifier);
    }

    CATCH_SECTION("Formatting options may be combined")
    {
        Specifier specifier {};
        specifier.m_type = Specifier::Type::General;

        specifier.m_position = 1;
        specifier.m_alignment = Specifier::Alignment::Center;
        test_format(make_format(FMT("{1:^}"), f, f), specifier);

        specifier.m_fill = FLY_CHR(char_type, '_');
        test_format(make_format(FMT("{1:_^}"), f, f), specifier);

        specifier.m_sign = Specifier::Sign::Always;
        test_format(make_format(FMT("{1:_^+}"), f, f), specifier);

        specifier.m_alternate_form = true;
        test_format(make_format(FMT("{1:_^+#}"), f, f), specifier);

        specifier.m_zero_padding = false; // Zero-padding ignored due to alignment.
        test_format(make_format(FMT("{1:_^+#0}"), f, f), specifier);

        specifier.set_width(Specifier::SizeOrPosition::Type::Size, 1);
        test_format(make_format(FMT("{1:_^+#01}"), f, f), specifier);

        specifier.set_precision(Specifier::SizeOrPosition::Type::Size, 2);
        test_format(make_format(FMT("{1:_^+#01.2}"), f, f), specifier);

        specifier.m_locale_specific_form = true;
        test_format(make_format(FMT("{1:_^+#01.2L}"), f, f), specifier);

        specifier.m_type = Specifier::Type::Fixed;
        specifier.m_case = Specifier::Case::Upper;
        test_format(make_format(FMT("{1:_^+#01.2LF}"), f, f), specifier);

        specifier.m_fill = std::nullopt;
        test_format(make_format(FMT("{1:^+#01.2LF}"), f, f), specifier);

        specifier.m_alignment = Specifier::Alignment::Default;
        specifier.m_zero_padding = true; // Zero-padding accepted now.
        test_format(make_format(FMT("{1:+#01.2LF}"), f, f), specifier);

        specifier.m_sign = Specifier::Sign::Default;
        test_format(make_format(FMT("{1:#01.2LF}"), f, f), specifier);

        specifier.m_alternate_form = false;
        test_format(make_format(FMT("{1:01.2LF}"), f, f), specifier);

        specifier.m_zero_padding = false;
        test_format(make_format(FMT("{1:1.2LF}"), f, f), specifier);

        specifier.m_width = std::nullopt;
        test_format(make_format(FMT("{1:.2LF}"), f, f), specifier);

        specifier.m_precision = std::nullopt;
        test_format(make_format(FMT("{1:LF}"), f, f), specifier);

        specifier.m_locale_specific_form = false;
        test_format(make_format(FMT("{1:F}"), f, f), specifier);
    }

    CATCH_SECTION("Specifiers track their size in the format string")
    {
        auto format = make_format(FMT("ab {0} cd {1:d} ef {2:#0x}"), 1, 2, 3);
        CATCH_CHECK_FALSE(format.has_error());

        auto specifier1 = format.next_specifier();
        CATCH_REQUIRE(specifier1);
        CATCH_CHECK(specifier1->m_size == 3);

        auto specifier2 = format.next_specifier();
        CATCH_REQUIRE(specifier2);
        CATCH_CHECK(specifier2->m_size == 5);

        auto specifier3 = format.next_specifier();
        CATCH_REQUIRE(specifier3);
        CATCH_CHECK(specifier3->m_size == 7);

        CATCH_CHECK_FALSE(format.next_specifier());
    }
}

#if defined(FLY_COMPILER_DISABLE_CONSTEVAL)

// This test is broken up because otherwise it is too large for Windows and a stack overflow occurs.
CATCH_TEMPLATE_TEST_CASE(
    "BasicFormatStringErrors",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;

    using char_type = typename StringType::value_type;

    constexpr const GenericType g {};
    constexpr const auto c = FLY_CHR(char_type, 'a');
    constexpr const auto s = FLY_STR(char_type, "a");
    constexpr const int i = 1;
    constexpr const float f = 3.14f;
    constexpr const bool b = true;
    constexpr const DefaultFormattedEnum d = DefaultFormattedEnum::One;
    constexpr const UserFormattedEnum u = UserFormattedEnum::One;

    CATCH_SECTION("Cannot format non-streamable types")
    {
        struct Unstreamable
        {
        };

        test_error(make_format(FMT("{}"), Unstreamable {}), s_non_streamable_parameter);
        test_error(make_format(FMT("{} {}"), 1, Unstreamable {}), s_non_streamable_parameter);
        test_error(make_format(FMT("{} {}"), Unstreamable {}, 1), s_non_streamable_parameter);
    }

    CATCH_SECTION("Cannot parse single opening brace")
    {
        test_error(make_format(FMT("{")), s_unclosed_string);
        test_error(make_format(FMT("{:")), s_unclosed_string);
    }

    CATCH_SECTION("Cannot parse single closing brace")
    {
        test_error(make_format(FMT("}")), s_unescaped_close);
    }

    CATCH_SECTION("Cannot exceed maximum number of replacement fields")
    {
        test_error(
            make_format( // There are 65 format replacement fields here.
                FMT("{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}"
                    "{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}"
                    "{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}{0}"),
                1),
            s_too_many_specifiers);
    }

    CATCH_SECTION("Cannot parse negative position")
    {
        test_error(make_format(FMT("{-1}"), 1), s_unclosed_string);
    }

    CATCH_SECTION("Automatically incremented position may not exceed number of parameters")
    {
        test_error(make_format(FMT("{}")), s_bad_position);
        test_error(make_format(FMT("{} {}"), 1), s_bad_position);
    }

    CATCH_SECTION("Manually specified position may not exceed number of parameters")
    {
        test_error(make_format(FMT("{0}")), s_bad_position);
        test_error(make_format(FMT("{1}"), 1), s_bad_position);
    }

    CATCH_SECTION("Cannot mix automatic and manual position options")
    {
        test_error(make_format(FMT("{0} {}"), 1, 2), s_position_mismatch);
        test_error(make_format(FMT("{} {1}"), 1, 2), s_position_mismatch);
    }

    CATCH_SECTION("Fill character must not be opening or closing brace")
    {
        test_error(make_format(FMT("{:{^}"), 1), s_bad_fill);
        test_error(make_format(FMT("{:}^}"), 1), s_bad_fill);
    }

    CATCH_SECTION("Fill character must be ASCII")
    {
        {
            constexpr const char_type fmt[] {
                static_cast<char_type>(0x7b), // {
                static_cast<char_type>(0x3a), // :
                static_cast<char_type>(0x80), // Non-ASCII
                static_cast<char_type>(0x5e), // ^
                static_cast<char_type>(0x7d), // }
            };

            test_error(make_format(fmt, 1), s_non_ascii_fill);
        }
        {
            constexpr const char_type fmt[] {
                static_cast<char_type>(0x7b), // {
                static_cast<char_type>(0x3a), // :
                static_cast<char_type>(0xff), // Non-ASCII
                static_cast<char_type>(0x5e), // ^
                static_cast<char_type>(0x7d), // }
            };

            test_error(make_format(fmt, 1), s_non_ascii_fill);
        }
    }

    CATCH_SECTION("Sign only valid for numeric types")
    {
        test_error(make_format(FMT("{:+}"), s), s_bad_sign);
        test_error(make_format(FMT("{:+}"), b), s_bad_sign);

        test_error(make_format(FMT("{:-}"), s), s_bad_sign);
        test_error(make_format(FMT("{:-}"), b), s_bad_sign);

        test_error(make_format(FMT("{: }"), s), s_bad_sign);
        test_error(make_format(FMT("{: }"), b), s_bad_sign);
    }

    CATCH_SECTION("Alternate form only valid for non-decimal numeric types")
    {
        test_error(make_format(FMT("{:#d}"), i), s_bad_alternate_form);
        test_error(make_format(FMT("{:#}"), s), s_bad_alternate_form);
        test_error(make_format(FMT("{:#}"), g), s_bad_alternate_form);
    }

    CATCH_SECTION("Zero-padding only valid for numeric types")
    {
        test_error(make_format(FMT("{:0}"), s), s_bad_zero_padding);
        test_error(make_format(FMT("{:0}"), b), s_bad_zero_padding);
    }

    CATCH_SECTION("Width value must be positive")
    {
        // Double zero because the first zero is parsed as the zero-padding indicator.
        test_error(make_format(FMT("{:00}"), 1), s_bad_width);

        // Double negative because the negative sign is parsed as the sign indicator.
        test_error(make_format(FMT("{:--1}"), 1), s_unclosed_string);
    }

    CATCH_SECTION("Width position must be integral")
    {
        test_error(make_format(FMT("{:{}}"), 1, s), s_bad_width_position);
        test_error(make_format(FMT("{0:{1}}"), 1, s), s_bad_width_position);
    }

    CATCH_SECTION("Width position value must be postive")
    {
        test_error(make_format(FMT("{0:{-1}}"), s, 1), s_unclosed_string);
    }

    CATCH_SECTION("Width position replacement field may only contain position field")
    {
        test_error(make_format(FMT("{0:{1:}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{0:{1:^}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{0:{1:+}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{0:{1:d}}"), s, 1), s_unclosed_string);
    }

    CATCH_SECTION("Cannot specify both width value and width position")
    {
        test_error(make_format(FMT("{:1{}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{:{}1}"), s, 1), s_unclosed_string);
    }

    CATCH_SECTION("Precision value must follow decimal point")
    {
        test_error(make_format(FMT("{:.}"), 1), s_missing_precision);
    }

    CATCH_SECTION("Precision value must be non-negative")
    {
        test_error(make_format(FMT("{:.-1}"), 1), s_missing_precision);
    }

    CATCH_SECTION("Precision value only valid for string and floating point types")
    {
        test_error(make_format(FMT("{:.1}"), 1), s_bad_precision);
    }

    CATCH_SECTION("Precision position must be integral")
    {
        test_error(make_format(FMT("{:.{}}"), s, s), s_bad_precision_position);
        test_error(make_format(FMT("{0:.{1}}"), s, s), s_bad_precision_position);
    }

    CATCH_SECTION("Precision position only valid for string and floating point types")
    {
        test_error(make_format(FMT("{:.{}}"), 1, 1), s_bad_precision);
        test_error(make_format(FMT("{0:.{1}}"), 1, 1), s_bad_precision);
    }

    CATCH_SECTION("Precision position value must be postive")
    {
        test_error(make_format(FMT("{0:.{-1}}"), s, 1), s_unclosed_string);
    }

    CATCH_SECTION("Precision position replacement field may only contain position field")
    {
        test_error(make_format(FMT("{0:.{1:}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{0:.{1:^}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{0:.{1:+}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{0:.{1:d}}"), s, 1), s_unclosed_string);
    }

    CATCH_SECTION("Cannot specify both precision value and precision position")
    {
        test_error(make_format(FMT("{:.1{}}"), s, 1), s_unclosed_string);
        test_error(make_format(FMT("{:.{}1}"), s, 1), s_unclosed_string);
    }

    CATCH_SECTION("Locale-specific form only valid for numeric and boolean types")
    {
        test_error(make_format(FMT("{:L}"), s), s_bad_locale);
    }

    CATCH_SECTION("Precision type mismatch (character)")
    {
        test_error(make_format(FMT("{:c}"), g), s_bad_generic);
        test_error(make_format(FMT("{:c}"), u), s_bad_generic);
        test_error(make_format(FMT("{:c}"), s), s_bad_string);
        test_error(make_format(FMT("{:c}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:c}"), f), s_bad_float);
    }

    CATCH_SECTION("Precision type mismatch (string)")
    {
        test_error(make_format(FMT("{:c}"), g), s_bad_generic);
        test_error(make_format(FMT("{:c}"), u), s_bad_generic);
        test_error(make_format(FMT("{:s}"), c), s_bad_character);
        test_error(make_format(FMT("{:s}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:s}"), i), s_bad_integer);
        test_error(make_format(FMT("{:s}"), d), s_bad_integer);
        test_error(make_format(FMT("{:s}"), f), s_bad_float);
    }

    CATCH_SECTION("Precision type mismatch (pointer)")
    {
        test_error(make_format(FMT("{:p}"), g), s_bad_generic);
        test_error(make_format(FMT("{:p}"), c), s_bad_character);
        test_error(make_format(FMT("{:p}"), i), s_bad_integer);
        test_error(make_format(FMT("{:p}"), d), s_bad_integer);
        test_error(make_format(FMT("{:p}"), f), s_bad_float);
        test_error(make_format(FMT("{:p}"), b), s_bad_bool);
    }

    CATCH_SECTION("Precision type mismatch (binary)")
    {
        test_error(make_format(FMT("{:b}"), g), s_bad_generic);
        test_error(make_format(FMT("{:b}"), u), s_bad_generic);
        test_error(make_format(FMT("{:b}"), s), s_bad_string);
        test_error(make_format(FMT("{:b}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:b}"), f), s_bad_float);

        test_error(make_format(FMT("{:B}"), g), s_bad_generic);
        test_error(make_format(FMT("{:B}"), s), s_bad_string);
        test_error(make_format(FMT("{:B}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:B}"), f), s_bad_float);
    }

    CATCH_SECTION("Precision type mismatch (octal)")
    {
        test_error(make_format(FMT("{:o}"), g), s_bad_generic);
        test_error(make_format(FMT("{:o}"), u), s_bad_generic);
        test_error(make_format(FMT("{:o}"), s), s_bad_string);
        test_error(make_format(FMT("{:o}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:o}"), f), s_bad_float);
    }

    CATCH_SECTION("Precision type mismatch (decimal)")
    {
        test_error(make_format(FMT("{:d}"), g), s_bad_generic);
        test_error(make_format(FMT("{:d}"), u), s_bad_generic);
        test_error(make_format(FMT("{:d}"), s), s_bad_string);
        test_error(make_format(FMT("{:d}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:d}"), f), s_bad_float);
    }

    CATCH_SECTION("Precision type mismatch (hex)")
    {
        test_error(make_format(FMT("{:x}"), g), s_bad_generic);
        test_error(make_format(FMT("{:x}"), u), s_bad_generic);
        test_error(make_format(FMT("{:x}"), s), s_bad_string);
        test_error(make_format(FMT("{:x}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:x}"), f), s_bad_float);

        test_error(make_format(FMT("{:X}"), g), s_bad_generic);
        test_error(make_format(FMT("{:X}"), u), s_bad_generic);
        test_error(make_format(FMT("{:X}"), s), s_bad_string);
        test_error(make_format(FMT("{:X}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:X}"), f), s_bad_float);
    }

    CATCH_SECTION("Precision type mismatch (hexfloat)")
    {
        test_error(make_format(FMT("{:a}"), g), s_bad_generic);
        test_error(make_format(FMT("{:a}"), u), s_bad_generic);
        test_error(make_format(FMT("{:a}"), c), s_bad_character);
        test_error(make_format(FMT("{:a}"), s), s_bad_string);
        test_error(make_format(FMT("{:a}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:a}"), i), s_bad_integer);
        test_error(make_format(FMT("{:a}"), d), s_bad_integer);
        test_error(make_format(FMT("{:a}"), b), s_bad_bool);

        test_error(make_format(FMT("{:A}"), g), s_bad_generic);
        test_error(make_format(FMT("{:A}"), u), s_bad_generic);
        test_error(make_format(FMT("{:A}"), c), s_bad_character);
        test_error(make_format(FMT("{:A}"), s), s_bad_string);
        test_error(make_format(FMT("{:A}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:A}"), i), s_bad_integer);
        test_error(make_format(FMT("{:A}"), d), s_bad_integer);
        test_error(make_format(FMT("{:A}"), b), s_bad_bool);
    }

    CATCH_SECTION("Precision type mismatch (scientific)")
    {
        test_error(make_format(FMT("{:e}"), g), s_bad_generic);
        test_error(make_format(FMT("{:e}"), u), s_bad_generic);
        test_error(make_format(FMT("{:e}"), c), s_bad_character);
        test_error(make_format(FMT("{:e}"), s), s_bad_string);
        test_error(make_format(FMT("{:e}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:e}"), i), s_bad_integer);
        test_error(make_format(FMT("{:e}"), d), s_bad_integer);
        test_error(make_format(FMT("{:e}"), b), s_bad_bool);

        test_error(make_format(FMT("{:E}"), g), s_bad_generic);
        test_error(make_format(FMT("{:E}"), u), s_bad_generic);
        test_error(make_format(FMT("{:E}"), c), s_bad_character);
        test_error(make_format(FMT("{:E}"), s), s_bad_string);
        test_error(make_format(FMT("{:E}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:E}"), i), s_bad_integer);
        test_error(make_format(FMT("{:E}"), d), s_bad_integer);
        test_error(make_format(FMT("{:E}"), b), s_bad_bool);
    }

    CATCH_SECTION("Precision type mismatch (fixed)")
    {
        test_error(make_format(FMT("{:f}"), g), s_bad_generic);
        test_error(make_format(FMT("{:f}"), u), s_bad_generic);
        test_error(make_format(FMT("{:f}"), c), s_bad_character);
        test_error(make_format(FMT("{:f}"), s), s_bad_string);
        test_error(make_format(FMT("{:f}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:f}"), i), s_bad_integer);
        test_error(make_format(FMT("{:f}"), d), s_bad_integer);
        test_error(make_format(FMT("{:f}"), b), s_bad_bool);

        test_error(make_format(FMT("{:F}"), g), s_bad_generic);
        test_error(make_format(FMT("{:F}"), u), s_bad_generic);
        test_error(make_format(FMT("{:F}"), c), s_bad_character);
        test_error(make_format(FMT("{:F}"), s), s_bad_string);
        test_error(make_format(FMT("{:F}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:F}"), i), s_bad_integer);
        test_error(make_format(FMT("{:F}"), d), s_bad_integer);
        test_error(make_format(FMT("{:F}"), b), s_bad_bool);
    }

    CATCH_SECTION("Precision type mismatch (general)")
    {
        test_error(make_format(FMT("{:g}"), g), s_bad_generic);
        test_error(make_format(FMT("{:g}"), u), s_bad_generic);
        test_error(make_format(FMT("{:g}"), c), s_bad_character);
        test_error(make_format(FMT("{:g}"), s), s_bad_string);
        test_error(make_format(FMT("{:g}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:g}"), i), s_bad_integer);
        test_error(make_format(FMT("{:g}"), d), s_bad_integer);
        test_error(make_format(FMT("{:g}"), b), s_bad_bool);

        test_error(make_format(FMT("{:G}"), g), s_bad_generic);
        test_error(make_format(FMT("{:G}"), u), s_bad_generic);
        test_error(make_format(FMT("{:G}"), c), s_bad_character);
        test_error(make_format(FMT("{:G}"), s), s_bad_string);
        test_error(make_format(FMT("{:G}"), &g), s_bad_pointer);
        test_error(make_format(FMT("{:G}"), i), s_bad_integer);
        test_error(make_format(FMT("{:G}"), d), s_bad_integer);
        test_error(make_format(FMT("{:G}"), b), s_bad_bool);
    }

    CATCH_SECTION("Cannot parse combined presentation types")
    {
        test_error(make_format(FMT("{:cs}"), c), s_unclosed_string);
        test_error(make_format(FMT("{:ss}"), s), s_unclosed_string);
        test_error(make_format(FMT("{:ps}"), &g), s_unclosed_string);
        test_error(make_format(FMT("{:bs}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:Bs}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:os}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:ds}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:xs}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:Xs}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:as}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:As}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:es}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:Es}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:fs}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:Fs}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:gs}"), f), s_unclosed_string);
        test_error(make_format(FMT("{:Gs}"), f), s_unclosed_string);
    }

    CATCH_SECTION("Cannot parse non-presentation types")
    {
        test_error(make_format(FMT("{:h}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:i}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:j}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:k}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:l}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:m}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:n}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:q}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:r}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:t}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:u}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:v}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:w}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:y}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:z}"), i), s_unclosed_string);

        test_error(make_format(FMT("{:C}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:D}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:H}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:I}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:J}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:K}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:LL}"), i), s_unclosed_string); // First L parsed as locale.
        test_error(make_format(FMT("{:M}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:N}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:O}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:P}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:Q}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:R}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:S}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:T}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:U}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:V}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:W}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:Y}"), i), s_unclosed_string);
        test_error(make_format(FMT("{:Z}"), i), s_unclosed_string);
    }

    CATCH_SECTION("Cannot parse erroneous whitespace")
    {
        test_error(make_format(FMT("{ 0:_^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0 :_^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0: _^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_ ^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^ +#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+ #01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+# 01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#0 1.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01 .2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01. 2Lf}"), f), s_missing_precision);
        test_error(make_format(FMT("{0:_^+#01.2 Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01.2L f}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01.2Lf }"), f), s_unclosed_string);

        test_error(make_format(FMT("{\t0:_^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0\t:_^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:\t_^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_\t^+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^\t+#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+\t#01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#\t01.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#0\t1.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01\t.2Lf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01.\t2Lf}"), f), s_missing_precision);
        test_error(make_format(FMT("{0:_^+#01.2\tLf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01.2L\tf}"), f), s_unclosed_string);
        test_error(make_format(FMT("{0:_^+#01.2Lf\t}"), f), s_unclosed_string);
    }
}

#endif

CATCH_TEMPLATE_TEST_CASE(
    "BasicFormatParameters",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;

    using char_type = typename StringType::value_type;
    using Specifier = fly::detail::BasicFormatSpecifier<char_type>;

    ConstructorCounter::reset();
    std::size_t visit_count = 0;

    auto make_specifier = [](std::size_t position) -> Specifier
    {
        Specifier specifier {};
        specifier.m_position = position;

        return specifier;
    };

    auto visitor = [&visit_count](auto &&, const auto &)
    {
        ++visit_count;
    };

    CATCH_SECTION("Empty parameters do not cause an error")
    {
        fly::detail::BasicFormatParameters<StringType> parameters;

        parameters.visit(make_specifier(0), visitor);
        CATCH_CHECK(visit_count == 0);
    }

    CATCH_SECTION("A single parameter can be visited, but no others")
    {
        fly::detail::BasicFormatParameters<StringType, int> parameters(1);

        parameters.visit(make_specifier(0), visitor);
        CATCH_CHECK(visit_count == 1);

        parameters.visit(make_specifier(1), visitor);
        CATCH_CHECK(visit_count == 1);
    }

    CATCH_SECTION("Parameters can be copied if of a compatible type")
    {
        fly::detail::BasicFormatParameters<StringType, int> parameters(1);

        auto value1 = parameters.template get<int>(0);
        CATCH_REQUIRE(value1);
        CATCH_CHECK(value1 == 1);

        auto value2 = parameters.template get<std::size_t>(0);
        CATCH_REQUIRE(value2);
        CATCH_CHECK(value2 == 1_zu);

        auto value3 = parameters.template get<std::size_t>(1);
        CATCH_CHECK_FALSE(value3);
    }

    CATCH_SECTION("Parameters cannot be copied if of a non-integral type")
    {
        fly::detail::BasicFormatParameters<StringType, std::string> parameters("ab");

        auto value = parameters.template get<std::string>(0);
        CATCH_CHECK_FALSE(value);
    }

    CATCH_SECTION("Parameters cannot be copied if of an incompatible type")
    {
        fly::detail::BasicFormatParameters<StringType, int> parameters(1);

        auto value = parameters.template get<std::string>(0);
        CATCH_CHECK_FALSE(value);
    }

    CATCH_SECTION("Stored parameters are not copied or moved")
    {
        ConstructorCounter c1;
        const ConstructorCounter c2;

        using T1 = std::add_lvalue_reference_t<decltype(c1)>;
        using T2 = std::add_lvalue_reference_t<decltype(c2)>;

        fly::detail::BasicFormatParameters<StringType, T1, T2> parameters(c1, c2);

        parameters.visit(make_specifier(0), visitor);
        CATCH_CHECK(visit_count == 1);

        parameters.visit(make_specifier(1), visitor);
        CATCH_CHECK(visit_count == 2);

        parameters.visit(make_specifier(2), visitor);
        CATCH_CHECK(visit_count == 2);

        CATCH_CHECK(ConstructorCounter::default_constructor_count() == 2);
        CATCH_CHECK(ConstructorCounter::copy_constructor_count() == 0);
        CATCH_CHECK(ConstructorCounter::move_constructor_count() == 0);
    }
}
