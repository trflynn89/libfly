#define NOMINMAX

#include <regex>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <fly/logging/logger.h>
#include <fly/string/string.h>

namespace
{
    //==========================================================================
    class Base
    {
    public:
        Base(const std::string &str, int num) :
            m_str(str),
            m_num(num)
        {
        }

        std::string GetStr() const { return m_str; };
        int GetNum() const { return m_num; };

        size_t Hash() const
        {
            static std::hash<std::string> strHasher;
            static std::hash<int> numHasher;
            static int magic = 0x9e3779b9;

            size_t strHash = strHasher(m_str);
            size_t numHash = numHasher(m_num);

            // Derived from boost::hash_combine
            return (strHash ^ (numHash + magic + (strHash << 6) + (strHash >> 2)));
        }

    private:
        std::string m_str;
        int m_num;
    };

    //==========================================================================
    class Hashable : public Base
    {
    public:
        Hashable(const std::string &str, int num) : Base(str, num)
        {
        }
    };

    //==========================================================================
    class Streamable : public Base
    {
    public:
        Streamable(const std::string &str, int num) : Base(str, num)
        {
        }

        friend std::ostream &operator << (std::ostream &, const Streamable &);
    };

    std::ostream &operator << (std::ostream &stream, const Streamable &obj)
    {
        stream << '[' << obj.GetStr() << ' ' << std::hex << obj.GetNum() << std::dec << ']';
        return stream;
    }

    //==========================================================================
    class HashableAndStreamable : public Base
    {
    public:
        HashableAndStreamable(const std::string &str, int num) : Base(str, num)
        {
        }

        friend std::ostream &operator << (std::ostream &, const HashableAndStreamable &);
    };

    std::ostream &operator << (std::ostream &stream, const HashableAndStreamable &obj)
    {
        stream << '[' << obj.GetStr() << ' ' << std::hex << obj.GetNum() << std::dec << ']';
        return stream;
    }

    //==========================================================================
    template <typename T>
    std::string min_to_string()
    {
        long long min = std::numeric_limits<T>::min();
        return std::to_string(min - 1);
    }

    //==========================================================================
    template <typename T>
    std::string max_to_string()
    {
        unsigned long long max = std::numeric_limits<T>::max();
        return std::to_string(max + 1);
    }
}

//==============================================================================
namespace std
{
    template <>
    struct hash<Hashable>
    {
        size_t operator()(const Hashable &value) const
        {
            return value.Hash();
        }
    };

    template <>
    struct hash<HashableAndStreamable>
    {
        size_t operator()(const HashableAndStreamable &value) const
        {
            return value.Hash();
        }
    };
}

//==============================================================================
TEST(StringTest, SplitTest)
{
    static const int numSectors = 10;
    std::vector<std::string> inputSplit(numSectors);

    std::string input;
    char delim = ' ';

    for (int i = 0; i < numSectors; ++i)
    {
        std::string curr = fly::String::GenerateRandomString(10);

        inputSplit[i] = curr;
        input += curr + delim;
    }

    std::vector<std::string> outputSplit = fly::String::Split(input, delim);
    ASSERT_EQ(inputSplit.size(), outputSplit.size());

    for (int i = 0; i < numSectors; ++i)
    {
        ASSERT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TEST(StringTest, MaxSplitTest)
{
    static const int numSectors = 10;
    static const int maxSectors = 6;
    std::vector<std::string> inputSplit(maxSectors);

    std::string input;
    char delim = ';';

    for (int i = 0; i < numSectors; ++i)
    {
        std::string curr = fly::String::GenerateRandomString(10);

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

    std::vector<std::string> outputSplit = fly::String::Split(input, delim, maxSectors);
    ASSERT_EQ(inputSplit.size(), outputSplit.size());

    for (int i = 0; i < maxSectors; ++i)
    {
        ASSERT_EQ(inputSplit[i], outputSplit[i]);
    }
}

//==============================================================================
TEST(StringTest, TrimTest)
{
    std::string str1;
    std::string str2("   abc");
    std::string str3("abc   ");
    std::string str4("   abc   ");
    std::string str5(" \n\t\r  abc  \n\t\r ");
    std::string str6(" \n\t\r  a   c  \n\t\r ");
    std::string str7(" \n\t\r  a\n \tc  \n\t\r ");

    fly::String::Trim(str1);
    fly::String::Trim(str2);
    fly::String::Trim(str3);
    fly::String::Trim(str4);
    fly::String::Trim(str5);
    fly::String::Trim(str6);
    fly::String::Trim(str7);

    EXPECT_EQ(str1, std::string());
    EXPECT_EQ(str2, std::string("abc"));
    EXPECT_EQ(str3, std::string("abc"));
    EXPECT_EQ(str4, std::string("abc"));
    EXPECT_EQ(str5, std::string("abc"));
    EXPECT_EQ(str6, std::string("a   c"));
    EXPECT_EQ(str7, std::string("a\n \tc"));
}

//==============================================================================
TEST(StringTest, ReplaceAllTest)
{
    std::string source("To Be Replaced! To Be Replaced!");
    std::string search("Be Replaced");
    std::string replace("new value");
    std::string result("To new value! To new value!");

    fly::String::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TEST(StringTest, ReplaceAllWithEmptyTest)
{
    std::string source("To Be Replaced! To Be Replaced!");
    std::string search;
    std::string replace("new value");
    std::string result("To Be Replaced! To Be Replaced!");

    fly::String::ReplaceAll(source, search, replace);
    ASSERT_EQ(source, result);
}

//==============================================================================
TEST(StringTest, RemoveAllTest)
{
    std::string source("To Be Replaced! To Be Replaced!");
    std::string search("Be Rep");
    std::string result("To laced! To laced!");

    fly::String::RemoveAll(source, search);
    ASSERT_EQ(source, result);
}

//==============================================================================
TEST(StringTest, RemoveAllWithEmptyTest)
{
    std::string source("To Be Replaced! To Be Replaced!");
    std::string search;
    std::string result("To Be Replaced! To Be Replaced!");

    fly::String::RemoveAll(source, search);
    ASSERT_EQ(source, result);
}

//==============================================================================
TEST(StringTest, StartsWithTest)
{
    EXPECT_TRUE(fly::String::StartsWith("", ""));
    EXPECT_TRUE(fly::String::StartsWith("a", ""));
    EXPECT_TRUE(fly::String::StartsWith("abc", 'a'));
    EXPECT_TRUE(fly::String::StartsWith("abc", "a"));
    EXPECT_TRUE(fly::String::StartsWith("abc", "ab"));
    EXPECT_TRUE(fly::String::StartsWith("abc", "abc"));

    EXPECT_FALSE(fly::String::StartsWith("", 'a'));
    EXPECT_FALSE(fly::String::StartsWith("", "a"));
    EXPECT_FALSE(fly::String::StartsWith("b", 'a'));
    EXPECT_FALSE(fly::String::StartsWith("a", "ab"));
    EXPECT_FALSE(fly::String::StartsWith("ab", "abc"));
    EXPECT_FALSE(fly::String::StartsWith("abc", "abd"));
}

//==============================================================================
TEST(StringTest, EndsWithTest)
{
    EXPECT_TRUE(fly::String::EndsWith("", ""));
    EXPECT_TRUE(fly::String::EndsWith("a", ""));
    EXPECT_TRUE(fly::String::EndsWith("abc", 'c'));
    EXPECT_TRUE(fly::String::EndsWith("abc", "c"));
    EXPECT_TRUE(fly::String::EndsWith("abc", "bc"));
    EXPECT_TRUE(fly::String::EndsWith("abc", "abc"));

    EXPECT_FALSE(fly::String::EndsWith("", "a"));
    EXPECT_FALSE(fly::String::EndsWith("a", "ba"));
    EXPECT_FALSE(fly::String::EndsWith("ab", "a"));
    EXPECT_FALSE(fly::String::EndsWith("ab", 'a'));
    EXPECT_FALSE(fly::String::EndsWith("abc", "dbc"));
}

//==============================================================================
TEST(StringTest, GenerateRandomStringTest)
{
    static const int length = (1 << 20);

    std::string random = fly::String::GenerateRandomString(length);
    ASSERT_EQ(length, random.length());
}

//==============================================================================
TEST(StringTest, EntropyTest)
{
    std::string str1 = fly::String::GenerateRandomString(1 << 10);
    std::string str2("A quick brown fox jumped over the lazy dog");
    std::string str3("repeatrepeatrepeatrepeatrepeatrepeatrepeat");
    std::string str4("rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr");

    float ent1 = fly::String::CalculateEntropy(str1);
    float ent2 = fly::String::CalculateEntropy(str2);
    float ent3 = fly::String::CalculateEntropy(str3);
    float ent4 = fly::String::CalculateEntropy(str4);

    LOGC("E1=%f, E2=%f, E3=%f, E4=%f", ent1, ent2, ent3, ent4);

    // Expect entropy to decrease for less random strings
    EXPECT_GT(ent1, ent2);
    EXPECT_GT(ent2, ent3);
    EXPECT_GT(ent3, ent4);
}

//==============================================================================
TEST(StringTest, FormatTest)
{
    EXPECT_EQ("", fly::String::Format(""));
    EXPECT_EQ("%", fly::String::Format("%"));
    EXPECT_EQ("%%", fly::String::Format("%%"));
    EXPECT_EQ("%d", fly::String::Format("%d"));
    EXPECT_EQ("This is a test", fly::String::Format("This is a test"));
    EXPECT_EQ("there are no formatters", fly::String::Format("there are no formatters", 1, 2, 3, 4));
    EXPECT_EQ("test some string s", fly::String::Format("test %s %c", std::string("some string"), 's'));
    EXPECT_EQ("test 1 true 2.100000 false 1.230000e+02 0xff", fly::String::Format("test %d %d %f %d %e %x", 1, true, 2.1f, false, 123.0, 255));
}

//==============================================================================
TEST(StringTest, JoinTest)
{
    Hashable obj1("hello", 0xdead);
    Streamable obj2("goodbye", 0xbeef);
    HashableAndStreamable obj3("world", 0xf00d);

    std::string str("a");
    const char *ctr = "b";
    char arr[] = { 'c', '\0' };
    char chr = 'd';

    EXPECT_EQ("a", fly::String::Join('.', str));
    EXPECT_EQ("b", fly::String::Join('.', ctr));
    EXPECT_EQ("c", fly::String::Join('.', arr));
    EXPECT_EQ("d", fly::String::Join('.', chr));

    EXPECT_EQ("a,a", fly::String::Join(',', str, str));
    EXPECT_EQ("a,b", fly::String::Join(',', str, ctr));
    EXPECT_EQ("a,c", fly::String::Join(',', str, arr));
    EXPECT_EQ("a,d", fly::String::Join(',', str, chr));
    EXPECT_EQ("b,a", fly::String::Join(',', ctr, str));
    EXPECT_EQ("b,b", fly::String::Join(',', ctr, ctr));
    EXPECT_EQ("b,c", fly::String::Join(',', ctr, arr));
    EXPECT_EQ("b,d", fly::String::Join(',', ctr, chr));
    EXPECT_EQ("c,a", fly::String::Join(',', arr, str));
    EXPECT_EQ("c,b", fly::String::Join(',', arr, ctr));
    EXPECT_EQ("c,c", fly::String::Join(',', arr, arr));
    EXPECT_EQ("c,d", fly::String::Join(',', arr, chr));
    EXPECT_EQ("d,a", fly::String::Join(',', chr, str));
    EXPECT_EQ("d,b", fly::String::Join(',', chr, ctr));
    EXPECT_EQ("d,c", fly::String::Join(',', chr, arr));
    EXPECT_EQ("d,d", fly::String::Join(',', chr, chr));

    EXPECT_EQ("[goodbye beef]", fly::String::Join('.', obj2));
    EXPECT_EQ("a:[goodbye beef]:c:d", fly::String::Join(':', str, obj2, arr, chr));
    EXPECT_EQ("a:c:d", fly::String::Join(':', str, arr, chr));

#ifndef BUILD_WINDOWS

    std::regex test("(\\[0x[0-9a-fA-F]+\\]:2:\\[goodbye beef\\]:\\[world f00d\\])");
    ASSERT_TRUE(std::regex_match(fly::String::Join(':', obj1, 2, obj2, obj3), test));

#endif // BUILD_WINDOWS
}

//==============================================================================
TEST(StringTest, ConvertTest)
{
    // BOOL
    EXPECT_EQ(fly::String::Convert<std::string>("abc"), "abc");

    // STRING
    EXPECT_EQ(fly::String::Convert<bool>("0"), false);
    EXPECT_EQ(fly::String::Convert<bool>("1"), true);
    EXPECT_THROW(fly::String::Convert<bool>("-1"), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<bool>("2"), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<bool>("abc"), std::invalid_argument);

    // CHAR
    EXPECT_EQ(fly::String::Convert<char>("0"), '\0');
    EXPECT_EQ(fly::String::Convert<char>("65"), 'A');
    EXPECT_THROW(fly::String::Convert<char>(min_to_string<char>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<char>(max_to_string<char>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<char>("abc"), std::invalid_argument);

    EXPECT_EQ(fly::String::Convert<unsigned char>("0"), '\0');
    EXPECT_EQ(fly::String::Convert<unsigned char>("200"), (unsigned char)200);
    EXPECT_THROW(fly::String::Convert<unsigned char>(min_to_string<unsigned char>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<unsigned char>(max_to_string<unsigned char>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<unsigned char>("abc"), std::invalid_argument);

    // SHORT
    EXPECT_EQ(fly::String::Convert<short>("-4000"), (short)-4000);
    EXPECT_EQ(fly::String::Convert<short>("4000"), (short)4000);
    EXPECT_THROW(fly::String::Convert<short>(min_to_string<short>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<short>(max_to_string<short>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<short>("abc"), std::invalid_argument);

    EXPECT_EQ(fly::String::Convert<unsigned short>("0"), (unsigned short)0);
    EXPECT_EQ(fly::String::Convert<unsigned short>("4000"), (unsigned short)4000);
    EXPECT_THROW(fly::String::Convert<unsigned short>(min_to_string<unsigned short>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<unsigned short>(max_to_string<unsigned short>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<unsigned short>("abc"), std::invalid_argument);

    // INT
    EXPECT_EQ(fly::String::Convert<int>("-4000"), (int)-4000);
    EXPECT_EQ(fly::String::Convert<int>("4000"), (int)4000);
    EXPECT_THROW(fly::String::Convert<int>(min_to_string<int>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<int>(max_to_string<int>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<int>("abc"), std::invalid_argument);

    EXPECT_EQ(fly::String::Convert<unsigned int>("0"), (unsigned int)0);
    EXPECT_EQ(fly::String::Convert<unsigned int>("4000"), (unsigned int)4000);
    EXPECT_THROW(fly::String::Convert<unsigned int>(min_to_string<unsigned int>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<unsigned int>(max_to_string<unsigned int>()), std::out_of_range);
    EXPECT_THROW(fly::String::Convert<unsigned int>("abc"), std::invalid_argument);

    // LONG
    EXPECT_EQ(fly::String::Convert<long>("-4000"), (long)-4000);
    EXPECT_EQ(fly::String::Convert<long>("4000"), (long)4000);
    EXPECT_THROW(fly::String::Convert<long>("abc"), std::invalid_argument);

    EXPECT_EQ(fly::String::Convert<unsigned long>("0"), (unsigned long)0);
    EXPECT_EQ(fly::String::Convert<unsigned long>("4000"), (unsigned long)4000);
    EXPECT_THROW(fly::String::Convert<unsigned long>("abc"), std::invalid_argument);

    // LONG LONG
    EXPECT_EQ(fly::String::Convert<long long>("-4000"), (long long)-4000);
    EXPECT_EQ(fly::String::Convert<long long>("4000"), (long long)4000);
    EXPECT_THROW(fly::String::Convert<long long>("abc"), std::invalid_argument);

    EXPECT_EQ(fly::String::Convert<unsigned long long>("0"), (unsigned long long)0);
    EXPECT_EQ(fly::String::Convert<unsigned long long>("4000"), (unsigned long long)4000);
    EXPECT_THROW(fly::String::Convert<unsigned long long>("abc"), std::invalid_argument);

    // FLOAT
    EXPECT_EQ(fly::String::Convert<float>("-4000.123f"), -4000.123f);
    EXPECT_EQ(fly::String::Convert<float>("4000.456f"), 4000.456f);
    EXPECT_THROW(fly::String::Convert<float>("abc"), std::invalid_argument);

    // DOUBLE
    EXPECT_EQ(fly::String::Convert<double>("-4000.123"), -4000.123);
    EXPECT_EQ(fly::String::Convert<double>("4000.456"), 4000.456);
    EXPECT_THROW(fly::String::Convert<double>("abc"), std::invalid_argument);

    // LONG DOUBLE
    EXPECT_EQ(fly::String::Convert<long double>("-4000.123L"), -4000.123L);
    EXPECT_EQ(fly::String::Convert<long double>("4000.456L"), 4000.456L);
    EXPECT_THROW(fly::String::Convert<long double>("abc"), std::invalid_argument);
}

