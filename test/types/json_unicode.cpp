#include "fly/types/json/json.hpp"

#include <gtest/gtest.h>

#include <sstream>

namespace {

void validate_fail(const std::string &test) noexcept(false)
{
    SCOPED_TRACE(test);

    fly::Json actual;

    EXPECT_THROW({ actual = test; }, fly::JsonException);
}

void validate_pass(
    const std::string &test,
    const std::string &expected) noexcept(false)
{
    SCOPED_TRACE(test);

    fly::Json actual;

    EXPECT_NO_THROW({ actual = test; });

    std::stringstream ss;
    ss << actual;

    EXPECT_EQ(actual, expected);

    fly::Json repeat = actual;
    EXPECT_EQ(actual, repeat);
}

void validate_pass(const std::string &test) noexcept(false)
{
    validate_pass(test, test);
}

} // namespace

//==============================================================================
TEST(JsonTest, UnicodeConversion)
{
    validate_fail("\\u");
    validate_fail("\\u0");
    validate_fail("\\u00");
    validate_fail("\\u000");
    validate_fail("\\u000z");

    validate_pass("\\u0040", u8"\u0040");
    validate_pass("\\u007A", u8"\u007A");
    validate_pass("\\u007a", u8"\u007a");
    validate_pass("\\u00c4", u8"\u00c4");
    validate_pass("\\u00e4", u8"\u00e4");
    validate_pass("\\u0298", u8"\u0298");
    validate_pass("\\u0800", u8"\u0800");
    validate_pass("\\uffff", u8"\uffff");

    validate_fail("\\uDC00");
    validate_fail("\\uDFFF");
    validate_fail("\\uD800");
    validate_fail("\\uDBFF");
    validate_fail("\\uD800\\u");
    validate_fail("\\uD800\\z");
    validate_fail("\\uD800\\u0");
    validate_fail("\\uD800\\u00");
    validate_fail("\\uD800\\u000");
    validate_fail("\\uD800\\u0000");
    validate_fail("\\uD800\\u000z");
    validate_fail("\\uD800\\uDBFF");
    validate_fail("\\uD800\\uE000");
    validate_fail("\\uD800\\uFFFF");

    validate_pass("\\uD800\\uDC00", u8"\U00010000");
    validate_pass("\\uD803\\uDE6D", u8"\U00010E6D");
    validate_pass("\\uD834\\uDD1E", u8"\U0001D11E");
    validate_pass("\\uDBFF\\uDFFF", u8"\U0010FFFF");
}

//==============================================================================
TEST(JsonTest, MarkusKuhnStress)
{
    // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt

    // 1  Some correct UTF-8 text
    validate_pass("κόσμε");

    // 2  Boundary condition test cases

    // 2.1  First possible sequence of a certain length

    // 2.1.1  1 byte  (U-00000001)
    validate_fail("\x01");

    // 2.1.2  2 bytes (U-00000080)
    validate_pass("\xc2\x80");

    // 2.1.3  3 bytes (U-00000800)
    validate_pass("\xe0\xa0\x80");

    // 2.1.4  4 bytes (U-00010000)
    validate_pass("\xf0\x90\x80\x80");

    // 2.1.5  5 bytes (U-00200000)
    validate_fail("\xf8\x88\x80\x80\x80");

    // 2.1.6  6 bytes (U-04000000)
    validate_fail("\xfc\x84\x80\x80\x80\x80");

    // 2.2  Last possible sequence of a certain length

    // 2.2.1  1 byte  (U-0000007F)
    validate_pass("\x7f");

    // 2.2.2  2 bytes (U-000007FF)
    validate_pass("\xdf\xbf");

    // 2.2.3  3 bytes (U-0000FFFF)
    validate_pass("\xef\xbf\xbf");

    // 2.1.4  4 bytes (U-00200000)
    validate_fail("\xf7\xbf\xbf\xbf");

    // 2.1.5  5 bytes (U-03FFFFFF)
    validate_fail("\xfb\xbf\xbf\xbf\xbf");

    // 2.1.6  6 bytes (U-7FFFFFFF)
    validate_fail("\xfd\xbf\xbf\xbf\xbf\xbf");

    // 2.3  Other boundary conditions

    // 2.3.1  U-0000D7FF
    validate_pass("\xed\x9f\xbf");

    // 2.3.2  U-0000E000
    validate_pass("\xee\x80\x80");

    // 2.3.3  U-0000FFFD
    validate_pass("\xef\xbf\xbd");

    // 2.3.4  U-0010FFFF
    validate_pass("\xf4\x8f\xbf\xbf");

    // 2.3.5  U-00110000
    validate_fail("\xf4\x90\x80\x80");

    // 3  Malformed sequences

    // 3.1  Unexpected continuation bytes

    // 3.1.1  First continuation byte 0x80
    validate_fail("\x80");

    // 3.1.2 Last  continuation byte 0xbf
    validate_fail("\xbf");

    // 3.1.3  2 continuation bytes
    validate_fail("\x80\xbf");

    // 3.1.4  3 continuation bytes
    validate_fail("\x80\xbf\x80");

    // 3.1.5  4 continuation bytes
    validate_fail("\x80\xbf\x80\xbf");

    // 3.1.6  5 continuation bytes
    validate_fail("\x80\xbf\x80\xbf\x80");

    // 3.1.7  6 continuation bytes
    validate_fail("\x80\xbf\x80\xbf\x80\xbf");

    // 3.1.8  7 continuation bytes
    validate_fail("\x80\xbf\x80\xbf\x80\xbf\x80");

    // 3.1.9  Sequence of all 64 possible continuation bytes (0x80-0xbf)
    validate_fail(
        "\x80\x81\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f\x90"
        "\x91\x92\x93\x94\x95\x96\x97\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f\xa0\xa1"
        "\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xab\xac\xad\xae\xaf\xb0\xb1\xb2"
        "\xb3\xb4\xb5\xb6\xb7\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf");

    // 3.2  Lonely start characters

    // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf), each followed
    // by a space character
    validate_fail(
        "\xc0 \xc1 \xc2 \xc3 \xc4 \xc5 \xc6 \xc7 \xc8 \xc9 \xca \xcb \xcc \xcd "
        "\xce \xcf \xd0 \xd1 \xd2 \xd3 \xd4 \xd5 \xd6 \xd7 \xd8 \xd9 \xda \xdb "
        "\xdc \xdd \xde \xdf");
    validate_fail("\xc0 ");
    validate_fail("\xc1 ");
    validate_fail("\xc2 ");
    validate_fail("\xc3 ");
    validate_fail("\xc4 ");
    validate_fail("\xc5 ");
    validate_fail("\xc6 ");
    validate_fail("\xc7 ");
    validate_fail("\xc8 ");
    validate_fail("\xc9 ");
    validate_fail("\xca ");
    validate_fail("\xcb ");
    validate_fail("\xcc ");
    validate_fail("\xcd ");
    validate_fail("\xce ");
    validate_fail("\xcf ");
    validate_fail("\xd0 ");
    validate_fail("\xd1 ");
    validate_fail("\xd2 ");
    validate_fail("\xd3 ");
    validate_fail("\xd4 ");
    validate_fail("\xd5 ");
    validate_fail("\xd6 ");
    validate_fail("\xd7 ");
    validate_fail("\xd8 ");
    validate_fail("\xd9 ");
    validate_fail("\xda ");
    validate_fail("\xdb ");
    validate_fail("\xdc ");
    validate_fail("\xdd ");
    validate_fail("\xde ");
    validate_fail("\xdf ");

    // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef) each followed
    // by a space character
    validate_fail(
        "\xe0 \xe1 \xe2 \xe3 \xe4 \xe5 \xe6 \xe7 \xe8 \xe9 \xea \xeb \xec \xed "
        "\xee \xef");
    validate_fail("\xe0 ");
    validate_fail("\xe1 ");
    validate_fail("\xe2 ");
    validate_fail("\xe3 ");
    validate_fail("\xe4 ");
    validate_fail("\xe5 ");
    validate_fail("\xe6 ");
    validate_fail("\xe7 ");
    validate_fail("\xe8 ");
    validate_fail("\xe9 ");
    validate_fail("\xea ");
    validate_fail("\xeb ");
    validate_fail("\xec ");
    validate_fail("\xed ");
    validate_fail("\xee ");
    validate_fail("\xef ");

    // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7), each followed
    // by a space character
    validate_fail("\xf0 \xf1 \xf2 \xf3 \xf4 \xf5 \xf6 \xf7");
    validate_fail("\xf0 ");
    validate_fail("\xf1 ");
    validate_fail("\xf2 ");
    validate_fail("\xf3 ");
    validate_fail("\xf4 ");
    validate_fail("\xf5 ");
    validate_fail("\xf6 ");
    validate_fail("\xf7 ");

    // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb), each followed
    // by a space character
    validate_fail("\xf8 \xf9 \xfa \xfb");
    validate_fail("\xf8 ");
    validate_fail("\xf9 ");
    validate_fail("\xfa ");
    validate_fail("\xfb ");

    // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd), each followed
    // by a space character
    validate_fail("\xfc \xfd");
    validate_fail("\xfc ");
    validate_fail("\xfc ");

    // 3.3  Sequences with last continuation byte missing

    // 3.3.1  2-byte sequence with last byte missing (U+0000)
    validate_fail("\xc0");

    // 3.3.2  3-byte sequence with last byte missing (U+0000)
    validate_fail("\xe0\x80");

    // 3.3.3  4-byte sequence with last byte missing (U+0000)
    validate_fail("\xf0\x80\x80");

    // 3.3.4  5-byte sequence with last byte missing (U+0000)
    validate_fail("\xf8\x80\x80\x80");

    // 3.3.5  6-byte sequence with last byte missing (U+0000)
    validate_fail("\xfc\x80\x80\x80\x80");

    // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
    validate_fail("\xdf");

    // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
    validate_fail("\xef\xbf");

    // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
    validate_fail("\xf7\xbf\xbf");

    // 3.3.9  5-byte sequence with last byte missing (U-03FFFFFF)
    validate_fail("\xfb\xbf\xbf\xbf");

    // 3.3.10 6-byte sequence with last byte missing (U-7FFFFFFF)
    validate_fail("\xfd\xbf\xbf\xbf\xbf");

    // 3.4  Concatenation of incomplete sequences

    // All the 10 sequences of 3.3 concatenated
    validate_fail(
        "\xc0\xe0\x80\xf0\x80\x80\xf8\x80\x80\x80\xfc\x80\x80\x80\x80\xdf\xef"
        "\xbf\xf7\xbf\xbf\xfb\xbf\xbf\xbf\xfd\xbf\xbf\xbf\xbf");

    // 3.5  Impossible bytes

    // 3.5.1  fe
    validate_fail("\xfe");

    // 3.5.2  ff
    validate_fail("\xff");

    // 3.5.3  fe fe ff ff
    validate_fail("\xfe\xfe\xff\xff");

    // 4  Overlong sequences

    // 4.1  Examples of an overlong ASCII character

    // 4.1.1 U+002F = c0 af
    validate_fail("\xc0\xaf");

    // 4.1.2 U+002F = e0 80 af
    validate_fail("\xe0\x80\xaf");

    // 4.1.3 U+002F = f0 80 80 af
    validate_fail("\xf0\x80\x80\xaf");

    // 4.1.4 U+002F = f8 80 80 80 af
    validate_fail("\xf8\x80\x80\x80\xaf");

    // 4.1.5 U+002F = fc 80 80 80 80 af
    validate_fail("\xfc\x80\x80\x80\x80\xaf");

    // 4.2  Maximum overlong sequences

    // 4.2.1  U-0000007F = c1 bf
    validate_fail("\xc1\xbf");

    // 4.2.2  U-000007FF = e0 9f bf
    validate_fail("\xe0\x9f\xbf");

    // 4.2.3  U-0000FFFF = f0 8f bf bf
    validate_fail("\xf0\x8f\xbf\xbf");

    // 4.2.4  U-001FFFFF = f8 87 bf bf bf
    validate_fail("\xf8\x87\xbf\xbf\xbf");

    // 4.2.5  U-03FFFFFF = fc 83 bf bf bf bf
    validate_fail("\xfc\x83\xbf\xbf\xbf\xbf");

    // 4.3  Overlong representation of the NUL character

    // 4.3.1  U+0000 = c0 80
    validate_fail("\xc0\x80");

    // 4.3.2  U+0000 = e0 80 80
    validate_fail("\xe0\x80\x80");

    // 4.3.3  U+0000 = f0 80 80 80
    validate_fail("\xf0\x80\x80\x80");

    // 4.3.4  U+0000 = f8 80 80 80 80
    validate_fail("\xf8\x80\x80\x80\x80");

    // 4.3.5  U+0000 = fc 80 80 80 80 80
    validate_fail("\xfc\x80\x80\x80\x80\x80");

    // 5  Illegal code positions

    // 5.1 Single UTF-16 surrogates

    // 5.1.1  U+D800 = ed a0 80
    validate_fail("\xed\xa0\x80");

    // 5.1.2  U+DB7F = ed ad bf
    validate_fail("\xed\xad\xbf");

    // 5.1.3  U+DB80 = ed ae 80
    validate_fail("\xed\xae\x80");

    // 5.1.4  U+DBFF = ed af bf
    validate_fail("\xed\xaf\xbf");

    // 5.1.5  U+DC00 = ed b0 80
    validate_fail("\xed\xb0\x80");

    // 5.1.6  U+DF80 = ed be 80
    validate_fail("\xed\xbe\x80");

    // 5.1.7  U+DFFF = ed bf bf
    validate_fail("\xed\xbf\xbf");

    // 5.2 Paired UTF-16 surrogates

    // 5.2.1  U+D800 U+DC00 = ed a0 80 ed b0 80
    validate_fail("\xed\xa0\x80\xed\xb0\x80");

    // 5.2.2  U+D800 U+DFFF = ed a0 80 ed bf bf
    validate_fail("\xed\xa0\x80\xed\xbf\xbf");

    // 5.2.3  U+DB7F U+DC00 = ed ad bf ed b0 80
    validate_fail("\xed\xad\xbf\xed\xb0\x80");

    // 5.2.4  U+DB7F U+DFFF = ed ad bf ed bf bf
    validate_fail("\xed\xad\xbf\xed\xbf\xbf");

    // 5.2.5  U+DB80 U+DC00 = ed ae 80 ed b0 80
    validate_fail("\xed\xae\x80\xed\xb0\x80");

    // 5.2.6  U+DB80 U+DFFF = ed ae 80 ed bf bf
    validate_fail("\xed\xae\x80\xed\xbf\xbf");

    // 5.2.7  U+DBFF U+DC00 = ed af bf ed b0 80
    validate_fail("\xed\xaf\xbf\xed\xb0\x80");

    // 5.2.8  U+DBFF U+DFFF = ed af bf ed bf bf
    validate_fail("\xed\xaf\xbf\xed\xbf\xbf");

    // 5.3 Noncharacter code positions

    // 5.3.1  U+FFFE = ef bf be
    validate_pass("\xef\xbf\xbe");

    // 5.3.2  U+FFFF = ef bf bf
    validate_pass("\xef\xbf\xbf");

    // 5.3.3  U+FDD0 .. U+FDEF
    validate_pass("\xef\xb7\x90");
    validate_pass("\xef\xb7\x91");
    validate_pass("\xef\xb7\x92");
    validate_pass("\xef\xb7\x93");
    validate_pass("\xef\xb7\x94");
    validate_pass("\xef\xb7\x95");
    validate_pass("\xef\xb7\x96");
    validate_pass("\xef\xb7\x97");
    validate_pass("\xef\xb7\x98");
    validate_pass("\xef\xb7\x99");
    validate_pass("\xef\xb7\x9a");
    validate_pass("\xef\xb7\x9b");
    validate_pass("\xef\xb7\x9c");
    validate_pass("\xef\xb7\x9d");
    validate_pass("\xef\xb7\x9e");
    validate_pass("\xef\xb7\x9f");
    validate_pass("\xef\xb7\xa0");
    validate_pass("\xef\xb7\xa1");
    validate_pass("\xef\xb7\xa2");
    validate_pass("\xef\xb7\xa3");
    validate_pass("\xef\xb7\xa4");
    validate_pass("\xef\xb7\xa5");
    validate_pass("\xef\xb7\xa6");
    validate_pass("\xef\xb7\xa7");
    validate_pass("\xef\xb7\xa8");
    validate_pass("\xef\xb7\xa9");
    validate_pass("\xef\xb7\xaa");
    validate_pass("\xef\xb7\xab");
    validate_pass("\xef\xb7\xac");
    validate_pass("\xef\xb7\xad");
    validate_pass("\xef\xb7\xae");
    validate_pass("\xef\xb7\xaf");

    // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
    validate_pass("\xf0\x9f\xbf\xbf");
    validate_pass("\xf0\xaf\xbf\xbf");
    validate_pass("\xf0\xbf\xbf\xbf");
    validate_pass("\xf1\x8f\xbf\xbf");
    validate_pass("\xf1\x9f\xbf\xbf");
    validate_pass("\xf1\xaf\xbf\xbf");
    validate_pass("\xf1\xbf\xbf\xbf");
    validate_pass("\xf2\x8f\xbf\xbf");
    validate_pass("\xf2\x9f\xbf\xbf");
    validate_pass("\xf2\xaf\xbf\xbf");
}

//==============================================================================
TEST(JsonTest, MarkusKuhnExtended)
{
    // Exceptions not caught by Markus Kuhn's stress test
    validate_fail("\x22");
    validate_fail("\x5c");

    validate_fail("\xe0\xa0\x79");
    validate_fail("\xe0\xa0\xff");

    validate_fail("\xed\x80\x79");
    validate_fail("\xed\x80\xff");

    validate_fail("\xf0\x90\x79");
    validate_fail("\xf0\x90\xff");
    validate_fail("\xf0\x90\x80\x79");
    validate_fail("\xf0\x90\x80\xff");

    validate_fail("\xf1\x80\x79");
    validate_fail("\xf1\x80\xff");
    validate_fail("\xf1\x80\x80\x79");
    validate_fail("\xf1\x80\x80\xff");

    validate_fail("\xf4\x80\x79");
    validate_fail("\xf4\x80\xff");
    validate_fail("\xf4\x80\x80\x79");
    validate_fail("\xf4\x80\x80\xff");
}
