#include "fly/types/string/string.hpp"
#include "fly/types/string/string_literal.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

#include <optional>
#include <vector>

//==================================================================================================
TYPED_TEST(BasicStringTest, EmptyString)
{
    DECLARE_ALIASES

    string_type test;
    std::optional<string_type> actual;

    auto begin = test.cend();
    const auto end = test.cend();

    EXPECT_FALSE(StringClass::decode_codepoint(begin, end).has_value());

    actual = StringClass::escape_all_codepoints(test);
    ASSERT_TRUE(actual.has_value());
    EXPECT_EQ(actual.value(), test);

    actual = StringClass::unescape_all_codepoints(test);
    ASSERT_TRUE(actual.has_value());
    EXPECT_EQ(actual.value(), test);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, PastTheEndIterators)
{
    DECLARE_ALIASES

    string_type test;

    auto begin = test.cend();
    const auto end = test.cend();

    EXPECT_FALSE(StringClass::decode_codepoint(begin, end).has_value());
    EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
    EXPECT_FALSE(StringClass::unescape_codepoint(begin, end).has_value());
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

        EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
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

            EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
            EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
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

            EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
            EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
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

            EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
            EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
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

    auto validate_fail = [](string_type &&test, codepoint_type codepoint) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_FALSE(StringClass::encode_codepoint(codepoint).has_value());
        EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
    };

    for (codepoint_type ch = 0xd800; ch <= 0xdfff; ++ch)
    {
        if constexpr (sizeof(char_type) == 1)
        {
            string_type test;
            test += static_cast<char_type>(0xe0 | (ch >> 12));
            test += static_cast<char_type>(0x80 | ((ch >> 6) & 0x3f));
            test += static_cast<char_type>(0x80 | (ch & 0x3f));

            validate_fail(std::move(test), ch);
        }
        else
        {
            // Note: UTF-16 doesn't actually hit the reserved codepoint error because the reserved
            // codepoints are invalid alone, and thus fail earlier.
            validate_fail(string_type(1, static_cast<char_type>(ch)), ch);
        }
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, OutOfRangeCodepoint)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test, codepoint_type codepoint) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_FALSE(StringClass::encode_codepoint(codepoint).has_value());
        EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
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

            validate_fail(std::move(test), ch);
        }
        else if constexpr (sizeof(char_type) == 2)
        {
            // Note: UTF-16 doesn't actually hit the out-of-range error because the out-of-range
            // codepoints are invalid surrogates, and thus fail earlier.
            string_type test;
            test += static_cast<char_type>(0xd800 | ((ch - 0x10000) >> 10));
            test += static_cast<char_type>(0xdc00 | ((ch - 0x10000) & 0x3ff));

            validate_fail(std::move(test), ch);
        }
        else if constexpr (sizeof(char_type) == 4)
        {
            validate_fail(string_type(1, static_cast<char_type>(ch)), ch);
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

        EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::escape_all_codepoints(test).has_value());
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

        EXPECT_FALSE(StringClass::escape_codepoint(begin, end).has_value());
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingPrintableASCIINotEncoded)
{
    DECLARE_ALIASES

    auto validate_pass = [](codepoint_type ch) {
        SCOPED_TRACE(ch);

        const string_type test(1, static_cast<char_type>(ch));
        auto begin = test.cbegin();
        const auto end = test.cend();

        std::optional<string_type> actual;

        actual = StringClass::encode_codepoint(ch);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), test);

        actual = StringClass::escape_codepoint(begin, end);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), test);

        actual = StringClass::escape_all_codepoints(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), test);
    };

    for (codepoint_type ch = 0x20; ch < 0x7f; ++ch)
    {
        validate_pass(ch);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EncodingNonPrintableASCIIEncodedWithLowerU)
{
    DECLARE_ALIASES

    auto validate_pass = [](codepoint_type ch) {
        SCOPED_TRACE(ch);

        // ASCII symbols should always be encoded with \u.
        const string_type expected =
            FLY_STR(char_type, "\\u") + StringClass::create_hex_string(ch, 4);
        const string_type test(1, static_cast<char_type>(ch));

        std::optional<string_type> actual;
        {
            auto begin = test.cbegin();
            const auto end = test.cend();

            actual = StringClass::encode_codepoint(ch);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), test);

            actual = StringClass::template escape_codepoint<'u'>(begin, end);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), expected);

            actual = StringClass::template escape_all_codepoints<'u'>(test);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), expected);
        }
        {
            auto begin = test.cbegin();
            const auto end = test.cend();

            actual = StringClass::encode_codepoint(ch);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), test);

            actual = StringClass::template escape_codepoint<'U'>(begin, end);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), expected);

            actual = StringClass::template escape_all_codepoints<'U'>(test);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), expected);
        }
    };

    for (codepoint_type ch = 0; ch < 0x20; ++ch)
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

        std::optional<string_type> actual;

        actual = StringClass::template escape_codepoint<'u'>(begin, end);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);

        actual = StringClass::template escape_all_codepoints<'u'>(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);
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

        std::optional<string_type> actual;

        actual = StringClass::template escape_codepoint<'U'>(begin, end);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);

        actual = StringClass::template escape_all_codepoints<'U'>(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);
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

        std::optional<string_type> actual = StringClass::template escape_all_codepoints<'u'>(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);
    };

    validate_pass(FLY_STR(char_type, "No Unicode!"), FLY_STR(char_type, "No Unicode!"));

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

        std::optional<string_type> actual = StringClass::template escape_all_codepoints<'U'>(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);
    };

    validate_pass(FLY_STR(char_type, "No Unicode!"), FLY_STR(char_type, "No Unicode!"));

    validate_pass(
        FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"),
        FLY_STR(char_type, "\\U0001f355 in the morning, \\U0001f355 in the evening"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, DecodingInvalidEscapeSequences)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_FALSE(StringClass::unescape_codepoint(begin, end).has_value());
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

        EXPECT_FALSE(StringClass::unescape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::unescape_all_codepoints(test).has_value());
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

        EXPECT_FALSE(StringClass::unescape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::unescape_all_codepoints(test).has_value());
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

        EXPECT_FALSE(StringClass::unescape_codepoint(begin, end).has_value());
        EXPECT_FALSE(StringClass::unescape_all_codepoints(test).has_value());
    };

    // Low surrogate only.
    for (codepoint_type ch = 0xdc00; ch <= 0xdfff; ++ch)
    {
        validate_fail(FLY_STR(char_type, "\\u") + StringClass::create_hex_string(ch, 4));
    }

    // High surrogate only.
    for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        validate_fail(FLY_STR(char_type, "\\u") + StringClass::create_hex_string(ch, 4));
    }

    // High surrogate followed by non-surrogate.
    for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        string_type high_surrogate(
            FLY_STR(char_type, "\\u") + StringClass::create_hex_string(ch, 4));
        string_type low_surrogate(FLY_STR(char_type, "\\u0000"));

        validate_fail(high_surrogate + low_surrogate);
    }

    // High surrogate followed by high surrogate.
    for (codepoint_type ch = 0xd800; ch <= 0xdbff; ++ch)
    {
        string_type high_surrogate(
            FLY_STR(char_type, "\\u") + StringClass::create_hex_string(ch, 4));

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

        std::optional<string_type> actual;

        actual = StringClass::unescape_codepoint(begin, end);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);

        actual = StringClass::unescape_all_codepoints(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);
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

        std::optional<string_type> actual = StringClass::unescape_all_codepoints(test);
        ASSERT_TRUE(actual.has_value());
        EXPECT_EQ(actual.value(), expected);
    };

    validate_pass(FLY_STR(char_type, "No Unicode!"), FLY_STR(char_type, "No Unicode!"));
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
    //
    // Note: Any test of 5- or 6-byte sequences have been removed here. BasicString only supports up
    // to 4-byte UTF-8 sequences (Unicode planes 1 - 16). The 5- and 6- byte sequences indeed fail,
    // but not for the reasons the stress test expects. See:
    // https://unicode.org/mail-arch/unicode-ml/Archives-Old/UML018/0332.html
    if constexpr (sizeof(char_type) == 1)
    {
        auto validate_pass = [](string_type &&test, codepoint_type expected, int line) {
            SCOPED_TRACE(test.c_str());
            SCOPED_TRACE(line);

            auto it = test.cbegin();
            const auto end = test.cend();

            std::optional<codepoint_type> actual = StringClass::decode_codepoint(it, end);
            ASSERT_TRUE(actual.has_value());
            EXPECT_EQ(actual.value(), expected);
        };

        auto validate_pass_all =
            [](string_type &&test, std::vector<codepoint_type> expected, int line) {
                SCOPED_TRACE(test.c_str());
                SCOPED_TRACE(line);

                std::size_t index = 0;

                auto it = test.cbegin();
                const auto end = test.cend();

                for (; (it != end) && (index < expected.size()); ++index)
                {
                    std::optional<codepoint_type> actual = StringClass::decode_codepoint(it, end);
                    ASSERT_TRUE(actual.has_value());
                    EXPECT_EQ(actual.value(), expected[index]);
                }

                EXPECT_EQ(index, expected.size());
                EXPECT_EQ(it, end);
            };

        auto validate_fail = [](string_type &&test, std::size_t expected, int line) {
            SCOPED_TRACE(test.c_str());
            SCOPED_TRACE(line);

            const auto end = test.cend();
            std::size_t actual = 0;

            for (auto it = test.cbegin(); it != end;)
            {
                if (!StringClass::decode_codepoint(it, end))
                {
                    ++actual;
                }
            }

            EXPECT_EQ(actual, expected);
        };

        // 1  Some correct UTF-8 text
        validate_pass_all("κόσμε", {0x03ba, 0x1f79, 0x03c3, 0x03bc, 0x03b5}, __LINE__);

        // 2  Boundary condition test cases

        // 2.1  First possible sequence of a certain length

        // 2.1.1  1 byte  (U-00000000)
        validate_pass(string_type(1, 0x00), 0x0000, __LINE__);

        // 2.1.2  2 bytes (U-00000080)
        validate_pass("\xc2\x80", 0x0080, __LINE__);

        // 2.1.3  3 bytes (U-00000800)
        validate_pass("\xe0\xa0\x80", 0x0800, __LINE__);

        // 2.1.4  4 bytes (U-00010000)
        validate_pass("\xf0\x90\x80\x80", 0x10000, __LINE__);

        // 2.2  Last possible sequence of a certain length

        // 2.2.1  1 byte  (U-0000007F)
        validate_pass("\x7f", 0x007f, __LINE__);

        // 2.2.2  2 bytes (U-000007FF)
        validate_pass("\xdf\xbf", 0x07ff, __LINE__);

        // 2.2.3  3 bytes (U-0000FFFF)
        validate_pass("\xef\xbf\xbf", 0xffff, __LINE__);

        // 2.1.4  4 bytes (U-001FFFFF)
        validate_fail("\xf7\xbf\xbf\xbf", 1, __LINE__);

        // 2.3  Other boundary conditions

        // 2.3.1  U-0000D7FF = ed 9f bf
        validate_pass("\xed\x9f\xbf", 0xd7ff, __LINE__);

        // 2.3.2  U-0000E000 = ee 80 80
        validate_pass("\xee\x80\x80", 0xe000, __LINE__);

        // 2.3.3  U-0000FFFD = ef bf bd
        validate_pass("\xef\xbf\xbd", 0xfffd, __LINE__);

        // 2.3.4  U-0010FFFF = f4 8f bf bf
        validate_pass("\xf4\x8f\xbf\xbf", 0x10ffff, __LINE__);

        // 2.3.5  U-00110000 = f4 90 80 80
        validate_fail("\xf4\x90\x80\x80", 1, __LINE__);

        // 3  Malformed sequences

        // 3.1  Unexpected continuation bytes

        // 3.1.1  First continuation byte 0x80
        validate_fail("\x80", 1, __LINE__);

        // 3.1.2 Last continuation byte 0xbf
        validate_fail("\xbf", 1, __LINE__);

        // 3.1.3  2 continuation bytes
        validate_fail("\x80\xbf", 2, __LINE__);

        // 3.1.4  3 continuation bytes
        validate_fail("\x80\xbf\x80", 3, __LINE__);

        // 3.1.5  4 continuation bytes
        validate_fail("\x80\xbf\x80\xbf", 4, __LINE__);

        // 3.1.6  5 continuation bytes
        validate_fail("\x80\xbf\x80\xbf\x80", 5, __LINE__);

        // 3.1.7  6 continuation bytes
        validate_fail("\x80\xbf\x80\xbf\x80\xbf", 6, __LINE__);

        // 3.1.8  7 continuation bytes
        validate_fail("\x80\xbf\x80\xbf\x80\xbf\x80", 7, __LINE__);

        // 3.1.9  Sequence of all 64 possible continuation bytes (0x80-0xbf)
        string_type test_3_1_9;

        for (codepoint_type ch = 0x80; ch <= 0xbf; ++ch)
        {
            validate_fail(string_type(1, ch), 1, __LINE__);
            test_3_1_9 += ch;
        }

        validate_fail(std::move(test_3_1_9), 64, __LINE__);

        // 3.2  Lonely start characters

        auto validate_fail_sequence =
            [&validate_fail](codepoint_type begin, codepoint_type end, int line) {
                string_type test_3_2;

                for (codepoint_type ch = begin; ch <= end; ++ch)
                {
                    validate_fail(string_type(1, ch) + " ", 1, line);
                    test_3_2 += ch;
                    test_3_2 += ' ';
                }

                validate_fail(std::move(test_3_2), end - begin + 1, line);
            };

        // 3.2.1  All 32 first bytes of 2-byte sequences (0xc0-0xdf), each followed by a space
        // character
        validate_fail_sequence(0xc0, 0xdf, __LINE__);

        // 3.2.2  All 16 first bytes of 3-byte sequences (0xe0-0xef) each followed by a space
        // character
        validate_fail_sequence(0xe0, 0xef, __LINE__);

        // 3.2.3  All 8 first bytes of 4-byte sequences (0xf0-0xf7), each followed by a space
        // character
        validate_fail_sequence(0xf0, 0xf7, __LINE__);

        // 3.2.4  All 4 first bytes of 5-byte sequences (0xf8-0xfb), each followed by a space
        // character
        validate_fail_sequence(0xf8, 0xfb, __LINE__);

        // 3.2.5  All 2 first bytes of 6-byte sequences (0xfc-0xfd), each followed by a space
        // character
        validate_fail_sequence(0xfc, 0xfd, __LINE__);

        // 3.3  Sequences with last continuation byte missing

        // 3.3.1  2-byte sequence with last byte missing (U+0000)
        validate_fail("\xc0", 1, __LINE__);

        // 3.3.2  3-byte sequence with last byte missing (U+0000)
        validate_fail("\xe0\x80", 1, __LINE__);

        // 3.3.3  4-byte sequence with last byte missing (U+0000)
        validate_fail("\xf0\x80\x80", 1, __LINE__);

        // 3.3.6  2-byte sequence with last byte missing (U-000007FF)
        validate_fail("\xdf", 1, __LINE__);

        // 3.3.7  3-byte sequence with last byte missing (U-0000FFFF)
        validate_fail("\xef\xbf", 1, __LINE__);

        // 3.3.8  4-byte sequence with last byte missing (U-001FFFFF)
        validate_fail("\xf7\xbf\xbf", 1, __LINE__);

        // 3.4  Concatenation of incomplete sequences

        // All the 6 sequences of 3.3 concatenated
        validate_fail("\xc0\xe0\x80\xf0\x80\x80\xdf\xef\xbf\xf7\xbf\xbf", 6, __LINE__);

        // 3.5  Impossible bytes

        // 3.5.1  fe
        validate_fail("\xfe", 1, __LINE__);

        // 3.5.2  ff
        validate_fail("\xff", 1, __LINE__);

        // 3.5.3  fe fe ff ff
        validate_fail("\xfe\xfe\xff\xff", 4, __LINE__);

        // 4  Overlong sequences

        // 4.1  Examples of an overlong ASCII character

        // 4.1.1 U+002F = c0 af
        validate_fail("\xc0\xaf", 1, __LINE__);

        // 4.1.2 U+002F = e0 80 af
        validate_fail("\xe0\x80\xaf", 1, __LINE__);

        // 4.1.3 U+002F = f0 80 80 af
        validate_fail("\xf0\x80\x80\xaf", 1, __LINE__);

        // 4.2  Maximum overlong sequences

        // 4.2.1  U-0000007F = c1 bf
        validate_fail("\xc1\xbf", 1, __LINE__);

        // 4.2.2  U-000007FF = e0 9f bf
        validate_fail("\xe0\x9f\xbf", 1, __LINE__);

        // 4.2.3  U-0000FFFF = f0 8f bf bf
        validate_fail("\xf0\x8f\xbf\xbf", 1, __LINE__);

        // 4.3  Overlong representation of the NUL character

        // 4.3.1  U+0000 = c0 80
        validate_fail("\xc0\x80", 1, __LINE__);

        // 4.3.2  U+0000 = e0 80 80
        validate_fail("\xe0\x80\x80", 1, __LINE__);

        // 4.3.3  U+0000 = f0 80 80 80
        validate_fail("\xf0\x80\x80\x80", 1, __LINE__);

        // 5  Illegal code positions

        // 5.1 Single UTF-16 surrogates

        // 5.1.1  U+D800 = ed a0 80
        validate_fail("\xed\xa0\x80", 1, __LINE__);

        // 5.1.2  U+DB7F = ed ad bf
        validate_fail("\xed\xad\xbf", 1, __LINE__);

        // 5.1.3  U+DB80 = ed ae 80
        validate_fail("\xed\xae\x80", 1, __LINE__);

        // 5.1.4  U+DBFF = ed af bf
        validate_fail("\xed\xaf\xbf", 1, __LINE__);

        // 5.1.5  U+DC00 = ed b0 80
        validate_fail("\xed\xb0\x80", 1, __LINE__);

        // 5.1.6  U+DF80 = ed be 80
        validate_fail("\xed\xbe\x80", 1, __LINE__);

        // 5.1.7  U+DFFF = ed bf bf
        validate_fail("\xed\xbf\xbf", 1, __LINE__);

        // 5.2 Paired UTF-16 surrogates

        // 5.2.1  U+D800 U+DC00 = ed a0 80 ed b0 80
        validate_fail("\xed\xa0\x80\xed\xb0\x80", 2, __LINE__);

        // 5.2.2  U+D800 U+DFFF = ed a0 80 ed bf bf
        validate_fail("\xed\xa0\x80\xed\xbf\xbf", 2, __LINE__);

        // 5.2.3  U+DB7F U+DC00 = ed ad bf ed b0 80
        validate_fail("\xed\xad\xbf\xed\xb0\x80", 2, __LINE__);

        // 5.2.4  U+DB7F U+DFFF = ed ad bf ed bf bf
        validate_fail("\xed\xad\xbf\xed\xbf\xbf", 2, __LINE__);

        // 5.2.5  U+DB80 U+DC00 = ed ae 80 ed b0 80
        validate_fail("\xed\xae\x80\xed\xb0\x80", 2, __LINE__);

        // 5.2.6  U+DB80 U+DFFF = ed ae 80 ed bf bf
        validate_fail("\xed\xae\x80\xed\xbf\xbf", 2, __LINE__);

        // 5.2.7  U+DBFF U+DC00 = ed af bf ed b0 80
        validate_fail("\xed\xaf\xbf\xed\xb0\x80", 2, __LINE__);

        // 5.2.8  U+DBFF U+DFFF = ed af bf ed bf bf
        validate_fail("\xed\xaf\xbf\xed\xbf\xbf", 2, __LINE__);

        // 5.3 Noncharacter code positions

        // 5.3.1  U+FFFE = ef bf be
        validate_pass("\xef\xbf\xbe", 0xfffe, __LINE__);

        // 5.3.2  U+FFFF = ef bf bf
        validate_pass("\xef\xbf\xbf", 0xffff, __LINE__);

        // 5.3.3  U+FDD0 .. U+FDEF
        validate_pass("\xef\xb7\x90", 0xfdd0, __LINE__);
        validate_pass("\xef\xb7\x91", 0xfdd1, __LINE__);
        validate_pass("\xef\xb7\x92", 0xfdd2, __LINE__);
        validate_pass("\xef\xb7\x93", 0xfdd3, __LINE__);
        validate_pass("\xef\xb7\x94", 0xfdd4, __LINE__);
        validate_pass("\xef\xb7\x95", 0xfdd5, __LINE__);
        validate_pass("\xef\xb7\x96", 0xfdd6, __LINE__);
        validate_pass("\xef\xb7\x97", 0xfdd7, __LINE__);
        validate_pass("\xef\xb7\x98", 0xfdd8, __LINE__);
        validate_pass("\xef\xb7\x99", 0xfdd9, __LINE__);
        validate_pass("\xef\xb7\x9a", 0xfdda, __LINE__);
        validate_pass("\xef\xb7\x9b", 0xfddb, __LINE__);
        validate_pass("\xef\xb7\x9c", 0xfddc, __LINE__);
        validate_pass("\xef\xb7\x9d", 0xfddd, __LINE__);
        validate_pass("\xef\xb7\x9e", 0xfdde, __LINE__);
        validate_pass("\xef\xb7\x9f", 0xfddf, __LINE__);
        validate_pass("\xef\xb7\xa0", 0xfde0, __LINE__);
        validate_pass("\xef\xb7\xa1", 0xfde1, __LINE__);
        validate_pass("\xef\xb7\xa2", 0xfde2, __LINE__);
        validate_pass("\xef\xb7\xa3", 0xfde3, __LINE__);
        validate_pass("\xef\xb7\xa4", 0xfde4, __LINE__);
        validate_pass("\xef\xb7\xa5", 0xfde5, __LINE__);
        validate_pass("\xef\xb7\xa6", 0xfde6, __LINE__);
        validate_pass("\xef\xb7\xa7", 0xfde7, __LINE__);
        validate_pass("\xef\xb7\xa8", 0xfde8, __LINE__);
        validate_pass("\xef\xb7\xa9", 0xfde9, __LINE__);
        validate_pass("\xef\xb7\xaa", 0xfdea, __LINE__);
        validate_pass("\xef\xb7\xab", 0xfdeb, __LINE__);
        validate_pass("\xef\xb7\xac", 0xfdec, __LINE__);
        validate_pass("\xef\xb7\xad", 0xfded, __LINE__);
        validate_pass("\xef\xb7\xae", 0xfdee, __LINE__);
        validate_pass("\xef\xb7\xaf", 0xfdef, __LINE__);

        // 5.3.4  U+nFFFE U+nFFFF (for n = 1..10)
        validate_pass("\xf0\x9f\xbf\xbe", 0x1fffe, __LINE__);
        validate_pass("\xf0\x9f\xbf\xbf", 0x1ffff, __LINE__);
        validate_pass("\xf0\xaf\xbf\xbe", 0x2fffe, __LINE__);
        validate_pass("\xf0\xaf\xbf\xbf", 0x2ffff, __LINE__);
        validate_pass("\xf0\xbf\xbf\xbe", 0x3fffe, __LINE__);
        validate_pass("\xf0\xbf\xbf\xbf", 0x3ffff, __LINE__);
        validate_pass("\xf1\x8f\xbf\xbe", 0x4fffe, __LINE__);
        validate_pass("\xf1\x8f\xbf\xbf", 0x4ffff, __LINE__);
        validate_pass("\xf1\x9f\xbf\xbe", 0x5fffe, __LINE__);
        validate_pass("\xf1\x9f\xbf\xbf", 0x5ffff, __LINE__);
        validate_pass("\xf1\xaf\xbf\xbe", 0x6fffe, __LINE__);
        validate_pass("\xf1\xaf\xbf\xbf", 0x6ffff, __LINE__);
        validate_pass("\xf1\xbf\xbf\xbe", 0x7fffe, __LINE__);
        validate_pass("\xf1\xbf\xbf\xbf", 0x7ffff, __LINE__);
        validate_pass("\xf2\x8f\xbf\xbe", 0x8fffe, __LINE__);
        validate_pass("\xf2\x8f\xbf\xbf", 0x8ffff, __LINE__);
        validate_pass("\xf2\x9f\xbf\xbe", 0x9fffe, __LINE__);
        validate_pass("\xf2\x9f\xbf\xbf", 0x9ffff, __LINE__);
        validate_pass("\xf2\xaf\xbf\xbe", 0xafffe, __LINE__);
        validate_pass("\xf2\xaf\xbf\xbf", 0xaffff, __LINE__);
    }
}
