#include "fly/types/json/json_exception.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

//==================================================================================================
TEST(JsonExceptionTest, Exception)
{
    std::stringstream stream;
    fly::Json string = "abc";
    stream << string;

    bool thrown = false;

    try
    {
        throw fly::JsonException(string, "some message");
    }
    catch (const fly::JsonException &e)
    {
        const std::string what(e.what());

        const std::string expect("*some message*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==================================================================================================
TEST(JsonExceptionTest, JsonIteratorException)
{
    std::stringstream stream;
    fly::Json string = "abc";
    stream << string;

    bool thrown = false;

    try
    {
        throw fly::JsonIteratorException(string, "some message");
    }
    catch (const fly::JsonIteratorException &e)
    {
        const std::string what(e.what());

        const std::string expect("*some message*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==================================================================================================
TEST(JsonExceptionTest, BadJsonComparisonException)
{
    std::stringstream stringstream;
    fly::Json string = "abc";
    stringstream << string;

    std::stringstream numberstream;
    fly::Json number = 12389;
    numberstream << number;

    bool thrown = false;

    try
    {
        throw fly::BadJsonComparisonException(string, number);
    }
    catch (const fly::BadJsonComparisonException &e)
    {
        const std::string what(e.what());

        const std::string expect("*" + stringstream.str() + "*" + numberstream.str() + "*");
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==================================================================================================
TEST(JsonExceptionTest, NullJsonException)
{
    std::stringstream stream;
    fly::Json string = "abc";
    stream << string;

    bool thrown = false;

    try
    {
        throw fly::NullJsonException(string);
    }
    catch (const fly::NullJsonException &e)
    {
        const std::string what(e.what());

        const std::string expect("*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}

//==================================================================================================
TEST(JsonExceptionTest, OutOfRangeJsonException)
{
    std::stringstream stream;
    fly::Json string = "abc";
    stream << string;

    bool thrown = false;

    try
    {
        throw fly::OutOfRangeJsonException(string, 12389);
    }
    catch (const fly::OutOfRangeJsonException &e)
    {
        const std::string what(e.what());

        const std::string expect("*12389*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}
