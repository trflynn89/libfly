#include "fly/types/string.h"

#include <gtest/gtest.h>

#include <regex>
#include <string>
#include <vector>

namespace {

//==========================================================================
template <typename StringType>
class Streamable
{
public:
    using string_type = typename StringType::string_type;
    using ostream_type = typename StringType::ostream_type;

    Streamable(const string_type &str, int num) noexcept :
        m_str(str),
        m_num(num)
    {
    }

    string_type GetStr() const noexcept
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
    string_type m_str;
    int m_num;
};

//==========================================================================
template <typename StringType, typename T>
typename StringType::string_type minstr() noexcept
{
    static constexpr std::intmax_t min = std::numeric_limits<T>::min();

    if constexpr (std::is_same_v<StringType, fly::String>)
    {
        return std::to_string(min - 1);
    }
    else if constexpr (std::is_same_v<StringType, fly::WString>)
    {
        return std::to_wstring(min - 1);
    }
}

//==========================================================================
template <typename StringType, typename T>
typename StringType::string_type maxstr() noexcept
{
    static constexpr std::uintmax_t max = std::numeric_limits<T>::max();

    if constexpr (std::is_same_v<StringType, fly::String>)
    {
        return std::to_string(max + 1);
    }
    else if constexpr (std::is_same_v<StringType, fly::WString>)
    {
        return std::to_wstring(max + 1);
    }
}

} // namespace

//==========================================================================
template <typename T>
struct BasicStringTest : public ::testing::Test
{
    using StringType = T;
};

using StringTypes = ::testing::Types<fly::String, fly::WString>;
TYPED_TEST_CASE(BasicStringTest, StringTypes);

#define ASSIGN_TEST_STRING(Type, var, raw)                                     \
    if constexpr (std::is_same_v<Type, fly::String>)                           \
    {                                                                          \
        var = raw;                                                             \
    }                                                                          \
    else if constexpr (std::is_same_v<Type, fly::WString>)                     \
    {                                                                          \
        var = L##raw;                                                          \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        ASSERT_TRUE(false);                                                    \
    }

//==============================================================================
TYPED_TEST(BasicStringTest, SplitTest)
{
    using StringType = typename TestFixture::StringType;

    static constexpr int numSectors = 10;
    std::vector<StringType::string_type> inputSplit(numSectors);

    constexpr StringType::value_type delim = ' ';
    StringType::string_type input;

    for (int i = 0; i < numSectors; ++i)
    {
        StringType::string_type curr = StringType::GenerateRandomString(10);

        inputSplit[i] = curr;
        input += curr + delim;
    }

    const auto outputSplit = StringType::Split(input, delim);
    ASSERT_EQ(inputSplit.size(), outputSplit.size());

    for (int i = 0; i < numSectors; ++i)
    {
        ASSERT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, MaxSplitTest)
{
    using StringType = typename TestFixture::StringType;

    static constexpr int numSectors = 10;
    static constexpr int maxSectors = 6;
    std::vector<StringType::string_type> inputSplit(maxSectors);

    constexpr StringType::value_type delim = ';';
    StringType::string_type input;

    for (int i = 0; i < numSectors; ++i)
    {
        StringType::string_type curr = StringType::GenerateRandomString(10);

        if (i < maxSectors)
        {
            inputSplit[i] = curr;
        }
        else
        {
            inputSplit.back() += delim;
            inputSplit.back() += curr;
        }

        input += curr + delim;
    }

    const auto outputSplit = StringType::Split(input, delim, maxSectors);
    ASSERT_EQ(inputSplit.size(), outputSplit.size());

    for (int i = 0; i < maxSectors; ++i)
    {
        ASSERT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TYPED_TEST(BasicStringTest, TrimTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type test1, test2, test3, test4, test5, test6, test7;
    StringType::string_type expt1, expt2, expt3, expt4;

    ASSIGN_TEST_STRING(StringType, test2, "   abc");
    ASSIGN_TEST_STRING(StringType, test3, "abc   ");
    ASSIGN_TEST_STRING(StringType, test4, "   abc   ");
    ASSIGN_TEST_STRING(StringType, test5, " \n\t\r  abc  \n\t\r ");
    ASSIGN_TEST_STRING(StringType, test6, " \n\t\r  a   c  \n\t\r ");
    ASSIGN_TEST_STRING(StringType, test7, " \n\t\r  a\n \tc  \n\t\r ");

    ASSIGN_TEST_STRING(StringType, expt2, "abc");
    ASSIGN_TEST_STRING(StringType, expt3, "a   c");
    ASSIGN_TEST_STRING(StringType, expt4, "a\n \tc");

    StringType::Trim(test1);
    StringType::Trim(test2);
    StringType::Trim(test3);
    StringType::Trim(test4);
    StringType::Trim(test5);
    StringType::Trim(test6);
    StringType::Trim(test7);

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
    using StringType = typename TestFixture::StringType;

    StringType::string_type source, search, replace, result;
    ASSIGN_TEST_STRING(StringType, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(StringType, search, "Be Replaced");
    ASSIGN_TEST_STRING(StringType, replace, "new value");
    ASSIGN_TEST_STRING(StringType, result, "To new value! To new value!");

    StringType::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithCharTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type source, search, result;
    ASSIGN_TEST_STRING(StringType, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(StringType, search, "Be Replaced");
    StringType::value_type replace('x');
    ASSIGN_TEST_STRING(StringType, result, "To x! To x!");

    StringType::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ReplaceAllWithEmptyTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type source, search, replace, result;
    ASSIGN_TEST_STRING(StringType, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(StringType, replace, "new value");
    ASSIGN_TEST_STRING(StringType, result, "To Be Replaced! To Be Replaced!");

    StringType::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, RemoveAllTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type source, search, result;
    ASSIGN_TEST_STRING(StringType, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(StringType, search, "Be Rep");
    ASSIGN_TEST_STRING(StringType, result, "To laced! To laced!");

    StringType::RemoveAll(source, search);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, RemoveAllWithEmptyTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type source, search, result;
    ASSIGN_TEST_STRING(StringType, source, "To Be Replaced! To Be Replaced!");
    ASSIGN_TEST_STRING(StringType, result, "To Be Replaced! To Be Replaced!");

    StringType::RemoveAll(source, search);
    ASSERT_EQ(source, result);
}

//==============================================================================
TYPED_TEST(BasicStringTest, StartsWithTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type test1, test2;

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "");
    EXPECT_TRUE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "");
    EXPECT_TRUE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    EXPECT_TRUE(StringType::StartsWith(test1, 'a'));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "a");
    EXPECT_TRUE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "ab");
    EXPECT_TRUE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "abc");
    EXPECT_TRUE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "");
    EXPECT_FALSE(StringType::StartsWith(test1, 'a'));

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "a");
    EXPECT_FALSE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "b");
    EXPECT_FALSE(StringType::StartsWith(test1, 'a'));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "ab");
    EXPECT_FALSE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "ab");
    ASSIGN_TEST_STRING(StringType, test2, "abc");
    EXPECT_FALSE(StringType::StartsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "abd");
    EXPECT_FALSE(StringType::StartsWith(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, EndsWithTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type test1, test2;

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "");
    EXPECT_TRUE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "");
    EXPECT_TRUE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    EXPECT_TRUE(StringType::EndsWith(test1, 'c'));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "c");
    EXPECT_TRUE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "bc");
    EXPECT_TRUE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "abc");
    EXPECT_TRUE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "a");
    EXPECT_FALSE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "ba");
    EXPECT_FALSE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "ab");
    ASSIGN_TEST_STRING(StringType, test2, "a");
    EXPECT_FALSE(StringType::EndsWith(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "ab");
    EXPECT_FALSE(StringType::EndsWith(test1, 'a'));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "dbc");
    EXPECT_FALSE(StringType::EndsWith(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, WildcardTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type test1, test2;

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "*");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "**");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "a");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "b");
    ASSIGN_TEST_STRING(StringType, test2, "*");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "c");
    ASSIGN_TEST_STRING(StringType, test2, "**");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "a*");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "a*c");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "a*");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "*b*");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "*bc");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "*c");
    EXPECT_TRUE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "");
    ASSIGN_TEST_STRING(StringType, test2, "");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "b");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "b*");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "a");
    ASSIGN_TEST_STRING(StringType, test2, "*b");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "a");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "b*");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "*b");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));

    ASSIGN_TEST_STRING(StringType, test1, "abc");
    ASSIGN_TEST_STRING(StringType, test2, "*d*");
    EXPECT_FALSE(StringType::WildcardMatch(test1, test2));
}

//==============================================================================
TYPED_TEST(BasicStringTest, GenerateRandomStringTest)
{
    using StringType = typename TestFixture::StringType;

    static constexpr StringType::size_type length = (1 << 10);

    const auto random = StringType::GenerateRandomString(length);
    ASSERT_EQ(length, random.length());
}

//==============================================================================
TYPED_TEST(BasicStringTest, FormatTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type expected;
    const StringType::value_type *format;
    StringType::string_type arg;

    ASSIGN_TEST_STRING(StringType, expected, "");
    ASSIGN_TEST_STRING(StringType, format, "");
    EXPECT_EQ(expected, StringType::Format(format));

    ASSIGN_TEST_STRING(StringType, expected, "%");
    ASSIGN_TEST_STRING(StringType, format, "%");
    EXPECT_EQ(expected, StringType::Format(format));
    EXPECT_EQ(expected, StringType::Format(format, 1));

    ASSIGN_TEST_STRING(StringType, expected, "%%");
    ASSIGN_TEST_STRING(StringType, format, "%%");
    EXPECT_EQ(expected, StringType::Format(format));

    ASSIGN_TEST_STRING(StringType, expected, "%d");
    ASSIGN_TEST_STRING(StringType, format, "%d");
    EXPECT_EQ(expected, StringType::Format(format));

    ASSIGN_TEST_STRING(StringType, expected, "This is a test");
    ASSIGN_TEST_STRING(StringType, format, "This is a test");
    EXPECT_EQ(expected, StringType::Format(format));

    ASSIGN_TEST_STRING(StringType, expected, "there are no formatters");
    ASSIGN_TEST_STRING(StringType, format, "there are no formatters");
    EXPECT_EQ(expected, StringType::Format(format, 1, 2, 3, 4));

    ASSIGN_TEST_STRING(StringType, expected, "test some string s");
    ASSIGN_TEST_STRING(StringType, format, "test %s %c");
    ASSIGN_TEST_STRING(StringType, arg, "some string");
    EXPECT_EQ(expected, StringType::Format(format, arg, 's'));

    ASSIGN_TEST_STRING(
        StringType, expected, "test 1 true 2.100000 false 1.230000e+02 0xff");
    ASSIGN_TEST_STRING(StringType, format, "test %d %d %f %d %e %x");
    EXPECT_EQ(
        expected, StringType::Format(format, 1, true, 2.1f, false, 123.0, 255));
}

//==============================================================================
TYPED_TEST(BasicStringTest, JoinTest)
{
    using StringType = typename TestFixture::StringType;

    StringType::string_type str, expected;
    const StringType::value_type *ctr;
    const StringType::value_type arr[] = {'c', '\0'};
    const StringType::value_type chr = 'd';
    ASSIGN_TEST_STRING(StringType, str, "a");
    ASSIGN_TEST_STRING(StringType, ctr, "b");

    ASSIGN_TEST_STRING(StringType, expected, "goodbye");
    const Streamable<StringType> obj1(expected, 0xbeef);

    ASSIGN_TEST_STRING(StringType, expected, "a");
    EXPECT_EQ(expected, StringType::Join('.', str));

    ASSIGN_TEST_STRING(StringType, expected, "b");
    EXPECT_EQ(expected, StringType::Join('.', ctr));

    ASSIGN_TEST_STRING(StringType, expected, "c");
    EXPECT_EQ(expected, StringType::Join('.', arr));

    ASSIGN_TEST_STRING(StringType, expected, "d");
    EXPECT_EQ(expected, StringType::Join('.', chr));

    ASSIGN_TEST_STRING(StringType, expected, "a,a");
    EXPECT_EQ(expected, StringType::Join(',', str, str));

    ASSIGN_TEST_STRING(StringType, expected, "a,b");
    EXPECT_EQ(expected, StringType::Join(',', str, ctr));

    ASSIGN_TEST_STRING(StringType, expected, "a,c");
    EXPECT_EQ(expected, StringType::Join(',', str, arr));

    ASSIGN_TEST_STRING(StringType, expected, "a,d");
    EXPECT_EQ(expected, StringType::Join(',', str, chr));

    ASSIGN_TEST_STRING(StringType, expected, "b,a");
    EXPECT_EQ(expected, StringType::Join(',', ctr, str));

    ASSIGN_TEST_STRING(StringType, expected, "b,b");
    EXPECT_EQ(expected, StringType::Join(',', ctr, ctr));

    ASSIGN_TEST_STRING(StringType, expected, "b,c");
    EXPECT_EQ(expected, StringType::Join(',', ctr, arr));

    ASSIGN_TEST_STRING(StringType, expected, "b,d");
    EXPECT_EQ(expected, StringType::Join(',', ctr, chr));

    ASSIGN_TEST_STRING(StringType, expected, "c,a");
    EXPECT_EQ(expected, StringType::Join(',', arr, str));

    ASSIGN_TEST_STRING(StringType, expected, "c,b");
    EXPECT_EQ(expected, StringType::Join(',', arr, ctr));

    ASSIGN_TEST_STRING(StringType, expected, "c,c");
    EXPECT_EQ(expected, StringType::Join(',', arr, arr));

    ASSIGN_TEST_STRING(StringType, expected, "c,d");
    EXPECT_EQ(expected, StringType::Join(',', arr, chr));

    ASSIGN_TEST_STRING(StringType, expected, "d,a");
    EXPECT_EQ(expected, StringType::Join(',', chr, str));

    ASSIGN_TEST_STRING(StringType, expected, "d,b");
    EXPECT_EQ(expected, StringType::Join(',', chr, ctr));

    ASSIGN_TEST_STRING(StringType, expected, "d,c");
    EXPECT_EQ(expected, StringType::Join(',', chr, arr));

    ASSIGN_TEST_STRING(StringType, expected, "d,d");
    EXPECT_EQ(expected, StringType::Join(',', chr, chr));

    ASSIGN_TEST_STRING(StringType, expected, "[goodbye beef]");
    EXPECT_EQ(expected, StringType::Join('.', obj1));

    ASSIGN_TEST_STRING(StringType, expected, "a:[goodbye beef]:c:d");
    EXPECT_EQ(expected, StringType::Join(':', str, obj1, arr, chr));

    ASSIGN_TEST_STRING(StringType, expected, "a:c:d");
    EXPECT_EQ(expected, StringType::Join(':', str, arr, chr));
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertStringTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_EQ(StringType::Convert<StringType::string_type>(s), s);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertBoolTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "0");
    EXPECT_EQ(StringType::Convert<bool>(s), false);

    ASSIGN_TEST_STRING(StringType, s, "1");
    EXPECT_EQ(StringType::Convert<bool>(s), true);

    ASSIGN_TEST_STRING(StringType, s, "-1");
    EXPECT_THROW(StringType::Convert<bool>(s), std::out_of_range);

    ASSIGN_TEST_STRING(StringType, s, "2");
    EXPECT_THROW(StringType::Convert<bool>(s), std::out_of_range);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<bool>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<bool>(s), std::invalid_argument);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertCharTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    using char_type = StringType::value_type;
    using uchar_type = std::make_unsigned_t<char_type>;

    ASSIGN_TEST_STRING(StringType, s, "0");
    EXPECT_EQ(StringType::Convert<char_type>(s), '\0');
    EXPECT_EQ(StringType::Convert<uchar_type>(s), '\0');

    ASSIGN_TEST_STRING(StringType, s, "65");
    EXPECT_EQ(StringType::Convert<char_type>(s), 'A');
    EXPECT_EQ(StringType::Convert<uchar_type>(s), (uchar_type)65);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<char_type>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<uchar_type>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<char_type>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<uchar_type>(s), std::invalid_argument);

    EXPECT_THROW(
        StringType::Convert<char_type>(minstr<StringType, char_type>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<char_type>(maxstr<StringType, char_type>()),
        std::out_of_range);

    EXPECT_THROW(
        StringType::Convert<uchar_type>(minstr<StringType, uchar_type>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<uchar_type>(maxstr<StringType, uchar_type>()),
        std::out_of_range);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt8Test)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "0");
    EXPECT_EQ(StringType::Convert<std::int8_t>(s), (std::int8_t)0);
    EXPECT_EQ(StringType::Convert<std::uint8_t>(s), (std::uint8_t)0);

    ASSIGN_TEST_STRING(StringType, s, "100");
    EXPECT_EQ(StringType::Convert<std::int8_t>(s), (std::int8_t)100);
    EXPECT_EQ(StringType::Convert<std::uint8_t>(s), (std::uint8_t)100);

    ASSIGN_TEST_STRING(StringType, s, "-100");
    EXPECT_EQ(StringType::Convert<std::int8_t>(s), (std::int8_t)-100);
    EXPECT_THROW(StringType::Convert<std::uint8_t>(s), std::out_of_range);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint8_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<std::int8_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint8_t>(s), std::invalid_argument);

    EXPECT_THROW(
        StringType::Convert<std::int8_t>(minstr<StringType, std::int8_t>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<std::int8_t>(maxstr<StringType, std::int8_t>()),
        std::out_of_range);

    EXPECT_THROW(
        StringType::Convert<std::uint8_t>(minstr<StringType, std::uint8_t>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<std::uint8_t>(maxstr<StringType, std::uint8_t>()),
        std::out_of_range);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt16Test)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "0");
    EXPECT_EQ(StringType::Convert<std::int16_t>(s), (std::int16_t)0);
    EXPECT_EQ(StringType::Convert<std::uint16_t>(s), (std::uint16_t)0);

    ASSIGN_TEST_STRING(StringType, s, "100");
    EXPECT_EQ(StringType::Convert<std::int16_t>(s), (std::int16_t)100);
    EXPECT_EQ(StringType::Convert<std::uint16_t>(s), (std::uint16_t)100);

    ASSIGN_TEST_STRING(StringType, s, "-100");
    EXPECT_EQ(StringType::Convert<std::int16_t>(s), (std::int16_t)-100);
    EXPECT_THROW(StringType::Convert<std::uint16_t>(s), std::out_of_range);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint16_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<std::int16_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint16_t>(s), std::invalid_argument);

    EXPECT_THROW(
        StringType::Convert<std::int16_t>(minstr<StringType, std::int16_t>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<std::int16_t>(maxstr<StringType, std::int16_t>()),
        std::out_of_range);

    EXPECT_THROW(
        StringType::Convert<std::uint16_t>(minstr<StringType, std::uint16_t>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<std::uint16_t>(maxstr<StringType, std::uint16_t>()),
        std::out_of_range);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt32Test)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "0");
    EXPECT_EQ(StringType::Convert<std::int32_t>(s), (std::int32_t)0);
    EXPECT_EQ(StringType::Convert<std::uint32_t>(s), (std::uint32_t)0);

    ASSIGN_TEST_STRING(StringType, s, "100");
    EXPECT_EQ(StringType::Convert<std::int32_t>(s), (std::int32_t)100);
    EXPECT_EQ(StringType::Convert<std::uint32_t>(s), (std::uint32_t)100);

    ASSIGN_TEST_STRING(StringType, s, "-100");
    EXPECT_EQ(StringType::Convert<std::int32_t>(s), (std::int32_t)-100);
    EXPECT_THROW(StringType::Convert<std::uint32_t>(s), std::out_of_range);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint32_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<std::int32_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint32_t>(s), std::invalid_argument);

    EXPECT_THROW(
        StringType::Convert<std::int32_t>(minstr<StringType, std::int32_t>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<std::int32_t>(maxstr<StringType, std::int32_t>()),
        std::out_of_range);

    EXPECT_THROW(
        StringType::Convert<std::uint32_t>(minstr<StringType, std::uint32_t>()),
        std::out_of_range);
    EXPECT_THROW(
        StringType::Convert<std::uint32_t>(maxstr<StringType, std::uint32_t>()),
        std::out_of_range);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertInt64Test)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "0");
    EXPECT_EQ(StringType::Convert<std::int64_t>(s), (std::int64_t)0);
    EXPECT_EQ(StringType::Convert<std::uint64_t>(s), (std::uint64_t)0);

    ASSIGN_TEST_STRING(StringType, s, "100");
    EXPECT_EQ(StringType::Convert<std::int64_t>(s), (std::int64_t)100);
    EXPECT_EQ(StringType::Convert<std::uint64_t>(s), (std::uint64_t)100);

    ASSIGN_TEST_STRING(StringType, s, "-100");
    EXPECT_EQ(StringType::Convert<std::int64_t>(s), (std::int64_t)-100);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint64_t>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<std::int64_t>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<std::uint64_t>(s), std::invalid_argument);
}

//==============================================================================
TYPED_TEST(BasicStringTest, ConvertDecimalTest)
{
    using StringType = typename TestFixture::StringType;
    StringType::string_type s;

    ASSIGN_TEST_STRING(StringType, s, "-400.123");
    EXPECT_EQ(StringType::Convert<float>(s), -400.123f);
    EXPECT_EQ(StringType::Convert<double>(s), -400.123);
    EXPECT_EQ(StringType::Convert<long double>(s), -400.123L);

    ASSIGN_TEST_STRING(StringType, s, "400.456");
    EXPECT_EQ(StringType::Convert<float>(s), 400.456f);
    EXPECT_EQ(StringType::Convert<double>(s), 400.456);
    EXPECT_EQ(StringType::Convert<long double>(s), 400.456L);

    ASSIGN_TEST_STRING(StringType, s, "abc");
    EXPECT_THROW(StringType::Convert<float>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<double>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<long double>(s), std::invalid_argument);

    ASSIGN_TEST_STRING(StringType, s, "2a");
    EXPECT_THROW(StringType::Convert<float>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<double>(s), std::invalid_argument);
    EXPECT_THROW(StringType::Convert<long double>(s), std::invalid_argument);
}
