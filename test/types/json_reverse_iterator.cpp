#include "fly/types/json/detail/json_reverse_iterator.hpp"

#include "test/types/json_helpers.hpp"

#include "fly/types/json/json.hpp"

#include "catch2/catch.hpp"

CATCH_TEST_CASE("JsonReverseIterator", "[json]")
{
    using iterator = fly::Json::iterator;
    using reverse_iterator = fly::Json::reverse_iterator;

    CATCH_SECTION("Must not be able to perform operations on null iterators")
    {
        reverse_iterator it1;
        reverse_iterator it2;

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

        CATCH_CHECK_NOTHROW(reverse_iterator(it1));
        CATCH_CHECK_NOTHROW(it2 = it1);
    }

    CATCH_SECTION("Must not be able to perform operations on manually constructed null iterators")
    {
        iterator null(nullptr, iterator::Position::Begin);
        reverse_iterator it1(null);
        reverse_iterator it2(null);

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

        CATCH_CHECK_NOTHROW(reverse_iterator(it1));
        CATCH_CHECK_NOTHROW(it2 = it1);
    }

    CATCH_SECTION("Must not be able to compare null iterators")
    {
        reverse_iterator it1;
        reverse_iterator it2;

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

        reverse_iterator it1;
        reverse_iterator it2(json.begin());

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

        reverse_iterator it1(json.begin());
        reverse_iterator it2;

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

        reverse_iterator it1(json1.begin());
        reverse_iterator it2(json2.begin());

        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 == it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 != it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 < it2, json2, json1);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 <= it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 > it2, json1, json2);
        CATCH_CHECK_THROWS_BAD_COMPARISON(it1 >= it2, json2, json1);
        CATCH_CHECK_NOTHROW(it1 - it2);
    }

    CATCH_SECTION("Check operations that are valid for JSON objects")
    {
        fly::Json json {{"a", 1}, {"b", 2}, {"c", 3}, {"d", 4}, {"e", 5}, {"f", 6}};

        reverse_iterator it1(json.end());
        reverse_iterator it2(json.end());
        reverse_iterator it3(json.begin());

        CATCH_CHECK_NOTHROW(*it1);
        CATCH_CHECK_NOTHROW(it1->empty());
        CATCH_CHECK_THROWS_ITERATOR(it1[0], "JSON type invalid for iterator offset: (%s)", json);
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

        reverse_iterator it1(json.end());
        reverse_iterator it2(json.end());
        reverse_iterator it3(json.begin());

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

    CATCH_SECTION("Validate the JSON references stored by iterators")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = json.size();

        reverse_iterator it;

        for (it = json.rbegin(); it != json.rend(); ++it, --size)
        {
            CATCH_CHECK(*it == json[size - 1]);
            CATCH_CHECK(&(*it) == &json[size - 1]);
        }

        CATCH_CHECK_THROWS_NULL_WITH(*it, json);
    }

    CATCH_SECTION("Validate the JSON pointers stored by iterators")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = json.size();

        reverse_iterator it;

        for (it = json.rbegin(); it != json.rend(); ++it, --size)
        {
            fly::Json::pointer pt = it.operator->();

            CATCH_CHECK(*pt == json[size - 1]);
            CATCH_CHECK(pt == &json[size - 1]);
        }

        CATCH_CHECK_THROWS_NULL_WITH(it.operator->(), json);
    }

    CATCH_SECTION("Validate the iterator offset operator")
    {
        fly::Json json {1, 2, 3};
        fly::Json::size_type size = json.size();

        reverse_iterator it1 = json.rbegin();
        reverse_iterator it2 = json.rend();

        for (fly::Json::size_type i = 0; i < size; ++i)
        {
            auto offset = static_cast<reverse_iterator::difference_type>(i);

            CATCH_CHECK(it1[offset] == json[size - i - 1]);
            CATCH_CHECK(&it1[offset] == &json[size - i - 1]);
        }

        for (fly::Json::size_type i = json.size() - 1; i < json.size(); --i)
        {
            auto offset = static_cast<reverse_iterator::difference_type>(i - json.size());

            CATCH_CHECK(it2[offset] == json[size - i - 1]);
            CATCH_CHECK(&it2[offset] == &json[size - i - 1]);
        }

        CATCH_CHECK_THROWS_NULL_WITH(it1[3], json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(it1[4], -4, json);
        CATCH_CHECK_THROWS_NULL_WITH(it2[0], json);
    }

    CATCH_SECTION("Validate the iterator equality and inequality operators")
    {
        fly::Json json {1, 2, 3};

        reverse_iterator it1 = json.rbegin();
        reverse_iterator it2 = json.rbegin();

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

        reverse_iterator it1 = json.rbegin();
        reverse_iterator it2 = json.rbegin();

        CATCH_CHECK(it1 <= it2);
        CATCH_CHECK(it1 <= (it2 + 1));
        CATCH_CHECK(it1 < (it2 + 1));
        CATCH_CHECK(it1 <= (it2 + 2));
        CATCH_CHECK(it1 < (it2 + 2));
    }

    CATCH_SECTION("Validate the iterator greater-than and greater-than-or-equal operators")
    {
        fly::Json json {1, 2, 3};

        reverse_iterator it1 = json.rbegin();
        reverse_iterator it2 = json.rbegin();

        CATCH_CHECK(it1 >= it2);
        CATCH_CHECK((it1 + 1) >= it2);
        CATCH_CHECK((it1 + 1) > it2);
        CATCH_CHECK((it1 + 2) >= it2);
        CATCH_CHECK((it1 + 2) > it2);
    }

    CATCH_SECTION("Validate the iterator pre- and post-increment operators")
    {
        fly::Json json {1, 2, 3};

        reverse_iterator it1 = json.rbegin();
        reverse_iterator it2 = it1;
        CATCH_CHECK(++it1 == it1);
        CATCH_CHECK(it1 == it2 + 1);

        it2 = it1;
        CATCH_CHECK(it1++ == it2);
        CATCH_CHECK(it1 == it2 + 1);

        it1 = json.rend();
        CATCH_CHECK_THROWS_OUT_OF_RANGE(++it1, -1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(it1++, -1, json);
    }

    CATCH_SECTION("Validate the iterator pre- and post-decrement operators")
    {
        fly::Json json {1, 2, 3};

        reverse_iterator it1 = json.rend();
        reverse_iterator it2 = it1;
        CATCH_CHECK(--it1 == it1);
        CATCH_CHECK(it1 == it2 - 1);

        it2 = it1;
        CATCH_CHECK(it1-- == it2);
        CATCH_CHECK(it1 == it2 - 1);

        it1 = json.rbegin();
        CATCH_CHECK_THROWS_OUT_OF_RANGE(--it1, 1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(it1--, 1, json);
    }

    CATCH_SECTION("Validate the iterator addition operator")
    {
        fly::Json json {1, 2, 3};

        reverse_iterator it1 = json.rbegin();
        reverse_iterator it2 = it1;
        reverse_iterator it3 = it1;
        ++it2;
        ++it3;
        ++it3;

        CATCH_CHECK((it1 += 1) == it2);
        CATCH_CHECK(it1 == it2);

        it1 = json.rbegin();
        CATCH_CHECK((it1 += 2) == it3);
        CATCH_CHECK(it1 == it3);

        it1 = json.rbegin();
        CATCH_CHECK((it1 + 1) == it2);
        CATCH_CHECK(it1 < it2);

        CATCH_CHECK((it1 + 2) == it3);
        CATCH_CHECK(it1 < it3);

        CATCH_CHECK((1 + it1) == it2);
        CATCH_CHECK(it1 < it2);

        CATCH_CHECK((2 + it1) == it3);
        CATCH_CHECK(it1 < it3);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.rbegin() + 4, -4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.rend() + 1, -1, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.crbegin() + 4, -4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.crend() + 1, -1, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(4 + json.rbegin(), -4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(1 + json.rend(), -1, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(4 + json.crbegin(), -4, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(1 + json.crend(), -1, json);
    }

    CATCH_SECTION("Validate the iterator subtraction operator")
    {
        fly::Json json {1, 2, 3};

        reverse_iterator it1 = json.rend();
        reverse_iterator it2 = it1;
        reverse_iterator it3 = it1;
        --it2;
        --it3;
        --it3;

        CATCH_CHECK((it1 -= 1) == it2);
        CATCH_CHECK(it1 == it2);

        it1 = json.rend();
        CATCH_CHECK((it1 -= 2) == it3);
        CATCH_CHECK(it1 == it3);

        it1 = json.rend();
        CATCH_CHECK((it1 - 1) == it2);
        CATCH_CHECK(it1 > it2);

        CATCH_CHECK((it1 - 2) == it3);
        CATCH_CHECK(it1 > it3);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.rbegin() - 1, 1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.rend() - 4, 4, json);

        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.crbegin() - 1, 1, json);
        CATCH_CHECK_THROWS_OUT_OF_RANGE(json.crend() - 4, 4, json);
    }

    CATCH_SECTION("Validate the iterator difference operator")
    {
        fly::Json json1 {1, 2, 3};
        fly::Json json2 {4, 5, 6};

        CATCH_CHECK((json1.rend() - json1.rbegin()) == 3);
        CATCH_CHECK((json1.rbegin() - json1.rend()) == -3);

        CATCH_CHECK(((json1.rbegin() + 1) - json1.rbegin()) == 1);
        CATCH_CHECK((json1.rbegin() - (json1.rbegin() + 1)) == -1);

        CATCH_CHECK(((json1.rbegin() + 2) - json1.rbegin()) == 2);
        CATCH_CHECK((json1.rbegin() - (json1.rbegin() + 2)) == -2);

        CATCH_CHECK((json2.rbegin() - json1.rbegin()) != 0);
        CATCH_CHECK((json1.rbegin() - json2.rbegin()) != 0);
    }

    CATCH_SECTION("Validate the JSON key accessor")
    {
        fly::Json json {{"a", 1}, {"b", 2}};

        reverse_iterator it2 = json.rbegin();
        reverse_iterator it1 = it2++;

        CATCH_CHECK(it1.key() == FLY_JSON_STR("b"));
        CATCH_CHECK(it2.key() == FLY_JSON_STR("a"));

        CATCH_CHECK_THROWS_NULL_WITH(json.rend().key(), json);
        CATCH_CHECK_THROWS_NULL_WITH(json.crend().key(), json);
    }

    CATCH_SECTION("Validate the JSON value accessor")
    {
        fly::Json json1 {{"a", 1}, {"b", 2}};
        fly::Json json2 {4, 5, 6};

        reverse_iterator it2 = json1.rbegin();
        reverse_iterator it1 = it2++;

        reverse_iterator it3 = json2.rbegin();
        reverse_iterator it4 = it3 + 1;
        reverse_iterator it5 = it4 + 1;

        CATCH_CHECK(it1.value() == 2);
        CATCH_CHECK(it2.value() == 1);

        CATCH_CHECK(it3.value() == 6);
        CATCH_CHECK(it4.value() == 5);
        CATCH_CHECK(it5.value() == 4);

        CATCH_CHECK_THROWS_NULL_WITH(json1.rend().value(), json1);
        CATCH_CHECK_THROWS_NULL_WITH(json2.rend().value(), json2);

        CATCH_CHECK_THROWS_NULL_WITH(json1.crend().value(), json1);
        CATCH_CHECK_THROWS_NULL_WITH(json2.crend().value(), json2);
    }
}
