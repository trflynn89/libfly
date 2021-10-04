#include "fly/types/json/json.hpp"

#include "test/types/json/json_helpers.hpp"

#include "catch2/catch_test_macros.hpp"

#include <array>
#include <deque>
#include <forward_list>
#include <functional>
#include <list>
#include <map>
#include <set>
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

    CATCH_SECTION("Check the reverse iterator at the beginning of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto begin1 = json1.rbegin();
        CATCH_CHECK(*begin1 == 3);
        CATCH_CHECK_FALSE(std::is_const_v<decltype(begin1)::value_type>);

        auto cbegin1 = json1.crbegin();
        CATCH_CHECK(*cbegin1 == 3);
        CATCH_CHECK(std::is_const_v<decltype(cbegin1)::value_type>);

        auto begin2 = json2.rbegin();
        CATCH_CHECK(*begin2 == 6);
        CATCH_CHECK(std::is_const_v<decltype(begin2)::value_type>);

        auto cbegin2 = json2.crbegin();
        CATCH_CHECK(*cbegin2 == 6);
        CATCH_CHECK(begin2 == cbegin2);
        CATCH_CHECK(std::is_const_v<decltype(cbegin2)::value_type>);
    }

    CATCH_SECTION("Check the reverse iterator at the end of a JSON instance")
    {
        fly::Json json1 {1, 2, 3};
        const fly::Json json2 {4, 5, 6};

        auto end1 = json1.rend();
        CATCH_CHECK(*(end1 - 1) == 1);
        CATCH_CHECK_FALSE(std::is_const_v<decltype(end1)::value_type>);

        auto cend1 = json1.crend();
        CATCH_CHECK(*(cend1 - 1) == 1);
        CATCH_CHECK(std::is_const_v<decltype(cend1)::value_type>);

        auto end2 = json2.rend();
        CATCH_CHECK(*(end2 - 1) == 4);
        CATCH_CHECK(std::is_const_v<decltype(end2)::value_type>);

        auto cend2 = json2.crend();
        CATCH_CHECK(*(cend2 - 1) == 4);
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

    CATCH_SECTION("Serialize a JSON instance")
    {
        fly::Json string = "abc";
        fly::Json object = {{"a", 1}, {"b", 2}};
        fly::Json array = {'7', 8};
        fly::Json boolean = true;
        fly::Json sign = 1;
        fly::Json unsign = static_cast<unsigned int>(1);
        fly::Json floating = 1.0f;
        fly::Json null = nullptr;

        CATCH_CHECK(string.serialize() == FLY_JSON_STR("\"abc\""));
        CATCH_CHECK(object.serialize() == FLY_JSON_STR("{\"a\":1,\"b\":2}"));
        CATCH_CHECK(array.serialize() == FLY_JSON_STR("[55,8]"));
        CATCH_CHECK(boolean.serialize() == FLY_JSON_STR("true"));
        CATCH_CHECK(sign.serialize() == FLY_JSON_STR("1"));
        CATCH_CHECK(unsign.serialize() == FLY_JSON_STR("1"));
        CATCH_CHECK(floating.serialize() == FLY_JSON_STR("1"));
        CATCH_CHECK(null.serialize() == FLY_JSON_STR("null"));
    }

    CATCH_SECTION("Serialize a JSON instance, expecting special symbols to be escaped")
    {
        {
            fly::Json json = "abc\\\"def";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\\"def\""));
        }
        {
            fly::Json json = "abc\\\\def";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\\\def\""));
        }
        {
            fly::Json json = "abc\\bdef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\bdef\""));
        }
        {
            fly::Json json = "abc\\fdef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\fdef\""));
        }
        {
            fly::Json json = "abc\\ndef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\ndef\""));
        }
        {
            fly::Json json = "abc\\rdef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\rdef\""));
        }
        {
            fly::Json json = "abc\\tdef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\tdef\""));
        }
        {
            fly::Json json = "abc\xce\xa9zef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\u03a9zef\""));
        }
        {
            fly::Json json = "abc\xf0\x9f\x8d\x95zef";
            CATCH_CHECK(json.serialize() == FLY_JSON_STR("\"abc\\ud83c\\udf55zef\""));
        }
    }

    CATCH_SECTION("Hash a JSON instance")
    {
        fly::Json string1 = "abc";
        fly::Json string2 = "def";

        fly::Json object1 = {{"a", 1}, {"b", 2}};
        fly::Json object2 = {{"c", 1}, {"d", 2}};

        fly::Json array1 = {'7', 8};
        fly::Json array2 = {'9', 0};

        fly::Json boolean1 = true;
        fly::Json boolean2 = false;

        fly::Json sign1 = 1;
        fly::Json sign2 = 2;

        fly::Json unsign1 = static_cast<unsigned int>(1);
        fly::Json unsign2 = static_cast<unsigned int>(2);

        fly::Json floating1 = 1.0f;
        fly::Json floating2 = 2.0f;

        fly::Json null1 = nullptr;
        fly::Json null2;

        std::hash<fly::Json> hasher {};

        CATCH_CHECK(hasher(string1) != 0);
        CATCH_CHECK(hasher(string1) == hasher(string1));
        CATCH_CHECK(hasher(string1) != hasher(string2));

        CATCH_CHECK(hasher(object1) != 0);
        CATCH_CHECK(hasher(object1) == hasher(object1));
        CATCH_CHECK(hasher(object1) != hasher(object2));

        CATCH_CHECK(hasher(array1) != 0);
        CATCH_CHECK(hasher(array1) == hasher(array1));
        CATCH_CHECK(hasher(array1) != hasher(array2));

        CATCH_CHECK(hasher(boolean1) != 0);
        CATCH_CHECK(hasher(boolean1) == hasher(boolean1));
        CATCH_CHECK(hasher(boolean1) != hasher(boolean2));

        CATCH_CHECK(hasher(sign1) != 0);
        CATCH_CHECK(hasher(sign1) == hasher(sign1));
        CATCH_CHECK(hasher(sign1) != hasher(sign2));

        CATCH_CHECK(hasher(unsign1) != 0);
        CATCH_CHECK(hasher(unsign1) == hasher(unsign1));
        CATCH_CHECK(hasher(unsign1) != hasher(unsign2));

        CATCH_CHECK(hasher(floating1) != 0);
        CATCH_CHECK(hasher(floating1) == hasher(floating1));
        CATCH_CHECK(hasher(floating1) != hasher(floating2));

        CATCH_CHECK(hasher(null1) != 0);
        CATCH_CHECK(hasher(null1) == hasher(null1));
        CATCH_CHECK(hasher(null1) == hasher(null2));
    }
}

CATCH_TEMPLATE_TEST_CASE("JsonFormat", "[json]", char, wchar_t, char8_t, char16_t, char32_t)
{
    using BasicString = fly::BasicString<TestType>;
    using char_type = typename BasicString::char_type;

    CATCH_SECTION("Format a JSON instance")
    {
        fly::Json string = "abc";
        fly::Json object = {{"a", 1}, {"b", 2}};
        fly::Json array = {'7', 8};
        fly::Json boolean = true;
        fly::Json sign = 1;
        fly::Json unsign = static_cast<unsigned int>(1);
        fly::Json floating = 1.0f;
        fly::Json null = nullptr;

        CATCH_CHECK(BasicString::format(J_ARR("{}"), string) == J_STR("\"abc\""));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), object) == J_STR("{\"a\":1,\"b\":2}"));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), array) == J_STR("[55,8]"));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), boolean) == J_STR("true"));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), sign) == J_STR("1"));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), unsign) == J_STR("1"));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), floating) == J_STR("1"));
        CATCH_CHECK(BasicString::format(J_ARR("{}"), null) == J_STR("null"));
    }
}
