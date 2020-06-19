#include "fly/types/string/string.hpp"

#include "fly/types/numeric/literals.hpp"
#include "test/types/string_test.hpp"

#include <gtest/gtest.h>

#include <regex>
#include <vector>

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
TYPED_TEST(BasicStringTest, GenerateRandomString)
{
    DECLARE_ALIASES

    static constexpr size_type s_size = (1 << 10);

    const auto random = StringClass::generate_random_string(s_size);
    EXPECT_EQ(s_size, random.size());
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

        EXPECT_EQ("\\u00f0\\u0178\\u008d\\u2022", stream.str());
    }
}
