#include "fly/types/string.h"

#include <gtest/gtest.h>

#include <regex>
#include <string>
#include <vector>

namespace {

//==============================================================================
template <typename StringType>
class Streamable
{
public:
    using ostream_type = typename fly::BasicStringTraits<
        StringType>::streamer_type::ostream_type;

    Streamable(const StringType &str, int num) noexcept : m_str(str), m_num(num)
    {
    }

    StringType GetStr() const noexcept
    {
        return m_str;
    };

    int GetNum() const noexcept
    {
        return m_num;
    };

    friend ostream_type &operator<<(ostream_type &stream, const Streamable &obj)
    {
        stream << '[';
        stream << obj.GetStr() << ' ' << std::hex << obj.GetNum() << std::dec;
        stream << ']';

        return stream;
    }

private:
    StringType m_str;
    int m_num;
};

//==============================================================================
class NotStreamable
{
};

//==============================================================================
template <typename StringType, typename T>
StringType minstr() noexcept
{
    static constexpr std::intmax_t min = std::numeric_limits<T>::min();

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        return std::to_string(min - 1);
    }
    else if constexpr (std::is_same_v<StringType, std::wstring>)
    {
        return std::to_wstring(min - 1);
    }
}

//==============================================================================
template <typename StringType, typename T>
StringType maxstr() noexcept
{
    static constexpr std::uintmax_t max = std::numeric_limits<T>::max();

    if constexpr (std::is_same_v<StringType, std::string>)
    {
        return std::to_string(max + 1);
    }
    else if constexpr (std::is_same_v<StringType, std::wstring>)
    {
        return std::to_wstring(max + 1);
    }
}

} // namespace

//==============================================================================
template <typename T>
struct BasicStringTest : public ::testing::Test
{
    using string_type = T;
};

using StringTypes =
    ::testing::Types<std::string, std::wstring, std::u16string, std::u32string>;

TYPED_TEST_CASE(BasicStringTest, StringTypes);

//==============================================================================
TYPED_TEST(BasicStringTest, SplitTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    static constexpr std::uint32_t numSectors = 10;
    std::vector<string_type> inputSplit(numSectors);

    constexpr char_type delim = ' ';
    string_type input;

    for (std::uint32_t i = 0; i < numSectors; ++i)
    {
        const string_type curr = StringClass::GenerateRandomString(10);

        input += curr + delim;
        inputSplit[i] = curr;
    }

    const auto outputSplit = StringClass::Split(input, delim);
    EXPECT_EQ(inputSplit.size(), outputSplit.size());

    for (std::uint32_t i = 0; i < numSectors; ++i)
    {
        EXPECT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, MaxSplitTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    static constexpr std::uint32_t numSectors = 10;
    static constexpr std::uint32_t maxSectors = 6;
    std::vector<string_type> inputSplit(maxSectors);

    constexpr char_type delim = ';';
    string_type input;

    for (std::uint32_t i = 0; i < numSectors; ++i)
    {
        const string_type curr = StringClass::GenerateRandomString(10);
        input += curr + delim;

        if (i < maxSectors)
        {
            inputSplit[i] = curr;
        }
        else
        {
            inputSplit.back() += delim;
            inputSplit.back() += curr;
        }
    }

    const auto outputSplit = StringClass::Split(input, delim, maxSectors);
    EXPECT_EQ(inputSplit.size(), outputSplit.size());

    for (std::uint32_t i = 0; i < maxSectors; ++i)
    {
        EXPECT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, TrimTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

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

    StringClass::Trim(test1);
    StringClass::Trim(test2);
    StringClass::Trim(test3);
    StringClass::Trim(test4);
    StringClass::Trim(test5);
    StringClass::Trim(test6);
    StringClass::Trim(test7);

    EXPECT_EQ(test1, expected1);
    EXPECT_EQ(test2, expected2);
    EXPECT_EQ(test3, expected2);
    EXPECT_EQ(test4, expected2);
    EXPECT_EQ(test5, expected2);
    EXPECT_EQ(test6, expected3);
    EXPECT_EQ(test7, expected4);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type search = FLY_STR(char_type, "Be Replaced");
    const string_type replace = FLY_STR(char_type, "new value");

    StringClass::ReplaceAll(source, search, replace);
    EXPECT_EQ(source, FLY_STR(char_type, "To new value! To new value!"));
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithCharTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type search = FLY_STR(char_type, "Be Replaced");
    const char_type replace('x');

    StringClass::ReplaceAll(source, search, replace);
    EXPECT_EQ(source, FLY_STR(char_type, "To x! To x!"));
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithEmptyTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type replace = FLY_STR(char_type, "new value");

    StringClass::ReplaceAll(source, string_type(), replace);
    EXPECT_EQ(source, FLY_STR(char_type, "To Be Replaced! To Be Replaced!"));
}

//==============================================================================
TYPED_TEST(BasicStringTest, RemoveAllTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");
    const string_type search = FLY_STR(char_type, "Be Rep");

    StringClass::RemoveAll(source, search);
    EXPECT_EQ(source, FLY_STR(char_type, "To laced! To laced!"));
}

//==============================================================================
TYPED_TEST(BasicStringTest, RemoveAllWithEmptyTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type source = FLY_STR(char_type, "To Be Replaced! To Be Replaced!");

    StringClass::RemoveAll(source, string_type());
    EXPECT_EQ(source, FLY_STR(char_type, "To Be Replaced! To Be Replaced!"));
}

//==============================================================================
TYPED_TEST(BasicStringTest, StartsWithTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type test1, test2;

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::StartsWith(test1, 'a'));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "ab");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "");
    EXPECT_FALSE(StringClass::StartsWith(test1, 'a'));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "b");
    EXPECT_FALSE(StringClass::StartsWith(test1, 'a'));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "ab");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "ab");
    test2 = FLY_STR(char_type, "abc");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "abd");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, EndsWithTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type test1, test2;

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::EndsWith(test1, 'c'));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "c");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "bc");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "abc");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "ba");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "ab");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));

    test1 = FLY_STR(char_type, "ab");
    EXPECT_FALSE(StringClass::EndsWith(test1, 'a'));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "dbc");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, WildcardTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type test1, test2;

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "**");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "a");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "b");
    test2 = FLY_STR(char_type, "*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "c");
    test2 = FLY_STR(char_type, "**");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a*c");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*b*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*bc");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*c");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "");
    test2 = FLY_STR(char_type, "");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "b");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "b*");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "a");
    test2 = FLY_STR(char_type, "*b");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "a");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "b*");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*b");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    test1 = FLY_STR(char_type, "abc");
    test2 = FLY_STR(char_type, "*d*");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, GenerateRandomStringTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using size_type = typename fly::BasicStringTraits<string_type>::size_type;

    static constexpr size_type length = (1 << 10);

    const auto random = StringClass::GenerateRandomString(length);
    EXPECT_EQ(length, random.length());
}

//==============================================================================
TYPED_TEST(BasicStringTest, FormatTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    using streamed_type = typename traits::streamer_type::streamed_type;
    using streamed_char = typename streamed_type::value_type;

    streamed_type expected;
    const char_type *format;
    string_type arg;

    EXPECT_EQ(streamed_type(), StringClass::Format(FLY_STR(char_type, "")));

    expected = FLY_STR(streamed_char, "%");
    format = FLY_STR(char_type, "%");
    EXPECT_EQ(expected, StringClass::Format(format));
    EXPECT_EQ(expected, StringClass::Format(format, 1));

    expected = FLY_STR(streamed_char, "%%");
    format = FLY_STR(char_type, "%%");
    EXPECT_EQ(expected, StringClass::Format(format));

    format = FLY_STR(char_type, "%d");
    EXPECT_EQ(FLY_STR(streamed_char, "%d"), StringClass::Format(format));
    EXPECT_EQ(FLY_STR(streamed_char, "1"), StringClass::Format(format, 1));

    expected = FLY_STR(streamed_char, "This is a test");
    format = FLY_STR(char_type, "This is a test");
    EXPECT_EQ(expected, StringClass::Format(format));

    expected = FLY_STR(streamed_char, "there are no formatters");
    format = FLY_STR(char_type, "there are no formatters");
    EXPECT_EQ(expected, StringClass::Format(format, 1, 2, 3, 4));

    expected = FLY_STR(streamed_char, "test some string s");
    format = FLY_STR(char_type, "test %s %c");
    arg = FLY_STR(char_type, "some string");
    EXPECT_EQ(expected, StringClass::Format(format, arg, 's'));

    expected =
        FLY_STR(streamed_char, "test 1 true 2.100000 false 1.230000e+02 0xff");
    format = FLY_STR(char_type, "test %d %d %f %d %e %x");
    EXPECT_EQ(
        expected,
        StringClass::Format(format, 1, true, 2.1f, false, 123.0, 255));
}

//==============================================================================
TYPED_TEST(BasicStringTest, JoinTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    using streamed_type = typename traits::streamer_type::streamed_type;
    using streamed_char = typename streamed_type::value_type;

    string_type str = FLY_STR(char_type, "a");
    const char_type *ctr = FLY_STR(char_type, "b");
    const char_type arr[] = {'c', '\0'};
    const char_type chr = 'd';

    const Streamable<streamed_type> obj1(FLY_STR(streamed_char, "hi"), 0xbeef);
    const NotStreamable obj2;

    EXPECT_EQ(FLY_STR(streamed_char, "a"), StringClass::Join('.', str));
    EXPECT_EQ(FLY_STR(streamed_char, "b"), StringClass::Join('.', ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "c"), StringClass::Join('.', arr));
    EXPECT_EQ(FLY_STR(streamed_char, "d"), StringClass::Join('.', chr));
    EXPECT_EQ(FLY_STR(streamed_char, "a,a"), StringClass::Join(',', str, str));
    EXPECT_EQ(FLY_STR(streamed_char, "a,b"), StringClass::Join(',', str, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "a,c"), StringClass::Join(',', str, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "a,d"), StringClass::Join(',', str, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "b,a"), StringClass::Join(',', ctr, str));
    EXPECT_EQ(FLY_STR(streamed_char, "b,b"), StringClass::Join(',', ctr, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "b,c"), StringClass::Join(',', ctr, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "b,d"), StringClass::Join(',', ctr, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "c,a"), StringClass::Join(',', arr, str));
    EXPECT_EQ(FLY_STR(streamed_char, "c,b"), StringClass::Join(',', arr, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "c,c"), StringClass::Join(',', arr, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "c,d"), StringClass::Join(',', arr, chr));
    EXPECT_EQ(FLY_STR(streamed_char, "d,a"), StringClass::Join(',', chr, str));
    EXPECT_EQ(FLY_STR(streamed_char, "d,b"), StringClass::Join(',', chr, ctr));
    EXPECT_EQ(FLY_STR(streamed_char, "d,c"), StringClass::Join(',', chr, arr));
    EXPECT_EQ(FLY_STR(streamed_char, "d,d"), StringClass::Join(',', chr, chr));
    EXPECT_EQ(
        FLY_STR(streamed_char, "[hi beef]"), StringClass::Join('.', obj1));
    EXPECT_EQ(
        FLY_STR(streamed_char, "a:[hi beef]:c:d"),
        StringClass::Join(':', str, obj1, arr, chr));
    EXPECT_EQ(
        FLY_STR(streamed_char, "a:c:d"), StringClass::Join(':', str, arr, chr));

    std::basic_regex<streamed_char> test(
        FLY_STR(streamed_char, "\\[0x[0-9a-fA-F]+\\]:2:\\[hi beef\\]"));
    EXPECT_TRUE(std::regex_match(StringClass::Join(':', obj2, 2, obj1), test));
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertStringTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s = FLY_STR(char_type, "abc");
    EXPECT_EQ(StringClass::template Convert<string_type>(s), s);

    const char_type *c = FLY_STR(char_type, "def");
    EXPECT_EQ(StringClass::template Convert<string_type>(c), c);

    char_type *d = (char_type *)FLY_STR(char_type, "ghi");
    EXPECT_EQ(StringClass::template Convert<string_type>(d), d);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertBoolTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template Convert<bool>(s), false);

    s = FLY_STR(char_type, "1");
    EXPECT_EQ(StringClass::template Convert<bool>(s), true);

    s = FLY_STR(char_type, "-1");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::out_of_range);

    s = FLY_STR(char_type, "2");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::invalid_argument);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertCharTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    using streamed_type = typename traits::streamer_type::streamed_type;
    using streamed_char = typename streamed_type::value_type;
    using ustreamed_char = std::make_unsigned_t<streamed_char>;

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template Convert<streamed_char>(s), '\0');
    EXPECT_EQ(StringClass::template Convert<ustreamed_char>(s), '\0');

    s = FLY_STR(char_type, "65");
    EXPECT_EQ(StringClass::template Convert<streamed_char>(s), 'A');
    EXPECT_EQ(
        StringClass::template Convert<ustreamed_char>(s), (ustreamed_char)65);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(
        StringClass::template Convert<streamed_char>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<ustreamed_char>(s),
        std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(
        StringClass::template Convert<streamed_char>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<ustreamed_char>(s),
        std::invalid_argument);

    if constexpr (fly::BasicStringTraits<string_type>::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template Convert<streamed_char>(
                minstr<string_type, streamed_char>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<streamed_char>(
                maxstr<string_type, streamed_char>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template Convert<ustreamed_char>(
                minstr<string_type, ustreamed_char>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<ustreamed_char>(
                maxstr<string_type, ustreamed_char>()),
            std::out_of_range);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt8Test)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template Convert<std::int8_t>(s), (std::int8_t)0);
    EXPECT_EQ(StringClass::template Convert<std::uint8_t>(s), (std::uint8_t)0);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(StringClass::template Convert<std::int8_t>(s), (std::int8_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint8_t>(s), (std::uint8_t)100);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(StringClass::template Convert<std::int8_t>(s), (std::int8_t)-100);
    EXPECT_THROW(
        StringClass::template Convert<std::uint8_t>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint8_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint8_t>(s), std::invalid_argument);

    if constexpr (fly::BasicStringTraits<string_type>::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template Convert<std::int8_t>(
                minstr<string_type, std::int8_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<std::int8_t>(
                maxstr<string_type, std::int8_t>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template Convert<std::uint8_t>(
                minstr<string_type, std::uint8_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<std::uint8_t>(
                maxstr<string_type, std::uint8_t>()),
            std::out_of_range);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt16Test)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template Convert<std::int16_t>(s), (std::int16_t)0);
    EXPECT_EQ(
        StringClass::template Convert<std::uint16_t>(s), (std::uint16_t)0);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(
        StringClass::template Convert<std::int16_t>(s), (std::int16_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint16_t>(s), (std::uint16_t)100);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(
        StringClass::template Convert<std::int16_t>(s), (std::int16_t)-100);
    EXPECT_THROW(
        StringClass::template Convert<std::uint16_t>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint16_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint16_t>(s), std::invalid_argument);

    if constexpr (fly::BasicStringTraits<string_type>::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template Convert<std::int16_t>(
                minstr<string_type, std::int16_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<std::int16_t>(
                maxstr<string_type, std::int16_t>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template Convert<std::uint16_t>(
                minstr<string_type, std::uint16_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<std::uint16_t>(
                maxstr<string_type, std::uint16_t>()),
            std::out_of_range);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt32Test)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template Convert<std::int32_t>(s), (std::int32_t)0);
    EXPECT_EQ(
        StringClass::template Convert<std::uint32_t>(s), (std::uint32_t)0);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(
        StringClass::template Convert<std::int32_t>(s), (std::int32_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint32_t>(s), (std::uint32_t)100);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(
        StringClass::template Convert<std::int32_t>(s), (std::int32_t)-100);
    EXPECT_THROW(
        StringClass::template Convert<std::uint32_t>(s), std::out_of_range);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint32_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint32_t>(s), std::invalid_argument);

    if constexpr (fly::BasicStringTraits<string_type>::has_stoi_family_v)
    {
        EXPECT_THROW(
            StringClass::template Convert<std::int32_t>(
                minstr<string_type, std::int32_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<std::int32_t>(
                maxstr<string_type, std::int32_t>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template Convert<std::uint32_t>(
                minstr<string_type, std::uint32_t>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<std::uint32_t>(
                maxstr<string_type, std::uint32_t>()),
            std::out_of_range);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt64Test)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s;

    s = FLY_STR(char_type, "0");
    EXPECT_EQ(StringClass::template Convert<std::int64_t>(s), (std::int64_t)0);
    EXPECT_EQ(
        StringClass::template Convert<std::uint64_t>(s), (std::uint64_t)0);

    s = FLY_STR(char_type, "100");
    EXPECT_EQ(
        StringClass::template Convert<std::int64_t>(s), (std::int64_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint64_t>(s), (std::uint64_t)100);

    s = FLY_STR(char_type, "-100");
    EXPECT_EQ(
        StringClass::template Convert<std::int64_t>(s), (std::int64_t)-100);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint64_t>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint64_t>(s), std::invalid_argument);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertDecimalTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    string_type s;

    s = FLY_STR(char_type, "-400.123");
    EXPECT_EQ(StringClass::template Convert<float>(s), -400.123f);
    EXPECT_EQ(StringClass::template Convert<double>(s), -400.123);
    EXPECT_EQ(StringClass::template Convert<long double>(s), -400.123L);

    s = FLY_STR(char_type, "400.456");
    EXPECT_EQ(StringClass::template Convert<float>(s), 400.456f);
    EXPECT_EQ(StringClass::template Convert<double>(s), 400.456);
    EXPECT_EQ(StringClass::template Convert<long double>(s), 400.456L);

    s = FLY_STR(char_type, "abc");
    EXPECT_THROW(
        StringClass::template Convert<float>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<double>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<long double>(s), std::invalid_argument);

    s = FLY_STR(char_type, "2a");
    EXPECT_THROW(
        StringClass::template Convert<float>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<double>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<long double>(s), std::invalid_argument);
}

//==============================================================================
TYPED_TEST(BasicStringTest, BasicStringStreamerTest)
{
    using string_type = typename TestFixture::string_type;
    using traits = typename fly::BasicStringTraits<string_type>;
    using char_type = typename traits::char_type;

    // Extra test to make sure the hexadecimal conversion for std::u16string and
    // std::u32string in BasicStringStreamer is exercised correctly.
    if constexpr (!fly::BasicStringTraits<string_type>::has_stoi_family_v)
    {
        string_type s = FLY_STR(char_type, "\u00f0\u0178\u008d\u2022");

        typename fly::BasicStringTraits<string_type>::ostringstream_type stream;
        fly::BasicStringStreamer<string_type>::Stream(stream, s);

        EXPECT_EQ("[0xf0][0x178][0x8d][0x2022]", stream.str());
    }
}
