#include "fly/fly.hpp"
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

CATCH_TEMPLATE_TEST_CASE(
    "JsonConversion",
    "[json]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using string_type = TestType;
    using char_type = typename string_type::value_type;

    CATCH_SECTION("Convert a JSON instance to string-like types")
    {
        fly::Json json;

        json = "abc";
        CATCH_CHECK(string_type(json) == J_STR("abc"));

        json = L"def";
        CATCH_CHECK(string_type(json) == J_STR("def"));

        json = u8"ghi";
        CATCH_CHECK(string_type(json) == J_STR("ghi"));

        json = u"jkl";
        CATCH_CHECK(string_type(json) == J_STR("jkl"));

        json = U"mno";
        CATCH_CHECK(string_type(json) == J_STR("mno"));

        json = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK(string_type(json) == J_STR("{\"a\":1,\"b\":2}"));

        json = {J_CHR('7'), 8};
        CATCH_CHECK(string_type(json) == J_STR("[55,8]"));

        json = true;
        CATCH_CHECK(string_type(json) == J_STR("true"));

        json = 1;
        CATCH_CHECK(string_type(json) == J_STR("1"));

        json = static_cast<unsigned int>(1);
        CATCH_CHECK(string_type(json) == J_STR("1"));

        json = 1.0f;
        CATCH_CHECK(string_type(json) == J_STR("1"));

        json = nullptr;
        CATCH_CHECK(string_type(json) == J_STR("null"));
    }

    CATCH_SECTION("Convert a JSON instance to object-like types")
    {
        auto validate = [](auto *name, auto &test1, auto &test2, auto &test3)
        {
            CATCH_CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;
            using T3 = std::decay_t<decltype(test3)>;

            test1 = T1 {{J_STR("a"), 2}, {J_STR("b"), 4}};
            test2 = T2 {{J_STR("a"), "2"}, {J_STR("b"), "4"}};
            test3 = T3 {{J_STR("a"), 2}, {J_STR("b"), "4"}};

            {
                fly::Json json = test1;
                CATCH_CHECK(T1(json) == test1);
                CATCH_CHECK(T2(json) == test2);
            }
            {
                fly::Json json = test2;
                CATCH_CHECK(T1(json) == test1);
                CATCH_CHECK(T2(json) == test2);
            }
            {
                fly::Json json = test3;
                CATCH_CHECK(T1(json) == test1);
                CATCH_CHECK(T2(json) == test2);
                CATCH_CHECK(T3(json) == test3);
            }
            {
                fly::Json json = {{"a", true}};
                CATCH_CHECK_THROWS_JSON(T1(json), "JSON type is not numeric: (%s)", json["a"]);
            }
            {
                fly::Json json = {{"a", J_STR("string")}};
                CATCH_CHECK_THROWS_JSON(T1(json), "JSON type is not numeric: (%s)", json["a"]);
            }
        };

        std::map<string_type, int> map1;
        std::map<string_type, std::string> map2;
        std::map<string_type, fly::Json> map3;
        validate("map", map1, map2, map3);

        std::multimap<string_type, int> multimap1;
        std::multimap<string_type, std::string> multimap2;
        std::multimap<string_type, fly::Json> multimap3;
        validate("multimap", multimap1, multimap2, multimap3);

        std::unordered_map<string_type, int> unordered_map1;
        std::unordered_map<string_type, std::string> unordered_map2;
        std::unordered_map<string_type, fly::Json> unordered_map3;
        validate("unordered_map", unordered_map1, unordered_map2, unordered_map3);

        std::unordered_multimap<string_type, int> unordered_multimap1;
        std::unordered_multimap<string_type, std::string> unordered_multimap2;
        std::unordered_multimap<string_type, fly::Json> unordered_multimap3;
        validate(
            "unordered_multimap",
            unordered_multimap1,
            unordered_multimap2,
            unordered_multimap3);
    }

    CATCH_SECTION("Fail to convert a JSON instance to object-like types")
    {
        auto invalidate = [](fly::Json json)
        {
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::map<string_type, fly::Json>(json))),
                "JSON type is not an object: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::multimap<string_type, fly::Json>(json))),
                "JSON type is not an object: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::unordered_map<string_type, fly::Json>(json))),
                "JSON type is not an object: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::unordered_multimap<string_type, fly::Json>(json))),
                "JSON type is not an object: (%s)",
                json);
        };

        invalidate(J_STR("abc"));
        invalidate({J_CHR('7'), 8});
        invalidate(true);
        invalidate(1);
        invalidate(static_cast<unsigned int>(1));
        invalidate(1.0f);
        invalidate(nullptr);
    }

    CATCH_SECTION("Convert a JSON instance to array-like types")
    {
        auto validate2 = [](auto *name, auto &test1, auto &test2)
        {
            CATCH_CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;

            test1 = T1 {50, 60, 70, 80};
            test2 = T2 {J_STR("50"), J_STR("60"), J_STR("70"), J_STR("80")};

            {
                fly::Json json = test1;
                CATCH_CHECK(T1(json) == test1);
                CATCH_CHECK(T2(json) == test2);
            }
            {
                fly::Json json = test2;
                CATCH_CHECK(T1(json) == test1);
                CATCH_CHECK(T2(json) == test2);
            }
            {
                fly::Json json = {true};
                CATCH_CHECK_THROWS_JSON(
                    FLY_UNUSED((std::array<int, 1>(json))),
                    "JSON type is not numeric: (%s)",
                    json[0]);
            }
            {
                fly::Json json = {J_STR("string")};
                CATCH_CHECK_THROWS_JSON(
                    FLY_UNUSED((std::array<int, 1>(json))),
                    "JSON type is not numeric: (%s)",
                    json[0]);
            }
        };

        auto validate3 = [&](auto *name, auto &test1, auto &test2, auto &test3)
        {
            validate2(name, test1, test2);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;
            using T3 = std::decay_t<decltype(test3)>;

            test3 = T3 {50, J_STR("60"), 70, J_STR("80")};

            fly::Json json = test3;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);
            CATCH_CHECK(T3(json) == test3);
        };

        std::array<int, 4> array1;
        std::array<string_type, 4> array2;
        std::array<fly::Json, 4> array3;
        validate3("array", array1, array2, array3);

        std::array<int, 1> array4 = {7};
        std::array<int, 2> array5 = {7, 8};
        std::array<int, 3> array6 = {7, 8, 0};
        fly::Json json = array5;
        CATCH_CHECK(decltype(array4)(json) == array4);
        CATCH_CHECK(decltype(array5)(json) == array5);
        CATCH_CHECK(decltype(array6)(json) == array6);

        std::deque<int> deque1;
        std::deque<string_type> deque2;
        std::deque<fly::Json> deque3;
        validate3("deque", deque1, deque2, deque3);

        std::forward_list<int> forward_list1;
        std::forward_list<string_type> forward_list2;
        std::forward_list<fly::Json> forward_list3;
        validate3("forward_list", forward_list1, forward_list2, forward_list3);

        std::list<int> list1;
        std::list<string_type> list2;
        std::list<fly::Json> list3;
        validate3("list", list1, list2, list3);

        std::multiset<int> multiset1;
        std::multiset<string_type> multiset2;
        // std::multiset<fly::Json> multiset3;
        validate2("multiset", multiset1, multiset2);

        std::set<int> set1;
        std::set<string_type> set2;
        // std::set<fly::Json> set3;
        validate2("set", set1, set2);

        std::unordered_multiset<int> unordered_multiset1;
        std::unordered_multiset<string_type> unordered_multiset2;
        // std::unordered_multiset<fly::Json> unordered_multiset3;
        validate2("unordered_multiset", unordered_multiset1, unordered_multiset2);

        std::unordered_set<int> unordered_set1;
        std::unordered_set<string_type> unordered_set2;
        // std::unordered_set<fly::Json> unordered_set3;
        validate2("unordered_set", unordered_set1, unordered_set2);

        std::vector<int> vector1;
        std::vector<string_type> vector2;
        std::vector<fly::Json> vector3;
        validate3("vector", vector1, vector2, vector3);
    }

    CATCH_SECTION("Fail to convert a JSON instance to array-like types")
    {
        auto invalidate = [](fly::Json json)
        {
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::array<int, 1>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::deque<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::forward_list<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::list<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::multiset<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::set<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::unordered_multiset<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::unordered_set<int>(json))),
                "JSON type is not an array: (%s)",
                json);
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::vector<int>(json))),
                "JSON type is not an array: (%s)",
                json);
        };

        invalidate(J_STR("abc"));
        invalidate({{J_STR("a"), 1}, {J_STR("b"), 2}});
        invalidate(true);
        invalidate(1);
        invalidate(static_cast<unsigned int>(1));
        invalidate(1.0f);
        invalidate(nullptr);
    }

    CATCH_SECTION("Convert a JSON instance to Boolean-like types")
    {
        fly::Json json;

        json = J_STR("");
        CATCH_CHECK_FALSE(bool(json));
        json = J_STR("abc");
        CATCH_CHECK(bool(json));

        json = std::map<std::string, int>();
        CATCH_CHECK_FALSE(bool(json));
        json = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK(bool(json));

        json = std::vector<int>();
        CATCH_CHECK_FALSE(bool(json));
        json = {7, 8};
        CATCH_CHECK(bool(json));

        json = true;
        CATCH_CHECK(bool(json));
        json = false;
        CATCH_CHECK_FALSE(bool(json));

        json = 1;
        CATCH_CHECK(bool(json));
        json = 0;
        CATCH_CHECK_FALSE(bool(json));

        json = static_cast<unsigned int>(1);
        CATCH_CHECK(bool(json));
        json = static_cast<unsigned int>(0);
        CATCH_CHECK_FALSE(bool(json));

        json = 1.0f;
        CATCH_CHECK(bool(json));
        json = 0.0f;
        CATCH_CHECK_FALSE(bool(json));

        json = nullptr;
        CATCH_CHECK_FALSE(bool(json));
    }

    CATCH_SECTION("Convert a JSON instance to signed-integer-like types")
    {
        fly::Json json;

        json = J_STR("abc");
        CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: (%s)", json);

        json = J_STR("123");
        CATCH_CHECK(int(json) == 123);

        json = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: (%s)", json);

        json = {7, 8};
        CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: (%s)", json);

        json = true;
        CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: (%s)", json);

        char_type ch = J_CHR('a');
        json = ch;
        CATCH_CHECK(char_type(json) == ch);

        int sign = 12;
        json = sign;
        CATCH_CHECK(int(json) == sign);

        unsigned int unsign = static_cast<unsigned int>(12);
        json = unsign;
        CATCH_CHECK(int(json) == int(unsign));

        float floating = 3.14f;
        json = floating;
        CATCH_CHECK(int(json) == int(floating));

        json = nullptr;
        CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: (%s)", json);
    }

    CATCH_SECTION("Convert a JSON instance to unsigned-integer-like types")
    {
        fly::Json json;

        json = J_STR("abc");
        CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: (%s)", json);

        json = J_STR("123");
        CATCH_CHECK(unsigned(json) == unsigned(123));

        json = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: (%s)", json);

        json = {7, 8};
        CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: (%s)", json);

        json = true;
        CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: (%s)", json);

        char_type ch = J_CHR('a');
        json = ch;
        CATCH_CHECK(static_cast<unsigned char>(json) == static_cast<unsigned char>(ch));

        int sign = 12;
        json = sign;
        CATCH_CHECK(unsigned(json) == unsigned(sign));

        unsigned int unsign = static_cast<unsigned int>(12);
        json = unsign;
        CATCH_CHECK(unsigned(json) == unsign);

        float floating = 3.14f;
        json = floating;
        CATCH_CHECK(unsigned(json) == unsigned(floating));

        json = nullptr;
        CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: (%s)", json);
    }

    CATCH_SECTION("Convert a JSON instance to floating-point-like types")
    {
        fly::Json json;

        json = J_STR("abc");
        CATCH_CHECK_THROWS_JSON(float(json), "JSON type is not numeric: (%s)", json);

        json = J_STR("123.5");
        CATCH_CHECK(float(json) == Approx(123.5f));

        json = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK_THROWS_JSON(float(json), "JSON type is not numeric: (%s)", json);

        json = {7, 8};
        CATCH_CHECK_THROWS_JSON(float(json), "JSON type is not numeric: (%s)", json);

        json = true;
        CATCH_CHECK_THROWS_JSON(float(json), "JSON type is not numeric: (%s)", json);

        char_type ch = J_CHR('a');
        json = ch;
        CATCH_CHECK(float(json) == Approx(float(ch)));

        int sign = 12;
        json = sign;
        CATCH_CHECK(float(json) == Approx(float(sign)));

        unsigned int unsign = static_cast<unsigned int>(12);
        json = unsign;
        CATCH_CHECK(float(json) == Approx(float(unsign)));

        float floating = 3.14f;
        json = floating;
        CATCH_CHECK(float(json) == Approx(floating));

        json = nullptr;
        CATCH_CHECK_THROWS_JSON(float(json), "JSON type is not numeric: (%s)", json);
    }

    CATCH_SECTION("Convert a JSON instance to null-like types")
    {
        fly::Json json;

        json = J_STR("abc");
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = {{J_STR("a"), 1}, {J_STR("b"), 2}};
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = {7, 8};
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = true;
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = J_CHR('a');
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = 12;
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = static_cast<unsigned int>(12);
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = 3.14f;
        CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: (%s)", json);

        json = nullptr;
        CATCH_CHECK(std::nullptr_t(json) == nullptr);
    }
}
