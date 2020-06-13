#include "fly/types/string/string.hpp"
#include "fly/types/string/string_exception.hpp"
#include "fly/types/string/string_literal.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

namespace {

//==================================================================================================
template <typename StringType, typename CharType>
StringType to_hex(CharType ch, std::size_t length)
{
    static const auto *digits = FLY_STR(typename StringType::value_type, "0123456789abcdef");
    StringType hex(length, FLY_CHR(typename StringType::value_type, '0'));

    for (std::size_t i = 0, j = (length - 1) * 4; i < length; ++i, j -= 4)
    {
        hex[i] = digits[(ch >> j) & 0x0f];
    }

    return hex;
}

} // namespace

//==================================================================================================
TYPED_TEST(BasicStringTest, EmptyString)
{
    DECLARE_ALIASES

    string_type test;
    string_type actual;

    EXPECT_NO_THROW(actual = StringClass::escape_unicode_string(test));
    EXPECT_EQ(actual, test);

    EXPECT_NO_THROW(actual = StringClass::unescape_unicode_string(test));
    EXPECT_EQ(actual, test);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, PastTheEndIterators)
{
    DECLARE_ALIASES

    string_type test;

    auto begin = test.cend();
    const auto end = test.cend();

    EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
    EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf8EncodingInvalidLeadingByte)
{
    DECLARE_ALIASES

    if constexpr (sizeof(char_type) == 1)
    {
        string_type test("\xff");

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf8EncodingInvalidContinuationByte)
{
    DECLARE_ALIASES

    if constexpr (sizeof(char_type) == 1)
    {
        auto validate_fail = [](string_type &&test) {
            SCOPED_TRACE(test.c_str());

            auto begin = test.cbegin();
            const auto end = test.cend();

            EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
            EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
        };

        // Second byte of U+1f355 masked with 0b0011'1111.
        validate_fail("\xf0\x1f\x8d\x9f");

        // Third byte of U+1f355 masked with 0b0011'1111.
        validate_fail("\xf0\x9f\x0d\x9f");

        // Fourth byte of U+1f355 masked with 0b0011'1111.
        validate_fail("\xf0\x9f\x8d\x1f");
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf8EncodingOverlong)
{
    DECLARE_ALIASES

    if constexpr (sizeof(char_type) == 1)
    {
        auto validate_fail = [](string_type &&test) {
            SCOPED_TRACE(test.c_str());

            auto begin = test.cbegin();
            const auto end = test.cend();

            EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
            EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
        };

        // U+0021 2-byte overlong encoding.
        validate_fail("\xc0\xa1");

        // U+0021 3-byte overlong encoding.
        validate_fail("\xe0\x80\xa1");

        // U+0021 4-byte overlong encoding.
        validate_fail("\xf0\x80\x80\xa1");
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf16EncodingInvalidSurrogates)
{
    DECLARE_ALIASES

    if constexpr (sizeof(char_type) == 2)
    {
        auto validate_fail = [](string_type &&test) {
            SCOPED_TRACE(test.c_str());

            auto begin = test.cbegin();
            const auto end = test.cend();

            EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
            EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
        };

        // Low surrogate only.
        for (char_type ch = 0xdc00; ch <= 0xdfff; ++ch)
        {
            validate_fail(string_type(1, ch));
        }

        // High surrogate only.
        for (char_type ch = 0xd800; ch <= 0xdbff; ++ch)
        {
            validate_fail(string_type(1, ch));
        }

        // High surrogate followed by non-surrogate.
        for (char_type ch = 0xd800; ch <= 0xdbff; ++ch)
        {
            string_type high_surrogate(1, ch);
            string_type low_surrogate(1, 0);

            validate_fail(high_surrogate + low_surrogate);
        }

        // High surrogate followed by high surrogate.
        for (char_type ch = 0xd800; ch <= 0xdbff; ++ch)
        {
            string_type high_surrogate(1, ch);

            validate_fail(high_surrogate + high_surrogate);
        }
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ReservedCodepoint)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
    };

    for (codepoint_type ch = 0xd800; ch <= 0xdfff; ++ch)
    {
        if constexpr (sizeof(char_type) == 1)
        {
            string_type test;
            test += static_cast<char_type>(0xe0 | (ch >> 12));
            test += static_cast<char_type>(0x80 | ((ch >> 6) & 0x3f));
            test += static_cast<char_type>(0x80 | (ch & 0x3f));

            validate_fail(std::move(test));
        }
        else
        {
            // Note: UTF-16 doesn't actually hit the reserved codepoint exception because the
            // reserved codepoints are invalid alone, and thus fail earlier.
            validate_fail(string_type(1, static_cast<char_type>(ch)));
        }
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, OutOfRangeCodepoint)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
    };

    // Iterating all the way to numeric_limits<char_type>::max() takes way too long.
    for (codepoint_type ch = 0x110000; ch <= 0x1100ff; ++ch)
    {
        if constexpr (sizeof(char_type) == 1)
        {
            string_type test;
            test += static_cast<char_type>(0xf0 | (ch >> 18));
            test += static_cast<char_type>(0x80 | ((ch >> 12) & 0x3f));
            test += static_cast<char_type>(0x80 | ((ch >> 6) & 0x3f));
            test += static_cast<char_type>(0x80 | (ch & 0x3f));

            validate_fail(std::move(test));
        }
        else if constexpr (sizeof(char_type) == 2)
        {
            // Note: UTF-16 doesn't actually hit the out-of-range exception because the out-of-range
            // codepoints are invalid surrogates, and thus fail earlier.
            string_type test;
            test += static_cast<char_type>(0xd800 | ((ch - 0x10000) >> 10));
            test += static_cast<char_type>(0xdc00 | ((ch - 0x10000) & 0x3ff));

            validate_fail(std::move(test));
        }
        else if constexpr (sizeof(char_type) == 4)
        {
            validate_fail(string_type(1, static_cast<char_type>(ch)));
        }
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingNotEnoughData)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
    };

    if constexpr (sizeof(char_type) == 1)
    {
        // First byte of U+1f355.
        validate_fail("\xf0");

        // First two bytes of U+1f355.
        validate_fail("\xf0\x9f");

        // First three bytes of U+1f355.
        validate_fail("\xf0\x9f\x8d");
    }
    else if constexpr (sizeof(char_type) == 2)
    {
        // High surrogate for U+1f355.
        validate_fail(string_type(1, 0xd83c));
    }
    else if constexpr (sizeof(char_type) == 4)
    {
        // UTF-32 encoding really only fails if there is no data.
        string_type test;
        auto begin = test.cend();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingPrintableASCIINotEncoded)
{
    DECLARE_ALIASES

    auto validate_pass = [](char_type ch) {
        SCOPED_TRACE(static_cast<codepoint_type>(ch));

        const string_type test(1, ch);
        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::escape_unicode_character(begin, end));
        EXPECT_EQ(actual, test);

        ASSERT_NO_THROW(actual = StringClass::escape_unicode_string(test));
        EXPECT_EQ(actual, test);
    };

    for (char_type ch = 0x20; ch < 0x7f; ++ch)
    {
        validate_pass(ch);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingNonPrintableASCIIEncodedWithLowerU)
{
    DECLARE_ALIASES

    auto validate_pass = [](char_type ch) {
        SCOPED_TRACE(static_cast<codepoint_type>(ch));

        // ASCII symbols should always be encoded with \u.
        const string_type expected = FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4);
        const string_type test(1, ch);
        {
            auto begin = test.cbegin();
            const auto end = test.cend();

            string_type actual;
            ASSERT_NO_THROW(
                actual = StringClass::template escape_unicode_character<'u'>(begin, end));
            EXPECT_EQ(actual, expected);

            ASSERT_NO_THROW(actual = StringClass::template escape_unicode_string<'u'>(test));
            EXPECT_EQ(actual, expected);
        }
        {
            auto begin = test.cbegin();
            const auto end = test.cend();

            string_type actual;
            ASSERT_NO_THROW(
                actual = StringClass::template escape_unicode_character<'U'>(begin, end));
            EXPECT_EQ(actual, expected);

            ASSERT_NO_THROW(actual = StringClass::template escape_unicode_string<'U'>(test));
            EXPECT_EQ(actual, expected);
        }
    };

    for (char_type ch = 0; ch < 0x20; ++ch)
    {
        validate_pass(ch);
    }

    validate_pass(0x7f);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingToLowerU)
{
    DECLARE_ALIASES

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::template escape_unicode_character<'u'>(begin, end));
        EXPECT_EQ(actual, expected);

        ASSERT_NO_THROW(actual = StringClass::template escape_unicode_string<'u'>(test));
        EXPECT_EQ(actual, expected);
    };

    validate_pass(FLY_STR(char_type, "\U00010000"), FLY_STR(char_type, "\\ud800\\udc00"));
    validate_pass(FLY_STR(char_type, "\U00010e6d"), FLY_STR(char_type, "\\ud803\\ude6d"));
    validate_pass(FLY_STR(char_type, "\U0001d11e"), FLY_STR(char_type, "\\ud834\\udd1e"));
    validate_pass(FLY_STR(char_type, "\U0001f355"), FLY_STR(char_type, "\\ud83c\\udf55"));
    validate_pass(FLY_STR(char_type, "\U0010ffff"), FLY_STR(char_type, "\\udbff\\udfff"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingToUpperU)
{
    DECLARE_ALIASES

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::template escape_unicode_character<'U'>(begin, end));
        EXPECT_EQ(actual, expected);

        ASSERT_NO_THROW(actual = StringClass::template escape_unicode_string<'U'>(test));
        EXPECT_EQ(actual, expected);
    };

    validate_pass(FLY_STR(char_type, "\U00010000"), FLY_STR(char_type, "\\U00010000"));
    validate_pass(FLY_STR(char_type, "\U00010e6d"), FLY_STR(char_type, "\\U00010e6d"));
    validate_pass(FLY_STR(char_type, "\U0001d11e"), FLY_STR(char_type, "\\U0001d11e"));
    validate_pass(FLY_STR(char_type, "\U0001f355"), FLY_STR(char_type, "\\U0001f355"));
    validate_pass(FLY_STR(char_type, "\U0010ffff"), FLY_STR(char_type, "\\U0010ffff"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingStringToLowerU)
{
    DECLARE_ALIASES

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::template escape_unicode_string<'u'>(test));
        EXPECT_EQ(actual, expected);
    };

    validate_pass(FLY_STR(char_type, "No unicode!"), FLY_STR(char_type, "No unicode!"));

    validate_pass(
        FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
        FLY_STR(char_type, "\\ud83c\\udf55 in the morning, \\ud83c\\udf55 in the evening"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingStringToUpperU)
{
    DECLARE_ALIASES

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::template escape_unicode_string<'U'>(test));
        EXPECT_EQ(actual, expected);
    };

    validate_pass(FLY_STR(char_type, "No unicode!"), FLY_STR(char_type, "No unicode!"));

    validate_pass(
        FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
        FLY_STR(char_type, "\\U0001f355 in the morning, \\U0001f355 in the evening"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, InvalidEscapeSequences)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
    };

    validate_fail(FLY_STR(char_type, ""));
    validate_fail(FLY_STR(char_type, "f"));
    validate_fail(FLY_STR(char_type, "\\f"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, DecodingNotEnoughData)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::unescape_unicode_string(test), fly::UnicodeException);
    };

    validate_fail(FLY_STR(char_type, "\\u"));
    validate_fail(FLY_STR(char_type, "\\u0"));
    validate_fail(FLY_STR(char_type, "\\u00"));
    validate_fail(FLY_STR(char_type, "\\u000"));

    validate_fail(FLY_STR(char_type, "\\ud800\\u"));
    validate_fail(FLY_STR(char_type, "\\ud800\\u0"));
    validate_fail(FLY_STR(char_type, "\\ud800\\u00"));
    validate_fail(FLY_STR(char_type, "\\ud800\\u000"));

    validate_fail(FLY_STR(char_type, "\\U"));
    validate_fail(FLY_STR(char_type, "\\U0"));
    validate_fail(FLY_STR(char_type, "\\U00"));
    validate_fail(FLY_STR(char_type, "\\U000"));
    validate_fail(FLY_STR(char_type, "\\U0000"));
    validate_fail(FLY_STR(char_type, "\\U00000"));
    validate_fail(FLY_STR(char_type, "\\U000000"));
    validate_fail(FLY_STR(char_type, "\\U0000000"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, DecodingNonHexadecimal)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::unescape_unicode_string(test), fly::UnicodeException);
    };

    validate_fail(FLY_STR(char_type, "\\u000z"));
    validate_fail(FLY_STR(char_type, "\\ud800\\u000z"));
    validate_fail(FLY_STR(char_type, "\\U0000000z"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, DecodingInvalidSurrogates)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::unescape_unicode_string(test), fly::UnicodeException);
    };

    // Low surrogate only.
    for (codepoint_type ch = 0xdc00; ch <= 0xdfff; ++ch)
    {
        validate_fail(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));
    }

    // High surrogate only.
    for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        validate_fail(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));
    }

    // High surrogate followed by non-surrogate.
    for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        string_type high_surrogate(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));
        string_type low_surrogate(FLY_STR(char_type, "\\u0000"));

        validate_fail(high_surrogate + low_surrogate);
    }

    // High surrogate followed by high surrogate.
    for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        string_type high_surrogate(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));

        validate_fail(high_surrogate + high_surrogate);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, DecodingValid)
{
    DECLARE_ALIASES

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::unescape_unicode_character(begin, end));
        EXPECT_EQ(actual, expected);

        ASSERT_NO_THROW(actual = StringClass::unescape_unicode_string(test));
        EXPECT_EQ(actual, expected);
    };

    validate_pass(FLY_STR(char_type, "\\u0040"), FLY_STR(char_type, "\u0040"));
    validate_pass(FLY_STR(char_type, "\\u007a"), FLY_STR(char_type, "\u007a"));
    validate_pass(FLY_STR(char_type, "\\u007a"), FLY_STR(char_type, "\u007a"));
    validate_pass(FLY_STR(char_type, "\\u00c4"), FLY_STR(char_type, "\u00c4"));
    validate_pass(FLY_STR(char_type, "\\u00e4"), FLY_STR(char_type, "\u00e4"));
    validate_pass(FLY_STR(char_type, "\\u0298"), FLY_STR(char_type, "\u0298"));
    validate_pass(FLY_STR(char_type, "\\u0800"), FLY_STR(char_type, "\u0800"));
    validate_pass(FLY_STR(char_type, "\\uffff"), FLY_STR(char_type, "\uffff"));

    validate_pass(FLY_STR(char_type, "\\ud800\\udc00"), FLY_STR(char_type, "\U00010000"));
    validate_pass(FLY_STR(char_type, "\\ud803\\ude6d"), FLY_STR(char_type, "\U00010e6d"));
    validate_pass(FLY_STR(char_type, "\\ud834\\udd1e"), FLY_STR(char_type, "\U0001d11e"));
    validate_pass(FLY_STR(char_type, "\\udbff\\udfff"), FLY_STR(char_type, "\U0010ffff"));

    validate_pass(FLY_STR(char_type, "\\U00010000"), FLY_STR(char_type, "\U00010000"));
    validate_pass(FLY_STR(char_type, "\\U00010e6d"), FLY_STR(char_type, "\U00010e6d"));
    validate_pass(FLY_STR(char_type, "\\U0001d11e"), FLY_STR(char_type, "\U0001d11e"));
    validate_pass(FLY_STR(char_type, "\\U0010ffff"), FLY_STR(char_type, "\U0010ffff"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, DecodingStringValid)
{
    DECLARE_ALIASES

    auto validate_pass = [](const string_type &test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        string_type actual;
        ASSERT_NO_THROW(actual = StringClass::unescape_unicode_string(test));
        EXPECT_EQ(actual, expected);
    };

    validate_pass(FLY_STR(char_type, "No unicode!"), FLY_STR(char_type, "No unicode!"));
    validate_pass(FLY_STR(char_type, "Other escape \t"), FLY_STR(char_type, "Other escape \t"));
    validate_pass(FLY_STR(char_type, "Other escape \\t"), FLY_STR(char_type, "Other escape \\t"));

    validate_pass(
        FLY_STR(char_type, "\\U0001f355 in the morning, \\U0001f355 in the evening"),
        FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, MarkusKuhnStressTest)
{
    DECLARE_ALIASES

    // Markus Kuhn UTF-8 decoder capability and stress test
    // http://www.cl.cam.ac.uk/~mgk25/ucs/examples/UTF-8-test.txt
    if constexpr (sizeof(char_type) == 1)
    {
        auto validate_pass = [](string_type &&test, string_type &&expected) {
            SCOPED_TRACE(test.c_str());

            string_type actual;
            ASSERT_NO_THROW(actual = StringClass::escape_unicode_string(test));
            EXPECT_EQ(actual, expected);
        };

        auto validate_fail = [](string_type &&test) {
            SCOPED_TRACE(test.c_str());

            EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
        };

        // 1  Some correct UTF-8 text
        validate_pass("κόσμε", "\\u03ba\\u1f79\\u03c3\\u03bc\\u03b5");

        // 2  Boundary condition test cases

        // 2.1  First possible sequence of a certain length

        // 2.1.1  1 byte  (U-00000000)
        validate_pass(string_type(1, 0x00), "\\u0000");

        // 2.1.2  2 bytes (U-00000080)
        validate_pass("\xc2\x80", "\\u0080");

        // 2.1.3  3 bytes (U-00000800)
        validate_pass("\xe0\xa0\x80", "\\u0800");

        // 2.1.4  4 bytes (U-00010000)
        validate_pass("\xf0\x90\x80\x80", "\\U00010000");

        // 2.1.5  5 bytes (U-00200000)
        validate_fail("\xf8\x88\x80\x80\x80");

        // 2.1.6  6 bytes (U-04000000)
        validate_fail("\xfc\x84\x80\x80\x80\x80");

        // 2.2  Last possible sequence of a certain length

        // 2.2.1  1 byte  (U-0000007F)
        validate_pass("\x7f", "\\u007f");

        // 2.2.2  2 bytes (U-000007FF)
        validate_pass("\xdf\xbf", "\\u07ff");

        // 2.2.3  3 bytes (U-0000FFFF)
        validate_pass("\xef\xbf\xbf", "\\uffff");

        // 2.1.4  4 bytes (U-001FFFFF)
        validate_fail("\xf7\xbf\xbf\xbf");

        // 2.1.5  5 bytes (U-03FFFFFF)
        validate_fail("\xfb\xbf\xbf\xbf\xbf");

        // 2.1.6  6 bytes (U-7FFFFFFF)
        validate_fail("\xfd\xbf\xbf\xbf\xbf\xbf");

        // 2.3  Other boundary conditions

        // 2.3.1  U-0000D7FF = ed 9f bf
        validate_pass("\xed\x9f\xbf", "\\ud7ff");

        // 2.3.2  U-0000E000 = ee 80 80
        validate_pass("\xee\x80\x80", "\\ue000");

        // 2.3.3  U-0000FFFD = ef bf bd
        validate_pass("\xef\xbf\xbd", "\\ufffd");

        // 2.3.4  U-0010FFFF = f4 8f bf bf
        validate_pass("\xf4\x8f\xbf\xbf", "\\U0010ffff");

        // 2.3.5  U-00110000 = f4 90 80 80
        validate_fail("\xf4\x90\x80\x80");

        // 3  Malformed sequences

        // 3.1  Unexpected continuation bytes

        // 3.1.1  First continuation byte 0x80
        validate_fail("\x80");

        // 3.1.2 Last continuation byte 0xbf
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
        string_type test_3_1_9;

        for (codepoint_type ch = 0x80; ch <= 0xbf; ++ch)
        {
            validate_fail(string_type(1, ch));
            test_3_1_9 += ch;
        }

        validate_fail(std::move(test_3_1_9));

        // 3.2  Lonely start characters

        auto validate_fail_sequence = [&validate_fail](codepoint_type begin, codepoint_type end) {
            string_type test_3_2;

            for (codepoint_type ch = begin; ch <= end; ++ch)
            {
                validate_fail(string_type(1, ch) + " ");
                test_3_2 += ch;
                test_3_2 += ' ';
            }

            validate_fail(std::move(test_3_2));
        };

        // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf), each followed by a space
        // character
        validate_fail_sequence(0xc0, 0xdf);

        // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef) each followed by a space
        // character
        validate_fail_sequence(0xe0, 0xef);

        // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7), each followed by a space
        // character
        validate_fail_sequence(0xf0, 0xf7);

        // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb), each followed by a space
        // character
        validate_fail_sequence(0xf8, 0xfb);

        // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd), each followed by a space
        // character
        validate_fail_sequence(0xfc, 0xfd);

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
            "\xc0\xe0\x80\xf0\x80\x80\xf8\x80\x80\x80\xfc\x80\x80\x80\x80\xdf\xef\xbf\xf7\xbf\xbf"
            "\xfb\xbf\xbf\xbf\xfd\xbf\xbf\xbf\xbf");

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
        validate_pass("\xef\xbf\xbe", "\\ufffe");

        // 5.3.2  U+FFFF = ef bf bf
        validate_pass("\xef\xbf\xbf", "\\uffff");

        // 5.3.3  U+FDD0 .. U+FDEF
        validate_pass("\xef\xb7\x90", "\\ufdd0");
        validate_pass("\xef\xb7\x91", "\\ufdd1");
        validate_pass("\xef\xb7\x92", "\\ufdd2");
        validate_pass("\xef\xb7\x93", "\\ufdd3");
        validate_pass("\xef\xb7\x94", "\\ufdd4");
        validate_pass("\xef\xb7\x95", "\\ufdd5");
        validate_pass("\xef\xb7\x96", "\\ufdd6");
        validate_pass("\xef\xb7\x97", "\\ufdd7");
        validate_pass("\xef\xb7\x98", "\\ufdd8");
        validate_pass("\xef\xb7\x99", "\\ufdd9");
        validate_pass("\xef\xb7\x9a", "\\ufdda");
        validate_pass("\xef\xb7\x9b", "\\ufddb");
        validate_pass("\xef\xb7\x9c", "\\ufddc");
        validate_pass("\xef\xb7\x9d", "\\ufddd");
        validate_pass("\xef\xb7\x9e", "\\ufdde");
        validate_pass("\xef\xb7\x9f", "\\ufddf");
        validate_pass("\xef\xb7\xa0", "\\ufde0");
        validate_pass("\xef\xb7\xa1", "\\ufde1");
        validate_pass("\xef\xb7\xa2", "\\ufde2");
        validate_pass("\xef\xb7\xa3", "\\ufde3");
        validate_pass("\xef\xb7\xa4", "\\ufde4");
        validate_pass("\xef\xb7\xa5", "\\ufde5");
        validate_pass("\xef\xb7\xa6", "\\ufde6");
        validate_pass("\xef\xb7\xa7", "\\ufde7");
        validate_pass("\xef\xb7\xa8", "\\ufde8");
        validate_pass("\xef\xb7\xa9", "\\ufde9");
        validate_pass("\xef\xb7\xaa", "\\ufdea");
        validate_pass("\xef\xb7\xab", "\\ufdeb");
        validate_pass("\xef\xb7\xac", "\\ufdec");
        validate_pass("\xef\xb7\xad", "\\ufded");
        validate_pass("\xef\xb7\xae", "\\ufdee");
        validate_pass("\xef\xb7\xaf", "\\ufdef");

        // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
        validate_pass("\xf0\x9f\xbf\xbe", "\\U0001fffe");
        validate_pass("\xf0\x9f\xbf\xbf", "\\U0001ffff");
        validate_pass("\xf0\xaf\xbf\xbe", "\\U0002fffe");
        validate_pass("\xf0\xaf\xbf\xbf", "\\U0002ffff");
        validate_pass("\xf0\xbf\xbf\xbe", "\\U0003fffe");
        validate_pass("\xf0\xbf\xbf\xbf", "\\U0003ffff");
        validate_pass("\xf1\x8f\xbf\xbe", "\\U0004fffe");
        validate_pass("\xf1\x8f\xbf\xbf", "\\U0004ffff");
        validate_pass("\xf1\x9f\xbf\xbe", "\\U0005fffe");
        validate_pass("\xf1\x9f\xbf\xbf", "\\U0005ffff");
        validate_pass("\xf1\xaf\xbf\xbe", "\\U0006fffe");
        validate_pass("\xf1\xaf\xbf\xbf", "\\U0006ffff");
        validate_pass("\xf1\xbf\xbf\xbe", "\\U0007fffe");
        validate_pass("\xf1\xbf\xbf\xbf", "\\U0007ffff");
        validate_pass("\xf2\x8f\xbf\xbe", "\\U0008fffe");
        validate_pass("\xf2\x8f\xbf\xbf", "\\U0008ffff");
        validate_pass("\xf2\x9f\xbf\xbe", "\\U0009fffe");
        validate_pass("\xf2\x9f\xbf\xbf", "\\U0009ffff");
        validate_pass("\xf2\xaf\xbf\xbe", "\\U000afffe");
        validate_pass("\xf2\xaf\xbf\xbf", "\\U000affff");
    }
}
