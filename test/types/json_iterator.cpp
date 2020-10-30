#include "fly/types/json/detail/json_iterator.hpp"

#include "fly/types/json/json.hpp"
#include "test/types/json_macros.hpp"

#include <catch2/catch.hpp>

CATCH_TEST_CASE("JsonIterator", "[json]")
{
    using iterator = fly::Json::iterator;

    CATCH_SECTION("Check JSON types that are allowed to provide iterators")
    {
        fly::Json null = nullptr;
        CATCH_CHECK_THROWS_ITERATOR(
            iterator(&null, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            null);

        fly::Json string = "abc";
        CATCH_CHECK_THROWS_ITERATOR(
            iterator(&string, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            string);

        fly::Json object = {{"a", 1}, {"b", 2}};
        CATCH_CHECK_NOTHROW(iterator(&object, iterator::Position::Begin));

        fly::Json array = {'7', 8};
        CATCH_CHECK_NOTHROW(iterator(&array, iterator::Position::Begin));

        fly::Json boolean = true;
        CATCH_CHECK_THROWS_ITERATOR(
            iterator(&boolean, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            boolean);

        fly::Json sign = 1;
        CATCH_CHECK_THROWS_ITERATOR(
            iterator(&sign, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            sign);

        fly::Json unsign = static_cast<unsigned int>(1);
        CATCH_CHECK_THROWS_ITERATOR(
            iterator(&unsign, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            unsign);

        fly::Json floatt = 1.0f;
        CATCH_CHECK_THROWS_ITERATOR(
            iterator(&floatt, iterator::Position::Begin),
            "JSON type invalid for iteration: (%s)",
            floatt);
    }

    CATCH_SECTION("Must not be able to perform operations on null iterators")
    {
        iterator it1;
        iterator it2;

        CATCH_CHECK_THROWS_NULL(*it1);
        CATCH_CHECK_THROWS_NULL(it1->empty());
        CATCH_CHECK_THROWS_NULL(it1[0]);
        CATCH_CHECK_THROWS_NULL(++it1);
        CATCH_CHECK_THROWS_NULL(it1++);
        CATCH_CHECK_THROWS_NULL(--it1);
        CATCH_CHECK_THROWS_NULL(it1--);
        CATCH_CHECK_THROWS_NULL(it1 += 1);
        CATCH_CHECK_THROWS_NULL(it1 -= 1);
        CATCH_CHECK_THROWS_NULL(it1 + 1);
        CATCH_CHECK_THROWS_NULL(1 + it1);
        CATCH_CHECK_THROWS_NULL(it1 - 1);
        CATCH_CHECK_THROWS_NULL(it1.key());
        CATCH_CHECK_THROWS_NULL(it1.value());

        CATCH_CHECK_NOTHROW(iterator(it1));
        CATCH_CHECK_NOTHROW(it2 = it1);
    }

    CATCH_SECTION("Must not be able to perform operations on manually constructed null iterators")
    {
        iterator it1(nullptr, iterator::Position::Begin);
        iterator it2(nullptr, iterator::Position::Begin);

        CATCH_CHECK_THROWS_NULL(*it1);
        CATCH_CHECK_THROWS_NULL(it1->empty());
        CATCH_CHECK_THROWS_NULL(it1[0]);
        CATCH_CHECK_THROWS_NULL(++it1);
        CATCH_CHECK_THROWS_NULL(it1++);
        CATCH_CHECK_THROWS_NULL(--it1);
        CATCH_CHECK_THROWS_NULL(it1--);
        CATCH_CHECK_THROWS_NULL(it1 += 1);
        CATCH_CHECK_THROWS_NULL(it1 -= 1);
        CATCH_CHECK_THROWS_NULL(it1 + 1);
        CATCH_CHECK_THROWS_NULL(1 + it1);
        CATCH_CHECK_THROWS_NULL(it1 - 1);
        CATCH_CHECK_THROWS_NULL(it1.key());
        CATCH_CHECK_THROWS_NULL(it1.value());

        CATCH_CHECK_NOTHROW(iterator(it1));
        CATCH_CHECK_NOTHROW(it2 = it1);
    }

    CATCH_SECTION("Must not be able to compare null iterators")
    {
        iterator it1;
        iterator it2;

        CATCH_CHECK_THROWS_NULL(it1 == it2);
        CATCH_CHECK_THROWS_NULL(it1 != it2);
        CATCH_CHECK_THROWS_NULL(it1 < it2);
        CATCH_CHECK_THROWS_NULL(it1 <= it2);
        CATCH_CHECK_THROWS_NULL(it1 > it2);
        CATCH_CHECK_THROWS_NULL(it1 >= it2);
        CATCH_CHECK_THROWS_NULL(it1 - it2);
    }

    CATCH_SECTION("Must not be able to compare a null iterator to a non-null iterator")
    {
        fly::Json json {1, 2, 3};

        iterator it1;
        iterator it2 = json.begin();

        CATCH_CHECK_THROWS_NULL(it1 == it2);
        CATCH_CHECK_THROWS_NULL(it1 != it2);
        CATCH_CHECK_THROWS_NULL(it1 < it2);
        CATCH_CHECK_THROWS_NULL(it1 <= it2);
        CATCH_CHECK_THROWS_NULL(it1 > it2);
        CATCH_CHECK_THROWS_NULL(it1 >= it2);
        CATCH_CHECK_THROWS_NULL(it1 - it2);
    }

    CATCH_SECTION("Must not be able to compare a non-null iterator to a null iterator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2;

        CATCH_CHECK_THROWS_NULL(it1 == it2);
        CATCH_CHECK_THROWS_NULL(it1 != it2);
        CATCH_CHECK_THROWS_NULL(it1 < it2);
        CATCH_CHECK_THROWS_NULL(it1 <= it2);
        CATCH_CHECK_THROWS_NULL(it1 > it2);
        CATCH_CHECK_THROWS_NULL(it1 >= it2);
        CATCH_CHECK_THROWS_NULL(it1 - it2);
    }

    CATCH_SECTION("Must not be able to compare iterators to different JSON values")
    {
        fly::Json json1 {1, 2, 3};
        fly::Json json2 {4, 5, 6};

        iterator it1 = json1.begin();
        iterator it2 = json2.begin();

        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 == it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 != it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 < it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 <= it2, json2, json1);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 > it2, json2, json1);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 >= it2, json1, json2);
        CATCH_CHECK_NOTHROW(it1 - it2);
    }

    CATCH_SECTION("Check operations that are valid for JSON objects")
    {
        fly::Json json {{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}, {"f", 6}};

        iterator it1 = json.begin();
        iterator it2 = json.begin();
        iterator it3 = json.end();

        CATCH_CHECK_NOTHROW(*it1);
        CATCH_CHECK_NOTHROW(it1->empty());
        CATCH_CHECK_THROWS_ITERATOR(it1[0], "JSON type invalid for offset operator: (%s)", json);
        CATCH_CHECK_NOTHROW(it1 == it2);
        CATCH_CHECK_NOTHROW(it1 != it2);
        CATCH_CHECK_THROWS_ITERATOR(
            it1 < it2,
            "JSON type invalid for comparison operator: (%s)",
            json);
        CATCH_CHECK_THROWS_ITERATOR(
            it1 <= it2,
            "JSON type invalid for comparison operator: (%s)",
            json);
        CATCH_CHECK_THROWS_ITERATOR(
            it1 > it2,
            "JSON type invalid for comparison operator: (%s)",
            json);
        CATCH_CHECK_THROWS_ITERATOR(
            it1 >= it2,
            "JSON type invalid for comparison operator: (%s)",
            json);
        CATCH_CHECK_NOTHROW(++it1);
        CATCH_CHECK_NOTHROW(it1++);
        CATCH_CHECK_NOTHROW(--it3);
        CATCH_CHECK_NOTHROW(it3--);
        CATCH_CHECK_THROWS_ITERATOR(it1 += 1, "JSON type invalid for iterator offset: (%s)", json);
        CATCH_CHECK_THROWS_ITERATOR(it3 -= 1, "JSON type invalid for iterator offset: (%s)", json);
        CATCH_CHECK_THROWS_ITERATOR(it1 + 1, "JSON type invalid for iterator offset: (%s)", json);
        CATCH_CHECK_THROWS_ITERATOR(1 + it1, "JSON type invalid for iterator offset: (%s)", json);
        CATCH_CHECK_THROWS_ITERATOR(it3 - 1, "JSON type invalid for iterator offset: (%s)", json);
        CATCH_CHECK_THROWS_ITERATOR(
            it1 - it2,
            "JSON type invalid for iterator difference: (%s)",
            json);
        CATCH_CHECK_NOTHROW(it1.key());
        CATCH_CHECK_NOTHROW(it1.value());
    }

    CATCH_SECTION("Check operations that are valid for JSON arrays")
    {
        fly::Json json {1, 2, 3, 4, 5, 6};

        iterator it1 = json.begin();
        iterator it2 = json.begin();
        iterator it3 = json.end();

        CATCH_CHECK_NOTHROW(*it1);
        CATCH_CHECK_NOTHROW(it1->empty());
        CATCH_CHECK_NOTHROW(it1[0]);
        CATCH_CHECK_NOTHROW(it1 == it2);
        CATCH_CHECK_NOTHROW(it1 != it2);
        CATCH_CHECK_NOTHROW(it1 < it2);
        CATCH_CHECK_NOTHROW(it1 <= it2);
        CATCH_CHECK_NOTHROW(it1 > it2);
        CATCH_CHECK_NOTHROW(it1 >= it2);
        CATCH_CHECK_NOTHROW(++it1);
        CATCH_CHECK_NOTHROW(it1++);
        CATCH_CHECK_NOTHROW(--it3);
        CATCH_CHECK_NOTHROW(it3--);
        CATCH_CHECK_NOTHROW(it1 += 1);
        CATCH_CHECK_NOTHROW(it3 -= 1);
        CATCH_CHECK_NOTHROW(it1 + 1);
        CATCH_CHECK_NOTHROW(1 + it1);
        CATCH_CHECK_NOTHROW(it3 - 1);
        CATCH_CHECK_NOTHROW(it1 - it2);
        CATCH_CHECK_THROWS_ITERATOR(it1.key(), "JSON type is not keyed: (%s)", json);
        CATCH_CHECK_NOTHROW(it1.value());
    }

    CATCH_SECTION("Ensure non-const iterators may be promoted to const iterators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        fly::Json::const_iterator it2 = it1;
        fly::Json::const_iterator it3;
        it3 = it1;

        CATCH_CHECK(*it1 == *it2);
        CATCH_CHECK(it2 == it3);
    }

    CATCH_SECTION("Validate the JSON references stored by iterators")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = 0;

        iterator it;

        for (it = json.begin(); it != json.end(); ++it, ++size)
        {
            CATCH_CHECK(*it == json[size]);
            CATCH_CHECK(&(*it) == &json[size]);
        }

        CATCH_CHECK_THROWS_NULL_WITH(*it, json);
    }

    CATCH_SECTION("Validate the JSON pointers stored by iterators")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = 0;

        iterator it;

        for (it = json.begin(); it != json.end(); ++it, ++size)
        {
            iterator::pointer pt = it.operator->();

            CATCH_CHECK(*pt == json[size]);
            CATCH_CHECK(pt == &json[size]);
        }

        CATCH_CHECK_THROWS_NULL_WITH(*it, json);
    }

    CATCH_SECTION("Validate the iterator offset operator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.end();

        for (fly::Json::size_type i = 0; i < json.size(); ++i)
        {
            auto offset = static_cast<iterator::difference_type>(i);

            CATCH_CHECK(it1[offset] == json[i]);
            CATCH_CHECK(&it1[offset] == &json[i]);
        }

        for (fly::Json::size_type i = json.size() - 1; i < json.size(); --i)
        {
            auto offset = static_cast<iterator::difference_type>(i - json.size());

            CATCH_CHECK(it2[offset] == json[i]);
            CATCH_CHECK(&it2[offset] == &json[i]);
        }

        CATCH_CHECK_THROWS_NULL_WITH(it1[3], json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(it1[4], 4, json);
        CATCH_CHECK_THROWS_NULL_WITH(it2[0], json);
    }

    CATCH_SECTION("Validate the iterator equality and inequality operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.begin();

        CATCH_CHECK(it1 == it1);
        CATCH_CHECK(it2 == it2);
        CATCH_CHECK(it1 == it2);
        CATCH_CHECK(it1 + 1 == it2 + 1);
        CATCH_CHECK(it1 + 2 == it2 + 2);

        CATCH_CHECK(it1 != (it2 + 1));
        CATCH_CHECK(it1 != (it2 + 2));
    }

    CATCH_SECTION("Validate the iterator less-than and less-than-or-equal operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.begin();

        CATCH_CHECK(it1 <= it2);
        CATCH_CHECK(it1 <= (it2 + 1));
        CATCH_CHECK(it1 < (it2 + 1));
        CATCH_CHECK(it1 <= (it2 + 2));
        CATCH_CHECK(it1 < (it2 + 2));
    }

    CATCH_SECTION("Validate the iterator greater-than and greater-than-or-equal operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = json.begin();

        CATCH_CHECK(it1 >= it2);
        CATCH_CHECK((it1 + 1) >= it2);
        CATCH_CHECK((it1 + 1) > it2);
        CATCH_CHECK((it1 + 2) >= it2);
        CATCH_CHECK((it1 + 2) > it2);
    }

    CATCH_SECTION("Validate the iterator pre- and post-increment operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = it1;
        CATCH_CHECK(++it1 == it1);
        CATCH_CHECK(it1 == it2 + 1);

        it2 = it1;
        CATCH_CHECK(it1++ == it2);
        CATCH_CHECK(it1 == it2 + 1);

        it1 = json.end();
        CATCH_CHECK_THROWS_OUT_OF_RANGE(++it1, 1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(it1++, 1, json);
    }

    CATCH_SECTION("Validate the iterator pre- and post-decrement operators")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.end();
        iterator it2 = it1;
        CATCH_CHECK(--it1 == it1);
        CATCH_CHECK(it1 == it2 - 1);

        it2 = it1;
        CATCH_CHECK(it1-- == it2);
        CATCH_CHECK(it1 == it2 - 1);

        it1 = json.begin();
        CATCH_CHECK_THROWS_OUT_OF_RANGE(--it1, -1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(it1--, -1, json);
    }

    CATCH_SECTION("Validate the iterator addition operator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.begin();
        iterator it2 = it1;
        iterator it3 = it1;
        ++it2;
        ++it3;
        ++it3;

        CATCH_CHECK((it1 += 1) == it2);
        CATCH_CHECK(it1 == it2);

        it1 = json.begin();
        CATCH_CHECK((it1 += 2) == it3);
        CATCH_CHECK(it1 == it3);

        it1 = json.begin();
        CATCH_CHECK((it1 + 1) == it2);
        CATCH_CHECK(it1 < it2);

        CATCH_CHECK((it1 + 2) == it3);
        CATCH_CHECK(it1 < it3);

        CATCH_CHECK((1 + it1) == it2);
        CATCH_CHECK(it1 < it2);

        CATCH_CHECK((2 + it1) == it3);
        CATCH_CHECK(it1 < it3);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.begin() + 4, 4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.end() + 1, 1, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.cbegin() + 4, 4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.cend() + 1, 1, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(4 + json.begin(), 4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(1 + json.end(), 1, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(4 + json.cbegin(), 4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(1 + json.cend(), 1, json);
    }

    CATCH_SECTION("Validate the iterator subtraction operator")
    {
        fly::Json json {1, 2, 3};

        iterator it1 = json.end();
        iterator it2 = it1;
        iterator it3 = it1;
        --it2;
        --it3;
        --it3;

        CATCH_CHECK((it1 -= 1) == it2);
        CATCH_CHECK(it1 == it2);

        it1 = json.end();
        CATCH_CHECK((it1 -= 2) == it3);
        CATCH_CHECK(it1 == it3);

        it1 = json.end();
        CATCH_CHECK((it1 - 1) == it2);
        CATCH_CHECK(it1 > it2);

        CATCH_CHECK((it1 - 2) == it3);
        CATCH_CHECK(it1 > it3);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.begin() - 1, -1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.end() - 4, -4, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.cbegin() - 1, -1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.cend() - 4, -4, json);
    }

    CATCH_SECTION("Validate the iterator difference operator")
    {
        fly::Json json1 {1, 2, 3};
        fly::Json json2 {4, 5, 6};

        CATCH_CHECK((json1.end() - json1.begin()) == 3);
        CATCH_CHECK((json1.begin() - json1.end()) == -3);

        CATCH_CHECK(((json1.begin() + 1) - json1.begin()) == 1);
        CATCH_CHECK((json1.begin() - (json1.begin() + 1)) == -1);

        CATCH_CHECK(((json1.begin() + 2) - json1.begin()) == 2);
        CATCH_CHECK((json1.begin() - (json1.begin() + 2)) == -2);

        CATCH_CHECK((json2.begin() - json1.begin()) != 0);
        CATCH_CHECK((json1.begin() - json2.begin()) != 0);
    }

    CATCH_SECTION("Validate the JSON key accessor")
    {
        fly::Json json {{"a", 1}, {"b", 2}};

        iterator it2 = json.begin();
        iterator it1 = it2++;

        CATCH_CHECK(it1.key() == FLY_JSON_STR("a"));
        CATCH_CHECK(it2.key() == FLY_JSON_STR("b"));

        CATCH_CHECK_THROWS_NULL_WITH(json.end().key(), json);
        CATCH_CHECK_THROWS_NULL_WITH(json.cend().key(), json);
    }

    CATCH_SECTION("Validate the JSON value accessor")
    {
        fly::Json json1 {{"a", 1}, {"b", 2}};
        fly::Json json2 {4, 5, 6};

        iterator it2 = json1.begin();
        iterator it1 = it2++;

        iterator it3 = json2.begin();
        iterator it4 = it3 + 1;
        iterator it5 = it4 + 1;

        CATCH_CHECK(it1.value() == 1);
        CATCH_CHECK(it2.value() == 2);

        CATCH_CHECK(it3.value() == 4);
        CATCH_CHECK(it4.value() == 5);
        CATCH_CHECK(it5.value() == 6);

        CATCH_CHECK_THROWS_NULL_WITH(json1.end().value(), json1);
        CATCH_CHECK_THROWS_NULL_WITH(json2.end().value(), json2);

        CATCH_CHECK_THROWS_NULL_WITH(json1.cend().value(), json1);
        CATCH_CHECK_THROWS_NULL_WITH(json2.cend().value(), json2);
    }
}
