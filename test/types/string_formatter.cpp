#include "fly/fly.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "fly/types/string/string_literal.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

#include <limits>

//==================================================================================================
TYPED_TEST(BasicStringTest, Format)
{
    DECLARE_ALIASES

    streamed_type expected;
    const char_type *format;
    string_type arg;

    EXPECT_EQ(streamed_type(), StringClass::format(FLY_STR(char_type, "")));

    expected = FLY_STR(streamed_char, "%");
    format = FLY_STR(char_type, "%");
    EXPECT_EQ(expected, StringClass::format(format));
    EXPECT_EQ(expected, StringClass::format(format, 1));

    expected = FLY_STR(streamed_char, "%");
    format = FLY_STR(char_type, "%%");
    EXPECT_EQ(expected, StringClass::format(format));

    expected = FLY_STR(streamed_char, "2.100000% 1");
    format = FLY_STR(char_type, "%f%% %d");
    EXPECT_EQ(expected, StringClass::format(format, 2.1f, 1));

    expected = FLY_STR(streamed_char, "This is a test");
    format = FLY_STR(char_type, "This is a test");
    EXPECT_EQ(expected, StringClass::format(format));

    expected = FLY_STR(streamed_char, "there are no formatters");
    format = FLY_STR(char_type, "there are no formatters");
    EXPECT_EQ(expected, StringClass::format(format, 1, 2, 3, 4));

    expected = FLY_STR(streamed_char, "test some string s");
    format = FLY_STR(char_type, "test %s %c");
    arg = FLY_STR(char_type, "some string");
    EXPECT_EQ(expected, StringClass::format(format, arg, 's'));

    expected = FLY_STR(streamed_char, "test 1 true 2.100000 false 1.230000e+02 0xff");
    format = FLY_STR(char_type, "test %d %d %f %d %e %x");
    EXPECT_EQ(expected, StringClass::format(format, 1, true, 2.1f, false, 123.0, 255));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_d)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%d");
    EXPECT_EQ(FLY_STR(streamed_char, "%d"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "1"), StringClass::format(format, 1));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_i)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%i");
    EXPECT_EQ(FLY_STR(streamed_char, "%i"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "1"), StringClass::format(format, 1));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_c)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%c");
    EXPECT_EQ(FLY_STR(streamed_char, "%c"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "a"), StringClass::format(format, FLY_CHR(char_type, 'a')));
    EXPECT_EQ(
        FLY_STR(streamed_char, "[0xa]"),
        StringClass::format(format, FLY_CHR(char_type, '\n')));
    EXPECT_EQ(
        FLY_STR(streamed_char, "EOF"),
        StringClass::format(format, static_cast<char_type>(std::char_traits<char_type>::eof())));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_x)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%x");
    EXPECT_EQ(FLY_STR(streamed_char, "%x"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "0xff"), StringClass::format(format, 255));

    format = FLY_STR(char_type, "%X");
    EXPECT_EQ(FLY_STR(streamed_char, "%X"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "0XFF"), StringClass::format(format, 255));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_o)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%o");
    EXPECT_EQ(FLY_STR(streamed_char, "%o"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "0377"), StringClass::format(format, 255));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_a)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%a");
    EXPECT_EQ(FLY_STR(streamed_char, "%a"), StringClass::format(format));
#if defined(FLY_WINDOWS)
    // Windows 0-pads std::hexfloat to match the stream precision, Linux does not.
    EXPECT_EQ(FLY_STR(streamed_char, "0x1.600000p+2"), StringClass::format(format, 5.5));
#else
    EXPECT_EQ(FLY_STR(streamed_char, "0x1.6p+2"), StringClass::format(format, 5.5));
#endif

    format = FLY_STR(char_type, "%A");
    EXPECT_EQ(FLY_STR(streamed_char, "%A"), StringClass::format(format));
#if defined(FLY_WINDOWS)
    // Windows 0-pads std::hexfloat to match the stream precision, Linux does not.
    EXPECT_EQ(FLY_STR(streamed_char, "0X1.600000P+2"), StringClass::format(format, 5.5));
#else
    EXPECT_EQ(FLY_STR(streamed_char, "0X1.6P+2"), StringClass::format(format, 5.5));
#endif
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_f)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%f");
    EXPECT_EQ(FLY_STR(streamed_char, "%f"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "nan"), StringClass::format(format, std::nan("")));
    EXPECT_EQ(
        FLY_STR(streamed_char, "inf"),
        StringClass::format(format, std::numeric_limits<float>::infinity()));
    EXPECT_EQ(FLY_STR(streamed_char, "2.100000"), StringClass::format(format, 2.1f));

    // Note: std::uppercase has no effect on std::fixed :(
    format = FLY_STR(char_type, "%F");
    EXPECT_EQ(FLY_STR(streamed_char, "%F"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "nan"), StringClass::format(format, std::nan("")));
    EXPECT_EQ(
        FLY_STR(streamed_char, "inf"),
        StringClass::format(format, std::numeric_limits<float>::infinity()));
    EXPECT_EQ(FLY_STR(streamed_char, "2.100000"), StringClass::format(format, 2.1f));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_g)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%g");
    EXPECT_EQ(FLY_STR(streamed_char, "%g"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "nan"), StringClass::format(format, std::nan("")));
    EXPECT_EQ(
        FLY_STR(streamed_char, "inf"),
        StringClass::format(format, std::numeric_limits<float>::infinity()));
    EXPECT_EQ(FLY_STR(streamed_char, "2.1"), StringClass::format(format, 2.1f));

    format = FLY_STR(char_type, "%G");
    EXPECT_EQ(FLY_STR(streamed_char, "%G"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "NAN"), StringClass::format(format, std::nan("")));
    EXPECT_EQ(
        FLY_STR(streamed_char, "INF"),
        StringClass::format(format, std::numeric_limits<float>::infinity()));
    EXPECT_EQ(FLY_STR(streamed_char, "2.1"), StringClass::format(format, 2.1f));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatTest_e)
{
    DECLARE_ALIASES

    const char_type *format;

    format = FLY_STR(char_type, "%e");
    EXPECT_EQ(FLY_STR(streamed_char, "%e"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "1.230000e+02"), StringClass::format(format, 123.0));

    format = FLY_STR(char_type, "%E");
    EXPECT_EQ(FLY_STR(streamed_char, "%E"), StringClass::format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "1.230000E+02"), StringClass::format(format, 123.0));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, FormatHex)
{
    DECLARE_ALIASES

    for (char_type ch = 0; ch <= 0xf; ++ch)
    {
        if ((ch >= 0) && (ch <= 9))
        {
            EXPECT_EQ(string_type(1, '0' + ch), StringClass::create_hex_string(ch, 1));
        }
        else
        {
            EXPECT_EQ(string_type(1, 'a' + ch - 10), StringClass::create_hex_string(ch, 1));
        }
    }

    EXPECT_EQ(FLY_STR(char_type, ""), StringClass::create_hex_string(0x1234, 0));
    EXPECT_EQ(FLY_STR(char_type, "4"), StringClass::create_hex_string(0x1234, 1));
    EXPECT_EQ(FLY_STR(char_type, "34"), StringClass::create_hex_string(0x1234, 2));
    EXPECT_EQ(FLY_STR(char_type, "234"), StringClass::create_hex_string(0x1234, 3));
    EXPECT_EQ(FLY_STR(char_type, "1234"), StringClass::create_hex_string(0x1234, 4));
    EXPECT_EQ(FLY_STR(char_type, "01234"), StringClass::create_hex_string(0x1234, 5));
    EXPECT_EQ(FLY_STR(char_type, "001234"), StringClass::create_hex_string(0x1234, 6));
    EXPECT_EQ(FLY_STR(char_type, "0001234"), StringClass::create_hex_string(0x1234, 7));
    EXPECT_EQ(FLY_STR(char_type, "00001234"), StringClass::create_hex_string(0x1234, 8));

    EXPECT_EQ(
        FLY_STR(char_type, "0123456789abcdef"),
        StringClass::create_hex_string(0x0123456789abcdef_u64, 16));
}
