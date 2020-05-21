#include "fly/types/json/json_exception.hpp"

#include "fly/types/json/json.hpp"
#include "fly/types/string/string.hpp"

#include <gtest/gtest.h>

#include <sstream>
#include <string>

//==============================================================================
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
        std::string what(e.what());

        std::string expect("*some message*" + stream.str() + "*");
        EXPECT_TRUE(fly::String::wildcard_match(what, expect));

        thrown = true;
    }

    EXPECT_TRUE(thrown);
}
