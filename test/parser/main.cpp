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
TEST(ParserExceptionTest, ExceptionTest)
{
    std::string file("test_file");
    int line = 123;
    std::string message("Bad file!");

    try
    {
        throw fly::ParserException(file, line, message);
    }
    catch (const fly::ParserException &ex)
    {
        std::string what(ex.what());

        EXPECT_NE(what.find(file), std::string::npos);
        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_NE(what.find(message), std::string::npos);
    }
}

//==============================================================================
class ParserTest : public ::testing::Test
{
public:
    ParserTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_file(fly::String::GenerateRandomString(10) + ".txt")
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
        const std::string path = fly::Path::Join(m_path, m_file);
        {
            std::ofstream stream(path, std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(path, std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    std::string m_path;
    std::string m_file;
};

//==============================================================================
class IniParserTest : public ParserTest
{
public:
    IniParserTest() :
        ParserTest(),
        m_spParser(std::make_shared<fly::IniParser>(m_path, m_file))
    {
    }

protected:
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
TEST(JsonTest, StringConversionTest)
{
    fly::Json json;

    std::string string = "abc";
    json = string;
    EXPECT_EQ(std::string(json), string);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(std::string(json), "{ \"a\" : 1, \"b\" : 2 }");

    json = { '7', 8 };
    EXPECT_EQ(std::string(json), "[ 55, 8 ]");

    json = true;
    EXPECT_EQ(std::string(json), "true");

    json = 1;
    EXPECT_EQ(std::string(json), "1");

    json = static_cast<unsigned int>(1);
    EXPECT_EQ(std::string(json), "1");

    json = 1.0f;
    EXPECT_EQ(std::string(json), "1");

    json = nullptr;
    EXPECT_EQ(std::string(json), "null");
}

//==============================================================================
TEST(JsonTest, ObjectConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    std::map<std::string, int> map = { { "a", 1 }, { "b", 2 } };
    std::multimap<std::string, int> multimap(map.begin(), map.end());
    json = map;
    EXPECT_EQ((std::map<std::string, int>(json)), map);
    EXPECT_EQ((std::multimap<std::string, int>(json)), multimap);

    std::map<std::string, int> empty = { };
    std::multimap<std::string, int> multiempty(empty.begin(), empty.end());
    json = empty;
    EXPECT_EQ((std::map<std::string, int>(json)), empty);
    EXPECT_EQ((std::multimap<std::string, int>(json)), multiempty);

    json = { '7', 8 };
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = 1;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);

    json = nullptr;
    EXPECT_THROW((std::map<std::string, fly::Json>(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, ArrayConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    std::vector<int> vector = { 7, 8 };
    std::array<int, 1> array1 = { 7 };
    std::array<int, 2> array2 = { 7, 8 };
    std::array<int, 3> array3 = { 7, 8, 0 };
    json = vector;
    EXPECT_EQ((std::vector<int>(json)), vector);
    EXPECT_EQ((std::array<int, 1>(json)), array1);
    EXPECT_EQ((std::array<int, 2>(json)), array2);
    EXPECT_EQ((std::array<int, 3>(json)), array3);

    std::vector<int> empty = { };
    std::array<int, 1> empty1 = { };
    std::array<int, 2> empty2 = { };
    std::array<int, 3> empty3 = { };
    json = empty;
    EXPECT_EQ((std::vector<int>(json)), empty);
    EXPECT_EQ((std::array<int, 1>(json)), empty1);
    EXPECT_EQ((std::array<int, 2>(json)), empty2);
    EXPECT_EQ((std::array<int, 3>(json)), empty3);

    json = true;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = 1;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = static_cast<unsigned int>(1);
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = 1.0f;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);

    json = nullptr;
    EXPECT_THROW((std::vector<int>(json)), fly::JsonException);
    EXPECT_THROW((std::array<int, 1>(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, BooleanConversionTest)
{
    fly::Json json;

    json = "";
    EXPECT_FALSE(bool(json));
    json = "abc";
    EXPECT_TRUE(bool(json));

    json = std::map<std::string, int>();
    EXPECT_FALSE(bool(json));
    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_TRUE(bool(json));

    json = std::vector<int>();
    EXPECT_FALSE(bool(json));
    json = { 7, 8 };
    EXPECT_TRUE(bool(json));

    json = true;
    EXPECT_TRUE(bool(json));
    json = false;
    EXPECT_FALSE(bool(json));

    json = 1;
    EXPECT_TRUE(bool(json));
    json = 0;
    EXPECT_FALSE(bool(json));

    json = static_cast<unsigned int>(1);
    EXPECT_TRUE(bool(json));
    json = static_cast<unsigned int>(0);
    EXPECT_FALSE(bool(json));

    json = 1.0f;
    EXPECT_TRUE(bool(json));
    json = 0.0f;
    EXPECT_FALSE(bool(json));

    json = nullptr;
    EXPECT_FALSE(bool(json));
}

//==============================================================================
TEST(JsonTest, SignedIntegerConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((int(json)), fly::JsonException);

    json = "123";
    EXPECT_EQ(int(json), 123);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((int(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((int(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((int(json)), fly::JsonException);

    char ch = 'a';
    json = ch;
    EXPECT_EQ(char(json), ch);

    int sign = 12;
    json = sign;
    EXPECT_EQ(int(json), sign);

    unsigned int unsign = static_cast<unsigned int>(12);
    json = unsign;
    EXPECT_EQ(int(json), int(unsign));

    float floating = 3.14f;
    json = floating;
    EXPECT_EQ(int(json), int(floating));

    json = nullptr;
    EXPECT_THROW((int(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, UnsignedIntegerConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    json = "123";
    EXPECT_EQ(unsigned(json), unsigned(123));

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((unsigned(json)), fly::JsonException);

    char ch = 'a';
    json = ch;
    EXPECT_EQ(static_cast<unsigned char>(json), static_cast<unsigned char>(ch));

    int sign = 12;
    json = sign;
    EXPECT_EQ(unsigned(json), unsigned(sign));

    unsigned int unsign = static_cast<unsigned int>(12);
    json = unsign;
    EXPECT_EQ(unsigned(json), unsign);

    float floating = 3.14f;
    json = floating;
    EXPECT_EQ(unsigned(json), unsigned(floating));

    json = nullptr;
    EXPECT_THROW((unsigned(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, FloatConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((float(json)), fly::JsonException);

    json = "123.5";
    EXPECT_EQ(float(json), 123.5f);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((float(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((float(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((float(json)), fly::JsonException);

    char ch = 'a';
    json = ch;
    EXPECT_EQ(float(json), float(ch));

    int sign = 12;
    json = sign;
    EXPECT_EQ(float(json), float(sign));

    unsigned int unsign = static_cast<unsigned int>(12);
    json = unsign;
    EXPECT_EQ(float(json), float(unsign));

    float floating = 3.14f;
    json = floating;
    EXPECT_EQ(float(json), floating);

    json = nullptr;
    EXPECT_THROW((float(json)), fly::JsonException);
}

//==============================================================================
TEST(JsonTest, NullConversionTest)
{
    fly::Json json;

    json = "abc";
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = { 7, 8 };
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = true;
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = 'a';
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = 12;
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = static_cast<unsigned int>(12);
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = 3.14f;
    EXPECT_THROW((std::nullptr_t(json)), fly::JsonException);

    json = nullptr;
    EXPECT_EQ((std::nullptr_t(json)), nullptr);
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
TEST(JsonTest, SizeTest)
{
    fly::Json json;

    json = "abcdef";
    EXPECT_EQ(json.Size(), 6);

    json = { { "a", 1 }, { "b", 2 } };
    EXPECT_EQ(json.Size(), 2);

    json = { '7', 8, 9, 10 };
    EXPECT_EQ(json.Size(), 4);

    json = true;
    EXPECT_EQ(json.Size(), 1);

    json = 1;
    EXPECT_EQ(json.Size(), 1);

    json = static_cast<unsigned int>(1);
    EXPECT_EQ(json.Size(), 1);

    json = 1.0f;
    EXPECT_EQ(json.Size(), 1);

    json = nullptr;
    EXPECT_EQ(json.Size(), 0);
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
class JsonParserTest : public ParserTest
{
public:
    JsonParserTest() :
        ParserTest(),
        m_spParser(std::make_shared<fly::JsonParser>(m_path, m_file))
    {
    }

protected:
    fly::JsonParserPtr m_spParser;
};

//==============================================================================
TEST_F(JsonParserTest, JsonCheckerTest)
{
    // Run the parser against test files from http://www.json.org/JSON_checker/
    // The following files are excluded from this test:
    //      - fail18.json: The parser has no max-depth

    // Get the path to this file
    std::vector<std::string> segments = fly::Path::Split(__FILE__);
    ASSERT_FALSE(segments.empty());
    segments.back() = "json_checker";

    std::string path;

    for (const std::string &segment : segments)
    {
        path = fly::Path::Join(path, segment);
    }

    // Validate each JSON file in the JSON checker directory
    std::vector<std::string> directories;
    std::vector<std::string> files;

    ASSERT_TRUE(fly::Path::ListPath(path, directories, files));

    for (const std::string &file : files)
    {
        m_spParser = std::make_shared<fly::JsonParser>(path, file);
        SCOPED_TRACE(file);

        if (fly::String::WildcardMatch(file, "pass*.json"))
        {
            EXPECT_NO_THROW(m_spParser->Parse());
        }
        else if (fly::String::WildcardMatch(file, "fail*.json"))
        {
            EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
        }
        else
        {
            FAIL() << "Unrecognized JSON file: " << file;
        }
    }
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingPathTest)
{
    m_spParser = std::make_shared<fly::JsonParser>(m_path + "foo", m_file);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_TRUE(m_spParser->GetJson().IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, NonExistingFileTest)
{
    m_spParser = std::make_shared<fly::JsonParser>(m_path, m_file + "foo");

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_TRUE(m_spParser->GetJson().IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, EmptyFileTest)
{
    const std::string contents;
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());
    EXPECT_TRUE(m_spParser->GetJson().IsNull());
}

//==============================================================================
TEST_F(JsonParserTest, EmptyObjectTest)
{
    const std::string contents("{}");
    CreateFile(contents);

    ASSERT_NO_THROW(m_spParser->Parse());

    fly::Json json = m_spParser->GetJson();
    EXPECT_TRUE(json.IsObject());
    EXPECT_TRUE(json.Size() == 0);
}

//==============================================================================
TEST_F(JsonParserTest, NonObjectOrArrayTest)
{
    CreateFile("\"\"");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("true");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("1");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("-1");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("3.14");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("null");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, BadlyFormedObjectTest)
{
    CreateFile("{");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("}");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ a\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ a\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" { }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : { }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" [");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : [");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" ]");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : ]");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" tru }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : tru }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" flse }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : flse }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" 1, }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : 1, }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, WhiteSpaceTest)
{
    CreateFile("{ \"a\" : 1 }");
    EXPECT_NO_THROW(m_spParser->Parse());

    CreateFile("\n{ \n \"a\" \n : \n \t\t 1 \r \n }\n");
    EXPECT_NO_THROW(m_spParser->Parse());

    CreateFile("{ \"a\t\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\n\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\r\" : 1 }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : \"b\n\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : \"b\r\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);

    CreateFile("{ \"a\" : \"b\t\" }");
    EXPECT_THROW(m_spParser->Parse(), fly::ParserException);
}

//==============================================================================
TEST_F(JsonParserTest, Test)
{
    auto spParser = std::make_shared<fly::JsonParser>("/tmp", "test.json");
    spParser->Parse();
}
