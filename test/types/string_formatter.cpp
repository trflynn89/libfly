#include "fly/fly.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "fly/types/string/string_literal.hpp"

#include "catch2/catch.hpp"

#include <limits>

using namespace fly::literals::numeric_literals;

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
    using streamed_type = typename BasicString::streamed_type;
    using streamed_char = typename streamed_type::value_type;

    const char_type *format;

    CATCH_SECTION("Format")
    {
        streamed_type expected;
        StringType arg;

        CATCH_CHECK(streamed_type() == BasicString::format(FLY_STR(char_type, "")));

        expected = FLY_STR(streamed_char, "%");
        format = FLY_STR(char_type, "%");
        CATCH_CHECK(expected == BasicString::format(format));
        CATCH_CHECK(expected == BasicString::format(format, 1));

        expected = FLY_STR(streamed_char, "%");
        format = FLY_STR(char_type, "%%");
        CATCH_CHECK(expected == BasicString::format(format));

        expected = FLY_STR(streamed_char, "2.100000% 1");
        format = FLY_STR(char_type, "%f%% %d");
        CATCH_CHECK(expected == BasicString::format(format, 2.1f, 1));

        expected = FLY_STR(streamed_char, "This is a test");
        format = FLY_STR(char_type, "This is a test");
        CATCH_CHECK(expected == BasicString::format(format));

        expected = FLY_STR(streamed_char, "there are no formatters");
        format = FLY_STR(char_type, "there are no formatters");
        CATCH_CHECK(expected == BasicString::format(format, 1, 2, 3, 4));

        expected = FLY_STR(streamed_char, "test some string s");
        format = FLY_STR(char_type, "test %s %c");
        arg = FLY_STR(char_type, "some string");
        CATCH_CHECK(expected == BasicString::format(format, arg, 's'));

        expected = FLY_STR(streamed_char, "test 1 true 2.100000 false 1.230000e+02 0xff");
        format = FLY_STR(char_type, "test %d %d %f %d %e %x");
        CATCH_CHECK(expected == BasicString::format(format, 1, true, 2.1f, false, 123.0, 255));
    }

    CATCH_SECTION("Format as an integer (%d)")
    {
        format = FLY_STR(char_type, "%d");
        CATCH_CHECK(FLY_STR(streamed_char, "%d") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "1") == BasicString::format(format, 1));
    }

    CATCH_SECTION("Format as an integer (%i)")
    {
        format = FLY_STR(char_type, "%i");
        CATCH_CHECK(FLY_STR(streamed_char, "%i") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "1") == BasicString::format(format, 1));
    }

    CATCH_SECTION("Format as a character (%c)")
    {
        format = FLY_STR(char_type, "%c");
        CATCH_CHECK(FLY_STR(streamed_char, "%c") == BasicString::format(format));
        CATCH_CHECK(
            FLY_STR(streamed_char, "a") == BasicString::format(format, FLY_CHR(char_type, 'a')));
        CATCH_CHECK(
            FLY_STR(streamed_char, "\\x0a") ==
            BasicString::format(format, FLY_CHR(char_type, '\n')));
        CATCH_CHECK(
            FLY_STR(streamed_char, "[EOF]") ==
            BasicString::format(
                format,
                static_cast<char_type>(std::char_traits<char_type>::eof())));

        CATCH_CHECK(FLY_STR(streamed_char, "a") == BasicString::format(format, 'a'));
        CATCH_CHECK(FLY_STR(streamed_char, "a") == BasicString::format(format, L'a'));
        CATCH_CHECK(FLY_STR(streamed_char, "a") == BasicString::format(format, u8'a'));
        CATCH_CHECK(FLY_STR(streamed_char, "a") == BasicString::format(format, u'a'));
        CATCH_CHECK(FLY_STR(streamed_char, "a") == BasicString::format(format, U'a'));
    }

    CATCH_SECTION("Format as a string (%s)")
    {
        format = FLY_STR(char_type, "%s");
        CATCH_CHECK(FLY_STR(streamed_char, "%s") == BasicString::format(format));
        CATCH_CHECK(
            FLY_STR(streamed_char, "\\u00f0\\u0178\\u008d\\u2022") ==
            BasicString::format(format, FLY_STR(char_type, "\u00f0\u0178\u008d\u2022")));

        CATCH_CHECK(
            FLY_STR(streamed_char, "std::string") ==
            BasicString::format(format, std::string("std::string")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::wstring") ==
            BasicString::format(format, std::wstring(L"std::wstring")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u8string") ==
            BasicString::format(format, std::u8string(u8"std::u8string")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u16string") ==
            BasicString::format(format, std::u16string(u"std::u16string")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u32string") ==
            BasicString::format(format, std::u32string(U"std::u32string")));

        CATCH_CHECK(
            FLY_STR(streamed_char, "std::string_view") ==
            BasicString::format(format, std::string_view("std::string_view")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::wstring_view") ==
            BasicString::format(format, std::wstring_view(L"std::wstring_view")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u8string_view") ==
            BasicString::format(format, std::u8string_view(u8"std::u8string_view")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u16string_view") ==
            BasicString::format(format, std::u16string_view(u"std::u16string_view")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u32string_view") ==
            BasicString::format(format, std::u32string_view(U"std::u32string_view")));

        CATCH_CHECK(
            FLY_STR(streamed_char, "std::string") == BasicString::format(format, "std::string"));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::wstring") == BasicString::format(format, L"std::wstring"));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u8string") ==
            BasicString::format(format, u8"std::u8string"));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u16string") ==
            BasicString::format(format, u"std::u16string"));
        CATCH_CHECK(
            FLY_STR(streamed_char, "std::u32string") ==
            BasicString::format(format, U"std::u32string"));
    }

    CATCH_SECTION("Format as a hexadecimal integer (%x)")
    {
        format = FLY_STR(char_type, "%x");
        CATCH_CHECK(FLY_STR(streamed_char, "%x") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "0xff") == BasicString::format(format, 255));

        format = FLY_STR(char_type, "%X");
        CATCH_CHECK(FLY_STR(streamed_char, "%X") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "0XFF") == BasicString::format(format, 255));
    }

    CATCH_SECTION("Format as an octal integer (%o)")
    {
        format = FLY_STR(char_type, "%o");
        CATCH_CHECK(FLY_STR(streamed_char, "%o") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "0377") == BasicString::format(format, 255));
    }

    CATCH_SECTION("Format as a hexadecimal floating point (%a)")
    {
        format = FLY_STR(char_type, "%a");
        CATCH_CHECK(FLY_STR(streamed_char, "%a") == BasicString::format(format));
#if defined(FLY_WINDOWS)
        // MSVC will always 0-pad std::hexfloat formatted strings. Clang and GCC do not.
        // https://github.com/microsoft/STL/blob/0b81475cc8087a7b615911d65b52b6a1fad87d7d/stl/inc/xlocnum#L1156
        CATCH_CHECK(
            FLY_STR(streamed_char, "0x1.6000000000000p+2") == BasicString::format(format, 5.5));
#else
        CATCH_CHECK(FLY_STR(streamed_char, "0x1.6p+2") == BasicString::format(format, 5.5));
#endif

        format = FLY_STR(char_type, "%A");
        CATCH_CHECK(FLY_STR(streamed_char, "%A") == BasicString::format(format));
#if defined(FLY_WINDOWS)
        // MSVC will always 0-pad std::hexfloat formatted strings. Clang and GCC do not.
        // https://github.com/microsoft/STL/blob/0b81475cc8087a7b615911d65b52b6a1fad87d7d/stl/inc/xlocnum#L1156
        CATCH_CHECK(
            FLY_STR(streamed_char, "0X1.6000000000000P+2") == BasicString::format(format, 5.5));
#else
        CATCH_CHECK(FLY_STR(streamed_char, "0X1.6P+2") == BasicString::format(format, 5.5));
#endif
    }

    CATCH_SECTION("Format as a floating point (%f)")
    {
        format = FLY_STR(char_type, "%f");
        CATCH_CHECK(FLY_STR(streamed_char, "%f") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "nan") == BasicString::format(format, std::nan("")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "inf") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CATCH_CHECK(FLY_STR(streamed_char, "2.100000") == BasicString::format(format, 2.1f));

        format = FLY_STR(char_type, "%F");
        CATCH_CHECK(FLY_STR(streamed_char, "%F") == BasicString::format(format));
#if defined(FLY_MACOS)
        // Only macOS seems to support std::uppercase with std::fixed :(
        CATCH_CHECK(FLY_STR(streamed_char, "NAN") == BasicString::format(format, std::nan("")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "INF") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
#else
        CATCH_CHECK(FLY_STR(streamed_char, "nan") == BasicString::format(format, std::nan("")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "inf") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
#endif
        CATCH_CHECK(FLY_STR(streamed_char, "2.100000") == BasicString::format(format, 2.1f));
    }

    CATCH_SECTION("Format as scientific notation (%e)")
    {
        format = FLY_STR(char_type, "%e");
        CATCH_CHECK(FLY_STR(streamed_char, "%e") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "1.230000e+02") == BasicString::format(format, 123.0));

        format = FLY_STR(char_type, "%E");
        CATCH_CHECK(FLY_STR(streamed_char, "%E") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "1.230000E+02") == BasicString::format(format, 123.0));
    }

    CATCH_SECTION("Format as a floating point or scientific notation (%g)")
    {
        format = FLY_STR(char_type, "%g");
        CATCH_CHECK(FLY_STR(streamed_char, "%g") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "nan") == BasicString::format(format, std::nan("")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "inf") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CATCH_CHECK(FLY_STR(streamed_char, "2.1") == BasicString::format(format, 2.1f));

        format = FLY_STR(char_type, "%G");
        CATCH_CHECK(FLY_STR(streamed_char, "%G") == BasicString::format(format));
        CATCH_CHECK(FLY_STR(streamed_char, "NAN") == BasicString::format(format, std::nan("")));
        CATCH_CHECK(
            FLY_STR(streamed_char, "INF") ==
            BasicString::format(format, std::numeric_limits<float>::infinity()));
        CATCH_CHECK(FLY_STR(streamed_char, "2.1") == BasicString::format(format, 2.1f));
    }

    CATCH_SECTION("Format as a hexadecimal string")
    {
        for (char_type ch = 0; ch <= 0xf; ++ch)
        {
            if ((ch >= 0) && (ch <= 9))
            {
                CATCH_CHECK(StringType(1, '0' + ch) == BasicString::create_hex_string(ch, 1));
            }
            else
            {
                CATCH_CHECK(StringType(1, 'a' + ch - 10) == BasicString::create_hex_string(ch, 1));
            }
        }

        CATCH_CHECK(FLY_STR(char_type, "") == BasicString::create_hex_string(0x1234, 0));
        CATCH_CHECK(FLY_STR(char_type, "4") == BasicString::create_hex_string(0x1234, 1));
        CATCH_CHECK(FLY_STR(char_type, "34") == BasicString::create_hex_string(0x1234, 2));
        CATCH_CHECK(FLY_STR(char_type, "234") == BasicString::create_hex_string(0x1234, 3));
        CATCH_CHECK(FLY_STR(char_type, "1234") == BasicString::create_hex_string(0x1234, 4));
        CATCH_CHECK(FLY_STR(char_type, "01234") == BasicString::create_hex_string(0x1234, 5));
        CATCH_CHECK(FLY_STR(char_type, "001234") == BasicString::create_hex_string(0x1234, 6));
        CATCH_CHECK(FLY_STR(char_type, "0001234") == BasicString::create_hex_string(0x1234, 7));
        CATCH_CHECK(FLY_STR(char_type, "00001234") == BasicString::create_hex_string(0x1234, 8));

        CATCH_CHECK(
            FLY_STR(char_type, "0123456789abcdef") ==
            BasicString::create_hex_string(0x0123456789abcdef_u64, 16));
    }
}
