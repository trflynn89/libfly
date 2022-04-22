#include "test/types/json/json_helpers.hpp"

#include "fly/fly.hpp"
#include "fly/types/json/json.hpp"

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include <array>

CATCH_JSON_STRING_TEST_CASE("JsonConversion")
{
    using json_type = typename TestType::first_type;
    using string_type = typename TestType::second_type;
    using char_type = typename string_type::value_type;

    fly::Json json = fly::test::create_json<json_type, string_type>();
    fly::Json empty = json_type();

    CATCH_SECTION("Convert a JSON instance to string-like types")
    {
        if constexpr (std::is_same_v<json_type, fly::json_string_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("abcdef"));
            CATCH_CHECK(string_type(empty) == J_STR(""));
        }
        else if constexpr (fly::JsonNumber<json_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("1"));
            CATCH_CHECK(string_type(empty) == J_STR("0"));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(string_type(json), "JSON type is not a string: ({})", json);
        }
    }

    CATCH_SECTION("Convert a JSON instance to object-like types")
    {
        auto validate = [&json]<typename T1, typename T2, typename T3>(char const *name) {
            CATCH_CAPTURE(name);

            auto test1 = T1 {{J_STR("a"), 2}, {J_STR("b"), 4}};
            auto test2 = T2 {{J_STR("a"), "2"}, {J_STR("b"), "4"}};
            auto test3 = T3 {{J_STR("a"), 2}, {J_STR("b"), "4"}};

            json = test1;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);

            json = test2;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);

            json = test3;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);
            CATCH_CHECK(T3(json) == test3);

            json = {{"a", true}};
            CATCH_CHECK_THROWS_JSON(T1(json), "JSON type is not numeric: ({})", json["a"]);

            json = {{"a", J_STR("string")}};
            CATCH_CHECK_THROWS_JSON(T1(json), "JSON type is not numeric: ({})", json["a"]);
        };

        auto invalidate = [&json]<typename T>(char const *name) {
            CATCH_CAPTURE(name);

            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((T(json))),
                "JSON type is not an object: ({})",
                json);
        };

        fly::test::run_test_for_object_types<json_type, string_type>(
            std::move(validate),
            std::move(invalidate));
    }

    CATCH_SECTION("Convert a JSON instance to array-like types")
    {
        auto validate = [&json]<typename T1, typename T2, typename T3>(char const *name) {
            CATCH_CAPTURE(name);

            auto test1 = T1 {50, 60, 70, 80};
            auto test2 = T2 {J_STR("50"), J_STR("60"), J_STR("70"), J_STR("80")};
            auto test3 = T3 {50, J_STR("60"), 70, J_STR("80")};

            json = test1;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);

            json = test2;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);

            json = test3;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);
            CATCH_CHECK(T3(json) == test3);

            json = {true};
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::array<int, 1>(json))),
                "JSON type is not numeric: ({})",
                json[0]);

            json = {J_STR("string")};
            CATCH_CHECK_THROWS_JSON(
                FLY_UNUSED((std::array<int, 1>(json))),
                "JSON type is not numeric: ({})",
                json[0]);
        };

        auto invalidate = [&json]<typename T>(char const *name) {
            CATCH_CAPTURE(name);

            CATCH_CHECK_THROWS_JSON(FLY_UNUSED((T(json))), "JSON type is not an array: ({})", json);
        };

        fly::test::run_test_for_array_types<json_type, string_type>(
            std::move(validate),
            std::move(invalidate));

        // Extra test to ensure std::array lengths are accounted for.
        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            std::array<int, 1> array1 = {7};
            std::array<int, 2> array2 = {7, 8};
            std::array<int, 3> array3 = {7, 8, 0};
            json = array2;

            CATCH_CHECK(decltype(array1)(json) == array1);
            CATCH_CHECK(decltype(array2)(json) == array2);
            CATCH_CHECK(decltype(array3)(json) == array3);
        }
    }

    CATCH_SECTION("Convert a JSON instance to Boolean-like types")
    {
        if constexpr (std::is_same_v<json_type, fly::json_null_type>)
        {
            CATCH_CHECK_FALSE(bool(json));
            CATCH_CHECK_FALSE(bool(empty));
        }
        else
        {
            CATCH_CHECK(bool(json));
            CATCH_CHECK_FALSE(bool(empty));
        }
    }

    CATCH_SECTION("Convert a JSON instance to signed-integer-like types")
    {
        if constexpr (fly::JsonNumber<json_type>)
        {
            CATCH_CHECK(int(json) == 1);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: ({})", json);

            if constexpr (std::is_same_v<json_type, fly::json_string_type>)
            {
                json = J_STR("-123");
                CATCH_CHECK(int(json) == int(-123));

                json = J_STR("123");
                CATCH_CHECK(int(json) == 123);
            }
        }
    }

    CATCH_SECTION("Convert a JSON instance to unsigned-integer-like types")
    {
        if constexpr (fly::JsonNumber<json_type>)
        {
            CATCH_CHECK(unsigned(json) == unsigned(1));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: ({})", json);

            if constexpr (std::is_same_v<json_type, fly::json_string_type>)
            {
                json = J_STR("-123");
                CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: ({})", json);

                json = J_STR("123");
                CATCH_CHECK(unsigned(json) == unsigned(123));
            }
        }
    }

    CATCH_SECTION("Convert a JSON instance to floating-point-like types")
    {
        if constexpr (fly::JsonNumber<json_type>)
        {
            CATCH_CHECK(float(json) == Catch::Approx(float(1)));
            CATCH_CHECK(double(json) == Catch::Approx(double(1)));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: ({})", json);

            if constexpr (std::is_same_v<json_type, fly::json_string_type>)
            {
                json = J_STR("123.5");
                CATCH_CHECK(float(json) == Catch::Approx(123.5f));
                CATCH_CHECK(double(json) == Catch::Approx(123.5f));
            }
        }
    }

    CATCH_SECTION("Convert a JSON instance to null-like types")
    {
        if constexpr (std::is_same_v<json_type, fly::json_null_type>)
        {
            CATCH_CHECK(std::nullptr_t(json) == nullptr);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: ({})", json);
        }
    }
}

CATCH_JSON_TEST_CASE("JsonMoveConversion")
{
    using json_type = TestType;

    fly::Json json = fly::test::create_json<json_type>();
    fly::Json empty = json_type();

    CATCH_SECTION("Transfer a JSON instance to a string")
    {
        if constexpr (std::is_same_v<json_type, fly::json_string_type>)
        {
            auto json_value = fly::json_string_type(std::move(json));
            CATCH_CHECK(json_value == "abcdef");
            CATCH_CHECK(json.is_null());

            auto empty_value = fly::json_string_type(std::move(empty));
            CATCH_CHECK(empty_value == "");
            CATCH_CHECK(empty.is_null());
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                fly::json_string_type(std::move(json)),
                "JSON type is not a string: ({})",
                json);
        }
    }

    CATCH_SECTION("Transfer a JSON instance to an object")
    {
        if constexpr (std::is_same_v<json_type, fly::json_object_type>)
        {
            auto json_value = fly::json_object_type(std::move(json));
            CATCH_CHECK(json_value == fly::json_object_type {{"a", 1}, {"b", 2}});
            CATCH_CHECK(json.is_null());

            auto empty_value = fly::json_object_type(std::move(empty));
            CATCH_CHECK(empty_value == fly::json_object_type {});
            CATCH_CHECK(empty.is_null());
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                fly::json_object_type(std::move(json)),
                "JSON type is not an object: ({})",
                json);
        }
    }

    CATCH_SECTION("Transfer a JSON instance to an array")
    {
        if constexpr (std::is_same_v<json_type, fly::json_array_type>)
        {
            auto json_value = fly::json_array_type(std::move(json));
            CATCH_CHECK(json_value == fly::json_array_type {'7', 8, 9, 10});
            CATCH_CHECK(json.is_null());

            auto empty_value = fly::json_array_type(std::move(empty));
            CATCH_CHECK(empty_value == fly::json_array_type {});
            CATCH_CHECK(empty.is_null());
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(
                fly::json_array_type(std::move(json)),
                "JSON type is not an array: ({})",
                json);
        }
    }
}
