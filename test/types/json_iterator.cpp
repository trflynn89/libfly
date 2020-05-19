#include "fly/types/json/detail/json_iterator.hpp"

#include "fly/fly.hpp"
#include "fly/types/json/json.hpp"
#include "fly/types/json/json_exception.hpp"

#include <gtest/gtest.h>

//==============================================================================
TEST(JsonIteratorTest, IteratorTypes)
{
    fly::Json null1 = nullptr;
    const fly::Json null2 = nullptr;
    EXPECT_THROW(FLY_UNUSED(null1.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null2.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null1.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null2.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null1.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null2.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null1.cend()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(null2.cend()), fly::JsonException);

    fly::Json string1 = "abc";
    const fly::Json string2 = "abc";
    EXPECT_THROW(FLY_UNUSED(string1.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string2.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string1.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string2.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string1.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string2.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string1.cend()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(string2.cend()), fly::JsonException);

    fly::Json object1 = {{"a", 1}, {"b", 2}};
    const fly::Json object2 = {{"a", 1}, {"b", 2}};
    EXPECT_NO_THROW(FLY_UNUSED(object1.begin()));
    EXPECT_NO_THROW(FLY_UNUSED(object2.begin()));
    EXPECT_NO_THROW(FLY_UNUSED(object1.cbegin()));
    EXPECT_NO_THROW(FLY_UNUSED(object2.cbegin()));
    EXPECT_NO_THROW(FLY_UNUSED(object1.end()));
    EXPECT_NO_THROW(FLY_UNUSED(object2.end()));
    EXPECT_NO_THROW(FLY_UNUSED(object1.cend()));
    EXPECT_NO_THROW(FLY_UNUSED(object2.cend()));

    fly::Json array1 = {'7', 8};
    const fly::Json array2 = {'7', 8};
    EXPECT_NO_THROW(FLY_UNUSED(array1.begin()));
    EXPECT_NO_THROW(FLY_UNUSED(array2.begin()));
    EXPECT_NO_THROW(FLY_UNUSED(array1.cbegin()));
    EXPECT_NO_THROW(FLY_UNUSED(array2.cbegin()));
    EXPECT_NO_THROW(FLY_UNUSED(array1.end()));
    EXPECT_NO_THROW(FLY_UNUSED(array2.end()));
    EXPECT_NO_THROW(FLY_UNUSED(array1.cend()));
    EXPECT_NO_THROW(FLY_UNUSED(array2.cend()));

    fly::Json boolean1 = true;
    const fly::Json boolean2 = true;
    EXPECT_THROW(FLY_UNUSED(boolean1.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean2.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean1.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean2.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean1.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean2.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean1.cend()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(boolean2.cend()), fly::JsonException);

    fly::Json signed1 = 1;
    const fly::Json signed2 = 1;
    EXPECT_THROW(FLY_UNUSED(signed1.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed2.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed1.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed2.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed1.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed2.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed1.cend()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(signed2.cend()), fly::JsonException);

    fly::Json unsigned1 = static_cast<unsigned int>(1);
    const fly::Json unsigned2 = static_cast<unsigned int>(1);
    EXPECT_THROW(FLY_UNUSED(unsigned1.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned2.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned1.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned2.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned1.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned2.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned1.cend()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(unsigned2.cend()), fly::JsonException);

    fly::Json float1 = 1.0f;
    const fly::Json float2 = 1.0f;
    EXPECT_THROW(FLY_UNUSED(float1.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float2.begin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float1.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float2.cbegin()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float1.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float2.end()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float1.cend()), fly::JsonException);
    EXPECT_THROW(FLY_UNUSED(float2.cend()), fly::JsonException);
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
