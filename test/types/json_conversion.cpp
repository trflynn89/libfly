#include "test/types/json_helpers.hpp"

#include "fly/fly.hpp"
#include "fly/types/json/json.hpp"

#include "catch2/catch.hpp"

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
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("null"));
            CATCH_CHECK(string_type(empty) == J_STR("null"));
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::string_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("abcdef"));
            CATCH_CHECK(string_type(empty) == J_STR(""));
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::object_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("{\"a\":1,\"b\":2}"));
            CATCH_CHECK(string_type(empty) == J_STR("{}"));
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::array_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("[55,8,9,10]"));
            CATCH_CHECK(string_type(empty) == J_STR("[]"));
        }
        else if constexpr (std::is_same_v<json_type, fly::JsonTraits::boolean_type>)
        {
            CATCH_CHECK(string_type(json) == J_STR("true"));
            CATCH_CHECK(string_type(empty) == J_STR("false"));
        }
        else
        {
            CATCH_CHECK(string_type(json) == J_STR("1"));
            CATCH_CHECK(string_type(empty) == J_STR("0"));
        }
    }

    CATCH_SECTION("Convert a JSON instance to object-like types")
    {
        auto validate = [&json](auto *name, auto &test1, auto &test2, auto &test3)
        {
            CATCH_CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;
            using T3 = std::decay_t<decltype(test3)>;

            test1 = T1 {{J_STR("a"), 2}, {J_STR("b"), 4}};
            test2 = T2 {{J_STR("a"), "2"}, {J_STR("b"), "4"}};
            test3 = T3 {{J_STR("a"), 2}, {J_STR("b"), "4"}};

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

        auto invalidate = [&json](auto *name, auto &test)
        {
            CATCH_CAPTURE(name);

            using T = std::decay_t<decltype(test)>;

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
        auto validate2 = [&json](auto *name, auto &test1, auto &test2)
        {
            CATCH_CAPTURE(name);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;

            test1 = T1 {50, 60, 70, 80};
            test2 = T2 {J_STR("50"), J_STR("60"), J_STR("70"), J_STR("80")};

            json = test1;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);

            json = test2;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);

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

        auto validate3 = [&json, &validate2](auto *name, auto &test1, auto &test2, auto &test3)
        {
            validate2(name, test1, test2);

            using T1 = std::decay_t<decltype(test1)>;
            using T2 = std::decay_t<decltype(test2)>;
            using T3 = std::decay_t<decltype(test3)>;

            test3 = T3 {50, J_STR("60"), 70, J_STR("80")};

            json = test3;
            CATCH_CHECK(T1(json) == test1);
            CATCH_CHECK(T2(json) == test2);
            CATCH_CHECK(T3(json) == test3);
        };

        auto invalidate = [&json](auto *name, auto &test)
        {
            CATCH_CAPTURE(name);

            using T = std::decay_t<decltype(test)>;

            CATCH_CHECK_THROWS_JSON(FLY_UNUSED((T(json))), "JSON type is not an array: ({})", json);
        };

        fly::test::run_test_for_array_types<json_type, string_type>(
            std::move(validate2),
            std::move(validate3),
            std::move(invalidate));

        // Extra test to ensure std::array lengths are accounted for.
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::array_type>)
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
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
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
        if constexpr (fly::JsonTraits::is_number_v<json_type>)
        {
            CATCH_CHECK(int(json) == 1);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: ({})", json);

            if constexpr (std::is_same_v<json_type, fly::JsonTraits::string_type>)
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
        if constexpr (fly::JsonTraits::is_number_v<json_type>)
        {
            CATCH_CHECK(unsigned(json) == unsigned(1));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(unsigned(json), "JSON type is not numeric: ({})", json);

            if constexpr (std::is_same_v<json_type, fly::JsonTraits::string_type>)
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
        if constexpr (fly::JsonTraits::is_number_v<json_type>)
        {
            CATCH_CHECK(float(json) == Approx(float(1)));
            CATCH_CHECK(double(json) == Approx(double(1)));
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(int(json), "JSON type is not numeric: ({})", json);

            if constexpr (std::is_same_v<json_type, fly::JsonTraits::string_type>)
            {
                json = J_STR("123.5");
                CATCH_CHECK(float(json) == Approx(123.5f));
                CATCH_CHECK(double(json) == Approx(123.5f));
            }
        }
    }

    CATCH_SECTION("Convert a JSON instance to null-like types")
    {
        if constexpr (std::is_same_v<json_type, fly::JsonTraits::null_type>)
        {
            CATCH_CHECK(std::nullptr_t(json) == nullptr);
        }
        else
        {
            CATCH_CHECK_THROWS_JSON(std::nullptr_t(json), "JSON type is not null: ({})", json);
        }
    }
}
