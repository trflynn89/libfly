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
    using char_type = typename StringType::value_type;
    using ostream_type = std::basic_ostream<char_type>;

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

#define ASSIGN_TEST_STRING(Type, var, raw)                                     \
    if constexpr (std::is_same_v<Type, std::string>)                           \
    {                                                                          \
        var = raw;                                                             \
    }                                                                          \
    else if constexpr (std::is_same_v<Type, std::wstring>)                     \
    {                                                                          \
        var = L##raw;                                                          \
    }                                                                          \
    else if constexpr (std::is_same_v<Type, std::u16string>)                   \
    {                                                                          \
        var = u##raw;                                                          \
    }                                                                          \
    else if constexpr (std::is_same_v<Type, std::u32string>)                   \
    {                                                                          \
        var = U##raw;                                                          \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        ASSERT_TRUE(false);                                                    \
    }

//==============================================================================
TYPED_TEST(BasicStringTest, SplitTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using char_type = typename StringClass::char_type;

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
    ASSERT_EQ(inputSplit.size(), outputSplit.size());

    for (std::uint32_t i = 0; i < numSectors; ++i)
    {
        ASSERT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, MaxSplitTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using char_type = typename StringClass::char_type;

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
    ASSERT_EQ(inputSplit.size(), outputSplit.size());

    for (std::uint32_t i = 0; i < maxSectors; ++i)
    {
        ASSERT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, TrimTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;

    string_type test1, test2, test3, test4, test5, test6, test7;
    string_type expt1, expt2, expt3, expt4;

    ASSIGN_TEST_STRING(string_type, test2, "   abc");
    ASSIGN_TEST_STRING(string_type, test3, "abc   ");
    ASSIGN_TEST_STRING(string_type, test4, "   abc   ");
    ASSIGN_TEST_STRING(string_type, test5, " \n\t\r  abc  \n\t\r ");
    ASSIGN_TEST_STRING(string_type, test6, " \n\t\r  a   c  \n\t\r ");
    ASSIGN_TEST_STRING(string_type, test7, " \n\t\r  a\n \tc  \n\t\r ");

    ASSIGN_TEST_STRING(string_type, expt2, "abc");
    ASSIGN_TEST_STRING(string_type, expt3, "a   c");
    ASSIGN_TEST_STRING(string_type, expt4, "a\n \tc");

    StringClass::Trim(test1);
    StringClass::Trim(test2);
    StringClass::Trim(test3);
    StringClass::Trim(test4);
    StringClass::Trim(test5);
    StringClass::Trim(test6);
    StringClass::Trim(test7);

    EXPECT_EQ(test1, expt1);
    EXPECT_EQ(test2, expt2);
    EXPECT_EQ(test3, expt2);
    EXPECT_EQ(test4, expt2);
    EXPECT_EQ(test5, expt2);
    EXPECT_EQ(test6, expt3);
    EXPECT_EQ(test7, expt4);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;

    string_type source, search, replace, result;
    ASSIGN_TEST_STRING(string_type, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(string_type, search, "Be Replaced");
    ASSIGN_TEST_STRING(string_type, replace, "new value");
    ASSIGN_TEST_STRING(string_type, result, "To new value! To new value!");

    StringClass::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithCharTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using char_type = typename StringClass::char_type;

    string_type source, search, result;
    ASSIGN_TEST_STRING(string_type, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(string_type, search, "Be Replaced");
    char_type replace('x');
    ASSIGN_TEST_STRING(string_type, result, "To x! To x!");

    StringClass::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithEmptyTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;

    string_type source, search, replace, result;
    ASSIGN_TEST_STRING(string_type, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(string_type, replace, "new value");
    ASSIGN_TEST_STRING(string_type, result, "To Be Replaced! To Be Replaced!");

    StringClass::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, RemoveAllTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;

    string_type source, search, result;
    ASSIGN_TEST_STRING(string_type, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(string_type, search, "Be Rep");
    ASSIGN_TEST_STRING(string_type, result, "To laced! To laced!");

    StringClass::RemoveAll(source, search);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, RemoveAllWithEmptyTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;

    string_type source, search, result;
    ASSIGN_TEST_STRING(string_type, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(string_type, result, "To Be Replaced! To Be Replaced!");

    StringClass::RemoveAll(source, search);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, StartsWithTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type test1, test2;

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    EXPECT_TRUE(StringClass::StartsWith(test1, 'a'));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "a");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "ab");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "abc");
    EXPECT_TRUE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "");
    EXPECT_FALSE(StringClass::StartsWith(test1, 'a'));

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "a");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "b");
    EXPECT_FALSE(StringClass::StartsWith(test1, 'a'));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "ab");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "ab");
    ASSIGN_TEST_STRING(string_type, test2, "abc");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "abd");
    EXPECT_FALSE(StringClass::StartsWith(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, EndsWithTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type test1, test2;

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    EXPECT_TRUE(StringClass::EndsWith(test1, 'c'));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "c");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "bc");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "abc");
    EXPECT_TRUE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "a");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "ba");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "ab");
    ASSIGN_TEST_STRING(string_type, test2, "a");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "ab");
    EXPECT_FALSE(StringClass::EndsWith(test1, 'a'));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "dbc");
    EXPECT_FALSE(StringClass::EndsWith(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, WildcardTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type test1, test2;

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "**");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "a");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "b");
    ASSIGN_TEST_STRING(string_type, test2, "*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "c");
    ASSIGN_TEST_STRING(string_type, test2, "**");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "a*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "a*c");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "a*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "*b*");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "*bc");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "*c");
    EXPECT_TRUE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "");
    ASSIGN_TEST_STRING(string_type, test2, "");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "b");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "b*");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "a");
    ASSIGN_TEST_STRING(string_type, test2, "*b");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "a");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "b*");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "*b");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(string_type, test1, "abc");
    ASSIGN_TEST_STRING(string_type, test2, "*d*");
    EXPECT_FALSE(StringClass::WildcardMatch(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, GenerateRandomStringTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using size_type = typename StringClass::size_type;

    static constexpr size_type length = (1 << 10);

    const auto random = StringClass::GenerateRandomString(length);
    ASSERT_EQ(length, random.length());
}

//==============================================================================
TYPED_TEST(BasicStringTest, FormatTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using char_type = typename StringClass::char_type;
    using streamed_type = typename StringClass::streamed_type;

    streamed_type expected;
    const char_type *format;
    string_type arg;

    ASSIGN_TEST_STRING(streamed_type, expected, "");
    ASSIGN_TEST_STRING(string_type, format, "");
    EXPECT_EQ(expected, StringClass::Format(format));

    ASSIGN_TEST_STRING(streamed_type, expected, "%");
    ASSIGN_TEST_STRING(string_type, format, "%");
    EXPECT_EQ(expected, StringClass::Format(format));
    EXPECT_EQ(expected, StringClass::Format(format, 1));

    ASSIGN_TEST_STRING(streamed_type, expected, "%%");
    ASSIGN_TEST_STRING(string_type, format, "%%");
    EXPECT_EQ(expected, StringClass::Format(format));

    ASSIGN_TEST_STRING(streamed_type, expected, "%d");
    ASSIGN_TEST_STRING(string_type, format, "%d");
    EXPECT_EQ(expected, StringClass::Format(format));

    ASSIGN_TEST_STRING(streamed_type, expected, "This is a test");
    ASSIGN_TEST_STRING(string_type, format, "This is a test");
    EXPECT_EQ(expected, StringClass::Format(format));

    ASSIGN_TEST_STRING(streamed_type, expected, "there are no formatters");
    ASSIGN_TEST_STRING(string_type, format, "there are no formatters");
    EXPECT_EQ(expected, StringClass::Format(format, 1, 2, 3, 4));

    ASSIGN_TEST_STRING(streamed_type, expected, "test some string s");
    ASSIGN_TEST_STRING(string_type, format, "test %s %c");
    ASSIGN_TEST_STRING(string_type, arg, "some string");
    EXPECT_EQ(expected, StringClass::Format(format, arg, 's'));

    ASSIGN_TEST_STRING(
        streamed_type,
        expected,
        "test 1 true 2.100000 false 1.230000e+02 0xff");
    ASSIGN_TEST_STRING(string_type, format, "test %d %d %f %d %e %x");
    EXPECT_EQ(
        expected,
        StringClass::Format(format, 1, true, 2.1f, false, 123.0, 255));
}

//==============================================================================
TYPED_TEST(BasicStringTest, JoinTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    using char_type = typename StringClass::char_type;
    using streamed_type = typename StringClass::streamed_type;

    streamed_type expected;

    string_type str;
    const char_type *ctr;
    const char_type arr[] = {'c', '\0'};
    const char_type chr = 'd';
    ASSIGN_TEST_STRING(string_type, str, "a");
    ASSIGN_TEST_STRING(string_type, ctr, "b");

    ASSIGN_TEST_STRING(streamed_type, expected, "goodbye");
    const Streamable<streamed_type> obj1(expected, 0xbeef);

    ASSIGN_TEST_STRING(streamed_type, expected, "a");
    EXPECT_EQ(expected, StringClass::Join('.', str));

    ASSIGN_TEST_STRING(streamed_type, expected, "b");
    EXPECT_EQ(expected, StringClass::Join('.', ctr));

    ASSIGN_TEST_STRING(streamed_type, expected, "c");
    EXPECT_EQ(expected, StringClass::Join('.', arr));

    ASSIGN_TEST_STRING(streamed_type, expected, "d");
    EXPECT_EQ(expected, StringClass::Join('.', chr));

    ASSIGN_TEST_STRING(streamed_type, expected, "a,a");
    EXPECT_EQ(expected, StringClass::Join(',', str, str));

    ASSIGN_TEST_STRING(streamed_type, expected, "a,b");
    EXPECT_EQ(expected, StringClass::Join(',', str, ctr));

    ASSIGN_TEST_STRING(streamed_type, expected, "a,c");
    EXPECT_EQ(expected, StringClass::Join(',', str, arr));

    ASSIGN_TEST_STRING(streamed_type, expected, "a,d");
    EXPECT_EQ(expected, StringClass::Join(',', str, chr));

    ASSIGN_TEST_STRING(streamed_type, expected, "b,a");
    EXPECT_EQ(expected, StringClass::Join(',', ctr, str));

    ASSIGN_TEST_STRING(streamed_type, expected, "b,b");
    EXPECT_EQ(expected, StringClass::Join(',', ctr, ctr));

    ASSIGN_TEST_STRING(streamed_type, expected, "b,c");
    EXPECT_EQ(expected, StringClass::Join(',', ctr, arr));

    ASSIGN_TEST_STRING(streamed_type, expected, "b,d");
    EXPECT_EQ(expected, StringClass::Join(',', ctr, chr));

    ASSIGN_TEST_STRING(streamed_type, expected, "c,a");
    EXPECT_EQ(expected, StringClass::Join(',', arr, str));

    ASSIGN_TEST_STRING(streamed_type, expected, "c,b");
    EXPECT_EQ(expected, StringClass::Join(',', arr, ctr));

    ASSIGN_TEST_STRING(streamed_type, expected, "c,c");
    EXPECT_EQ(expected, StringClass::Join(',', arr, arr));

    ASSIGN_TEST_STRING(streamed_type, expected, "c,d");
    EXPECT_EQ(expected, StringClass::Join(',', arr, chr));

    ASSIGN_TEST_STRING(streamed_type, expected, "d,a");
    EXPECT_EQ(expected, StringClass::Join(',', chr, str));

    ASSIGN_TEST_STRING(streamed_type, expected, "d,b");
    EXPECT_EQ(expected, StringClass::Join(',', chr, ctr));

    ASSIGN_TEST_STRING(streamed_type, expected, "d,c");
    EXPECT_EQ(expected, StringClass::Join(',', chr, arr));

    ASSIGN_TEST_STRING(streamed_type, expected, "d,d");
    EXPECT_EQ(expected, StringClass::Join(',', chr, chr));

    ASSIGN_TEST_STRING(streamed_type, expected, "[goodbye beef]");
    EXPECT_EQ(expected, StringClass::Join('.', obj1));

    ASSIGN_TEST_STRING(streamed_type, expected, "a:[goodbye beef]:c:d");
    EXPECT_EQ(expected, StringClass::Join(':', str, obj1, arr, chr));

    ASSIGN_TEST_STRING(streamed_type, expected, "a:c:d");
    EXPECT_EQ(expected, StringClass::Join(':', str, arr, chr));
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertStringTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_EQ(StringClass::template Convert<string_type>(s), s);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertBoolTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "0");
    EXPECT_EQ(StringClass::template Convert<bool>(s), false);

    ASSIGN_TEST_STRING(string_type, s, "1");
    EXPECT_EQ(StringClass::template Convert<bool>(s), true);

    ASSIGN_TEST_STRING(string_type, s, "-1");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::out_of_range);

    ASSIGN_TEST_STRING(string_type, s, "2");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::out_of_range);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
    EXPECT_THROW(StringClass::template Convert<bool>(s), std::invalid_argument);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertCharTest)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type s;

    using char_type = typename StringClass::streamed_type::value_type;
    using uchar_type = std::make_unsigned_t<char_type>;

    ASSIGN_TEST_STRING(string_type, s, "0");
    EXPECT_EQ(StringClass::template Convert<char_type>(s), '\0');
    EXPECT_EQ(StringClass::template Convert<uchar_type>(s), '\0');

    ASSIGN_TEST_STRING(string_type, s, "65");
    EXPECT_EQ(StringClass::template Convert<char_type>(s), 'A');
    EXPECT_EQ(StringClass::template Convert<uchar_type>(s), (uchar_type)65);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(
        StringClass::template Convert<char_type>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<uchar_type>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
    EXPECT_THROW(
        StringClass::template Convert<char_type>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<uchar_type>(s), std::invalid_argument);

    if constexpr (fly::if_string::convertible<string_type>)
    {
        EXPECT_THROW(
            StringClass::template Convert<char_type>(
                minstr<string_type, char_type>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<char_type>(
                maxstr<string_type, char_type>()),
            std::out_of_range);

        EXPECT_THROW(
            StringClass::template Convert<uchar_type>(
                minstr<string_type, uchar_type>()),
            std::out_of_range);
        EXPECT_THROW(
            StringClass::template Convert<uchar_type>(
                maxstr<string_type, uchar_type>()),
            std::out_of_range);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt8Test)
{
    using string_type = typename TestFixture::string_type;
    using StringClass = fly::BasicString<string_type>;
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "0");
    EXPECT_EQ(StringClass::template Convert<std::int8_t>(s), (std::int8_t)0);
    EXPECT_EQ(StringClass::template Convert<std::uint8_t>(s), (std::uint8_t)0);

    ASSIGN_TEST_STRING(string_type, s, "100");
    EXPECT_EQ(StringClass::template Convert<std::int8_t>(s), (std::int8_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint8_t>(s), (std::uint8_t)100);

    ASSIGN_TEST_STRING(string_type, s, "-100");
    EXPECT_EQ(StringClass::template Convert<std::int8_t>(s), (std::int8_t)-100);
    EXPECT_THROW(
        StringClass::template Convert<std::uint8_t>(s), std::out_of_range);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint8_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint8_t>(s), std::invalid_argument);

    if constexpr (fly::if_string::convertible<string_type>)
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
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "0");
    EXPECT_EQ(StringClass::template Convert<std::int16_t>(s), (std::int16_t)0);
    EXPECT_EQ(
        StringClass::template Convert<std::uint16_t>(s), (std::uint16_t)0);

    ASSIGN_TEST_STRING(string_type, s, "100");
    EXPECT_EQ(
        StringClass::template Convert<std::int16_t>(s), (std::int16_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint16_t>(s), (std::uint16_t)100);

    ASSIGN_TEST_STRING(string_type, s, "-100");
    EXPECT_EQ(
        StringClass::template Convert<std::int16_t>(s), (std::int16_t)-100);
    EXPECT_THROW(
        StringClass::template Convert<std::uint16_t>(s), std::out_of_range);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint16_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint16_t>(s), std::invalid_argument);

    if constexpr (fly::if_string::convertible<string_type>)
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
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "0");
    EXPECT_EQ(StringClass::template Convert<std::int32_t>(s), (std::int32_t)0);
    EXPECT_EQ(
        StringClass::template Convert<std::uint32_t>(s), (std::uint32_t)0);

    ASSIGN_TEST_STRING(string_type, s, "100");
    EXPECT_EQ(
        StringClass::template Convert<std::int32_t>(s), (std::int32_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint32_t>(s), (std::uint32_t)100);

    ASSIGN_TEST_STRING(string_type, s, "-100");
    EXPECT_EQ(
        StringClass::template Convert<std::int32_t>(s), (std::int32_t)-100);
    EXPECT_THROW(
        StringClass::template Convert<std::uint32_t>(s), std::out_of_range);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint32_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
    EXPECT_THROW(
        StringClass::template Convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint32_t>(s), std::invalid_argument);

    if constexpr (fly::if_string::convertible<string_type>)
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
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "0");
    EXPECT_EQ(StringClass::template Convert<std::int64_t>(s), (std::int64_t)0);
    EXPECT_EQ(
        StringClass::template Convert<std::uint64_t>(s), (std::uint64_t)0);

    ASSIGN_TEST_STRING(string_type, s, "100");
    EXPECT_EQ(
        StringClass::template Convert<std::int64_t>(s), (std::int64_t)100);
    EXPECT_EQ(
        StringClass::template Convert<std::uint64_t>(s), (std::uint64_t)100);

    ASSIGN_TEST_STRING(string_type, s, "-100");
    EXPECT_EQ(
        StringClass::template Convert<std::int64_t>(s), (std::int64_t)-100);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(
        StringClass::template Convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<std::uint64_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
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
    string_type s;

    ASSIGN_TEST_STRING(string_type, s, "-400.123");
    EXPECT_EQ(StringClass::template Convert<float>(s), -400.123f);
    EXPECT_EQ(StringClass::template Convert<double>(s), -400.123);
    EXPECT_EQ(StringClass::template Convert<long double>(s), -400.123L);

    ASSIGN_TEST_STRING(string_type, s, "400.456");
    EXPECT_EQ(StringClass::template Convert<float>(s), 400.456f);
    EXPECT_EQ(StringClass::template Convert<double>(s), 400.456);
    EXPECT_EQ(StringClass::template Convert<long double>(s), 400.456L);

    ASSIGN_TEST_STRING(string_type, s, "abc");
    EXPECT_THROW(
        StringClass::template Convert<float>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<double>(s), std::invalid_argument);
    EXPECT_THROW(
        StringClass::template Convert<long double>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(string_type, s, "2a");
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
    using StringClass = fly::BasicString<string_type>;
    using osstream_type = typename StringClass::osstream_type;
    using streamer = typename StringClass::streamer;

    // Extra test case to make sure the hexadecimal conversion feature in
    // BasicStringStreamer is handled correctly.
    if constexpr (!fly::if_string::convertible<string_type>)
    {
        string_type s;
        ASSIGN_TEST_STRING(string_type, s, "\u00f0\u0178\u008d\u2022");

        osstream_type ostream;
        streamer::Stream(ostream, s);
        EXPECT_EQ("[0xf0][0x178][0x8d][0x2022]", ostream.str());
    }
}
