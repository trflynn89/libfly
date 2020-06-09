#include "fly/types/string/string.hpp"
#include "fly/types/string/string_exception.hpp"
#include "fly/types/string/string_literal.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

//==================================================================================================
TYPED_TEST(BasicStringTest, InvalidSequences)
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
TYPED_TEST(BasicStringTest, Utf8CharacterConversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
    };

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        EXPECT_NO_THROW(actual = StringClass::unescape_unicode_character(begin, end));
        EXPECT_EQ(actual, expected);
    };

    validate_fail(FLY_STR(char_type, "\\u"));
    validate_fail(FLY_STR(char_type, "\\u0"));
    validate_fail(FLY_STR(char_type, "\\u00"));
    validate_fail(FLY_STR(char_type, "\\u000"));
    validate_fail(FLY_STR(char_type, "\\u000z"));

    validate_pass(FLY_STR(char_type, "\\u0040"), FLY_STR(char_type, "\u0040"));
    validate_pass(FLY_STR(char_type, "\\u007A"), FLY_STR(char_type, "\u007A"));
    validate_pass(FLY_STR(char_type, "\\u007a"), FLY_STR(char_type, "\u007a"));
    validate_pass(FLY_STR(char_type, "\\u00c4"), FLY_STR(char_type, "\u00c4"));
    validate_pass(FLY_STR(char_type, "\\u00e4"), FLY_STR(char_type, "\u00e4"));
    validate_pass(FLY_STR(char_type, "\\u0298"), FLY_STR(char_type, "\u0298"));
    validate_pass(FLY_STR(char_type, "\\u0800"), FLY_STR(char_type, "\u0800"));
    validate_pass(FLY_STR(char_type, "\\uffff"), FLY_STR(char_type, "\uffff"));

    validate_fail(FLY_STR(char_type, "\\uDC00"));
    validate_fail(FLY_STR(char_type, "\\uDFFF"));
    validate_fail(FLY_STR(char_type, "\\uD800"));
    validate_fail(FLY_STR(char_type, "\\uDBFF"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf16CharacterConversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
    };

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        EXPECT_NO_THROW(actual = StringClass::unescape_unicode_character(begin, end));
        EXPECT_EQ(actual, expected);
    };

    validate_fail(FLY_STR(char_type, "\\uD800\\u"));
    validate_fail(FLY_STR(char_type, "\\uD800\\z"));
    validate_fail(FLY_STR(char_type, "\\uD800\\u0"));
    validate_fail(FLY_STR(char_type, "\\uD800\\u00"));
    validate_fail(FLY_STR(char_type, "\\uD800\\u000"));
    validate_fail(FLY_STR(char_type, "\\uD800\\u0000"));
    validate_fail(FLY_STR(char_type, "\\uD800\\u000z"));
    validate_fail(FLY_STR(char_type, "\\uD800\\uDBFF"));
    validate_fail(FLY_STR(char_type, "\\uD800\\uE000"));
    validate_fail(FLY_STR(char_type, "\\uD800\\uFFFF"));

    validate_pass(FLY_STR(char_type, "\\uD800\\uDC00"), FLY_STR(char_type, "\U00010000"));
    validate_pass(FLY_STR(char_type, "\\uD803\\uDE6D"), FLY_STR(char_type, "\U00010E6D"));
    validate_pass(FLY_STR(char_type, "\\uD834\\uDD1E"), FLY_STR(char_type, "\U0001D11E"));
    validate_pass(FLY_STR(char_type, "\\uDBFF\\uDFFF"), FLY_STR(char_type, "\U0010FFFF"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf32CharacterConversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](string_type &&test) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        EXPECT_THROW(StringClass::unescape_unicode_character(begin, end), fly::UnicodeException);
    };

    auto validate_pass = [](string_type &&test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        auto begin = test.cbegin();
        const auto end = test.cend();

        string_type actual;
        EXPECT_NO_THROW(actual = StringClass::unescape_unicode_character(begin, end));
        EXPECT_EQ(actual, expected);
    };

    validate_fail(FLY_STR(char_type, "\\U"));
    validate_fail(FLY_STR(char_type, "\\U0"));
    validate_fail(FLY_STR(char_type, "\\U00"));
    validate_fail(FLY_STR(char_type, "\\U000"));
    validate_fail(FLY_STR(char_type, "\\U0000"));
    validate_fail(FLY_STR(char_type, "\\U00000"));
    validate_fail(FLY_STR(char_type, "\\U000000"));
    validate_fail(FLY_STR(char_type, "\\U0000000"));
    validate_fail(FLY_STR(char_type, "\\U0000000z"));

    validate_pass(FLY_STR(char_type, "\\U00010000"), FLY_STR(char_type, "\U00010000"));
    validate_pass(FLY_STR(char_type, "\\U00010E6D"), FLY_STR(char_type, "\U00010E6D"));
    validate_pass(FLY_STR(char_type, "\\U0001D11E"), FLY_STR(char_type, "\U0001D11E"));
    validate_pass(FLY_STR(char_type, "\\U0010FFFF"), FLY_STR(char_type, "\U0010FFFF"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, UnicodeStringConversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](const string_type &test) {
        SCOPED_TRACE(test.c_str());

        EXPECT_THROW(StringClass::unescape_unicode_string(test), fly::UnicodeException);
    };

    auto validate_pass = [](const string_type &test, string_type &&expected) {
        SCOPED_TRACE(test.c_str());

        string_type actual;
        EXPECT_NO_THROW(actual = StringClass::unescape_unicode_string(test));
        EXPECT_EQ(actual, expected);
    };

    validate_fail(FLY_STR(char_type, "\\U"));
    validate_fail(FLY_STR(char_type, "\\U0"));
    validate_fail(FLY_STR(char_type, "\\U00"));
    validate_fail(FLY_STR(char_type, "\\U000"));
    validate_fail(FLY_STR(char_type, "\\U0000"));
    validate_fail(FLY_STR(char_type, "\\U00000"));
    validate_fail(FLY_STR(char_type, "\\U000000"));
    validate_fail(FLY_STR(char_type, "\\U0000000"));
    validate_fail(FLY_STR(char_type, "\\U0000000z"));
    validate_fail(FLY_STR(char_type, "text \\U0000000z text"));

    validate_pass(FLY_STR(char_type, "No unicode!"), FLY_STR(char_type, "No unicode!"));
    validate_pass(FLY_STR(char_type, "Other escape \t"), FLY_STR(char_type, "Other escape \t"));
    validate_pass(FLY_STR(char_type, "Other escape \\t"), FLY_STR(char_type, "Other escape \\t"));

    validate_pass(FLY_STR(char_type, "\\U00010000"), FLY_STR(char_type, "\U00010000"));
    validate_pass(FLY_STR(char_type, "\\U00010E6D"), FLY_STR(char_type, "\U00010E6D"));
    validate_pass(FLY_STR(char_type, "\\U0001D11E"), FLY_STR(char_type, "\U0001D11E"));
    validate_pass(FLY_STR(char_type, "\\U0010FFFF"), FLY_STR(char_type, "\U0010FFFF"));

    validate_pass(
        FLY_STR(char_type, "\\U0001f355 in the morning, \\U0001f355 in the evening"),
        FLY_STR(char_type, "\U0001f355 in the morning, \U0001f355 in the evening"));
}
