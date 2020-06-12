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
};

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
        validate_fail(u8"\xf0\x1f\x8d\x9f");

        // Third byte of U+1f355 masked with 0b0011'1111.
        validate_fail(u8"\xf0\x9f\x0d\x9f");

        // Fourth byte of U+1f355 masked with 0b0011'1111.
        validate_fail(u8"\xf0\x9f\x8d\x1f");
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
TYPED_TEST(BasicStringTest, Utf32EncodingReservedCodepoint)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
    };

    if constexpr (sizeof(char_type) == 4)
    {
        for (char_type ch = 0xd800; ch <= 0xdfff; ++ch)
        {
            validate_fail(string_type(1, ch));
        }
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf32OutOfRangeCodepoint)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::escape_unicode_character(begin, end), fly::UnicodeException);
        EXPECT_THROW(StringClass::escape_unicode_string(test), fly::UnicodeException);
    };

    if constexpr (sizeof(char_type) == 4)
    {
        // Iterating all the way to numeric_limits<char_type>::max() takes way too long.
        for (char_type ch = 0x110000; ch <= 0x1100ff; ++ch)
        {
            validate_fail(string_type(1, ch));
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
        validate_fail(u8"\xf0");

        // First two bytes of U+1f355.
        validate_fail(u8"\xf0\x9f");

        // First three bytes of U+1f355.
        validate_fail(u8"\xf0\x9f\x8d");
    }
    else if constexpr (sizeof(char_type) == 2)
    {
        // High surrogate for U+1f355.
        validate_fail(u"\xd83c");
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
        SCOPED_TRACE(static_cast<std::uint32_t>(ch));

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
        SCOPED_TRACE(static_cast<std::uint32_t>(ch));

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
    for (std::uint32_t ch = 0xdc00; ch <= 0xdfff; ++ch)
    {
        validate_fail(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));
    }

    // High surrogate only.
    for (std::uint32_t ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        validate_fail(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));
    }

    // High surrogate followed by non-surrogate.
    for (std::uint32_t ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        string_type high_surrogate(FLY_STR(char_type, "\\u") + to_hex<string_type>(ch, 4));
        string_type low_surrogate(FLY_STR(char_type, "\\u0000"));

        validate_fail(high_surrogate + low_surrogate);
    }

    // High surrogate followed by high surrogate.
    for (std::uint32_t ch = 0xd800; ch <= 0xdbff; ++ch)
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
