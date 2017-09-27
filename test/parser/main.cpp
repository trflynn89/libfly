#include <array>
#include <deque>
#include <forward_list>
#include <fstream>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <gtest/gtest.h>

#include "fly/logger/logger.h"
#include "fly/parser/ini_parser.h"
#include "fly/parser/json_parser.h"
#include "fly/path/path.h"
#include "fly/string/string.h"

//==============================================================================
class IniParserTest : public ::testing::Test
{
public:
    IniParserTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt"),
        m_spParser(std::make_shared<fly::IniParser>(m_path, m_file))
    {
        LOGC("Using path '%s' : '%s'", m_path, m_file);
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
    /**
     * Create a file with the given contents.
     *
     * @param string Contents of the file to create.
     */
    void CreateFile(const std::string &contents)
    {
        {
            std::ofstream stream(GetFullPath(), std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(GetFullPath(), std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    /**
     * @return The full path to the file being monitored.
     */
    std::string GetFullPath() const
    {
        static const char sep = fly::Path::GetSeparator();
        return fly::String::Join(sep, m_path, m_file);
    }

    std::string m_path;
    std::string m_file;

    fly::IniParserPtr m_spParser;
};

//==============================================================================
TEST_F(IniParserTest, NonExistingPathTest)
{
    m_spParser = std::make_shared<fly::IniParser>(m_path + "foo", m_file);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, NonExistingFileTest)
{
    m_spParser = std::make_shared<fly::IniParser>(m_path, m_file + "foo");

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, EmptyFileTest)
{
    const std::string contents;
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, EmptySectionTest)
{
    const std::string contents("[section]");
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 0);
}

//==============================================================================
TEST_F(IniParserTest, NonEmptySectionTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 2);
}

//==============================================================================
TEST_F(IniParserTest, NonExistingTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetSize("section"), 2);
    EXPECT_EQ(m_spParser->GetSize("bad-section"), 0);
    EXPECT_EQ(m_spParser->GetSize("section-bad"), 0);
}

//==============================================================================
TEST_F(IniParserTest, CommentTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "; [other-section]\n"
        "; name=Jane Doe\n"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 1);
    EXPECT_EQ(m_spParser->GetSize("other-section"), 0);
}

//==============================================================================
TEST_F(IniParserTest, ErrantSpacesTest)
{
    const std::string contents(
        "   [section   ]  \n"
        "\t\t\n   name=John Doe\t  \n"
        "\taddress  = USA\t \r \n"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 2);
}

//==============================================================================
TEST_F(IniParserTest, QuotedValueTest)
{
    const std::string contents(
        "[section]\n"
        "name=\"  John Doe  \"\n"
        "address= \t '\tUSA'"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 2);
}

//==============================================================================
TEST_F(IniParserTest, MutlipleSectionTypeTest)
{
    const std::string contents(
        "[section1]\n"
        "name=John Doe\n"
        "age=26\n"
        "[section2]\n"
        "name=Jane Doe\n"
        "age=30.12\n"
        "[section3]\n"
        "name=Joe Doe\n"
        "noage=1\n"
    );

    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    EXPECT_EQ(m_spParser->GetSize(), 3);
    EXPECT_EQ(m_spParser->GetSize("section1"), 2);
    EXPECT_EQ(m_spParser->GetSize("section2"), 2);
    EXPECT_EQ(m_spParser->GetSize("section3"), 2);
}

//==============================================================================
TEST_F(IniParserTest, DuplicateSectionTest)
{
    const std::string contents1(
        "[section]\n"
        "name=John Doe\n"
        "[section]\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    const std::string contents2(
        "[  \tsection]\n"
        "name=John Doe\n"
        "[section  ]\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, DuplicateValueTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "name=Jane Doe\n"
    );

    CreateFile(contents);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, ImbalancedBraceTest)
{
    const std::string contents1(
        "[section\n"
        "name=John Doe\n"
    );

    const std::string contents2(
        "section]\n"
        "name=John Doe\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, ImbalancedQuoteTest)
{
    const std::string contents1(
        "[section]\n"
        "name=\"John Doe\n"
    );

    const std::string contents2(
        "[section]\n"
        "name=John Doe\"\n"
    );
    const std::string contents3(
        "[section]\n"
        "name='John Doe\n"
    );

    const std::string contents4(
        "[section]\n"
        "name=John Doe'\n"
    );

    const std::string contents5(
        "[section]\n"
        "name=\"John Doe'\n"
    );

    const std::string contents6(
        "[section]\n"
        "name='John Doe\"\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents3);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents4);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents5);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents6);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, MisplacedQuoteTest)
{
    const std::string contents1(
        "[section]\n"
        "\"name\"=John Doe\n"
    );

    const std::string contents2(
        "[section]\n"
        "\'name\'=John Doe\n"
    );

    const std::string contents3(
        "[\"section\"]\n"
        "name=John Doe\n"
    );

    const std::string contents4(
        "[\'section\']\n"
        "name=John Doe\n"
    );

    const std::string contents5(
        "\"[section]\"\n"
        "name=John Doe\n"
    );

    const std::string contents6(
        "\'[section]\'\n"
        "name=John Doe\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents3);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents4);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents5);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents6);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, MultipleAssignmentTest)
{
    const std::string contents1(
        "[section]\n"
        "name=John=Doe\n"
    );
    const std::string contents2(
        "[section]\n"
        "name=\"John=Doe\"\n"
    );

    CreateFile(contents1);
    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 1);

    CreateFile(contents2);
    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_EQ(m_spParser->GetSize(), 1);
    EXPECT_EQ(m_spParser->GetSize("section"), 1);
}

//==============================================================================
TEST_F(IniParserTest, MissingAssignmentTest)
{
    const std::string contents1(
        "[section]\n"
        "name\n"
    );

    const std::string contents2(
        "[section]\n"
        "name=\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, EarlyAssignmentTest)
{
    const std::string contents1(
        "name=John Doe\n"
        "[section]\n"
    );

    const std::string contents2(
        "name=\n"
        "[section]\n"
    );

    const std::string contents3(
        "name\n"
        "[section]\n"
    );

    CreateFile(contents1);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents2);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile(contents3);
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(IniParserTest, MultipleParseTest)
{
    const std::string contents(
        "[section]\n"
        "name=John Doe\n"
        "address=USA"
    );

    CreateFile(contents);

    for (int i = 0; i < 5; ++i)
    {
        ASSERT_NO_THROW(m_spParser->Parse());

        EXPECT_EQ(m_spParser->GetSize(), 1);
        EXPECT_EQ(m_spParser->GetSize("section"), 2);
    }
}

//==============================================================================
TEST(JsonTest, StringConstructorTest)
{
    const std::string str1("a");
    EXPECT_TRUE(fly::Json(str1).IsString());

    std::string str2("b");
    EXPECT_TRUE(fly::Json(str2).IsString());

    const char *cstr1 = "c";
    EXPECT_TRUE(fly::Json(cstr1).IsString());

    char *cstr2 = (char *)"d";
    EXPECT_TRUE(fly::Json(cstr2).IsString());

    const char arr1[] = { 'g', '\0' };
    EXPECT_TRUE(fly::Json(arr1).IsString());

    char arr2[] = { 'h', '\0' };
    EXPECT_TRUE(fly::Json(arr2).IsString());
}

//==============================================================================
TEST(JsonTest, ObjectConstructorTest)
{
    std::map<std::string, int> map = { { "a", 1 }, { "b", 2 } };
    EXPECT_TRUE(fly::Json(map).IsObject());

    std::multimap<std::string, int> multimap = { { "c", 3 }, { "d", 4 } };
    EXPECT_TRUE(fly::Json(multimap).IsObject());

    std::unordered_map<std::string, int> unordered_map = { { "e", 5 }, { "f", 6 } };
    EXPECT_TRUE(fly::Json(unordered_map).IsObject());

    std::unordered_multimap<std::string, int> unordered_multimap = { { "h", 7 }, { "i", 8 } };
    EXPECT_TRUE(fly::Json(unordered_multimap).IsObject());
}

//==============================================================================
TEST(JsonTest, ArrayConstructorTest)
{
    std::array<int, 4> array = { 10, 20, 30, 40 };
    EXPECT_TRUE(fly::Json(array).IsArray());
    EXPECT_FALSE(fly::Json(array).IsObjectLike());

    std::deque<int> deque = { 50, 60, 70, 80 };
    EXPECT_TRUE(fly::Json(deque).IsArray());
    EXPECT_FALSE(fly::Json(deque).IsObjectLike());

    std::forward_list<int> forward_list = { 90, 100, 110, 120 };
    EXPECT_TRUE(fly::Json(forward_list).IsArray());
    EXPECT_FALSE(fly::Json(forward_list).IsObjectLike());

    std::list<int> list = { 130, 140, 150, 160 };
    EXPECT_TRUE(fly::Json(list).IsArray());
    EXPECT_FALSE(fly::Json(list).IsObjectLike());

    std::multiset<std::string> multiset = { "a", "b", "c" };
    EXPECT_TRUE(fly::Json(multiset).IsArray());
    EXPECT_FALSE(fly::Json(multiset).IsObjectLike());

    std::set<std::string> set = { "d", "e", "f" };
    EXPECT_TRUE(fly::Json(set).IsArray());
    EXPECT_FALSE(fly::Json(set).IsObjectLike());

    std::unordered_multiset<std::string> unordered_multiset = { "g", "h", "i" };
    EXPECT_TRUE(fly::Json(unordered_multiset).IsArray());
    EXPECT_FALSE(fly::Json(unordered_multiset).IsObjectLike());

    std::unordered_set<std::string> unordered_set = { "j", "k", "l" };
    EXPECT_TRUE(fly::Json(unordered_set).IsArray());
    EXPECT_FALSE(fly::Json(unordered_set).IsObjectLike());

    std::vector<int> vector = { 170, 180, 190, 200 };
    EXPECT_TRUE(fly::Json(vector).IsArray());
    EXPECT_FALSE(fly::Json(vector).IsObjectLike());

    std::array<std::string, 2> object = { "nine", "ten" };
    EXPECT_TRUE(fly::Json(object).IsArray());
    EXPECT_TRUE(fly::Json(object).IsObjectLike());
}

//==============================================================================
TEST(JsonTest, BooleanConstructorTest)
{
    EXPECT_TRUE(fly::Json(true).IsBoolean());
    EXPECT_TRUE(fly::Json(false).IsBoolean());
}

//==============================================================================
TEST(JsonTest, SignedIntegerConstructorTest)
{
    EXPECT_TRUE(fly::Json(static_cast<char>(1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<short>(1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<int>(1)).IsSignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<int>(-1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::int32_t>(1)).IsSignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::int32_t>(-1)).IsSignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::int64_t>(1)).IsSignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::int64_t>(-1)).IsSignedInteger());
}

//==============================================================================
TEST(JsonTest, UnsignedIntegerConstructorTest)
{
    EXPECT_TRUE(fly::Json(static_cast<unsigned char>(1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<unsigned short>(1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(1)).IsUnsignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<unsigned int>(-1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(1)).IsUnsignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::uint32_t>(-1)).IsUnsignedInteger());

    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(1)).IsUnsignedInteger());
    EXPECT_TRUE(fly::Json(static_cast<std::uint64_t>(-1)).IsUnsignedInteger());
}

//==============================================================================
TEST(JsonTest, FloatConstructorTest)
{
    EXPECT_TRUE(fly::Json(static_cast<float>(1.0)).IsFloat());
    EXPECT_TRUE(fly::Json(static_cast<double>(1.0)).IsFloat());
    EXPECT_TRUE(fly::Json(static_cast<long double>(1.0)).IsFloat());
}

//==============================================================================
TEST(JsonTest, NullConstructorTest)
{
    EXPECT_TRUE(fly::Json().IsNull());
    EXPECT_TRUE(fly::Json(nullptr).IsNull());
}

//==============================================================================
TEST(JsonTest, InitializerListConstructorTest)
{
    const fly::Json empty = { };
    EXPECT_TRUE(fly::Json(empty).IsNull());

    const fly::Json array = { '7', 8, "nine", 10 };
    EXPECT_TRUE(fly::Json(array).IsArray());

    const fly::Json object = { { "a", 1 }, { "b", 2 } };
    EXPECT_TRUE(fly::Json(object).IsObject());

    const fly::Json almost = { { "a", 1 }, { "b", 2 }, 4 };
    EXPECT_TRUE(fly::Json(almost).IsArray());
}

//==============================================================================
TEST(JsonTest, CopyConstructorTest)
{
    fly::Json string = "abc";
    EXPECT_EQ(fly::Json(string), string);

    fly::Json object = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(fly::Json(object), object);

    fly::Json array = { '7', 8 };
    EXPECT_EQ(fly::Json(array), array);

    fly::Json boolean = true;
    EXPECT_EQ(fly::Json(boolean), boolean);

    fly::Json sign = 1;
    EXPECT_EQ(fly::Json(sign), sign);

    fly::Json unsign = static_cast<unsigned int>(1);
    EXPECT_EQ(fly::Json(unsign), unsign);

    fly::Json floating = 1.0f;
    EXPECT_EQ(fly::Json(floating), floating);

    fly::Json null = nullptr;
    EXPECT_EQ(fly::Json(null), null);
}

//==============================================================================
TEST(JsonTest, AssignmentTest)
{
    fly::Json json;

    fly::Json string = "abc";
    json = string;
    EXPECT_EQ(json, string);

    fly::Json object = { { "a", 1 }, { "b", 2 } };
    json = object;
    EXPECT_EQ(json, object);

    fly::Json array = { '7', 8 };
    json = array;
    EXPECT_EQ(json, array);

    fly::Json boolean = true;
    json = boolean;
    EXPECT_EQ(json, boolean);

    fly::Json sign = 1;
    json = sign;
    EXPECT_EQ(json, sign);

    fly::Json unsign = static_cast<unsigned int>(1);
    json = unsign;
    EXPECT_EQ(json, unsign);

    fly::Json floating = 1.0f;
    json = floating;
    EXPECT_EQ(json, floating);

    fly::Json null = nullptr;
    json = null;
    EXPECT_EQ(json, null);
}

//==============================================================================
TEST(JsonTest, ObjectAccessTest)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1["a"], fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2["a"], fly::JsonException);

    fly::Json object1 = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(object1["a"], 1);
    EXPECT_EQ(object1["b"], 2);
    EXPECT_NO_THROW(object1["c"]);
    EXPECT_EQ(object1["c"], nullptr);

    const fly::Json object2 = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(object2["a"], 1);
    EXPECT_EQ(object2["b"], 2);
    EXPECT_THROW(object2["c"], fly::JsonException);

    fly::Json array1 = { '7', 8 };
    EXPECT_THROW(array1["a"], fly::JsonException);

    const fly::Json array2 = { '7', 8 };
    EXPECT_THROW(array2["a"], fly::JsonException);

    fly::Json bool1 = true;
    EXPECT_THROW(bool1["a"], fly::JsonException);

    const fly::Json bool2 = true;
    EXPECT_THROW(bool2["a"], fly::JsonException);

    fly::Json signed1 = 1;
    EXPECT_THROW(signed1["a"], fly::JsonException);

    const fly::Json signed2 = 1;
    EXPECT_THROW(signed2["a"], fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned1["a"], fly::JsonException);

    const fly::Json unsigned2 = static_cast<unsigned int>(1);;
    EXPECT_THROW(unsigned2["a"], fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1["a"], fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2["a"], fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_NO_THROW(null1["a"]);
    EXPECT_TRUE(null1.IsObject());
    EXPECT_EQ(null1["a"], nullptr);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2["a"], fly::JsonException);
}

//==============================================================================
TEST(JsonTest, ArrayAccessTest)
{
    fly::Json string1 = "abc";
    EXPECT_THROW(string1[0], fly::JsonException);

    const fly::Json string2 = "abc";
    EXPECT_THROW(string2[0], fly::JsonException);

    fly::Json object1 = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW(object1[0], fly::JsonException);

    const fly::Json object2 = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW(object2[0], fly::JsonException);

    fly::Json array1 = { '7', 8 };
    EXPECT_EQ(array1[0], '7');
    EXPECT_EQ(array1[1], 8);
    EXPECT_NO_THROW(array1[2]);
    EXPECT_EQ(array1[2], nullptr);

    const fly::Json array2 = { '7', 8 };
    EXPECT_EQ(array2[0], '7');
    EXPECT_EQ(array2[1], 8);
    EXPECT_THROW(array2[2], fly::JsonException);

    fly::Json bool1 = true;
    EXPECT_THROW(bool1[0], fly::JsonException);

    const fly::Json bool2 = true;
    EXPECT_THROW(bool2[0], fly::JsonException);

    fly::Json signed1 = 1;
    EXPECT_THROW(signed1[0], fly::JsonException);

    const fly::Json signed2 = 1;
    EXPECT_THROW(signed2[0], fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    EXPECT_THROW(unsigned1[0], fly::JsonException);

    const fly::Json unsigned2 = static_cast<unsigned int>(1);;
    EXPECT_THROW(unsigned2[0], fly::JsonException);

    fly::Json float1 = 1.0f;
    EXPECT_THROW(float1[0], fly::JsonException);

    const fly::Json float2 = 1.0f;
    EXPECT_THROW(float2[0], fly::JsonException);

    fly::Json null1 = nullptr;
    EXPECT_NO_THROW(null1[0]);
    EXPECT_TRUE(null1.IsArray());
    EXPECT_EQ(null1[0], nullptr);

    const fly::Json null2 = nullptr;
    EXPECT_THROW(null2[0], fly::JsonException);
}

//==============================================================================
TEST(JsonTest, EqualityTest)
{
    fly::Json string1 = "abc";
    fly::Json string2 = "abc";
    fly::Json string3 = "def";

    fly::Json object1 = { { "a", 1 }, { "b", 2 } };
    fly::Json object2 = { { "a", 1 }, { "b", 2 } };
    fly::Json object3 = { { "a", 1 }, { "b", 3 } };

    fly::Json array1 = { '7', 8 };
    fly::Json array2 = { '7', 8 };
    fly::Json array3 = { '7', 9 };

    fly::Json bool1 = true;
    fly::Json bool2 = true;
    fly::Json bool3 = false;

    fly::Json signed1 = 1;
    fly::Json signed2 = 1;
    fly::Json signed3 = 0;

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    fly::Json unsigned2 = static_cast<unsigned int>(1);
    fly::Json unsigned3 = static_cast<unsigned int>(0);

    fly::Json float1 = 1.0f;
    fly::Json float2 = 1.0f;
    fly::Json float3 = 2.5f;

    EXPECT_EQ(string1, string1);
    EXPECT_EQ(string1, string2);
    EXPECT_NE(string1, string3);
    EXPECT_NE(string1, object1);
    EXPECT_NE(string1, array1);
    EXPECT_NE(string1, bool1);
    EXPECT_NE(string1, signed1);
    EXPECT_NE(string1, unsigned1);
    EXPECT_NE(string1, float1);

    EXPECT_EQ(object1, object1);
    EXPECT_EQ(object1, object2);
    EXPECT_NE(object1, object3);
    EXPECT_NE(object1, string1);
    EXPECT_NE(object1, array1);
    EXPECT_NE(object1, bool1);
    EXPECT_NE(object1, signed1);
    EXPECT_NE(object1, unsigned1);
    EXPECT_NE(object1, float1);

    EXPECT_EQ(array1, array1);
    EXPECT_EQ(array1, array2);
    EXPECT_NE(array1, array3);
    EXPECT_NE(array1, string1);
    EXPECT_NE(array1, object1);
    EXPECT_NE(array1, bool1);
    EXPECT_NE(array1, signed1);
    EXPECT_NE(array1, unsigned1);
    EXPECT_NE(array1, float1);

    EXPECT_EQ(bool1, bool1);
    EXPECT_EQ(bool1, bool2);
    EXPECT_NE(bool1, bool3);
    EXPECT_NE(bool1, string1);
    EXPECT_NE(bool1, object1);
    EXPECT_NE(bool1, array1);
    EXPECT_NE(bool1, signed1);
    EXPECT_NE(bool1, unsigned1);
    EXPECT_NE(bool1, float1);

    EXPECT_EQ(signed1, signed1);
    EXPECT_EQ(signed1, signed2);
    EXPECT_NE(signed1, signed3);
    EXPECT_NE(signed1, string1);
    EXPECT_NE(signed1, object1);
    EXPECT_NE(signed1, array1);
    EXPECT_NE(signed1, bool1);
    EXPECT_EQ(signed1, unsigned1);
    EXPECT_NE(signed1, unsigned3);
    EXPECT_EQ(signed1, float1);
    EXPECT_NE(signed1, float3);

    EXPECT_EQ(unsigned1, unsigned1);
    EXPECT_EQ(unsigned1, unsigned2);
    EXPECT_NE(unsigned1, unsigned3);
    EXPECT_NE(unsigned1, string1);
    EXPECT_NE(unsigned1, object1);
    EXPECT_NE(unsigned1, array1);
    EXPECT_NE(unsigned1, bool1);
    EXPECT_EQ(unsigned1, signed1);
    EXPECT_NE(unsigned1, signed3);
    EXPECT_EQ(unsigned1, float1);
    EXPECT_NE(unsigned1, float3);

    EXPECT_EQ(float1, float1);
    EXPECT_EQ(float1, float2);
    EXPECT_NE(float1, float3);
    EXPECT_NE(float1, string1);
    EXPECT_NE(float1, object1);
    EXPECT_NE(float1, array1);
    EXPECT_NE(float1, bool1);
    EXPECT_EQ(float1, signed1);
    EXPECT_NE(float1, signed3);
    EXPECT_EQ(float1, unsigned1);
    EXPECT_NE(float1, unsigned3);
}

//==============================================================================
TEST(JsonTest, StreamTest)
{
    std::stringstream stream;

    fly::Json string = "abc";
    fly::Json object = { { "a", 1 }, { "b", 2 } };
    fly::Json array = { '7', 8 };
    fly::Json boolean = true;
    fly::Json sign = 1;
    fly::Json unsign = static_cast<unsigned int>(1);
    fly::Json floating = 1.0f;
    fly::Json null = nullptr;

    stream << string;
    EXPECT_EQ(stream.str(), "\"abc\"");
    stream.str(std::string());

    stream << object;
    EXPECT_EQ(stream.str(), "{ \"a\" : 1, \"b\" : 2 }");
    stream.str(std::string());

    stream << array;
    EXPECT_EQ(stream.str(), "[ 55, 8 ]");
    stream.str(std::string());

    stream << boolean;
    EXPECT_EQ(stream.str(), "true");
    stream.str(std::string());

    stream << sign;
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    stream << unsign;
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    stream << floating;
    EXPECT_EQ(stream.str(), "1");
    stream.str(std::string());

    stream << null;
    EXPECT_EQ(stream.str(), "null");
    stream.str(std::string());
}

//==============================================================================
TEST(JsonExceptionTest, ExceptionTest)
{
    std::stringstream stream;
    fly::Json string = "abc";
    stream << string;

    bool thrown = false;

    try
    {
        throw fly::JsonException(string, "some message");
    }
    catch (const fly::JsonException &e)
    {
        std::string what(e.what());

        std::string expect("*some message*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::WildcardMatch(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==============================================================================
TEST(JsonParserTest, Test)
{
    auto spParser = std::make_shared<fly::JsonParser>("/tmp", "test.json");
    spParser->Parse();
}
