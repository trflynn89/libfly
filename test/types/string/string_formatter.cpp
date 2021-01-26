// By default, force FLY_CONSTEVAL to evaluate to 'constexpr' to cause BasicFormatString to store
// errors for runtime analysis. To manually test with 'consteval' for compilers that support it,
// comment out the following line (and note that parsing error tests will not be run).
#define FLY_COMPILER_DISABLE_CONSTEVAL

#include "fly/types/string/detail/string_formatter.hpp"

#include "fly/fly.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "fly/types/string/string_literal.hpp"

#include "catch2/catch.hpp"

#include <limits>
#include <type_traits>

using namespace fly::literals::numeric_literals;

namespace {

#define FMT(format) FLY_ARR(char_type, format)

template <typename StringType, typename... ParameterTypes>
using FormatString = fly::detail::BasicFormatString<
    fly::detail::is_like_supported_string_t<StringType>,
    std::type_identity_t<ParameterTypes>...>;

template <typename StringType, typename... ParameterTypes>
void test_format(
    FormatString<StringType, ParameterTypes...> &&format,
    StringType &&expected,
    ParameterTypes &&...parameters)
{
    using BasicString = fly::BasicString<fly::detail::is_like_supported_string_t<StringType>>;

    auto result = BasicString::format(
        std::forward<FormatString<StringType, ParameterTypes...>>(format),
        std::forward<ParameterTypes>(parameters)...);

    CATCH_CHECK(result == expected);
}

} // namespace

CATCH_TEMPLATE_TEST_CASE(
    "BasicStringFormatter",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using BasicString = fly::BasicString<StringType>;
    using char_type = typename BasicString::char_type;
    using view_type = typename BasicString::view_type;

    auto is_all_hex = [](view_type value)
    {
        if (value.starts_with(FLY_STR(char_type, "0x")))
        {
            value = value.substr(2);
        }

        for (const auto &ch : value)
        {
            if (!BasicString::is_x_digit(ch))
            {
                return false;
            }
        }

        return !value.empty();
    };

    CATCH_SECTION("Format string without replacement fields")
    {
        test_format(FMT(""), FMT(""));
        test_format(FMT("ab"), FMT("ab"));
    }

    CATCH_SECTION("Opening braces may be escaped and not parsed as a replacement field")
    {
        test_format(FMT("{{"), FMT("{"));
        test_format(FMT("{{{{"), FMT("{{"));
        test_format(FMT("{{ {{"), FMT("{ {"));
    }

    CATCH_SECTION("Closing braces may be escaped and not parsed as a replacement field")
    {
        test_format(FMT("}}"), FMT("}"));
        test_format(FMT("}}}}"), FMT("}}"));
        test_format(FMT("}} }}"), FMT("} }"));
    }

    CATCH_SECTION("Automatic positioning of format parameters formats in order")
    {
        test_format(FMT("{}"), FMT("1"), 1);
        test_format(FMT("{} {}"), FMT("1 2"), 1, 2);
        test_format(FMT("{} {} {}"), FMT("1 2 3"), 1, 2, 3);
    }

    CATCH_SECTION("Manual positioning of format parameters formats in order")
    {
        test_format(FMT("{0}"), FMT("1"), 1);
        test_format(FMT("{0} {1}"), FMT("1 2"), 1, 2);
        test_format(FMT("{1} {0}"), FMT("2 1"), 1, 2);
        test_format(FMT("{0} {1} {2}"), FMT("1 2 3"), 1, 2, 3);
        test_format(FMT("{2} {1} {0}"), FMT("3 2 1"), 1, 2, 3);
        test_format(FMT("{0} {1} {0}"), FMT("1 2 1"), 1, 2, 3);
    }

    CATCH_SECTION("Fill character defaults to a space character")
    {
        test_format(FMT("{:6}"), FMT("     1"), 1);
        test_format(FMT("{:4}_{:4}"), FMT("   1_ab  "), 1, FLY_STR(char_type, "ab"));
    }

    CATCH_SECTION("Fill character may be set")
    {
        test_format(FMT("{:*>6}"), FMT("*****1"), 1);
        test_format(FMT("{:|>4} {:_>4}"), FMT("|||1 __ab"), 1, FLY_STR(char_type, "ab"));
    }

    CATCH_SECTION("Fill character is placed outside of sign and base indicators")
    {
        test_format(FMT("{:*<+6}"), FMT("+1****"), 1);
        test_format(FMT("{:*< 6}"), FMT(" 1****"), 1);
        test_format(FMT("{:*<#6b}"), FMT("0b11**"), 0b11);
        test_format(FMT("{:*<#6B}"), FMT("0B11**"), 0b11);
        test_format(FMT("{:*<#6x}"), FMT("0x41**"), 0x41);
        test_format(FMT("{:*<#6X}"), FMT("0X41**"), 0x41);

        test_format(FMT("{:*>+6}"), FMT("****+1"), 1);
        test_format(FMT("{:*> 6}"), FMT("**** 1"), 1);
        test_format(FMT("{:*>#6b}"), FMT("**0b11"), 0b11);
        test_format(FMT("{:*>#6B}"), FMT("**0B11"), 0b11);
        test_format(FMT("{:*>#6x}"), FMT("**0x41"), 0x41);
        test_format(FMT("{:*>#6X}"), FMT("**0X41"), 0x41);

        test_format(FMT("{:*^+6}"), FMT("****+1"), 1);
        test_format(FMT("{:*^ 6}"), FMT("**** 1"), 1);
        test_format(FMT("{:*^#6b}"), FMT("**0b11"), 0b11);
        test_format(FMT("{:*^#6B}"), FMT("**0B11"), 0b11);
        test_format(FMT("{:*^#6x}"), FMT("**0x41"), 0x41);
        test_format(FMT("{:*^#6X}"), FMT("**0X41"), 0x41);
    }

    CATCH_SECTION("Alignment default is based on presentation type")
    {
        test_format(FMT("{:6}"), FMT("ab    "), FLY_STR(char_type, "ab"));
        test_format(FMT("{:6}"), FMT("     1"), 1);
        test_format(FMT("{:6b}"), FMT("    11"), 0b11);
        test_format(FMT("{:6.2f}"), FMT("  3.14"), 3.14);
    }

    CATCH_SECTION("Alignment may be set to left-alignment")
    {
        test_format(FMT("{:<6}"), FMT("ab    "), FLY_STR(char_type, "ab"));
        test_format(FMT("{:<6}"), FMT("1     "), 1);
        test_format(FMT("{:<6b}"), FMT("11    "), 0b11);
        test_format(FMT("{:<6.2f}"), FMT("3.14  "), 3.14);
    }

    CATCH_SECTION("Alignment may be set to right-alignment")
    {
        test_format(FMT("{:>6}"), FMT("    ab"), FLY_STR(char_type, "ab"));
        test_format(FMT("{:>6}"), FMT("     1"), 1);
        test_format(FMT("{:>6b}"), FMT("    11"), 0b11);
        test_format(FMT("{:>6.2f}"), FMT("  3.14"), 3.14);
    }

    CATCH_SECTION("Alignment affects sign and base indicators")
    {
        test_format(FMT("{:<+6}"), FMT("+1    "), 1);
        test_format(FMT("{:< 6}"), FMT(" 1    "), 1);
        test_format(FMT("{:<#6b}"), FMT("0b11  "), 0b11);
        test_format(FMT("{:<#6B}"), FMT("0B11  "), 0b11);
        test_format(FMT("{:<#6x}"), FMT("0x41  "), 0x41);
        test_format(FMT("{:<#6X}"), FMT("0X41  "), 0x41);

        test_format(FMT("{:>+6}"), FMT("    +1"), 1);
        test_format(FMT("{:> 6}"), FMT("     1"), 1);
        test_format(FMT("{:>#6b}"), FMT("  0b11"), 0b11);
        test_format(FMT("{:>#6B}"), FMT("  0B11"), 0b11);
        test_format(FMT("{:>#6x}"), FMT("  0x41"), 0x41);
        test_format(FMT("{:>#6X}"), FMT("  0X41"), 0x41);

        test_format(FMT("{:^+6}"), FMT("    +1"), 1);
        test_format(FMT("{:^ 6}"), FMT("     1"), 1);
        test_format(FMT("{:^#6b}"), FMT("  0b11"), 0b11);
        test_format(FMT("{:^#6B}"), FMT("  0B11"), 0b11);
        test_format(FMT("{:^#6x}"), FMT("  0x41"), 0x41);
        test_format(FMT("{:^#6X}"), FMT("  0X41"), 0x41);
    }

    CATCH_SECTION("Alignment may be set to center-alignment (defaults to type-based alignment)")
    {
        test_format(FMT("{:^6}"), FMT("ab    "), FLY_STR(char_type, "ab"));
        test_format(FMT("{:^6}"), FMT("     1"), 1);
        test_format(FMT("{:^6b}"), FMT("    11"), 0b11);
        test_format(FMT("{:^6.2f}"), FMT("  3.14"), 3.14);
    }

    CATCH_SECTION("Sign defaults to negative-only")
    {
        test_format(FMT("{}"), FMT("1"), 1);
        test_format(FMT("{}"), FMT("-1"), -1);
        test_format(FMT("{}"), FMT("3.14"), 3.14);
        test_format(FMT("{}"), FMT("-3.14"), -3.14);
    }

    CATCH_SECTION("Sign may be set to always be used")
    {
        test_format(FMT("{:+}"), FMT("+1"), 1);
        test_format(FMT("{:+}"), FMT("-1"), -1);
        test_format(FMT("{:+}"), FMT("+3.14"), 3.14);
        test_format(FMT("{:+}"), FMT("-3.14"), -3.14);
    }

    CATCH_SECTION("Sign may be set to use a space padding symbol")
    {
        test_format(FMT("{: }"), FMT(" 1"), 1);
        test_format(FMT("{: }"), FMT("-1"), -1);
        test_format(FMT("{: }"), FMT(" 3.14"), 3.14);
        test_format(FMT("{: }"), FMT("-3.14"), -3.14);

        // Ensure explicit padding does not change the postive padding.
        test_format(FMT("{:*^ }"), FMT(" 1"), 1);
    }

    CATCH_SECTION("Alternate form is not used by default")
    {
        test_format(FMT("{:b}"), FMT("1"), 1);
        test_format(FMT("{:B}"), FMT("1"), 1);
        test_format(FMT("{:o}"), FMT("1"), 1);
        test_format(FMT("{:x}"), FMT("1"), 1);
        test_format(FMT("{:X}"), FMT("1"), 1);
        test_format(FMT("{}"), FMT("1"), 1.0);
        test_format(FMT("{}"), FMT("1.2"), 1.2);
    }

    CATCH_SECTION("Alternate form adds prefix for integral types")
    {
        test_format(FMT("{:#b}"), FMT("0b1"), 1);
        test_format(FMT("{:#b}"), FMT("0b1"), 1U);
        test_format(FMT("{:#B}"), FMT("0B1"), 1);
        test_format(FMT("{:#B}"), FMT("0B1"), 1U);
        test_format(FMT("{:#o}"), FMT("01"), 1);
        test_format(FMT("{:#x}"), FMT("0x1"), 1);
        test_format(FMT("{:#X}"), FMT("0X1"), 1);
    }

    CATCH_SECTION("Alternate form preserves decimal for floating point types")
    {
        test_format(FMT("{:#g}"), FMT("1.00000"), 1.0);
        test_format(FMT("{:#g}"), FMT("1.20000"), 1.2);
    }

    CATCH_SECTION("Zero-padding is not used by default")
    {
        test_format(FMT("{:6b}"), FMT("    11"), 0b11);
        test_format(FMT("{:#6b}"), FMT("  0b11"), 0b11);
        test_format(FMT("{:6x}"), FMT("    41"), 0x41);
        test_format(FMT("{:#6x}"), FMT("  0x41"), 0x41);
        test_format(FMT("{:6}"), FMT("   -41"), -41);
        test_format(FMT("{:+6}"), FMT("   +41"), 41);
        test_format(FMT("{: 6}"), FMT("    41"), 41);
    }

    CATCH_SECTION("Zero-padding inserts zeros before sign and base indicators")
    {
        test_format(FMT("{:06b}"), FMT("000011"), 0b11);
        test_format(FMT("{:#06b}"), FMT("0b0011"), 0b11);
        test_format(FMT("{:06x}"), FMT("000041"), 0x41);
        test_format(FMT("{:#06x}"), FMT("0x0041"), 0x41);
        test_format(FMT("{:06}"), FMT("-00041"), -41);
        test_format(FMT("{:+06}"), FMT("+00041"), 41);
        test_format(FMT("{: 06}"), FMT(" 00041"), 41);
    }

    CATCH_SECTION("Zero-padding indicator ignored when alignment option is set")
    {
        test_format(FMT("{:>06b}"), FMT("    11"), 0b11);
        test_format(FMT("{:>#06b}"), FMT("  0b11"), 0b11);
        test_format(FMT("{:>06x}"), FMT("    41"), 0x41);
        test_format(FMT("{:>#06x}"), FMT("  0x41"), 0x41);
        test_format(FMT("{:>06}"), FMT("   -41"), -41);
        test_format(FMT("{:>+06}"), FMT("   +41"), 41);
        test_format(FMT("{:> 06}"), FMT("    41"), 41);
    }

    CATCH_SECTION("Width value may be set")
    {
        test_format(FMT("{:2}"), FMT("ab"), FLY_STR(char_type, "ab"));
        test_format(FMT("{:3}"), FMT("ab "), FLY_STR(char_type, "ab"));
        test_format(FMT("{:4}"), FMT("ab  "), FLY_STR(char_type, "ab"));
    }

    CATCH_SECTION("Width position may be set")
    {
        test_format(FMT("{:{}}"), FMT("ab"), FLY_STR(char_type, "ab"), 2);
        test_format(FMT("{0:{1}}"), FMT("ab "), FLY_STR(char_type, "ab"), 3);
        test_format(FMT("{1:{0}}"), FMT("ab  "), 4, FLY_STR(char_type, "ab"));
    }

    CATCH_SECTION("Width position is ignored if the position value is non-positive")
    {
        test_format(FMT("{:{}}"), FMT("ab"), FLY_STR(char_type, "ab"), -2);
        test_format(FMT("{0:{1}}"), FMT("ab"), FLY_STR(char_type, "ab"), -3);
        test_format(FMT("{1:{0}}"), FMT("ab"), -4, FLY_STR(char_type, "ab"));
        test_format(FMT("{1:{0}}"), FMT("ab"), 0, FLY_STR(char_type, "ab"));
    }

    CATCH_SECTION("Width value does not reduce values requiring larger width")
    {
        test_format(FMT("{:2}"), FMT("abcdef"), FLY_STR(char_type, "abcdef"));
        test_format(FMT("{:3}"), FMT("123456"), 123456);
    }

    CATCH_SECTION("Precision value sets floating point precision")
    {
        test_format(FMT("{:.3f}"), FMT("1.000"), 1.0);
        test_format(FMT("{:.2f}"), FMT("3.14"), 3.14159);
    }

    CATCH_SECTION("Precision value sets maximum string size")
    {
        test_format(FMT("{:.3s}"), FMT("ab"), FLY_STR(char_type, "ab"));
        test_format(FMT("{:.3s}"), FMT("abc"), FLY_STR(char_type, "abcdef"));
    }

    CATCH_SECTION("Precision position sets floating point precision")
    {
        test_format(FMT("{:.{}f}"), FMT("1.000"), 1.0, 3);
        test_format(FMT("{0:.{1}f}"), FMT("3.14"), 3.14159, 2);
        test_format(FMT("{1:.{0}f}"), FMT("3.14"), 2, 3.14159);
    }

    CATCH_SECTION("Precision position sets maximum string size")
    {
        test_format(FMT("{:.{}s}"), FMT("ab"), FLY_STR(char_type, "ab"), 3);
        test_format(FMT("{0:.{1}s}"), FMT("abc"), FLY_STR(char_type, "abcdef"), 3);
        test_format(FMT("{1:.{0}s}"), FMT("abc"), 3, FLY_STR(char_type, "abcdef"));
    }

    CATCH_SECTION("Precision position is ignored if the position value is negative")
    {
        test_format(FMT("{:.{}s}"), FMT("ab"), FLY_STR(char_type, "ab"), -3);
        test_format(FMT("{0:.{1}f}"), FMT("3.141590"), 3.14159, -2);
        test_format(FMT("{1:.{0}s}"), FMT("abcdef"), -3, FLY_STR(char_type, "abcdef"));
    }

    CATCH_SECTION("Presentation type may be set (character)")
    {
        test_format(FMT("{:c}"), FMT("a"), 'a');
        test_format(FMT("{:c}"), FMT("a"), L'a');
        test_format(FMT("{:c}"), FMT("a"), u8'a');
        test_format(FMT("{:c}"), FMT("a"), u'a');
        test_format(FMT("{:c}"), FMT("a"), U'a');
        test_format(FMT("{:c}"), FMT("\n"), FLY_CHR(char_type, '\n'));
        test_format(FMT("{:c}"), FMT("a"), 0x61);
        test_format(FMT("{:c}"), StringType(1, static_cast<char_type>(true)), true);
        test_format(FMT("{:c}"), StringType(1, static_cast<char_type>(false)), false);
    }

    CATCH_SECTION("Presentation type may be set (string)")
    {
        test_format(
            FMT("{:s}"),
            FLY_STR(char_type, "\u00f0\u0178\u008d\u2022"),
            FLY_STR(char_type, "\u00f0\u0178\u008d\u2022"));

        test_format(FMT("{:s}"), FMT("ab"), std::string("ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::wstring(L"ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::u8string(u8"ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::u16string(u"ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::u32string(U"ab"));

        test_format(FMT("{:s}"), FMT("ab"), std::string_view("ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::wstring_view(L"ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::u8string_view(u8"ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::u16string_view(u"ab"));
        test_format(FMT("{:s}"), FMT("ab"), std::u32string_view(U"ab"));

        test_format(FMT("{:s}"), FMT("ab"), "ab");
        test_format(FMT("{:s}"), FMT("ab"), L"ab");
        test_format(FMT("{:s}"), FMT("ab"), u8"ab");
        test_format(FMT("{:s}"), FMT("ab"), u"ab");
        test_format(FMT("{:s}"), FMT("ab"), U"ab");

        test_format(FMT("{:s}"), FMT("true"), true);
        test_format(FMT("{:s}"), FMT("false"), false);
    }

    CATCH_SECTION("Presentation type may be set (pointer)")
    {
        test_format(FMT("{:p}"), FMT("nullptr"), nullptr);

        int i = 0;
        auto result = BasicString::format(FMT("{:p}"), &i);
        CATCH_CHECK(is_all_hex(result));
    }

    CATCH_SECTION("Presentation type may be set (binary)")
    {
        test_format(FMT("{:b}"), FMT("1110111"), 0x77);
        test_format(FMT("{:b}"), FMT("1011111011101111"), 0xbeef);
        test_format(FMT("{:b}"), FMT("1"), true);
        test_format(FMT("{:b}"), FMT("0"), false);
        test_format(FMT("{:b}"), FMT("1000001"), char(0x41));
        test_format(FMT("{:b}"), FMT("1000001"), char8_t(0x41));
        test_format(FMT("{:b}"), FMT("1000001"), char16_t(0x41));
        test_format(FMT("{:b}"), FMT("1000001"), char32_t(0x41));
    }

    CATCH_SECTION("Presentation type may be set (octal)")
    {
        test_format(FMT("{:o}"), FMT("167"), 0x77);
        test_format(FMT("{:o}"), FMT("137357"), 0xbeef);
        test_format(FMT("{:o}"), FMT("1"), true);
        test_format(FMT("{:o}"), FMT("0"), false);
        test_format(FMT("{:o}"), FMT("101"), char(0x41));
        test_format(FMT("{:o}"), FMT("101"), char8_t(0x41));
        test_format(FMT("{:o}"), FMT("101"), char16_t(0x41));
        test_format(FMT("{:o}"), FMT("101"), char32_t(0x41));
    }

    CATCH_SECTION("Presentation type may be set (decimal)")
    {
        test_format(FMT("{:d}"), FMT("119"), 0x77);
        test_format(FMT("{:d}"), FMT("48879"), 0xbeef);
        test_format(FMT("{:d}"), FMT("1"), true);
        test_format(FMT("{:d}"), FMT("0"), false);
        test_format(FMT("{:d}"), FMT("65"), char(0x41));
        test_format(FMT("{:d}"), FMT("65"), char8_t(0x41));
        test_format(FMT("{:d}"), FMT("65"), char16_t(0x41));
        test_format(FMT("{:d}"), FMT("65"), char32_t(0x41));
    }

    CATCH_SECTION("Presentation type may be set (hex)")
    {
        test_format(FMT("{:x}"), FMT("77"), 0x77);
        test_format(FMT("{:x}"), FMT("beef"), 0xbeef);
        test_format(FMT("{:x}"), FMT("1"), true);
        test_format(FMT("{:x}"), FMT("0"), false);
        test_format(FMT("{:x}"), FMT("41"), char(0x41));
        test_format(FMT("{:x}"), FMT("41"), char8_t(0x41));
        test_format(FMT("{:x}"), FMT("41"), char16_t(0x41));
        test_format(FMT("{:x}"), FMT("41"), char32_t(0x41));

        test_format(FMT("{:X}"), FMT("BEEF"), 0xbeef);
    }

    CATCH_SECTION("Presentation type may be set (hexfloat)")
    {
        test_format(FMT("{:a}"), FMT("nan"), std::nan(""));
        test_format(FMT("{:a}"), FMT("inf"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:A}"), FMT("NAN"), std::nan(""));
        test_format(FMT("{:A}"), FMT("INF"), std::numeric_limits<float>::infinity());

        // MSVC will always 0-pad std::hexfloat formatted strings. Clang and GCC do not.
        // https://github.com/microsoft/STL/blob/0b81475cc8087a7b615911d65b52b6a1fad87d7d/stl/inc/xlocnum#L1156
        if constexpr (fly::is_windows())
        {
            test_format(FMT("{:a}"), FMT("0x1.6000000000000p+2"), 5.5);
            test_format(FMT("{:A}"), FMT("0X1.6000000000000P+2"), 5.5);
        }
        else
        {
            test_format(FMT("{:a}"), FMT("0x1.6p+2"), 5.5);
            test_format(FMT("{:A}"), FMT("0X1.6P+2"), 5.5);
        }
    }

    CATCH_SECTION("Presentation type may be set (scientific)")
    {
        test_format(FMT("{:e}"), FMT("nan"), std::nan(""));
        test_format(FMT("{:e}"), FMT("inf"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:e}"), FMT("1.230000e+02"), 123.0);

        test_format(FMT("{:E}"), FMT("NAN"), std::nan(""));
        test_format(FMT("{:E}"), FMT("INF"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:E}"), FMT("1.230000E+02"), 123.0);
    }

    CATCH_SECTION("Presentation type may be set (fixed)")
    {
        test_format(FMT("{:f}"), FMT("nan"), std::nan(""));
        test_format(FMT("{:f}"), FMT("inf"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:f}"), FMT("2.100000"), 2.1f);

        test_format(FMT("{:F}"), FMT("NAN"), std::nan(""));
        test_format(FMT("{:F}"), FMT("INF"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:F}"), FMT("2.100000"), 2.1f);
    }

    CATCH_SECTION("Presentation type may be set (general)")
    {
        test_format(FMT("{:g}"), FMT("nan"), std::nan(""));
        test_format(FMT("{:g}"), FMT("inf"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:g}"), FMT("2.1"), 2.1f);

        test_format(FMT("{:G}"), FMT("NAN"), std::nan(""));
        test_format(FMT("{:G}"), FMT("INF"), std::numeric_limits<float>::infinity());
        test_format(FMT("{:G}"), FMT("2.1"), 2.1f);
    }

    if constexpr (!std::is_same_v<StringType, typename BasicString::streamed_type>)
    {
        CATCH_SECTION("Invalid Unicode cannot be converted to UTF-8")
        {
            static constexpr const char_type s_invalid_utf8_leading_byte[] {
                static_cast<char_type>(0xff),
            };

            StringType result = BasicString::format(s_invalid_utf8_leading_byte);
            CATCH_CHECK(result.empty());
        }
    }

#if defined(FLY_COMPILER_DISABLE_CONSTEVAL)

    CATCH_SECTION("Formatter reports formatting errors")
    {
        auto result = BasicString::format(FMT("{:}"));
        CATCH_CHECK(result.starts_with(FMT("Ignored invalid formatter")));
    }

#endif
}