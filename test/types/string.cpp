#include "fly/types/string/string.hpp"

#include "fly/fly.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string_exception.hpp"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <regex>
#include <string>
#include <vector>

namespace {

//==================================================================================================
template <typename StringType>
class Streamable
{
public:
    using ostream_type = typename fly::BasicString<StringType>::ostream_type;

    Streamable(const StringType &str, int num) noexcept : m_str(str), m_num(num)
    {
    }

    StringType str() const noexcept
    {
        return m_str;
    };

    int num() const noexcept
    {
        return m_num;
    };

    friend ostream_type &operator<<(ostream_type &stream, const Streamable &obj)
    {
        stream << '[';
        stream << obj.str() << ' ' << std::hex << obj.num() << std::dec;
        stream << ']';

        return stream;
    }

private:
    StringType m_str;
    int m_num;
};

//==================================================================================================
class NotStreamable
{
};

//==================================================================================================
template <typename StringType, typename T>
StringType minstr() noexcept
{
    static constexpr std::intmax_t s_min = std::numeric_limits<T>::min();

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        return std::to_string(s_min - 1);
    }
    else if constexpr (std::is_same_v<StringType, std::wstring>)
    {
        return std::to_wstring(s_min - 1);
    }
}

//==================================================================================================
template <typename StringType, typename T>
StringType maxstr() noexcept
{
    static constexpr std::uintmax_t s_max = std::numeric_limits<T>::max();

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        return std::to_string(s_max + 1);
    }
    else if constexpr (std::is_same_v<StringType, std::wstring>)
    {
        return std::to_wstring(s_max + 1);
    }
}

} // namespace

//==================================================================================================
template <typename T>
struct BasicStringTest : public ::testing::Test
{
    using string_base_type = T;
};

using StringTypes = ::testing::Types<std::string, std::wstring, std::u16string, std::u32string>;

TYPED_TEST_SUITE(BasicStringTest, StringTypes, );

#define DECLARE_ALIASES                                                                            \
    using string_type [[maybe_unused]] = typename TestFixture::string_base_type;                   \
    using StringClass [[maybe_unused]] = fly::BasicString<string_type>;                            \
    using char_type [[maybe_unused]] = typename StringClass::char_type;                            \
    using size_type [[maybe_unused]] = typename StringClass::size_type;                            \
    using streamed_type [[maybe_unused]] = typename StringClass::streamed_type;                    \
    using streamed_char [[maybe_unused]] = typename streamed_type::value_type;                     \
    using ustreamed_char [[maybe_unused]] = std::make_unsigned_t<streamed_char>;

//==================================================================================================
TYPED_TEST(BasicStringTest, Split)
{
    DECLARE_ALIASES

    static constexpr std::uint32_t s_size = 10;
    std::vector<string_type> input_split(s_size);

    constexpr char_type delim = ' ';
    string_type input;

    for (std::uint32_t i = 0; i < s_size; ++i)
    {
        const string_type curr = StringClass::generate_random_string(10);

        input += curr + delim;
        input_split[i] = curr;
    }

    const auto output_split = StringClass::split(input, delim);
    EXPECT_EQ(input_split.size(), output_split.size());

    for (std::uint32_t i = 0; i < s_size; ++i)
    {
        EXPECT_EQ(input_split[i], output_split[i]);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, MaxSplit)
{
    DECLARE_ALIASES

    static constexpr std::uint32_t s_size = 10;
    static constexpr std::uint32_t s_count = 6;
    std::vector<string_type> input_split(s_count);

    constexpr char_type delim = ';';
    string_type input;

    for (std::uint32_t i = 0; i < s_size; ++i)
    {
        const string_type curr = StringClass::generate_random_string(10);
        input += curr + delim;

        if (i < s_count)
        {
            input_split[i] = curr;
        }
        else
        {
            input_split.back() += delim;
            input_split.back() += curr;
        }
    }

    const auto output_split = StringClass::split(input, delim, s_count);
    EXPECT_EQ(input_split.size(), output_split.size());

    for (std::uint32_t i = 0; i < s_count; ++i)
    {
        EXPECT_EQ(input_split[i], output_split[i]);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Trim)
{
    DECLARE_ALIASES

    string_type test1;
    string_type test2 = FLY_STR(char_type, "   abc");
    string_type test3 = FLY_STR(char_type, "abc   ");
    string_type test4 = FLY_STR(char_type, "   abc   ");
    string_type test5 = FLY_STR(char_type, " \n\t\r  abc  \n\t\r ");
    string_type test6 = FLY_STR(char_type, " \n\t\r  a   c  \n\t\r ");
    string_type test7 = FLY_STR(char_type, " \n\t\r  a\n \tc  \n\t\r ");

    const string_type expected1;
    const string_type expected2 = FLY_STR(char_type, "abc");
    const string_type expected3 = FLY_STR(char_type, "a   c");
    const string_type expected4 = FLY_STR(char_type, "a\n \tc");

    StringClass::trim(test1);
    StringClass::trim(test2);
    StringClass::trim(test3);
    StringClass::trim(test4);
    StringClass::trim(test5);
    StringClass::trim(test6);
    StringClass::trim(test7);

    EXPECT_EQ(test1, expected1);
    EXPECT_EQ(test2, expected2);
    EXPECT_EQ(test3, expected2);
    EXPECT_EQ(test4, expected2);
    EXPECT_EQ(test5, expected2);
    EXPECT_EQ(test6, expected3);
    EXPECT_EQ(test7, expected4);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ReplaceAll)
{
    DECLARE_ALIASES

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type search = FLY_STR(char_type, "Be Replaced");
    const string_type replace = FLY_STR(char_type, "new value");

    StringClass::replace_all(source, search, replace);
    EXPECT_EQ(source, FLY_STR(char_type, "To new value! To new value!"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithChar)
{
    DECLARE_ALIASES

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type search = FLY_STR(char_type, "Be Replaced");
    const char_type replace('x');

    StringClass::replace_all(source, search, replace);
    EXPECT_EQ(source, FLY_STR(char_type, "To x! To x!"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithEmpty)
{
    DECLARE_ALIASES

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type replace = FLY_STR(char_type, "new value");

    StringClass::replace_all(source, string_type(), replace);
    EXPECT_EQ(source, FLY_STR(char_type, "To Be Replaced! To Be Replaced!"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, RemoveAll)
{
    DECLARE_ALIASES

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type search = FLY_STR(char_type, "Be Rep");

    StringClass::remove_all(source, search);
    EXPECT_EQ(source, FLY_STR(char_type, "To laced! To laced!"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, RemoveAllWithEmpty)
{
    DECLARE_ALIASES

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");

    StringClass::remove_all(source, string_type());
    EXPECT_EQ(source, FLY_STR(char_type, "To Be Replaced! To Be Replaced!"));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, StartsWith)
{
    DECLARE_ALIASES

    string_type test1, test2;

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::starts_with(test1, 'a'));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a");
    EXPECT_TRUE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "ab");
    EXPECT_TRUE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "");
    EXPECT_FALSE(StringClass::starts_with(test1, 'a'));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "b");
    EXPECT_FALSE(StringClass::starts_with(test1, 'a'));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "ab");
    EXPECT_FALSE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "ab");
    test2 = FLY_STR(char_type, "abc");
    EXPECT_FALSE(StringClass::starts_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "abd");
    EXPECT_FALSE(StringClass::starts_with(test1, test2));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, EndsWith)
{
    DECLARE_ALIASES

    string_type test1, test2;

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::ends_with(test1, 'c'));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "c");
    EXPECT_TRUE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "bc");
    EXPECT_TRUE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "ba");
    EXPECT_FALSE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "ab");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::ends_with(test1, test2));

    test1 = FLY_STR(char_type, "ab");
    EXPECT_FALSE(StringClass::ends_with(test1, 'a'));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "dbc");
    EXPECT_FALSE(StringClass::ends_with(test1, test2));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Wildcard)
{
    DECLARE_ALIASES

    string_type test1, test2;

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "*");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "**");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "a");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "b");
    test2 = FLY_STR(char_type, "*");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "c");
    test2 = FLY_STR(char_type, "**");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a*");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a*c");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a*");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*b*");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*bc");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*c");
    EXPECT_TRUE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "b");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "b*");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "*b");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "b*");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*b");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*d*");
    EXPECT_FALSE(StringClass::wildcard_match(test1, test2));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, Utf8Conversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](const char_type *test) {
        SCOPED_TRACE(test);

        EXPECT_THROW(StringClass::parse_unicode_character(test), fly::UnicodeException);
    };

    auto validate_pass = [](const char_type *test, string_type &&expected) {
        SCOPED_TRACE(test);

        string_type actual;
        actual = StringClass::parse_unicode_character(test);
        EXPECT_EQ(actual, expected);
    };

    validate_fail(FLY_STR(char_type, "f"));
    validate_fail(FLY_STR(char_type, "\\f"));

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
TYPED_TEST(BasicStringTest, Utf16Conversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](const char_type *test) {
        SCOPED_TRACE(test);

        EXPECT_THROW(StringClass::parse_unicode_character(test), fly::UnicodeException);
    };

    auto validate_pass = [](const char_type *test, string_type &&expected) {
        SCOPED_TRACE(test);

        string_type actual;
        actual = StringClass::parse_unicode_character(test);
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
TYPED_TEST(BasicStringTest, Utf32Conversion)
{
    DECLARE_ALIASES

    auto validate_fail = [](const char_type *test) {
        SCOPED_TRACE(test);

        EXPECT_THROW(StringClass::parse_unicode_character(test), fly::UnicodeException);
    };

    auto validate_pass = [](const char_type *test, string_type &&expected) {
        SCOPED_TRACE(test);

        string_type actual;
        actual = StringClass::parse_unicode_character(test);
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
TYPED_TEST(BasicStringTest, GenerateRandomString)
{
    DECLARE_ALIASES

    static constexpr size_type s_size = (1 << 10);

    const auto random = StringClass::generate_random_string(s_size);
    EXPECT_EQ(s_size, random.size());
}

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
    // Windows 0-pads std::hexfloat to match the stream precision, Linux does not. This discrepency
    // should be fixed when length modifiers are added.
    EXPECT_EQ(FLY_STR(streamed_char, "0x1.600000p+2"), StringClass::format(format, 5.5));
#else
    EXPECT_EQ(FLY_STR(streamed_char, "0x1.6p+2"), StringClass::format(format, 5.5));
#endif

    format = FLY_STR(char_type, "%A");
    EXPECT_EQ(FLY_STR(streamed_char, "%A"), StringClass::format(format));
#if defined(FLY_WINDOWS)
    // Windows 0-pads std::hexfloat to match the stream precision, Linux does not. This discrepency
    // should be fixed when length modifiers are added.
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
TYPED_TEST(BasicStringTest, Join)
{
    DECLARE_ALIASES

    string_type str = FLY_STR(char_type, "a");
    const char_type *ctr = FLY_STR(char_type, "b");
    const char_type arr[] = {'c', '\0'};
    const char_type chr = 'd';

    const Streamable<streamed_type> obj1(FLY_STR(streamed_char, "hi"), 0xbeef);
    const NotStreamable obj2;

    EXPECT_EQ(FLY_STR(streamed_char, "a"), StringClass::join('.', str));
    EXPECT_EQ(FLY_STR(streamed_char, "b"), StringClass::join('.', ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "c"), StringClass::join('.', arr));
    EXPECT_EQ(FLY_STR(streamed_char, "d"), StringClass::join('.', chr));
    EXPECT_EQ(FLY_STR(streamed_char, "a,a"), StringClass::join(',', str, str));
    EXPECT_EQ(FLY_STR(streamed_char, "a,b"), StringClass::join(',', str, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "a,c"), StringClass::join(',', str, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "a,d"), StringClass::join(',', str, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "b,a"), StringClass::join(',', ctr, str));
    EXPECT_EQ(FLY_STR(streamed_char, "b,b"), StringClass::join(',', ctr, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "b,c"), StringClass::join(',', ctr, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "b,d"), StringClass::join(',', ctr, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "c,a"), StringClass::join(',', arr, str));
    EXPECT_EQ(FLY_STR(streamed_char, "c,b"), StringClass::join(',', arr, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "c,c"), StringClass::join(',', arr, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "c,d"), StringClass::join(',', arr, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "d,a"), StringClass::join(',', chr, str));
    EXPECT_EQ(FLY_STR(streamed_char, "d,b"), StringClass::join(',', chr, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "d,c"), StringClass::join(',', chr, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "d,d"), StringClass::join(',', chr, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "[hi beef]"), StringClass::join('.', obj1));
    EXPECT_EQ(
        FLY_STR(streamed_char, "a:[hi beef]:c:d"),
        StringClass::join(':', str, obj1, arr, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "a:c:d"), StringClass::join(':', str, arr, chr));

    std::basic_regex<streamed_char> test(
        FLY_STR(streamed_char, "\\[(0x)?[0-9a-fA-F]+\\]:2:\\[hi beef\\]"));
    EXPECT_TRUE(std::regex_match(StringClass::join(':', obj2, 2, obj1), test));
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertString)
{
    DECLARE_ALIASES

    string_type s = FLY_STR(char_type, "abc");
    EXPECT_EQ(StringClass::template convert<string_type>(s), s);

    const char_type *c = FLY_STR(char_type, "def");
    EXPECT_EQ(StringClass::template convert<string_type>(c), c);

    char_type *d = const_cast<char_type *>(FLY_STR(char_type, "ghi"));
    EXPECT_EQ(StringClass::template convert<string_type>(d), d);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertBool)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template convert<bool>(s), false);

    s = FLY_STR(char_type, "1");
    EXPECT_EQ(StringClass::template convert<bool>(s), true);

    s = FLY_STR(char_type, "-1");
    EXPECT_THROW(StringClass::template convert<bool>(s), std::out_of_range);

    s = FLY_STR(char_type, "2");
    EXPECT_THROW(StringClass::template convert<bool>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<bool>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<bool>(s), std::invalid_argument);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertChar)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template convert<streamed_char>(s), '\0');
    EXPECT_EQ(StringClass::template convert<ustreamed_char>(s), '\0');

    s = FLY_STR(char_type, "65");
    EXPECT_EQ(StringClass::template convert<streamed_char>(s), 'A');
    EXPECT_EQ(StringClass::template convert<ustreamed_char>(s), static_cast<ustreamed_char>(65));

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<streamed_char>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<ustreamed_char>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<streamed_char>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<ustreamed_char>(s), std::invalid_argument);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template convert<streamed_char>(minstr<string_type, streamed_char>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<streamed_char>(maxstr<string_type, streamed_char>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template convert<ustreamed_char>(minstr<string_type, ustreamed_char>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<ustreamed_char>(maxstr<string_type, ustreamed_char>()),
            std::out_of_range);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt8)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template convert<std::int8_t>(s), 0_i8);
    EXPECT_EQ(StringClass::template convert<std::uint8_t>(s), 0_u8);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(StringClass::template convert<std::int8_t>(s), 100_i8);
    EXPECT_EQ(StringClass::template convert<std::uint8_t>(s), 100_u8);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(StringClass::template convert<std::int8_t>(s), -100_i8);
    EXPECT_THROW(StringClass::template convert<std::uint8_t>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint8_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint8_t>(s), std::invalid_argument);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template convert<std::int8_t>(minstr<string_type, std::int8_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<std::int8_t>(maxstr<string_type, std::int8_t>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template convert<std::uint8_t>(minstr<string_type, std::uint8_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<std::uint8_t>(maxstr<string_type, std::uint8_t>()),
            std::out_of_range);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt16)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template convert<std::int16_t>(s), 0_i16);
    EXPECT_EQ(StringClass::template convert<std::uint16_t>(s), 0_u16);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(StringClass::template convert<std::int16_t>(s), 100_i16);
    EXPECT_EQ(StringClass::template convert<std::uint16_t>(s), 100_u16);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(StringClass::template convert<std::int16_t>(s), -100_i16);
    EXPECT_THROW(StringClass::template convert<std::uint16_t>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint16_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint16_t>(s), std::invalid_argument);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template convert<std::int16_t>(minstr<string_type, std::int16_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<std::int16_t>(maxstr<string_type, std::int16_t>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template convert<std::uint16_t>(minstr<string_type, std::uint16_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<std::uint16_t>(maxstr<string_type, std::uint16_t>()),
            std::out_of_range);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt32)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template convert<std::int32_t>(s), 0_i32);
    EXPECT_EQ(StringClass::template convert<std::uint32_t>(s), 0_u32);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(StringClass::template convert<std::int32_t>(s), 100_i32);
    EXPECT_EQ(StringClass::template convert<std::uint32_t>(s), 100_u32);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(StringClass::template convert<std::int32_t>(s), -100_i32);
    EXPECT_THROW(StringClass::template convert<std::uint32_t>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint32_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint32_t>(s), std::invalid_argument);

    if constexpr (StringClass::traits::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template convert<std::int32_t>(minstr<string_type, std::int32_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<std::int32_t>(maxstr<string_type, std::int32_t>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template convert<std::uint32_t>(minstr<string_type, std::uint32_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template convert<std::uint32_t>(maxstr<string_type, std::uint32_t>()),
            std::out_of_range);
    }
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertInt64)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template convert<std::int64_t>(s), 0_i64);
    EXPECT_EQ(StringClass::template convert<std::uint64_t>(s), 0_u64);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(StringClass::template convert<std::int64_t>(s), 100_i64);
    EXPECT_EQ(StringClass::template convert<std::uint64_t>(s), 100_u64);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(StringClass::template convert<std::int64_t>(s), -100_i64);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint64_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<std::uint64_t>(s), std::invalid_argument);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, ConvertDecimal)
{
    DECLARE_ALIASES

    string_type s;

    s = FLY_STR(char_type, "-400.123");
    EXPECT_EQ(StringClass::template convert<float>(s), -400.123f);
    EXPECT_EQ(StringClass::template convert<double>(s), -400.123);
    EXPECT_EQ(StringClass::template convert<long double>(s), -400.123L);

    s = FLY_STR(char_type, "400.456");
    EXPECT_EQ(StringClass::template convert<float>(s), 400.456f);
    EXPECT_EQ(StringClass::template convert<double>(s), 400.456);
    EXPECT_EQ(StringClass::template convert<long double>(s), 400.456L);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template convert<float>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<double>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<long double>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template convert<float>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<double>(s), std::invalid_argument);
    EXPECT_THROW(StringClass::template convert<long double>(s), std::invalid_argument);
}

//==================================================================================================
TYPED_TEST(BasicStringTest, BasicStringStreamer)
{
    DECLARE_ALIASES

    // Extra test to make sure the hexadecimal conversion for std::u16string and std::u32string in
    // detail::BasicStringStreamer is exercised correctly.
    if constexpr (!StringClass::traits::has_stoi_family_v)
    {
        const string_type s = FLY_STR(char_type, "\u00f0\u0178\u008d\u2022");

        typename StringClass::traits::ostringstream_type stream;
        fly::detail::BasicStringStreamer<string_type>::stream(stream, s);

        EXPECT_EQ("[0xf0][0x178][0x8d][0x2022]", stream.str());
    }
}
