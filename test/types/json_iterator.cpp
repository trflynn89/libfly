#include "fly/types/json/detail/json_iterator.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/json/json_exception.hpp"

#include <catch2/catch.hpp>

#define CHECK_THROWS_JSON(expression, ...)                                                         \
    CHECK_THROWS_MATCHES(                                                                          \
        expression,                                                                                \
        fly::JsonIteratorException,                                                                \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("JsonIteratorException: " __VA_ARGS__)))

#define CHECK_THROWS_BAD_COMPARISON(expression, json1, json2)                                      \
    CHECK_THROWS_MATCHES(                                                                          \
        expression,                                                                                \
        fly::BadJsonComparisonException,                                                           \
        Catch::Matchers::Exception::ExceptionMessageMatcher(fly::String::format(                   \
            "BadJsonComparisonException: Cannot compare iterators of different JSON instances: "   \
            "(%s) (%s)",                                                                           \
            json1,                                                                                 \
            json2)))

#define CHECK_THROWS_NULL(expression)                                                              \
    CHECK_THROWS_MATCHES(                                                                          \
        expression,                                                                                \
        fly::NullJsonException,                                                                    \
        Catch::Matchers::Exception::ExceptionMessageMatcher(                                       \
            fly::String::format("NullJsonException: Cannot dereference an empty or past-the-end "  \
                                "iterator")))

#define CHECK_THROWS_NULL_WITH(expression, json)                                                   \
    CHECK_THROWS_MATCHES(                                                                          \
        expression,                                                                                \
        fly::NullJsonException,                                                                    \
        Catch::Matchers::Exception::ExceptionMessageMatcher(fly::String::format(                   \
            "NullJsonException: Cannot dereference an empty or past-the-end "                      \
            "iterator: (%s)",                                                                      \
            json)))

#define CHECK_THROWS_OUT_OF_RANGE(expression, offset, json)                                        \
    CHECK_THROWS_MATCHES(                                                                          \
        expression,                                                                                \
        fly::OutOfRangeJsonException,                                                              \
        Catch::Matchers::Exception::ExceptionMessageMatcher(fly::String::format(                   \
            "OutOfRangeJsonException: Offset %d is out-of-range: (%s)",                            \
            offset,                                                                                \
            json)))

TEST_CASE("JsonIterator", "[json]")
{
    using iterator = fly::Json::iterator;

    SECTION("Check JSON types that are allowed to provide iterators")
    {
        fly::Json null = nullptr;
        CHECK_THROWS_JSON(
            iterator(&null, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            null);

        fly::Json string = "abc";
        CHECK_THROWS_JSON(
            iterator(&string, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        CHECK_NOTHROW(iterator(&object, iterator::Position::Begin));

        fly::Json array = {'7', 8};
        CHECK_NOTHROW(iterator(&array, iterator::Position::Begin));

        fly::Json boolean = true;
        CHECK_THROWS_JSON(
            iterator(&boolean, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            boolean);

        fly::Json sign = 1;
        CHECK_THROWS_JSON(
            iterator(&sign, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        CHECK_THROWS_JSON(
            iterator(&unsign, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            unsign);

        fly::Json floatt = 1.0f;
        CHECK_THROWS_JSON(
            iterator(&floatt, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            floatt);
    }

    SECTION("Must not be able to perform operations on null iterators")
    {
        iterator it1;
        iterator it2;

        CHECK_THROWS_NULL(*it1);
        CHECK_THROWS_NULL(it1->empty());
        CHECK_THROWS_NULL(it1[0]);
        CHECK_THROWS_NULL(++it1);
        CHECK_THROWS_NULL(it1++);
        CHECK_THROWS_NULL(--it1);
        CHECK_THROWS_NULL(it1--);
        CHECK_THROWS_NULL(it1 += 1);
        CHECK_THROWS_NULL(it1 -= 1);
        CHECK_THROWS_NULL(it1 + 1);
        CHECK_THROWS_NULL(1 + it1);
        CHECK_THROWS_NULL(it1 - 1);
        CHECK_THROWS_NULL(it1.key());
        CHECK_THROWS_NULL(it1.value());

        CHECK_NOTHROW(iterator(it1));
        CHECK_NOTHROW(it2 = it1);
    }

    SECTION("Must not be able to perform operations on manually constructed null iterators")
    {
        iterator it1(nullptr, iterator::Position::Begin);
        iterator it2(nullptr, iterator::Position::Begin);

        CHECK_THROWS_NULL(*it1);
        CHECK_THROWS_NULL(it1->empty());
        CHECK_THROWS_NULL(it1[0]);
        CHECK_THROWS_NULL(++it1);
        CHECK_THROWS_NULL(it1++);
        CHECK_THROWS_NULL(--it1);
        CHECK_THROWS_NULL(it1--);
        CHECK_THROWS_NULL(it1 += 1);
        CHECK_THROWS_NULL(it1 -= 1);
        CHECK_THROWS_NULL(it1 + 1);
        CHECK_THROWS_NULL(1 + it1);
        CHECK_THROWS_NULL(it1 - 1);
        CHECK_THROWS_NULL(it1.key());
        CHECK_THROWS_NULL(it1.value());

        CHECK_NOTHROW(iterator(it1));
        CHECK_NOTHROW(it2 = it1);
    }

    SECTION("Must not be able to compare null iterators")
    {
        iterator it1;
        iterator it2;

        CHECK_THROWS_NULL(it1 == it2);
        CHECK_THROWS_NULL(it1 != it2);
        CHECK_THROWS_NULL(it1 < it2);
        CHECK_THROWS_NULL(it1 <= it2);
        CHECK_THROWS_NULL(it1 > it2);
        CHECK_THROWS_NULL(it1 >= it2);
        CHECK_THROWS_NULL(it1 - it2);
    }

    SECTION("Must not be able to compare a null iterator to a non-null iterator")
    {
        fly::Json json {1, 2, 3};

        iterator it1;
        iterator it2 = json.begin();

        CHECK_THROWS_NULL(it1 == it2);
        CHECK_THROWS_NULL(it1 != it2);
        CHECK_THROWS_NULL(it1 < it2);
        CHECK_THROWS_NULL(it1 <= it2);
        CHECK_THROWS_NULL(it1 > it2);
        CHECK_THROWS_NULL(it1 >= it2);
        CHECK_THROWS_NULL(it1 - it2);
    }

    SECTION("Must not be able to compare a non-null iterator to a null iterator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2;

        CHECK_THROWS_NULL(it1 == it2);
        CHECK_THROWS_NULL(it1 != it2);
        CHECK_THROWS_NULL(it1 < it2);
        CHECK_THROWS_NULL(it1 <= it2);
        CHECK_THROWS_NULL(it1 > it2);
        CHECK_THROWS_NULL(it1 >= it2);
        CHECK_THROWS_NULL(it1 - it2);
    }

    SECTION("Must not be able to compare iterators to different JSON values")
    {
        fly::Json json1 {1, 2, 3};
        fly::Json json2 {4, 5, 6};

        iterator it1 = json1.begin();
        iterator it2 = json2.begin();

        CHECK_THROWS_BAD_COMPARISON(it1 == it2, json1, json2);
        CHECK_THROWS_BAD_COMPARISON(it1 != it2, json1, json2);
        CHECK_THROWS_BAD_COMPARISON(it1 < it2, json1, json2);
        CHECK_THROWS_BAD_COMPARISON(it1 <= it2, json2, json1);
        CHECK_THROWS_BAD_COMPARISON(it1 > it2, json2, json1);
        CHECK_THROWS_BAD_COMPARISON(it1 >= it2, json1, json2);
        CHECK_NOTHROW(it1 - it2);
    }

    SECTION("Check operations that are valid for JSON objects")
    {
        fly::Json json {{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}, {"f", 6}};

        iterator it1 = json.begin();
        iterator it2 = json.begin();
        iterator it3 = json.end();

        CHECK_NOTHROW(*it1);
        CHECK_NOTHROW(it1->empty());
        CHECK_THROWS_JSON(it1[0], "JSON type invalid for offset operator: (%s)", json);
        CHECK_NOTHROW(it1 == it2);
        CHECK_NOTHROW(it1 != it2);
        CHECK_THROWS_JSON(it1 < it2, "JSON type invalid for comparison operator: (%s)", json);
        CHECK_THROWS_JSON(it1 <= it2, "JSON type invalid for comparison operator: (%s)", json);
        CHECK_THROWS_JSON(it1 > it2, "JSON type invalid for comparison operator: (%s)", json);
        CHECK_THROWS_JSON(it1 >= it2, "JSON type invalid for comparison operator: (%s)", json);
        CHECK_NOTHROW(++it1);
        CHECK_NOTHROW(it1++);
        CHECK_NOTHROW(--it3);
        CHECK_NOTHROW(it3--);
        CHECK_THROWS_JSON(it1 += 1, "JSON type invalid for iterator offset: (%s)", json);
        CHECK_THROWS_JSON(it3 -= 1, "JSON type invalid for iterator offset: (%s)", json);
        CHECK_THROWS_JSON(it1 + 1, "JSON type invalid for iterator offset: (%s)", json);
        CHECK_THROWS_JSON(1 + it1, "JSON type invalid for iterator offset: (%s)", json);
        CHECK_THROWS_JSON(it3 - 1, "JSON type invalid for iterator offset: (%s)", json);
        CHECK_THROWS_JSON(it1 - it2, "JSON type invalid for iterator difference: (%s)", json);
        CHECK_NOTHROW(it1.key());
        CHECK_NOTHROW(it1.value());
    }

    SECTION("Check operations that are valid for JSON arrays")
    {
        fly::Json json {1, 2, 3, 4, 5, 6};

        iterator it1 = json.begin();
        iterator it2 = json.begin();
        iterator it3 = json.end();

        CHECK_NOTHROW(*it1);
        CHECK_NOTHROW(it1->empty());
        CHECK_NOTHROW(it1[0]);
        CHECK_NOTHROW(it1 == it2);
        CHECK_NOTHROW(it1 != it2);
        CHECK_NOTHROW(it1 < it2);
        CHECK_NOTHROW(it1 <= it2);
        CHECK_NOTHROW(it1 > it2);
        CHECK_NOTHROW(it1 >= it2);
        CHECK_NOTHROW(++it1);
        CHECK_NOTHROW(it1++);
        CHECK_NOTHROW(--it3);
        CHECK_NOTHROW(it3--);
        CHECK_NOTHROW(it1 += 1);
        CHECK_NOTHROW(it3 -= 1);
        CHECK_NOTHROW(it1 + 1);
        CHECK_NOTHROW(1 + it1);
        CHECK_NOTHROW(it3 - 1);
        CHECK_NOTHROW(it1 - it2);
        CHECK_THROWS_JSON(it1.key(), "JSON type is not keyed: (%s)", json);
        CHECK_NOTHROW(it1.value());
    }

    SECTION("Ensure non-const iterators may be promoted to const iterators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        fly::Json::const_iterator it2 = it1;
        fly::Json::const_iterator it3;
        it3 = it1;

        CHECK(*it1 == *it2);
        CHECK(it2 == it3);
    }

    SECTION("Validate the JSON references stored by iterators")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = 0;

        iterator it;

        for (it = json.begin(); it != json.end(); ++it, ++size)
        {
            CHECK(*it == json[size]);
            CHECK(&(*it) == &json[size]);
        }

        CHECK_THROWS_NULL_WITH(*it, json);
    }

    SECTION("Validate the JSON pointers stored by iterators")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = 0;

        iterator it;

        for (it = json.begin(); it != json.end(); ++it, ++size)
        {
            iterator::pointer pt = it.operator->();

            CHECK(*pt == json[size]);
            CHECK(pt == &json[size]);
        }

        CHECK_THROWS_NULL_WITH(*it, json);
    }

    SECTION("Validate the iterator offset operator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.end();

        for (fly::Json::size_type i = 0; i < json.size(); ++i)
        {
            auto offset = static_cast<iterator::difference_type>(i);

            CHECK(it1[offset] == json[i]);
            CHECK(&it1[offset] == &json[i]);
        }

        for (fly::Json::size_type i = json.size() - 1; i < json.size(); --i)
        {
            auto offset = static_cast<iterator::difference_type>(i - json.size());

            CHECK(it2[offset] == json[i]);
            CHECK(&it2[offset] == &json[i]);
        }

        CHECK_THROWS_NULL_WITH(it1[3], json);
        CHECK_THROWS_OUT_OF_RANGE(it1[4], 4, json);
        CHECK_THROWS_NULL_WITH(it2[0], json);
    }

    SECTION("Validate the iterator equality and inequality operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.begin();

        CHECK(it1 == it1);
        CHECK(it2 == it2);
        CHECK(it1 == it2);
        CHECK(it1 + 1 == it2 + 1);
        CHECK(it1 + 2 == it2 + 2);

        CHECK(it1 != (it2 + 1));
        CHECK(it1 != (it2 + 2));
    }

    SECTION("Validate the iterator less-than and less-than-or-equal operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.begin();

        CHECK(it1 <= it2);
        CHECK(it1 <= (it2 + 1));
        CHECK(it1 < (it2 + 1));
        CHECK(it1 <= (it2 + 2));
        CHECK(it1 < (it2 + 2));
    }

    SECTION("Validate the iterator greater-than and greater-than-or-equal operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.begin();

        CHECK(it1 >= it2);
        CHECK((it1 + 1) >= it2);
        CHECK((it1 + 1) > it2);
        CHECK((it1 + 2) >= it2);
        CHECK((it1 + 2) > it2);
    }

    SECTION("Validate the iterator pre- and post-increment operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = it1;
        CHECK(++it1 == it1);
        CHECK(it1 == it2 + 1);

        it2 = it1;
        CHECK(it1++ == it2);
        CHECK(it1 == it2 + 1);

        it1 = json.end();
        CHECK_THROWS_OUT_OF_RANGE(++it1, 1, json);
        CHECK_THROWS_OUT_OF_RANGE(it1++, 1, json);
    }

    SECTION("Validate the iterator pre- and post-decrement operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.end();
        iterator it2 = it1;
        CHECK(--it1 == it1);
        CHECK(it1 == it2 - 1);

        it2 = it1;
        CHECK(it1-- == it2);
        CHECK(it1 == it2 - 1);

        it1 = json.begin();
        CHECK_THROWS_OUT_OF_RANGE(--it1, -1, json);
        CHECK_THROWS_OUT_OF_RANGE(it1--, -1, json);
    }

    SECTION("Validate the iterator addition operator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = it1;
        iterator it3 = it1;
        ++it2;
        ++it3;
        ++it3;

        CHECK((it1 += 1) == it2);
        CHECK(it1 == it2);

        it1 = json.begin();
        CHECK((it1 += 2) == it3);
        CHECK(it1 == it3);

        it1 = json.begin();
        CHECK((it1 + 1) == it2);
        CHECK(it1 < it2);

        CHECK((it1 + 2) == it3);
        CHECK(it1 < it3);

        CHECK((1 + it1) == it2);
        CHECK(it1 < it2);

        CHECK((2 + it1) == it3);
        CHECK(it1 < it3);

        CHECK_THROWS_OUT_OF_RANGE(json.begin() + 4, 4, json);
        CHECK_THROWS_OUT_OF_RANGE(json.end() + 1, 1, json);

        CHECK_THROWS_OUT_OF_RANGE(json.cbegin() + 4, 4, json);
        CHECK_THROWS_OUT_OF_RANGE(json.cend() + 1, 1, json);

        CHECK_THROWS_OUT_OF_RANGE(4 + json.begin(), 4, json);
        CHECK_THROWS_OUT_OF_RANGE(1 + json.end(), 1, json);

        CHECK_THROWS_OUT_OF_RANGE(4 + json.cbegin(), 4, json);
        CHECK_THROWS_OUT_OF_RANGE(1 + json.cend(), 1, json);
    }

    SECTION("Validate the iterator subtraction operator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.end();
        iterator it2 = it1;
        iterator it3 = it1;
        --it2;
        --it3;
        --it3;

        CHECK((it1 -= 1) == it2);
        CHECK(it1 == it2);

        it1 = json.end();
        CHECK((it1 -= 2) == it3);
        CHECK(it1 == it3);

        it1 = json.end();
        CHECK((it1 - 1) == it2);
        CHECK(it1 > it2);

        CHECK((it1 - 2) == it3);
        CHECK(it1 > it3);

        CHECK_THROWS_OUT_OF_RANGE(json.begin() - 1, -1, json);
        CHECK_THROWS_OUT_OF_RANGE(json.end() - 4, -4, json);

        CHECK_THROWS_OUT_OF_RANGE(json.cbegin() - 1, -1, json);
        CHECK_THROWS_OUT_OF_RANGE(json.cend() - 4, -4, json);
    }

    SECTION("Validate the iterator difference operator")
    {
        fly::Json json1 {1, 2, 3};
        fly::Json json2 {4, 5, 6};

        CHECK((json1.end() - json1.begin()) == 3);
        CHECK((json1.begin() - json1.end()) == -3);

        CHECK(((json1.begin() + 1) - json1.begin()) == 1);
        CHECK((json1.begin() - (json1.begin() + 1)) == -1);

        CHECK(((json1.begin() + 2) - json1.begin()) == 2);
        CHECK((json1.begin() - (json1.begin() + 2)) == -2);

        CHECK((json2.begin() - json1.begin()) != 0);
        CHECK((json1.begin() - json2.begin()) != 0);
    }

    SECTION("Validate the JSON key accessor")
    {
        fly::Json json {{"a", 1}, {"b", 2}};

        iterator it2 = json.begin();
        iterator it1 = it2++;

        CHECK(it1.key() == "a");
        CHECK(it2.key() == "b");

        CHECK_THROWS_NULL_WITH(json.end().key(), json);
        CHECK_THROWS_NULL_WITH(json.cend().key(), json);
    }

    SECTION("Validate the JSON value accessor")
    {
        fly::Json json1 {{"a", 1}, {"b", 2}};
        fly::Json json2 {4, 5, 6};

        iterator it2 = json1.begin();
        iterator it1 = it2++;

        iterator it3 = json2.begin();
        iterator it4 = it3 + 1;
        iterator it5 = it4 + 1;

        CHECK(it1.value() == 1);
        CHECK(it2.value() == 2);

        CHECK(it3.value() == 4);
        CHECK(it4.value() == 5);
        CHECK(it5.value() == 6);

        CHECK_THROWS_NULL_WITH(json1.end().value(), json1);
        CHECK_THROWS_NULL_WITH(json2.end().value(), json2);

        CHECK_THROWS_NULL_WITH(json1.cend().value(), json1);
        CHECK_THROWS_NULL_WITH(json2.cend().value(), json2);
    }
}
