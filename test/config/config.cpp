#include "fly/config/config.hpp"

#include "fly/types/json/json.hpp"
#include "test/config/test_config.hpp"

#include <catch2/catch.hpp>

#include <cstddef>

CATCH_TEST_CASE("Config", "[config]")
{
    fly::test::TestConfig config;

    CATCH_SECTION("Non-existing values fallback to provided default")
    {
        CATCH_CHECK(config.get_value<std::string>("bad-name", "def") == "def");
    }

    CATCH_SECTION("Values that can't convert to the desired type fallback to provided default")
    {
        const fly::Json values = {{"name", "John Doe"}, {"address", "USA"}};

        config.update(values);

        CATCH_CHECK(config.get_value<int>("name", 12) == 12);
        CATCH_CHECK(config.get_value<std::nullptr_t>("address", nullptr) == nullptr);
    }

    CATCH_SECTION("Mixed conversion of value types")
    {
        const fly::Json values =
            {{"name", "John Doe"}, {"address", "123"}, {"employed", "1"}, {"age", "26.2"}};

        config.update(values);

        CATCH_CHECK(config.get_value<std::string>("name", "") == "John Doe");

        CATCH_CHECK(config.get_value<std::string>("address", "") == "123");
        CATCH_CHECK(config.get_value<int>("address", 0) == 123);
        CATCH_CHECK(config.get_value<unsigned int>("address", 0) == 123);
        CATCH_CHECK(config.get_value<float>("address", 0.0f) == Approx(123.0f));
        CATCH_CHECK(config.get_value<double>("address", 0.0) == Approx(123.0));

        CATCH_CHECK(config.get_value<std::string>("age", "") == "26.2");
        CATCH_CHECK(config.get_value<int>("age", 0) == 0);
        CATCH_CHECK(config.get_value<unsigned int>("age", 0) == 0);
        CATCH_CHECK(config.get_value<float>("age", 0.0f) == Approx(26.2f));
        CATCH_CHECK(config.get_value<double>("age", 0.0) == Approx(26.2));

        CATCH_CHECK(config.get_value<std::string>("employed", "") == "1");
        CATCH_CHECK(config.get_value<bool>("employed", false) == true);
        CATCH_CHECK(config.get_value<int>("employed", 0) == 1);
    }
}
