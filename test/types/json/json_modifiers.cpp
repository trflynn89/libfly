#include "test/types/json/json_helpers.hpp"

#include "fly/types/json/json.hpp"

#include "catch2/catch_test_macros.hpp"

CATCH_JSON_TEST_CASE("JsonModifiers")
{
    using json_type = TestType;

    fly::Json json = fly::test::create_json<json_type>();

    CATCH_SECTION("Clear JSON instances and verify they are then empty")
    {
        json.clear();

        if constexpr (fly::test::is_object_or_array_or_string_v<json_type>)
        {
            CATCH_CHECK(json.empty());
        }
        else
        {
            CATCH_CHECK(json == static_cast<json_type>(0));
        }
    }

    CATCH_SECTION("Insert a value into a JSON array")
    {
        const fly::Json array = {1, 2, 3, 4};
        const fly::Json value = 1;

        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), value),
                "Provided iterator is for a different Json instance");

            auto result = json.insert(json.begin(), value);
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 10});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == json.begin());
            CATCH_CHECK(*result == value);

            result = json.insert(json.end(), value);
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 10, 1});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 5));
            CATCH_CHECK(*result == value);

            result = json.insert(json.begin() + 4, value);
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 1, 10, 1});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 4));
            CATCH_CHECK(*result == value);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), value),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert a moved value into a JSON array")
    {
        const fly::Json array = {1, 2, 3};

        auto value = []() -> fly::Json
        {
            return fly::Json(1);
        };

        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), value()),
                "Provided iterator is for a different Json instance");

            auto result = json.insert(json.begin(), value());
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 10});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == json.begin());
            CATCH_CHECK(*result == value());

            result = json.insert(json.end(), value());
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 10, 1});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 5));
            CATCH_CHECK(*result == value());

            result = json.insert(json.begin() + 4, value());
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 1, 10, 1});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 4));
            CATCH_CHECK(*result == value());
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), value()),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert copies of a value into a JSON array")
    {
        const fly::Json array = {1, 2, 3};
        const fly::Json value = 1;

        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), value),
                "Provided iterator is for a different Json instance");

            auto result = json.insert(json.begin(), 0, value);
            CATCH_CHECK(json == fly::Json {'7', 8, 9, 10});
            CATCH_CHECK(json == fly::Json {'7', 8, 9, 10});
            CATCH_CHECK(result == json.begin());

            result = json.insert(json.begin(), 1, value);
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 10});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == json.begin());
            CATCH_CHECK(*result == value);

            result = json.insert(json.end(), 2, value);
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 10, 1, 1});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 5));
            CATCH_CHECK(*result == value);
            CATCH_REQUIRE((result + 1) != json.end());
            CATCH_CHECK(*(result + 1) == value);

            result = json.insert(json.begin() + 4, 3, value);
            CATCH_CHECK(json == fly::Json {1, '7', 8, 9, 1, 1, 1, 10, 1, 1});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 4));
            CATCH_CHECK(*result == value);
            CATCH_REQUIRE((result + 1) != json.end());
            CATCH_CHECK(*(result + 1) == value);
            CATCH_REQUIRE((result + 2) != json.end());
            CATCH_CHECK(*(result + 2) == value);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), 1, value),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert copies of a value into a JSON array")
    {
        const fly::Json object = {{"c", 3}, {"d", 4}};
        const fly::Json array = {1, 2, 3};

        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(json.begin(), json.begin(), array.end()),
                "Provided iterators are for different Json instances");

            CATCH_CHECK_THROWS_JSON(
                json.insert(json.begin(), object.begin(), object.end()),
                "Provided iterators' JSON type invalid for array insertion");

            CATCH_CHECK_THROWS_JSON(
                json.insert(json.begin(), json.begin(), json.end()),
                "Provided iterators may not belong to this Json instance: ({})",
                json);

            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), array.begin(), array.end()),
                "Provided iterator is for a different Json instance");

            auto result = json.insert(json.begin(), array.begin(), array.begin());
            CATCH_CHECK(json == fly::Json {'7', 8, 9, 10});
            CATCH_CHECK(result == json.begin());

            result = json.insert(json.begin(), array.begin(), array.end());
            CATCH_CHECK(json == fly::Json {1, 2, 3, '7', 8, 9, 10});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == json.begin());
            CATCH_CHECK(*result == fly::Json(1));

            result = json.insert(json.end(), array.begin(), array.end());
            CATCH_CHECK(json == fly::Json {1, 2, 3, '7', 8, 9, 10, 1, 2, 3});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 7));
            CATCH_CHECK(*result == fly::Json(1));

            result = json.insert(json.begin() + 6, array.begin(), array.end());
            CATCH_CHECK(json == fly::Json {1, 2, 3, '7', 8, 9, 1, 2, 3, 10, 1, 2, 3});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 6));
            CATCH_CHECK(*result == fly::Json(1));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), array.begin(), array.end()),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert a list of values into a JSON array")
    {
        const fly::Json array = {1, 2, 3};

        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), {1, 2, 3}),
                "Provided iterator is for a different Json instance");

            auto result = json.insert(json.begin(), {});
            CATCH_CHECK(json == fly::Json {'7', 8, 9, 10});
            CATCH_CHECK(result == json.begin());

            result = json.insert(json.begin(), {1, 2, 3});
            CATCH_CHECK(json == fly::Json {1, 2, 3, '7', 8, 9, 10});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == json.begin());
            CATCH_CHECK(*result == fly::Json(1));

            result = json.insert(json.end(), {1, 2, 3});
            CATCH_CHECK(json == fly::Json {1, 2, 3, '7', 8, 9, 10, 1, 2, 3});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 7));
            CATCH_CHECK(*result == fly::Json(1));

            result = json.insert(json.begin() + 6, {1, 2, 3});
            CATCH_CHECK(json == fly::Json {1, 2, 3, '7', 8, 9, 1, 2, 3, 10, 1, 2, 3});
            CATCH_REQUIRE(result != json.end());
            CATCH_CHECK(result == (json.begin() + 6));
            CATCH_CHECK(*result == fly::Json(1));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), {1, 2, 3}),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Emplace a value into a JSON array")
    {
        fly::Json value = 3;

        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::json_array_type>)
        {
            const auto size_before = json.size();
            auto &result = json.emplace_back(std::move(value));
            const auto size_after = json.size();

            CATCH_CHECK((size_after - size_before) == 1);
            CATCH_CHECK(result == 3);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.emplace_back(std::move(value)),
                "JSON type invalid for array emplacement: ({})",
                json);
        }
    }

    CATCH_SECTION("Push a value into a JSON array")
    {
        const fly::Json value1 = 3;
        const fly::Json value2 = 4;

        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::json_array_type>)
        {
            const auto starting_size = json.size();

            json.push_back(value1);
            CATCH_CHECK(json.size() == (starting_size + 1));
            CATCH_CHECK(json.back() == 3);

            json.push_back(value2);
            CATCH_CHECK(json.size() == (starting_size + 2));
            CATCH_CHECK(json.back() == 4);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.push_back(value1),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Push a moved value into a JSON array")
    {
        fly::Json value1 = 3;
        fly::Json value2 = 4;

        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::json_array_type>)
        {
            const auto starting_size = json.size();

            json.push_back(std::move(value1));
            CATCH_CHECK(json.size() == (starting_size + 1));
            CATCH_CHECK(json.back() == 3);

            json.push_back(std::move(value2));
            CATCH_CHECK(json.size() == (starting_size + 2));
            CATCH_CHECK(json.back() == 4);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.push_back(std::move(value1)),
                "JSON type invalid for array insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Pop a value from a JSON array")
    {
        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            json.pop_back();
            CATCH_CHECK(json == fly::Json {'7', 8, 9});

            json.pop_back();
            CATCH_CHECK(json == fly::Json {'7', 8});

            json.pop_back();
            CATCH_CHECK(json == fly::Json {'7'});

            json.pop_back();
            CATCH_CHECK(json == json_type());

            CATCH_CHECK_THROWS_JSON(json.erase(0), "Given index (0) not found: ({})", json);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.pop_back(),
                "JSON type invalid for erase(index): ({})",
                json);
        }
    }

    CATCH_SECTION("Erase a value from a JSON instance")
    {
        if constexpr (fly::JsonIterable<json_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.erase(fly::Json::const_iterator()),
                "Provided iterator is for a different Json instance",
                json);

            CATCH_CHECK_THROWS_JSON(
                json.erase(json.end()),
                "Provided iterator must not be past-the-end",
                json);

            if constexpr (std::is_same_v<json_type, fly::json_object_type>)
            {
                auto result = json.erase(json.begin());
                CATCH_CHECK(json == fly::Json {{"b", 2}});
                CATCH_CHECK(result == json.begin());
            }
            else
            {
                auto result = json.erase(json.begin());
                CATCH_CHECK(json == fly::Json {8, 9, 10});
                CATCH_CHECK(result == json.begin());

                result = json.erase(json.begin() + 1);
                CATCH_CHECK(json == fly::Json {8, 10});
                CATCH_CHECK(result == (json.begin() + 1));

                result = json.erase(json.end() - 1);
                CATCH_CHECK(json == fly::Json {8});
                CATCH_CHECK(result == json.end());
            }

            auto result = json.erase(json.begin());
            CATCH_CHECK(json == json_type());
            CATCH_CHECK(result == json.end());

            CATCH_CHECK_THROWS_JSON(
                json.erase(json.begin()),
                "Provided iterator must not be past-the-end",
                json);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.erase(fly::Json::const_iterator()),
                "JSON type invalid for erasure: ({})",
                json);
        }
    }

    CATCH_SECTION("Erase a range of values from a JSON instance")
    {
        if constexpr (fly::JsonIterable<json_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.erase(fly::Json::const_iterator(), fly::Json::const_iterator()),
                "Provided iterators are for a different Json instance",
                json);

            auto result = json.erase(json.begin(), json.end());
            CATCH_CHECK(json == json_type());
            CATCH_CHECK(result == json.end());

            json = fly::test::create_json<json_type>();

            if constexpr (std::is_same_v<json_type, fly::json_object_type>)
            {
                result = json.erase(json.begin(), json.find("b"));
                CATCH_CHECK(json == fly::Json {{"b", 2}});
                CATCH_CHECK(result == json.begin());

                result = json.erase(json.find("b"), json.end());
                CATCH_CHECK(json == json_type());
                CATCH_CHECK(result == json.end());
            }
            else
            {
                result = json.erase(json.begin(), json.begin() + 2);
                CATCH_CHECK(json == fly::Json {9, 10});
                CATCH_CHECK(result == json.begin());

                result = json.erase(json.begin() + 1, json.end());
                CATCH_CHECK(json == fly::Json {9});
                CATCH_CHECK(result == json.end());
            }

            result = json.erase(json.begin(), json.end());
            CATCH_CHECK(json == json_type());
            CATCH_CHECK(result == json.end());
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.erase(fly::Json::const_iterator(), fly::Json::const_iterator()),
                "JSON type invalid for erasure: ({})",
                json);
        }
    }

    CATCH_SECTION("Erase a value from a JSON array")
    {
        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            CATCH_CHECK_THROWS_JSON(json.erase(4), "Given index (4) not found: ({})", json);

            json.erase(0);
            CATCH_CHECK(json == fly::Json {8, 9, 10});

            json.erase(2);
            CATCH_CHECK(json == fly::Json {8, 9});

            json.erase(1);
            CATCH_CHECK(json == fly::Json {8});

            json.erase(0);
            CATCH_CHECK(json == json_type());

            CATCH_CHECK_THROWS_JSON(json.erase(0), "Given index (0) not found: ({})", json);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.erase(0),
                "JSON type invalid for erase(index): ({})",
                json);
        }
    }

    CATCH_SECTION("Swap a JSON instance with another JSON instance")
    {
        fly::Json json1 = 12389;
        fly::Json json2 = "string";
        fly::Json json3 = {1, 2, 3, 8, 9};

        json.swap(json1);
        CATCH_CHECK(json == 12389);
        CATCH_CHECK(json1 == fly::test::create_json<json_type>());

        json.swap(json2);
        CATCH_CHECK(json == "string");
        CATCH_CHECK(json2 == 12389);

        json.swap(json3);
        CATCH_CHECK(json == fly::Json {1, 2, 3, 8, 9});
        CATCH_CHECK(json3 == "string");
    }

    CATCH_SECTION("Swap a JSON instance with an array-like type")
    {
        auto validate = [&json]<typename T1, typename T2, typename T3>(const char *name)
        {
            CATCH_CAPTURE(name);
            json = {1, 2};

            auto test1 = T1 {10, 20, 30, 40};
            auto test2 =
                T2 {FLY_JSON_STR("50"), FLY_JSON_STR("60"), FLY_JSON_STR("70"), FLY_JSON_STR("80")};
            auto test3 = T3 {"a", 90, "b", 100};

            CATCH_CHECK_NOTHROW(json.swap(test1));
            CATCH_CHECK(T1(json) == T1 {10, 20, 30, 40});
            CATCH_CHECK(test1 == T1 {1, 2});

            CATCH_CHECK_NOTHROW(json.swap(test2));
            CATCH_CHECK(
                T2(json) ==
                T2 {FLY_JSON_STR("50"),
                    FLY_JSON_STR("60"),
                    FLY_JSON_STR("70"),
                    FLY_JSON_STR("80")});
            CATCH_CHECK(
                test2 ==
                T2 {FLY_JSON_STR("10"),
                    FLY_JSON_STR("20"),
                    FLY_JSON_STR("30"),
                    FLY_JSON_STR("40")});

            CATCH_CHECK_NOTHROW(json.swap(test1));
            CATCH_CHECK(T1(json) == T1 {1, 2});
            CATCH_CHECK(test1 == T1 {50, 60, 70, 80});

            json = {nullptr, true};
            CATCH_CHECK_NOTHROW(json.swap(test3));
            CATCH_CHECK(T3(json) == T3 {"a", 90, "b", 100});
            CATCH_CHECK(test3 == T3 {nullptr, true});
        };

        auto invalidate = [&json]<typename T>(const char *name)
        {
            CATCH_CAPTURE(name);
            T test {};

            CATCH_CHECK_THROWS_JSON(
                json.swap(test),
                "JSON type invalid for swap(array): ({})",
                json);
        };

        fly::test::run_test_for_array_types<json_type, fly::json_string_type>(
            std::move(validate),
            std::move(invalidate));
    }

    CATCH_SECTION("Merge a JSON instance into another JSON instance")
    {
        fly::Json object1 = {{"c", 3}, {"d", 4}};
        fly::Json object2 = {{"d", 5}, {"e", 6}};
        fly::Json object3 = {{"f", 7}, {"g", 8}};

        fly::Json int1 = fly::test::create_json<fly::json_signed_integer_type>();
        fly::Json int2 = fly::test::create_json<fly::json_signed_integer_type>();

        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::json_object_type>)
        {
            CATCH_CHECK_THROWS_JSON(
                json.merge(int1),
                "Other JSON type invalid for merging: ({})",
                int1);
            CATCH_CHECK_THROWS_JSON(
                json.merge(std::move(int2)),
                "Other JSON type invalid for merging: ({})",
                fly::test::create_json<fly::json_signed_integer_type>());

            CATCH_CHECK_NOTHROW(json.merge(object1));
            CATCH_REQUIRE(json.contains("c"));
            CATCH_CHECK(json["c"] == 3);
            CATCH_REQUIRE(json.contains("d"));
            CATCH_CHECK(json["d"] == 4);
            CATCH_CHECK_FALSE(object1.contains("c"));
            CATCH_CHECK_FALSE(object1.contains("d"));

            CATCH_CHECK_NOTHROW(json.merge(object2));
            CATCH_REQUIRE(json.contains("d"));
            CATCH_CHECK(json["d"] == 4);
            CATCH_REQUIRE(json.contains("e"));
            CATCH_CHECK(json["e"] == 6);
            CATCH_REQUIRE(object2.contains("d"));
            CATCH_CHECK(object2["d"] == 5);
            CATCH_CHECK_FALSE(object2.contains("e"));

            // Reset the JSON instance to allow null types to promote to objects during merge(&&).
            json = fly::test::create_json<json_type>();

            CATCH_CHECK_NOTHROW(json.merge(std::move(object3)));
            CATCH_REQUIRE(json.contains("f"));
            CATCH_CHECK(json["f"] == 7);
            CATCH_REQUIRE(json.contains("g"));
            CATCH_CHECK(json["g"] == 8);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.merge(object1),
                "JSON type invalid for merging: ({})",
                json);
            CATCH_CHECK_THROWS_JSON(
                json.merge(std::move(object2)),
                "JSON type invalid for merging: ({})",
                json);
        }
    }
}

CATCH_JSON_STRING_TEST_CASE("JsonModifiersByString")
{
    using json_type = typename TestType::first_type;
    using string_type = typename TestType::second_type;
    using char_type = typename string_type::value_type;

    fly::Json json = fly::test::create_json<json_type, string_type>();
    const string_type key = J_STR("k\\u0065y"); // "key"

    CATCH_SECTION("Insert a value into a JSON object")
    {
        const fly::Json value1 = 3;
        const fly::Json value2 = 4;

        if constexpr (std::is_same_v<json_type, fly::json_object_type>)
        {
            auto result = json.insert(key, value1);
            CATCH_CHECK(result.second);
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == value1);

            result = json.insert(key, value2);
            CATCH_CHECK_FALSE(result.second);
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == value1);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(key, value1),
                "JSON type invalid for object insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert a moved value into a JSON object")
    {
        fly::Json value1 = 3;
        fly::Json value2 = 4;

        if constexpr (std::is_same_v<json_type, fly::json_object_type>)
        {
            auto result = json.insert(key, std::move(value1));
            CATCH_CHECK(result.second);
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == 3);

            result = json.insert(key, std::move(value2));
            CATCH_CHECK_FALSE(result.second);
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == 3);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(key, std::move(value1)),
                "JSON type invalid for object insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert a value into or assign a value in a JSON object")
    {
        fly::Json value1 = 3;
        fly::Json value2 = 4;

        if constexpr (std::is_same_v<json_type, fly::json_object_type>)
        {
            auto result = json.insert_or_assign(key, std::move(value1));
            CATCH_CHECK(result.second);
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == 3);

            result = json.insert_or_assign(key, std::move(value2));
            CATCH_CHECK_FALSE(result.second);
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == 4);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(key, std::move(value1)),
                "JSON type invalid for object insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Insert a range of values into a JSON object")
    {
        const fly::Json object = {{J_STR("c"), fly::Json(3)}, {J_STR("d"), fly::Json(4)}};

        if constexpr (std::is_same_v<json_type, fly::json_object_type>)
        {
            const fly::Json array = {J_STR("c"), J_STR("d")};

            CATCH_CHECK_THROWS_JSON(
                json.insert(object.begin(), fly::Json::const_iterator()),
                "Provided iterators are for different Json instances");
            CATCH_CHECK_THROWS_JSON(
                json.insert(object.begin(), array.end()),
                "Provided iterators are for different Json instances");
            CATCH_CHECK_THROWS_JSON(
                json.insert(fly::Json::const_iterator(), fly::Json::const_iterator()),
                "Provided iterators' JSON type invalid for object insertion");
            CATCH_CHECK_THROWS_JSON(
                json.insert(array.begin(), array.end()),
                "Provided iterators' JSON type invalid for object insertion");

            json.insert(object.begin(), object.end());
            CATCH_REQUIRE(json.size() == 4);
            CATCH_CHECK(json["c"] == 3);
            CATCH_CHECK(json["d"] == 4);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.insert(object.begin(), object.end()),
                "JSON type invalid for object insertion: ({})",
                json);
        }
    }

    CATCH_SECTION("Emplace a value into a JSON object")
    {
        fly::Json value1 = 3;
        fly::Json value2 = 4;

        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::json_object_type>)
        {
            auto result = json.emplace(key, std::move(value1));
            CATCH_CHECK(result.second);
            CATCH_REQUIRE(result.first != json.end());
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == 3);

            result = json.emplace(key, std::move(value2));
            CATCH_CHECK_FALSE(result.second);
            CATCH_REQUIRE(result.first != json.end());
            CATCH_CHECK(result.first == json.find("key"));
            CATCH_CHECK(*(result.first) == 3);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.emplace(key, std::move(value1)),
                "JSON type invalid for object emplacement: ({})",
                json);
        }
    }

    CATCH_SECTION("Erase a value from a JSON object")
    {
        if constexpr (std::is_same_v<json_type, fly::json_object_type>)
        {
            auto result = json.erase(J_STR("a"));
            CATCH_CHECK(json == fly::Json {{"b", 2}});
            CATCH_CHECK(result == 1);

            result = json.erase(J_STR("b"));
            CATCH_CHECK(json == json_type());
            CATCH_CHECK(result == 1);

            result = json.erase(J_STR("c"));
            CATCH_CHECK(json == json_type());
            CATCH_CHECK(result == 0);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.erase(J_STR("a")),
                "JSON type invalid for erase(key): ({})",
                json);
        }
    }

    CATCH_SECTION("Swap a JSON instance with a string-like type")
    {
        string_type str = J_STR("ghijkl");

        if constexpr (std::is_same_v<json_type, fly::json_string_type>)
        {
            CATCH_CHECK_NOTHROW(json.swap(str));
            CATCH_CHECK(json == "ghijkl");
            CATCH_CHECK(str == J_STR("abcdef"));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json.swap(str),
                "JSON type invalid for swap(string): ({})",
                json);
        }
    }

    CATCH_SECTION("Swap a JSON instance with an object-like type")
    {
        auto validate = [&json]<typename T1, typename T2, typename T3>(const char *name)
        {
            CATCH_CAPTURE(name);
            json = {{J_STR("c"), 100}, {J_STR("d"), 200}};

            auto test1 = T1 {{J_STR("a"), 2}, {J_STR("b"), 4}};
            auto test2 = T2 {{J_STR("a"), "2"}, {J_STR("b"), "4"}};
            auto test3 = T3 {{J_STR("a"), 5}, {J_STR("b"), "6"}};

            CATCH_CHECK_NOTHROW(json.swap(test1));
            CATCH_CHECK(json == T1 {{J_STR("a"), 2}, {J_STR("b"), 4}});
            CATCH_CHECK(test1 == T1 {{J_STR("c"), 100}, {J_STR("d"), 200}});

            CATCH_CHECK_NOTHROW(json.swap(test2));
            CATCH_CHECK(json == T2 {{J_STR("a"), "2"}, {J_STR("b"), "4"}});
            CATCH_CHECK(test2 == T2 {{J_STR("a"), "2"}, {J_STR("b"), "4"}});

            CATCH_CHECK_NOTHROW(json.swap(test3));
            CATCH_CHECK(json == T3 {{J_STR("a"), 5}, {J_STR("b"), "6"}});
            CATCH_CHECK(test3 == T3 {{J_STR("a"), "2"}, {J_STR("b"), "4"}});

            CATCH_CHECK_NOTHROW(json.swap(test1));
            CATCH_CHECK(json == T1 {{J_STR("c"), 100}, {J_STR("d"), 200}});
            CATCH_CHECK(test1 == T1 {{J_STR("a"), 5}, {J_STR("b"), 6}});
        };

        auto invalidate = [&json]<typename T>(const char *name)
        {
            CATCH_CAPTURE(name);
            T test {};

            CATCH_CHECK_THROWS_JSON(
                json.swap(test),
                "JSON type invalid for swap(object): ({})",
                json);
        };

        fly::test::run_test_for_object_types<json_type, string_type>(
            std::move(validate),
            std::move(invalidate));
    }

    CATCH_SECTION("Merge an object-like type into a JSON instance")
    {
        auto validate = [&json]<typename T1, typename T2, typename T3>(const char *name)
        {
            json = fly::test::create_json<json_type, string_type>();
            CATCH_CAPTURE(name);

            auto test1 = T1 {{J_STR("c"), 3}, {J_STR("d"), 4}};
            auto test2 = T2 {{J_STR("d"), "5"}, {J_STR("e"), "6"}};
            auto test3 = T3 {{J_STR("f"), 7}, {J_STR("g"), "8"}};

            CATCH_CHECK_NOTHROW(json.merge(test1));
            CATCH_REQUIRE(json.contains(J_STR("c")));
            CATCH_CHECK(json[J_STR("c")] == 3);
            CATCH_REQUIRE(json.contains(J_STR("d")));
            CATCH_CHECK(json[J_STR("d")] == 4);
            CATCH_CHECK_FALSE(test1.contains(J_STR("c")));
            CATCH_CHECK_FALSE(test1.contains(J_STR("d")));

            CATCH_CHECK_NOTHROW(json.merge(test2));
            CATCH_REQUIRE(json.contains(J_STR("d")));
            CATCH_CHECK(json[J_STR("d")] == 4);
            CATCH_REQUIRE(json.contains(J_STR("e")));
            CATCH_CHECK(json[J_STR("e")] == "6");
            CATCH_REQUIRE(test2.contains(J_STR("d")));
            CATCH_CHECK(test2.find(J_STR("d"))->second == "5");
            CATCH_CHECK_FALSE(test2.contains(J_STR("e")));

            // Reset the JSON instance to allow null types to promote to objects during merge(&&).
            json = fly::test::create_json<json_type, string_type>();

            CATCH_CHECK_NOTHROW(json.merge(std::move(test3)));
            CATCH_REQUIRE(json.contains(J_STR("f")));
            CATCH_CHECK(json[J_STR("f")] == 7);
            CATCH_REQUIRE(json.contains(J_STR("g")));
            CATCH_CHECK(json[J_STR("g")] == "8");
        };

        auto invalidate = [&json]<typename T>(const char *name)
        {
            CATCH_CAPTURE(name);
            T test {};

            CATCH_CHECK_THROWS_JSON(json.merge(test), "JSON type invalid for merging: ({})", json);
            CATCH_CHECK_THROWS_JSON(
                json.merge(std::move(test)),
                "JSON type invalid for merging: ({})",
                json);
        };

        fly::test::run_test_for_object_types<
            json_type,
            string_type,
            decltype(validate),
            decltype(invalidate),
            fly::test::is_null_or_other_type_v<json_type, fly::json_object_type>>(
            std::move(validate),
            std::move(invalidate));
    }
}
