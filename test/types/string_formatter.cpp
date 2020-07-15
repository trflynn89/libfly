#include "fly/fly.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "fly/types/string/string_literal.hpp"

#include <catch2/catch.hpp>

#include <limits>

using namespace fly::literals::numeric_literals;

TEMPLATE_TEST_CASE(
    "BasicStringFormatter",
    "[string]",
    std::string,
    std::wstring,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using BasicString = fly::BasicString<StringType>;
    using char_type = typename BasicString::char_type;
    using streamed_type = typename BasicString::streamed_type;
    using streamed_char = typename streamed_type::value_type;

    const char_type *format;

    SECTION("Format")
    {
        streamed_type expected;
        StringType arg;

        CHECK(streamed_type() == BasicString::format(FLY_STR(char_type, "")));

        expected = FLY_STR(streamed_char, "%");
        format = FLY_STR(char_type, "%");
        CHECK(expected == BasicString::format(format));
        CHECK(expected == BasicString::format(format, 1));

        expected = FLY_STR(streamed_char, "%");
        format = FLY_STR(char_type, "%%");
        CHECK(expected == BasicString::format(format));

        expected = FLY_STR(streamed_char, "2.100000% 1");
        format = FLY_STR(char_type, "%f%% %d");
        CHECK(expected == BasicString::format(format, 2.1f, 1));

        expected = FLY_STR(streamed_char, "This is a test");
        format = FLY_STR(char_type, "This is a test");
        CHECK(expected == BasicString::format(format));

        expected = FLY_STR(streamed_char, "there are no formatters");
        format = FLY_STR(char_type, "there are no formatters");
        CHECK(expected == BasicString::format(format, 1, 2, 3, 4));

        expected = FLY_STR(streamed_char, "test some string s");
        format = FLY_STR(char_type, "test %s %c");
        arg = FLY_STR(char_type, "some string");
        CHECK(expected == BasicString::format(format, arg, 's'));

        expected = FLY_STR(streamed_char, "test 1 true 2.100000 false 1.230000e+02 0xff");
        format = FLY_STR(char_type, "test %d %d %f %d %e %x");
        CHECK(expected == BasicString::format(format, 1, true, 2.1f, false, 123.0, 255));
    }

    SECTION("Format as an integer (%d)")
    {
        format = FLY_STR(char_type, "%d");
        CHECK(FLY_STR(streamed_char, "%d") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "1") == BasicString::format(format, 1));
    }

    SECTION("Format as an integer (%i)")
    {
        format = FLY_STR(char_type, "%i");
        CHECK(FLY_STR(streamed_char, "%i") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "1") == BasicString::format(format, 1));
    }

    SECTION("Format as a character (%c)")
    {
        format = FLY_STR(char_type, "%c");
        CHECK(FLY_STR(streamed_char, "%c") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "a") == BasicString::format(format, FLY_CHR(char_type, 'a')));
        CHECK(
            FLY_STR(streamed_char, "\\x0a") ==
            BasicString::format(format, FLY_CHR(char_type, '\n')));
        CHECK(
            FLY_STR(streamed_char, "[EOF]") ==
            BasicString::format(
                format,
                static_cast<char_type>(std::char_traits<char_type>::eof())));
    }

    SECTION("Format as a string (%s)")
    {
        format = FLY_STR(char_type, "%s");
        CHECK(FLY_STR(streamed_char, "%s") == BasicString::format(format));
        CHECK(
            FLY_STR(streamed_char, "\\u00f0\\u0178\\u008d\\u2022") ==
            BasicString::format(format, FLY_STR(char_type, "\u00f0\u0178\u008d\u2022")));
    }

    SECTION("Format as a hexadecimal integer (%x)")
    {
        format = FLY_STR(char_type, "%x");
        CHECK(FLY_STR(streamed_char, "%x") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "0xff") == BasicString::format(format, 255));

        format = FLY_STR(char_type, "%X");
        CHECK(FLY_STR(streamed_char, "%X") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "0XFF") == BasicString::format(format, 255));
    }

    SECTION("Format as an octal integer (%o)")
    {
        format = FLY_STR(char_type, "%o");
        CHECK(FLY_STR(streamed_char, "%o") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "0377") == BasicString::format(format, 255));
    }

    SECTION("Format as a hexadecimal floating point (%a)")
    {
        format = FLY_STR(char_type, "%a");
        CHECK(FLY_STR(streamed_char, "%a") == BasicString::format(format));
#if defined(FLY_WINDOWS)
        // Windows 0-pads std::hexfloat to match the stream precision, Linux does not.
        CHECK(FLY_STR(streamed_char, "0x1.600000p+2") == BasicString::format(format, 5.5));
#else
        CHECK(FLY_STR(streamed_char, "0x1.6p+2") == BasicString::format(format, 5.5));
#endif

        format = FLY_STR(char_type, "%A");
        CHECK(FLY_STR(streamed_char, "%A") == BasicString::format(format));
#if defined(FLY_WINDOWS)
        // Windows 0-pads std::hexfloat to match the stream precision, Linux does not.
        CHECK(FLY_STR(streamed_char, "0X1.600000P+2") == BasicString::format(format, 5.5));
#else
        CHECK(FLY_STR(streamed_char, "0X1.6P+2") == BasicString::format(format, 5.5));
#endif
    }

    SECTION("Format as a floating point (%f)")
    {
        format = FLY_STR(char_type, "%f");
        CHECK(FLY_STR(streamed_char, "%f") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "nan") == BasicString::format(format, std::nan("")));
        CHECK(
            FLY_STR(streamed_char, "inf") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CHECK(FLY_STR(streamed_char, "2.100000") == BasicString::format(format, 2.1f));

        // Note: std::uppercase has no effect on std::fixed :(
        format = FLY_STR(char_type, "%F");
        CHECK(FLY_STR(streamed_char, "%F") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "nan") == BasicString::format(format, std::nan("")));
        CHECK(
            FLY_STR(streamed_char, "inf") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CHECK(FLY_STR(streamed_char, "2.100000") == BasicString::format(format, 2.1f));
    }

    SECTION("Format as scientific notation (%e)")
    {
        format = FLY_STR(char_type, "%e");
        CHECK(FLY_STR(streamed_char, "%e") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "1.230000e+02") == BasicString::format(format, 123.0));

        format = FLY_STR(char_type, "%E");
        CHECK(FLY_STR(streamed_char, "%E") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "1.230000E+02") == BasicString::format(format, 123.0));
    }

    SECTION("Format as a floating point or scientific notation (%g)")
    {
        format = FLY_STR(char_type, "%g");
        CHECK(FLY_STR(streamed_char, "%g") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "nan") == BasicString::format(format, std::nan("")));
        CHECK(
            FLY_STR(streamed_char, "inf") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CHECK(FLY_STR(streamed_char, "2.1") == BasicString::format(format, 2.1f));

        format = FLY_STR(char_type, "%G");
        CHECK(FLY_STR(streamed_char, "%G") == BasicString::format(format));
        CHECK(FLY_STR(streamed_char, "NAN") == BasicString::format(format, std::nan("")));
        CHECK(
            FLY_STR(streamed_char, "INF") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CHECK(FLY_STR(streamed_char, "2.1") == BasicString::format(format, 2.1f));
    }

    SECTION("Format as a hexadecimal string")
    {
        for (char_type ch = 0; ch <= 0xf; ++ch)
        {
            if ((ch >= 0) && (ch <= 9))
            {
                CHECK(StringType(1, '0' + ch) == BasicString::create_hex_string(ch, 1));
            }
            else
            {
                CHECK(StringType(1, 'a' + ch - 10) == BasicString::create_hex_string(ch, 1));
            }
        }

        CHECK(FLY_STR(char_type, "") == BasicString::create_hex_string(0x1234, 0));
        CHECK(FLY_STR(char_type, "4") == BasicString::create_hex_string(0x1234, 1));
        CHECK(FLY_STR(char_type, "34") == BasicString::create_hex_string(0x1234, 2));
        CHECK(FLY_STR(char_type, "234") == BasicString::create_hex_string(0x1234, 3));
        CHECK(FLY_STR(char_type, "1234") == BasicString::create_hex_string(0x1234, 4));
        CHECK(FLY_STR(char_type, "01234") == BasicString::create_hex_string(0x1234, 5));
        CHECK(FLY_STR(char_type, "001234") == BasicString::create_hex_string(0x1234, 6));
        CHECK(FLY_STR(char_type, "0001234") == BasicString::create_hex_string(0x1234, 7));
        CHECK(FLY_STR(char_type, "00001234") == BasicString::create_hex_string(0x1234, 8));

        CHECK(
            FLY_STR(char_type, "0123456789abcdef") ==
            BasicString::create_hex_string(0x0123456789abcdef_u64, 16));
    }
}
