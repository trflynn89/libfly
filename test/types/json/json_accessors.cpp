#include "test/types/json/json_helpers.hpp"

#include "fly/types/json/json.hpp"

#include "catch2/catch.hpp"

CATCH_JSON_TEST_CASE("JsonAccessors")
{
    using json_type = TestType;

    fly::Json json1 = fly::test::create_json<json_type>();
    const fly::Json json2 = fly::test::create_json<json_type>();

    CATCH_SECTION("Access a JSON array's values via the accessor 'at'")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::array_type>)
        {
            CATCH_CHECK(json1.at(0) == '7');
            CATCH_CHECK(json1.at(1) == 8);
            CATCH_CHECK(json1.at(2) == 9);
            CATCH_CHECK(json1.at(3) == 10);
            CATCH_CHECK_THROWS_JSON(json1.at(4), "Given index (4) not found: ({})", json1);

            CATCH_CHECK(json2.at(0) == '7');
            CATCH_CHECK(json2.at(1) == 8);
            CATCH_CHECK(json2.at(2) == 9);
            CATCH_CHECK(json2.at(3) == 10);
            CATCH_CHECK_THROWS_JSON(json2.at(4), "Given index (4) not found: ({})", json2);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.at(0),
                "JSON type invalid for operator[index]: ({})",
                json1);
            CATCH_CHECK_THROWS_JSON(
                json2.at(0),
                "JSON type invalid for operator[index]: ({})",
                json2);
        }
    }

    CATCH_SECTION("Access a JSON array's values via the access operator")
    {
        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::JsonTraits::array_type>)
        {
            if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
            {
                CATCH_CHECK_NOTHROW(json1[0]);
                CATCH_CHECK(json1.is_array());
                CATCH_CHECK(json1[0] == nullptr);

                CATCH_CHECK_THROWS_JSON(
                    json2[0],
                    "JSON type invalid for operator[index]: ({})",
                    json2);
            }
            else
            {
                CATCH_CHECK(json1[0] == '7');
                CATCH_CHECK(json1[1] == 8);
                CATCH_CHECK(json1[2] == 9);
                CATCH_CHECK(json1[3] == 10);
                CATCH_CHECK_NOTHROW(json1[4]);
                CATCH_CHECK(json1[4] == nullptr);

                CATCH_CHECK(json2[0] == '7');
                CATCH_CHECK(json2[1] == 8);
                CATCH_CHECK(json2[2] == 9);
                CATCH_CHECK(json2[3] == 10);
                CATCH_CHECK_THROWS_JSON(json2[4], "Given index (4) not found: ({})", json2);
            }
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(json1[0], "JSON type invalid for operator[index]: ({})", json1);
            CATCH_CHECK_THROWS_JSON(json2[0], "JSON type invalid for operator[index]: ({})", json2);
        }
    }

    CATCH_SECTION("Access a JSON instance's front element")
    {
        if constexpr (fly::JsonTraits::is_iterable_v<json_type>)
        {
            auto &front1 = json1.front();
            CATCH_CHECK(front1 == *(json1.begin()));

            const auto &front2 = json2.front();
            CATCH_CHECK(front2 == *(json2.begin()));

            fly::Json empty = json_type();
            CATCH_CHECK_THROWS_NULL_WITH(empty.front(), empty);
        }
        else
        {
            CATCH_CHECK_THROWS_ITERATOR(
                json1.front(),
                "JSON type invalid for iteration: ({})",
                json1);
            CATCH_CHECK_THROWS_ITERATOR(
                json2.front(),
                "JSON type invalid for iteration: ({})",
                json2);
        }
    }

    CATCH_SECTION("Access a JSON instance's back element")
    {
        if constexpr (fly::JsonTraits::is_iterable_v<json_type>)
        {
            auto &back1 = json1.back();
            CATCH_CHECK(back1 == *(--(json1.end())));

            const auto &back2 = json2.back();
            CATCH_CHECK(back2 == *(--(json2.end())));

            fly::Json empty = json_type();
            CATCH_CHECK_THROWS_NULL_WITH(empty.back(), empty);
        }
        else
        {
            CATCH_CHECK_THROWS_ITERATOR(
                json1.back(),
                "JSON type invalid for iteration: ({})",
                json1);
            CATCH_CHECK_THROWS_ITERATOR(
                json2.back(),
                "JSON type invalid for iteration: ({})",
                json2);
        }
    }

    CATCH_SECTION("Check a JSON instance for emptiness")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
        {
            CATCH_CHECK(json1.empty());
            CATCH_CHECK(json2.empty());
        }
        else
        {
            CATCH_CHECK_FALSE(json1.empty());
            CATCH_CHECK_FALSE(json2.empty());

            if constexpr (fly::test::is_object_or_array_or_string_v<json_type>)
            {
                CATCH_CHECK(fly::Json(json_type()).empty());
            }
        }
    }

    CATCH_SECTION("Check the size of a JSON instance")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
        {
            CATCH_CHECK(json1.size() == 0);
            CATCH_CHECK(json2.size() == 0);
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::string_type>)
        {
            CATCH_CHECK(json1.size() == 6);
            CATCH_CHECK(json2.size() == 6);
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            CATCH_CHECK(json1.size() == 2);
            CATCH_CHECK(json2.size() == 2);
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::array_type>)
        {
            CATCH_CHECK(json1.size() == 4);
            CATCH_CHECK(json2.size() == 4);
        }
        else
        {
            CATCH_CHECK(json1.size() == 1);
            CATCH_CHECK(json2.size() == 1);
        }
    }

    CATCH_SECTION("Change the size of a JSON instance")
    {
        constexpr bool is_string_or_array =
            fly::any_same_v<json_type, fly::JsonTraits::string_type, fly::JsonTraits::array_type>;

        if constexpr (is_string_or_array)
        {
            const auto size_before = json1.size();

            json1.resize(size_before * 2);
            CATCH_CHECK(json1.size() == (size_before * 2));

            json1.resize(size_before);
            CATCH_CHECK(json1.size() == size_before);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.resize(1),
                "JSON type invalid for capacity operations: ({})",
                json1);
        }
    }

    CATCH_SECTION("Check the capacity of a JSON instance")
    {
        constexpr bool is_string_or_array =
            fly::any_same_v<json_type, fly::JsonTraits::string_type, fly::JsonTraits::array_type>;

        if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
        {
            CATCH_CHECK(json1.capacity() == 0);
            CATCH_CHECK(json2.capacity() == 0);
        }
        else if constexpr (is_string_or_array)
        {
            const auto capacity1 = json1.capacity();
            const auto capacity2 = json2.capacity();
            CATCH_CHECK(capacity1 == capacity2);
            CATCH_CHECK(capacity1 > 0);
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            CATCH_CHECK(json1.capacity() == 2);
            CATCH_CHECK(json2.capacity() == 2);
        }
        else
        {
            CATCH_CHECK(json1.capacity() == 1);
            CATCH_CHECK(json2.capacity() == 1);
        }
    }

    CATCH_SECTION("Change the capacity of a JSON instance")
    {
        constexpr bool is_string_or_array =
            fly::any_same_v<json_type, fly::JsonTraits::string_type, fly::JsonTraits::array_type>;

        if constexpr (is_string_or_array)
        {
            const auto capacity_before = json1.capacity();
            json1.reserve(capacity_before * 2);
            const auto capacity_after = json1.capacity();
            CATCH_CHECK(capacity_after > capacity_before);

            json1.reserve(capacity_before);
            CATCH_CHECK(json1.capacity() == capacity_after);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.reserve(1),
                "JSON type invalid for capacity operations: ({})",
                json1);
        }
    }
}

CATCH_JSON_STRING_TEST_CASE("JsonAccessorsByString")
{
    using json_type = typename TestType::first_type;
    using string_type = typename TestType::second_type;
    using char_type = typename string_type::value_type;

    fly::Json json1 = fly::test::create_json<json_type, string_type>();
    const fly::Json json2 = fly::test::create_json<json_type, string_type>();

    CATCH_SECTION("Access a JSON object's values via the accessor 'at'")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            CATCH_CHECK(json1.at(J_STR("a")) == 1);
            CATCH_CHECK(json1.at(J_STR("b")) == 2);
            CATCH_CHECK_THROWS_JSON(json1.at(J_STR("c")), "Given key (c) not found: ({})", json1);

            CATCH_CHECK(json2.at(J_STR("a")) == 1);
            CATCH_CHECK(json2.at(J_STR("b")) == 2);
            CATCH_CHECK_THROWS_JSON(json2.at(J_STR("c")), "Given key (c) not found: ({})", json2);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.at(J_STR("a")),
                "JSON type invalid for operator[key]: ({})",
                json1);
            CATCH_CHECK_THROWS_JSON(
                json2.at(J_STR("a")),
                "JSON type invalid for operator[key]: ({})",
                json2);
        }
    }

    CATCH_SECTION("Access a JSON object's values via the access operator")
    {
        if constexpr (fly::test::is_null_or_other_type_v<json_type, fly::JsonTraits::object_type>)
        {
            if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
            {
                CATCH_CHECK_NOTHROW(json1[J_STR("a")]);
                CATCH_CHECK(json1.is_object());
                CATCH_CHECK(json1[J_STR("a")] == nullptr);

                CATCH_CHECK_THROWS_JSON(
                    json2[J_STR("a")],
                    "JSON type invalid for operator[key]: ({})",
                    json2);
            }
            else
            {
                CATCH_CHECK(json1[J_STR("a")] == 1);
                CATCH_CHECK(json1[J_STR("b")] == 2);
                CATCH_CHECK_NOTHROW(json1[J_STR("c")]);
                CATCH_CHECK(json1[J_STR("c")] == nullptr);

                CATCH_CHECK(json2[J_STR("a")] == 1);
                CATCH_CHECK(json2[J_STR("b")] == 2);
                CATCH_CHECK_THROWS_JSON(json2[J_STR("c")], "Given key (c) not found: ({})", json2);
            }
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1[J_STR("a")],
                "JSON type invalid for operator[key]: ({})",
                json1);
            CATCH_CHECK_THROWS_JSON(
                json2[J_STR("a")],
                "JSON type invalid for operator[key]: ({})",
                json2);
        }
    }

    CATCH_SECTION("Count the number of values with a key in a JSON object")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            const fly::Json empty = fly::JsonTraits::object_type();
            CATCH_CHECK(empty.count(J_STR("a")) == 0);
            CATCH_CHECK(empty.count(J_STR("b")) == 0);
            CATCH_CHECK(empty.count(J_STR("c")) == 0);

            CATCH_CHECK(json1.count(J_STR("a")) == 1);
            CATCH_CHECK(json1.count(J_STR("b")) == 1);
            CATCH_CHECK(json1.count(J_STR("c")) == 0);

            CATCH_CHECK(json2.count(J_STR("a")) == 1);
            CATCH_CHECK(json2.count(J_STR("b")) == 1);
            CATCH_CHECK(json2.count(J_STR("c")) == 0);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.count(J_STR("a")),
                "JSON type invalid for count(key): ({})",
                json1);
            CATCH_CHECK_THROWS_JSON(
                json2.count(J_STR("a")),
                "JSON type invalid for count(key): ({})",
                json2);
        }
    }

    CATCH_SECTION("Find a value with a key in a JSON object")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            auto it1a = json1.find(J_STR("a"));
            auto it1b = json1.find(J_STR("b"));
            auto it1c = json1.find(J_STR("c"));
            CATCH_REQUIRE(it1a != json1.end());
            CATCH_REQUIRE(it1b != json1.end());
            CATCH_CHECK(*it1a == 1);
            CATCH_CHECK(*it1b == 2);
            CATCH_CHECK(it1c == json1.end());

            auto it2a = json2.find(J_STR("a"));
            auto it2b = json2.find(J_STR("b"));
            auto it2c = json2.find(J_STR("c"));
            CATCH_REQUIRE(it2a != json2.cend());
            CATCH_REQUIRE(it2b != json2.cend());
            CATCH_CHECK(*it2a == 1);
            CATCH_CHECK(*it2b == 2);
            CATCH_CHECK(it2c == json2.cend());
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.find(J_STR("a")),
                "JSON type invalid for find(key): ({})",
                json1);
            CATCH_CHECK_THROWS_JSON(
                json2.find(J_STR("a")),
                "JSON type invalid for find(key): ({})",
                json2);
        }
    }

    CATCH_SECTION("Check for the existence of a key in a JSON object")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            const fly::Json empty = fly::JsonTraits::object_type();
            CATCH_CHECK_FALSE(empty.contains(J_STR("a")));
            CATCH_CHECK_FALSE(empty.contains(J_STR("b")));
            CATCH_CHECK_FALSE(empty.contains(J_STR("c")));

            CATCH_CHECK(json1.contains(J_STR("a")));
            CATCH_CHECK(json1.contains(J_STR("b")));
            CATCH_CHECK_FALSE(json1.contains(J_STR("c")));

            CATCH_CHECK(json2.contains(J_STR("a")));
            CATCH_CHECK(json2.contains(J_STR("b")));
            CATCH_CHECK_FALSE(json2.contains(J_STR("c")));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                json1.contains(J_STR("a")),
                "JSON type invalid for contains(key): ({})",
                json1);
            CATCH_CHECK_THROWS_JSON(
                json2.contains(J_STR("a")),
                "JSON type invalid for contains(key): ({})",
                json2);
        }
    }
}
