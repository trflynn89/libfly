#include "fly/types/json/json.hpp"

#include "test/types/json_macros.hpp"

#include <catch2/catch.hpp>

#include <array>
#include <deque>
#include <forward_list>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

CATCH_TEST_CASE("Json", "[json]")
{
    CATCH_SECTION("Assign a JSON instance's value to another JSON instance's value")
    {
        fly::Json json;

        fly::Json string = "abc";
        json = string;
        CATCH_CHECK(json == string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        json = object;
        CATCH_CHECK(json == object);

        fly::Json array = {'7', 8};
        json = array;
        CATCH_CHECK(json == array);

        fly::Json boolean = true;
        json = boolean;
        CATCH_CHECK(json == boolean);

        fly::Json sign = 1;
        json = sign;
        CATCH_CHECK(json == sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        json = unsign;
        CATCH_CHECK(json == unsign);

        fly::Json floating = 1.0f;
        json = floating;
        CATCH_CHECK(json == floating);

        fly::Json null = nullptr;
        json = null;
        CATCH_CHECK(json == null);
    }

    CATCH_SECTION("Access a JSON array's values via the access operator")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(string1[0], "JSON type invalid for operator[index]: (%s)", string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(string2[0], "JSON type invalid for operator[index]: (%s)", string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(object1[0], "JSON type invalid for operator[index]: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(object2[0], "JSON type invalid for operator[index]: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK(array1[0] == '7');
        CATCH_CHECK(array1[1] == 8);
        CATCH_CHECK_NOTHROW(array1[2]);
        CATCH_CHECK(array1[2] == nullptr);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK(array2[0] == '7');
        CATCH_CHECK(array2[1] == 8);
        CATCH_CHECK_THROWS_JSON(array2[2], "Given index (2) not found: (%s)", array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(bool1[0], "JSON type invalid for operator[index]: (%s)", bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(bool2[0], "JSON type invalid for operator[index]: (%s)", bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(signed1[0], "JSON type invalid for operator[index]: (%s)", signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(signed2[0], "JSON type invalid for operator[index]: (%s)", signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1[0],
            "JSON type invalid for operator[index]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2[0],
            "JSON type invalid for operator[index]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(float1[0], "JSON type invalid for operator[index]: (%s)", float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(float2[0], "JSON type invalid for operator[index]: (%s)", float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_NOTHROW(null1[0]);
        CATCH_CHECK(null1.is_array());
        CATCH_CHECK(null1[0] == nullptr);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(null2[0], "JSON type invalid for operator[index]: (%s)", null2);
    }

    CATCH_SECTION("Access a JSON array's values via the accessor 'at'")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(
            object1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_THROWS_JSON(
            object2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK(array1.at(0) == '7');
        CATCH_CHECK(array1.at(1) == 8);
        CATCH_CHECK_THROWS_JSON(array1.at(2), "Given index (2) not found: (%s)", array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK(array2.at(0) == '7');
        CATCH_CHECK(array2.at(1) == 8);
        CATCH_CHECK_THROWS_JSON(array2.at(2), "Given index (2) not found: (%s)", array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(bool1.at(0), "JSON type invalid for operator[index]: (%s)", bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(bool2.at(0), "JSON type invalid for operator[index]: (%s)", bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.at(0),
            "JSON type invalid for operator[index]: (%s)",
            float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float2.at(0),
            "JSON type invalid for operator[index]: (%s)",
            float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(null1.at(0), "JSON type invalid for operator[index]: (%s)", null1);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(null2.at(0), "JSON type invalid for operator[index]: (%s)", null2);
    }

    CATCH_SECTION("Check JSON instances for emptiness")
    {
        fly::Json json;

        json = "abcdef";
        CATCH_CHECK_FALSE(json.empty());

        json = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_FALSE(json.empty());

        json = {'7', 8, 9, 10};
        CATCH_CHECK_FALSE(json.empty());

        json = true;
        CATCH_CHECK_FALSE(json.empty());

        json = 1;
        CATCH_CHECK_FALSE(json.empty());

        json = static_cast<unsigned int>(1);
        CATCH_CHECK_FALSE(json.empty());

        json = 1.0f;
        CATCH_CHECK_FALSE(json.empty());

        json = nullptr;
        CATCH_CHECK(json.empty());

        json = "";
        CATCH_CHECK(json.empty());

        json = fly::JsonTraits::object_type();
        CATCH_CHECK(json.empty());

        json = fly::JsonTraits::array_type();
        CATCH_CHECK(json.empty());
    }

    CATCH_SECTION("Check the size of JSON instances")
    {
        fly::Json json;

        json = "abcdef";
        CATCH_CHECK(json.size() == 6);

        json = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(json.size() == 2);

        json = {'7', 8, 9, 10};
        CATCH_CHECK(json.size() == 4);

        json = true;
        CATCH_CHECK(json.size() == 1);

        json = 1;
        CATCH_CHECK(json.size() == 1);

        json = static_cast<unsigned int>(1);
        CATCH_CHECK(json.size() == 1);

        json = 1.0f;
        CATCH_CHECK(json.size() == 1);

        json = nullptr;
        CATCH_CHECK(json.size() == 0);
    }

    CATCH_SECTION("Check the iterator at the beginning of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto begin1 = json1.begin();
        CATCH_CHECK(*begin1 == 1);
        CATCH_CHECK_FALSE(std::is_const_v<decltype(begin1)::value_type>);

        auto cbegin1 = json1.cbegin();
        CATCH_CHECK(*cbegin1 == 1);
        CATCH_CHECK(std::is_const_v<decltype(cbegin1)::value_type>);

        auto begin2 = json2.begin();
        CATCH_CHECK(*begin2 == 4);
        CATCH_CHECK(std::is_const_v<decltype(begin2)::value_type>);

        auto cbegin2 = json2.cbegin();
        CATCH_CHECK(*cbegin2 == 4);
        CATCH_CHECK(begin2 == cbegin2);
        CATCH_CHECK(std::is_const_v<decltype(cbegin2)::value_type>);
    }

    CATCH_SECTION("Check the iterator at the end of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto end1 = json1.end();
        CATCH_CHECK(*(end1 - 1) == 3);
        CATCH_CHECK_FALSE(std::is_const_v<decltype(end1)::value_type>);

        auto cend1 = json1.cend();
        CATCH_CHECK(*(cend1 - 1) == 3);
        CATCH_CHECK(std::is_const_v<decltype(cend1)::value_type>);

        auto end2 = json2.end();
        CATCH_CHECK(*(end2 - 1) == 6);
        CATCH_CHECK(std::is_const_v<decltype(end2)::value_type>);

        auto cend2 = json2.cend();
        CATCH_CHECK(*(cend2 - 1) == 6);
        CATCH_CHECK(end2 == cend2);
        CATCH_CHECK(std::is_const_v<decltype(cend2)::value_type>);
    }

    CATCH_SECTION("Iterate over a JSON object using plain interators")
    {
        fly::Json json {{"a", 1}, {"b", 2}};
        {
            fly::Json::size_type size = 0;

            for (auto it = json.begin(); it != json.end(); ++it, ++size)
            {
                CATCH_CHECK(*it == (size == 0 ? 1 : 2));
                CATCH_CHECK(it.key() == (size == 0 ? FLY_JSON_STR("a") : FLY_JSON_STR("b")));
                CATCH_CHECK(it.value() == (size == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
            {
                CATCH_CHECK(*it == (size == 0 ? 1 : 2));
                CATCH_CHECK(it.key() == (size == 0 ? FLY_JSON_STR("a") : FLY_JSON_STR("b")));
                CATCH_CHECK(it.value() == (size == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Iterate over a JSON object using range-based for loops")
    {
        fly::Json json {{"a", 1}, {"b", 2}};
        {
            fly::Json::size_type size = 0;

            for (fly::Json &value : json)
            {
                CATCH_CHECK(value == (size++ == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (const fly::Json &value : json)
            {
                CATCH_CHECK(value == (size++ == 0 ? 1 : 2));
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Iterate over a JSON array using plain interators")
    {
        fly::Json json {1, 2, 3};
        {
            fly::Json::size_type size = 0;

            for (auto it = json.begin(); it != json.end(); ++it, ++size)
            {
                CATCH_CHECK(*it == json[size]);
                CATCH_CHECK(it.value() == json[size]);
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (auto it = json.cbegin(); it != json.cend(); ++it, ++size)
            {
                CATCH_CHECK(*it == json[size]);
                CATCH_CHECK(it.value() == json[size]);
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Iterate over a JSON array using range-based for loops")
    {
        fly::Json json {1, 2, 3};
        {
            fly::Json::size_type size = 0;

            for (fly::Json &value : json)
            {
                CATCH_CHECK(value == json[size++]);
            }

            CATCH_CHECK(size == json.size());
        }
        {
            fly::Json::size_type size = 0;

            for (const fly::Json &value : json)
            {
                CATCH_CHECK(value == json[size++]);
            }

            CATCH_CHECK(size == json.size());
        }
    }

    CATCH_SECTION("Compare JSON instances for equality")
    {
        fly::Json string1 = "abc";
        fly::Json string2 = "abc";
        fly::Json string3 = "def";

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        fly::Json object2 = {{"a", 1}, {"b", 2}};
        fly::Json object3 = {{"a", 1}, {"b", 3}};

        fly::Json array1 = {'7', 8};
        fly::Json array2 = {'7', 8};
        fly::Json array3 = {'7', 9};

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

        CATCH_CHECK(string1 == string1);
        CATCH_CHECK(string1 == string2);
        CATCH_CHECK(string1 != string3);
        CATCH_CHECK(string1 != object1);
        CATCH_CHECK(string1 != array1);
        CATCH_CHECK(string1 != bool1);
        CATCH_CHECK(string1 != signed1);
        CATCH_CHECK(string1 != unsigned1);
        CATCH_CHECK(string1 != float1);

        CATCH_CHECK(object1 == object1);
        CATCH_CHECK(object1 == object2);
        CATCH_CHECK(object1 != object3);
        CATCH_CHECK(object1 != string1);
        CATCH_CHECK(object1 != array1);
        CATCH_CHECK(object1 != bool1);
        CATCH_CHECK(object1 != signed1);
        CATCH_CHECK(object1 != unsigned1);
        CATCH_CHECK(object1 != float1);

        CATCH_CHECK(array1 == array1);
        CATCH_CHECK(array1 == array2);
        CATCH_CHECK(array1 != array3);
        CATCH_CHECK(array1 != string1);
        CATCH_CHECK(array1 != object1);
        CATCH_CHECK(array1 != bool1);
        CATCH_CHECK(array1 != signed1);
        CATCH_CHECK(array1 != unsigned1);
        CATCH_CHECK(array1 != float1);

        CATCH_CHECK(bool1 == bool1);
        CATCH_CHECK(bool1 == bool2);
        CATCH_CHECK(bool1 != bool3);
        CATCH_CHECK(bool1 != string1);
        CATCH_CHECK(bool1 != object1);
        CATCH_CHECK(bool1 != array1);
        CATCH_CHECK(bool1 != signed1);
        CATCH_CHECK(bool1 != unsigned1);
        CATCH_CHECK(bool1 != float1);

        CATCH_CHECK(signed1 == signed1);
        CATCH_CHECK(signed1 == signed2);
        CATCH_CHECK(signed1 != signed3);
        CATCH_CHECK(signed1 != string1);
        CATCH_CHECK(signed1 != object1);
        CATCH_CHECK(signed1 != array1);
        CATCH_CHECK(signed1 != bool1);
        CATCH_CHECK(signed1 == unsigned1);
        CATCH_CHECK(signed1 != unsigned3);
        CATCH_CHECK(signed1 == float1);
        CATCH_CHECK(signed1 != float3);

        CATCH_CHECK(unsigned1 == unsigned1);
        CATCH_CHECK(unsigned1 == unsigned2);
        CATCH_CHECK(unsigned1 != unsigned3);
        CATCH_CHECK(unsigned1 != string1);
        CATCH_CHECK(unsigned1 != object1);
        CATCH_CHECK(unsigned1 != array1);
        CATCH_CHECK(unsigned1 != bool1);
        CATCH_CHECK(unsigned1 == signed1);
        CATCH_CHECK(unsigned1 != signed3);
        CATCH_CHECK(unsigned1 == float1);
        CATCH_CHECK(unsigned1 != float3);

        CATCH_CHECK(float1 == float1);
        CATCH_CHECK(float1 == float2);
        CATCH_CHECK(float1 != float3);
        CATCH_CHECK(float1 != string1);
        CATCH_CHECK(float1 != object1);
        CATCH_CHECK(float1 != array1);
        CATCH_CHECK(float1 != bool1);
        CATCH_CHECK(float1 == signed1);
        CATCH_CHECK(float1 != signed3);
        CATCH_CHECK(float1 == unsigned1);
        CATCH_CHECK(float1 != unsigned3);
    }

    CATCH_SECTION("Stream a JSON instance")
    {
        std::stringstream stream;

        fly::Json string = "abc";
        fly::Json object = {{"a", 1}, {"b", 2}};
        fly::Json array = {'7', 8};
        fly::Json boolean = true;
        fly::Json sign = 1;
        fly::Json unsign = static_cast<unsigned int>(1);
        fly::Json floating = 1.0f;
        fly::Json null = nullptr;

        stream << string;
        CATCH_CHECK(stream.str() == "\"abc\"");
        stream.str(std::string());

        stream << object;
        CATCH_CHECK(stream.str() == "{\"a\":1,\"b\":2}");
        stream.str(std::string());

        stream << array;
        CATCH_CHECK(stream.str() == "[55,8]");
        stream.str(std::string());

        stream << boolean;
        CATCH_CHECK(stream.str() == "true");
        stream.str(std::string());

        stream << sign;
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << unsign;
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << floating;
        CATCH_CHECK(stream.str() == "1");
        stream.str(std::string());

        stream << null;
        CATCH_CHECK(stream.str() == "null");
        stream.str(std::string());
    }

    CATCH_SECTION("Stream a JSON instance, expecting special symbols to be escaped")
    {
        {
            fly::Json json = "abc\\\"def";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\\"def\"");
        }
        {
            fly::Json json = "abc\\\\def";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\\\def\"");
        }
        {
            fly::Json json = "abc\\bdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\bdef\"");
        }
        {
            fly::Json json = "abc\\fdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\fdef\"");
        }
        {
            fly::Json json = "abc\\ndef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\ndef\"");
        }
        {
            fly::Json json = "abc\\rdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\rdef\"");
        }
        {
            fly::Json json = "abc\\tdef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\tdef\"");
        }
        {
            fly::Json json = "abc\xce\xa9zef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\u03a9zef\"");
        }
        {
            fly::Json json = "abc\xf0\x9f\x8d\x95zef";

            std::stringstream stream;
            stream << json;

            const std::string str = stream.str();
            CATCH_CHECK(str == "\"abc\\ud83c\\udf55zef\"");
        }
    }
}

CATCH_TEMPLATE_TEST_CASE(
    "JsonStringOperations",
    "[json]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using string_type = TestType;
    using char_type = typename string_type::value_type;

    CATCH_SECTION("Access a JSON object's values via the access operator")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object1[J_STR("a")] == 1);
        CATCH_CHECK(object1[J_STR("b")] == 2);
        CATCH_CHECK_NOTHROW(object1[J_STR("c")]);
        CATCH_CHECK(object1[J_STR("c")] == nullptr);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object2[J_STR("a")] == 1);
        CATCH_CHECK(object2[J_STR("b")] == 2);
        CATCH_CHECK_THROWS_JSON(object2[J_STR("c")], "Given key (c) not found: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array1[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(
            bool1[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(
            bool2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_NOTHROW(null1[J_STR("a")]);
        CATCH_CHECK(null1.is_object());
        CATCH_CHECK(null1[J_STR("a")] == nullptr);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null2[J_STR("a")],
            "JSON type invalid for operator[key]: (%s)",
            null2);
    }

    CATCH_SECTION("Access a JSON object's values via the accessor 'at'")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object1.at(J_STR("a")) == 1);
        CATCH_CHECK(object1.at(J_STR("b")) == 2);
        CATCH_CHECK_THROWS_JSON(object1.at(J_STR("c")), "Given key (c) not found: (%s)", object1);

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object2.at(J_STR("a")) == 1);
        CATCH_CHECK(object2.at(J_STR("b")) == 2);
        CATCH_CHECK_THROWS_JSON(object2.at(J_STR("c")), "Given key (c) not found: (%s)", object2);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(
            bool1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(
            bool2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null1.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            null1);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null2.at(J_STR("a")),
            "JSON type invalid for operator[key]: (%s)",
            null2);
    }

    CATCH_SECTION("Count the number of values with a key in a JSON object")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            string1);

        fly::Json object1 = fly::JsonTraits::object_type();
        CATCH_CHECK(object1.count(J_STR("a")) == 0);
        CATCH_CHECK(object1.count(J_STR("b")) == 0);
        CATCH_CHECK(object1.count(J_STR("c")) == 0);

        fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object2.count(J_STR("a")) == 1);
        CATCH_CHECK(object2.count(J_STR("b")) == 1);
        CATCH_CHECK(object2.count(J_STR("c")) == 0);

        fly::Json object3 = {{"a", 1}, {"a", 2}};
        CATCH_CHECK(object3.count(J_STR("a")) == 1);
        CATCH_CHECK(object3.count(J_STR("b")) == 0);
        CATCH_CHECK(object3.count(J_STR("c")) == 0);

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            array1);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(
            bool1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            bool1);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            signed1);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            unsigned1);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            float1);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null1.count(J_STR("a")),
            "JSON type invalid for count(key): (%s)",
            null1);
    }

    CATCH_SECTION("Find a value with a key in a JSON object")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            string1);

        const fly::Json string2 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            string2);

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        auto it1a = object1.find(J_STR("a"));
        auto it1b = object1.find(J_STR("b"));
        auto it1c = object1.find(J_STR("c"));
        CATCH_REQUIRE(it1a != object1.end());
        CATCH_REQUIRE(it1b != object1.end());
        CATCH_CHECK(*it1a == 1);
        CATCH_CHECK(*it1b == 2);
        CATCH_CHECK(it1c == object1.end());

        const fly::Json object2 = {{"a", 1}, {"b", 2}};
        auto it2a = object2.find(J_STR("a"));
        auto it2b = object2.find(J_STR("b"));
        auto it2c = object2.find(J_STR("c"));
        CATCH_REQUIRE(it2a != object2.end());
        CATCH_REQUIRE(it2b != object2.end());
        CATCH_CHECK(*it2a == 1);
        CATCH_CHECK(*it2b == 2);
        CATCH_CHECK(it2c == object2.end());

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            array1);

        const fly::Json array2 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            array2);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(
            bool1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            bool1);

        const fly::Json bool2 = true;
        CATCH_CHECK_THROWS_JSON(
            bool2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            bool2);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            signed1);

        const fly::Json signed2 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            signed2);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            unsigned1);

        const fly::Json unsigned2 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            unsigned2);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            float1);

        const fly::Json float2 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            float2);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null1.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            null1);

        const fly::Json null2 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null2.find(J_STR("a")),
            "JSON type invalid for find(key): (%s)",
            null2);
    }

    CATCH_SECTION("Check for the existence of a key in a JSON object")
    {
        fly::Json string1 = "abc";
        CATCH_CHECK_THROWS_JSON(
            string1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            string1);

        fly::Json object1 = fly::JsonTraits::object_type();
        CATCH_CHECK_FALSE(object1.contains(J_STR("a")));
        CATCH_CHECK_FALSE(object1.contains(J_STR("b")));
        CATCH_CHECK_FALSE(object1.contains(J_STR("c")));

        fly::Json object2 = {{"a", 1}, {"b", 2}};
        CATCH_CHECK(object2.contains(J_STR("a")));
        CATCH_CHECK(object2.contains(J_STR("b")));
        CATCH_CHECK_FALSE(object2.contains(J_STR("c")));

        fly::Json object3 = {{"a", 1}, {"a", 2}};
        CATCH_CHECK(object3.contains(J_STR("a")));
        CATCH_CHECK_FALSE(object3.contains(J_STR("b")));
        CATCH_CHECK_FALSE(object3.contains(J_STR("c")));

        fly::Json array1 = {'7', 8};
        CATCH_CHECK_THROWS_JSON(
            array1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            array1);

        fly::Json bool1 = true;
        CATCH_CHECK_THROWS_JSON(
            bool1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            bool1);

        fly::Json signed1 = 1;
        CATCH_CHECK_THROWS_JSON(
            signed1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            signed1);

        fly::Json unsigned1 = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_JSON(
            unsigned1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            unsigned1);

        fly::Json float1 = 1.0f;
        CATCH_CHECK_THROWS_JSON(
            float1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            float1);

        fly::Json null1 = nullptr;
        CATCH_CHECK_THROWS_JSON(
            null1.contains(J_STR("a")),
            "JSON type invalid for contains(key): (%s)",
            null1);
    }
}
