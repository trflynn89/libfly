#include "fly/types/json/detail/json_iterator.hpp"

#include "fly/fly.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/json/json_exception.hpp"

#include <gtest/gtest.h>

//==============================================================================
TEST(JsonIteratorTest, IteratorTypes)
{
    using iterator = fly::Json::iterator;

    fly::Json null = nullptr;
    EXPECT_THROW(
        FLY_UNUSED(iterator(&null, iterator::Position::Begin)),
        fly::JsonException);

    fly::Json string = "abc";
    EXPECT_THROW(
        FLY_UNUSED(iterator(&string, iterator::Position::Begin)),
        fly::JsonException);

    fly::Json object = {{"a", 1}, {"b", 2}};
    EXPECT_NO_THROW(FLY_UNUSED(iterator(&object, iterator::Position::Begin)));

    fly::Json array = {'7', 8};
    EXPECT_NO_THROW(FLY_UNUSED(iterator(&array, iterator::Position::Begin)));

    fly::Json boolean = true;
    EXPECT_THROW(
        FLY_UNUSED(iterator(&boolean, iterator::Position::Begin)),
        fly::JsonException);

    fly::Json sign = 1;
    EXPECT_THROW(
        FLY_UNUSED(iterator(&sign, iterator::Position::Begin)),
        fly::JsonException);

    fly::Json unsign = static_cast<unsigned int>(1);
    EXPECT_THROW(
        FLY_UNUSED(iterator(&unsign, iterator::Position::Begin)),
        fly::JsonException);

    fly::Json floatt = 1.0f;
    EXPECT_THROW(
        FLY_UNUSED(iterator(&floatt, iterator::Position::Begin)),
        fly::JsonException);
}

//==============================================================================
TEST(JsonIteratorTest, NullIterator)
{
    fly::Json::iterator it1;
    fly::Json::iterator it2;

    EXPECT_THROW(FLY_UNUSED(*it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1->empty()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1[0]), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(++it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1++), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(--it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1--), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 += 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 -= 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 + 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(1 + it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1.key()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1.value()), fly::JsonException);

    EXPECT_NO_THROW((fly::Json::iterator(it1)));
    EXPECT_NO_THROW(it2 = it1);
}

//==============================================================================
TEST(JsonIteratorTest, NullIteratorFromNullJson)
{
    fly::Json::iterator it1(nullptr, fly::Json::iterator::Position::Begin);
    fly::Json::iterator it2(nullptr, fly::Json::iterator::Position::Begin);

    EXPECT_THROW(FLY_UNUSED(*it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1->empty()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1[0]), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(++it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1++), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(--it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1--), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 += 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 -= 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 + 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(1 + it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1.key()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1.value()), fly::JsonException);

    EXPECT_NO_THROW((fly::Json::iterator(it1)));
    EXPECT_NO_THROW(it2 = it1);
}

//==============================================================================
TEST(JsonIteratorTest, NullIteratorComparison)
{
    fly::Json::iterator it1;
    fly::Json::iterator it2;

    EXPECT_THROW(FLY_UNUSED(it1 == it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 != it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 < it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 <= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 > it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 >= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - it2), fly::JsonException);
}

//==============================================================================
TEST(JsonIteratorTest, NullIteratorLhsOnlyComparison)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1;
    fly::Json::iterator it2 = json.begin();

    EXPECT_THROW(FLY_UNUSED(it1 == it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 != it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 < it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 <= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 > it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 >= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - it2), fly::JsonException);
}

//==============================================================================
TEST(JsonIteratorTest, NullIteratorRhsOnlyComparison)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2;

    EXPECT_THROW(FLY_UNUSED(it1 == it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 != it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 < it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 <= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 > it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 >= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - it2), fly::JsonException);
}

//==============================================================================
TEST(JsonIteratorTest, DifferentJsonInstancesComparison)
{
    fly::Json json1 {1, 2, 3};
    fly::Json json2 {4, 5, 6};

    fly::Json::iterator it1 = json1.begin();
    fly::Json::iterator it2 = json2.begin();

    EXPECT_THROW(FLY_UNUSED(it1 == it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 != it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 < it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 <= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 > it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 >= it2), fly::JsonException);
    EXPECT_NO_THROW(FLY_UNUSED(it1 - it2));
}

//==============================================================================
TEST(JsonIteratorTest, OperationsOnObjects)
{
    fly::Json json {{"a", 1}, {"b", 2}};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = json.begin();

    EXPECT_NO_THROW(FLY_UNUSED(*it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1->empty()));
    EXPECT_THROW(FLY_UNUSED(it1[0]), fly::JsonException);
    EXPECT_NO_THROW(FLY_UNUSED(it1 == it2));
    EXPECT_NO_THROW(FLY_UNUSED(it1 != it2));
    EXPECT_THROW(FLY_UNUSED(it1 < it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 <= it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 > it2), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 >= it2), fly::JsonException);
    EXPECT_NO_THROW(FLY_UNUSED(++it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1++));
    EXPECT_NO_THROW(FLY_UNUSED(--it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1--));
    EXPECT_THROW(FLY_UNUSED(it1 += 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 -= 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 + 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(1 + it1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - 1), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(it1 - it2), fly::JsonException);
    EXPECT_NO_THROW(FLY_UNUSED(it1.key()));
    EXPECT_NO_THROW(FLY_UNUSED(it1.value()));
}

//==============================================================================
TEST(JsonIteratorTest, OperationsOnArrays)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = json.begin();

    EXPECT_NO_THROW(FLY_UNUSED(*it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1->empty()));
    EXPECT_NO_THROW(FLY_UNUSED(it1[0]));
    EXPECT_NO_THROW(FLY_UNUSED(it1 == it2));
    EXPECT_NO_THROW(FLY_UNUSED(it1 != it2));
    EXPECT_NO_THROW(FLY_UNUSED(it1 < it2));
    EXPECT_NO_THROW(FLY_UNUSED(it1 <= it2));
    EXPECT_NO_THROW(FLY_UNUSED(it1 > it2));
    EXPECT_NO_THROW(FLY_UNUSED(it1 >= it2));
    EXPECT_NO_THROW(FLY_UNUSED(++it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1++));
    EXPECT_NO_THROW(FLY_UNUSED(--it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1--));
    EXPECT_NO_THROW(FLY_UNUSED(it1 += 1));
    EXPECT_NO_THROW(FLY_UNUSED(it1 -= 1));
    EXPECT_NO_THROW(FLY_UNUSED(it1 + 1));
    EXPECT_NO_THROW(FLY_UNUSED(1 + it1));
    EXPECT_NO_THROW(FLY_UNUSED(it1 - 1));
    EXPECT_NO_THROW(FLY_UNUSED(it1 - it2));
    EXPECT_THROW(FLY_UNUSED(it1.key()), fly::JsonException);
    EXPECT_NO_THROW(FLY_UNUSED(it1.value()));
}

//==============================================================================
TEST(JsonIteratorTest, ConstPromotion)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::const_iterator it2 = it1;
    fly::Json::const_iterator it3;
    it3 = it1;

    EXPECT_EQ(*it1, *it2);
    EXPECT_EQ(it2, it3);
}

//==============================================================================
TEST(JsonIteratorTest, DereferenceToReference)
{
    fly::Json json {1, 2, 3};
    fly::Json::size_type size = 0;

    for (fly::Json::iterator it = json.begin(); it != json.end(); ++it, ++size)
    {
        EXPECT_EQ(*it, json[size]);
        EXPECT_EQ(&(*it), &json[size]);
    }
}

//==============================================================================
TEST(JsonIteratorTest, DereferenceToPointer)
{
    fly::Json json {1, 2, 3};
    fly::Json::size_type size = 0;

    for (fly::Json::iterator it = json.begin(); it != json.end(); ++it, ++size)
    {
        fly::Json::iterator::pointer pt = it.operator->();

        EXPECT_EQ(*pt, json[size]);
        EXPECT_EQ(pt, &json[size]);
    }
}

//==============================================================================
TEST(JsonIteratorTest, OffsetOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = json.end();

    for (fly::Json::size_type i = 0; i < json.size(); ++i)
    {
        auto offset = static_cast<fly::Json::iterator::difference_type>(i);

        EXPECT_EQ(it1[offset], json[i]);
        EXPECT_EQ(&it1[offset], &json[i]);
    }

    for (fly::Json::size_type i = json.size() - 1; i < json.size(); --i)
    {
        auto offset =
            static_cast<fly::Json::iterator::difference_type>(i - json.size());

        EXPECT_EQ(it2[offset], json[i]);
        EXPECT_EQ(&it2[offset], &json[i]);
    }
}

//==============================================================================
TEST(JsonIteratorTest, EqualityOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = json.begin();

    EXPECT_EQ(it1, it1);
    EXPECT_EQ(it2, it2);
    EXPECT_EQ(it1, it2);
    EXPECT_EQ(it1 + 1, it2 + 1);
    EXPECT_EQ(it1 + 2, it2 + 2);

    EXPECT_NE(it1, it2 + 1);
    EXPECT_NE(it1, it2 + 2);
}

//==============================================================================
TEST(JsonIteratorTest, LessThanOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = json.begin();

    EXPECT_LE(it1, it2);
    EXPECT_LE(it1, it2 + 1);
    EXPECT_LT(it1, it2 + 1);
    EXPECT_LE(it1, it2 + 2);
    EXPECT_LT(it1, it2 + 2);
}

//==============================================================================
TEST(JsonIteratorTest, GreaterThanOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = json.begin();

    EXPECT_GE(it1, it2);
    EXPECT_GE(it1 + 1, it2);
    EXPECT_GT(it1 + 1, it2);
    EXPECT_GE(it1 + 2, it2);
    EXPECT_GT(it1 + 2, it2);
}

//==============================================================================
TEST(JsonIteratorTest, IncrementOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = it1;
    EXPECT_EQ(++it1, it1);
    EXPECT_EQ(it1, it2 + 1);

    it2 = it1;
    EXPECT_EQ(it1++, it2);
    EXPECT_EQ(it1, it2 + 1);
}

//==============================================================================
TEST(JsonIteratorTest, DecrementOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.end();
    fly::Json::iterator it2 = it1;
    EXPECT_EQ(--it1, it1);
    EXPECT_EQ(it1, it2 - 1);

    it2 = it1;
    EXPECT_EQ(it1--, it2);
    EXPECT_EQ(it1, it2 - 1);
}

//==============================================================================
TEST(JsonIteratorTest, AdditionOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.begin();
    fly::Json::iterator it2 = it1;
    fly::Json::iterator it3 = it1;
    ++it2;
    ++it3;
    ++it3;

    EXPECT_EQ(it1 += 1, it2);
    EXPECT_EQ(it1, it2);

    it1 = json.begin();
    EXPECT_EQ(it1 += 2, it3);
    EXPECT_EQ(it1, it3);

    it1 = json.begin();
    EXPECT_EQ(it1 + 1, it2);
    EXPECT_LT(it1, it2);

    EXPECT_EQ(it1 + 2, it3);
    EXPECT_LT(it1, it3);

    EXPECT_EQ(1 + it1, it2);
    EXPECT_LT(it1, it2);

    EXPECT_EQ(2 + it1, it3);
    EXPECT_LT(it1, it3);
}

//==============================================================================
TEST(JsonIteratorTest, SubtractionOperator)
{
    fly::Json json {1, 2, 3};

    fly::Json::iterator it1 = json.end();
    fly::Json::iterator it2 = it1;
    fly::Json::iterator it3 = it1;
    --it2;
    --it3;
    --it3;

    EXPECT_EQ(it1 -= 1, it2);
    EXPECT_EQ(it1, it2);

    it1 = json.end();
    EXPECT_EQ(it1 -= 2, it3);
    EXPECT_EQ(it1, it3);

    it1 = json.end();
    EXPECT_EQ(it1 - 1, it2);
    EXPECT_GT(it1, it2);

    EXPECT_EQ(it1 - 2, it3);
    EXPECT_GT(it1, it3);
}

//==============================================================================
TEST(JsonIteratorTest, DifferenceOperator)
{
    fly::Json json1 {1, 2, 3};
    fly::Json json2 {4, 5, 6};

    EXPECT_EQ(json1.end() - json1.begin(), 3);
    EXPECT_EQ(json1.begin() - json1.end(), -3);

    EXPECT_EQ((json1.begin() + 1) - json1.begin(), 1);
    EXPECT_EQ(json1.begin() - (json1.begin() + 1), -1);

    EXPECT_EQ((json1.begin() + 2) - json1.begin(), 2);
    EXPECT_EQ(json1.begin() - (json1.begin() + 2), -2);

    EXPECT_NE(json2.begin() - json1.begin(), 0);
    EXPECT_NE(json1.begin() - json2.begin(), 0);
}

//==============================================================================
TEST(JsonIteratorTest, IteratorKey)
{
    fly::Json json {{"a", 1}, {"b", 2}};

    fly::Json::iterator it2 = json.begin();
    fly::Json::iterator it1 = it2++;

    EXPECT_EQ(it1.key(), "a");
    EXPECT_EQ(it2.key(), "b");
}

//==============================================================================
TEST(JsonIteratorTest, IteratorValue)
{
    fly::Json json1 {{"a", 1}, {"b", 2}};
    fly::Json json2 {4, 5, 6};

    fly::Json::iterator it2 = json1.begin();
    fly::Json::iterator it1 = it2++;

    fly::Json::iterator it3 = json2.begin();
    fly::Json::iterator it4 = it3 + 1;
    fly::Json::iterator it5 = it4 + 1;

    EXPECT_EQ(it1.value(), 1);
    EXPECT_EQ(it2.value(), 2);

    EXPECT_EQ(it3.value(), 4);
    EXPECT_EQ(it4.value(), 5);
    EXPECT_EQ(it5.value(), 6);
}
