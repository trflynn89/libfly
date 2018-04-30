#include <gtest/gtest.h>

#include "fly/parser/exceptions.h"
#include "fly/string/string.h"

//==============================================================================
TEST(ParserExceptionTest, ParserExceptionTest)
{
    int line = 123;
    int column = 456;
    std::string message("Bad file!");

    try
    {
        throw fly::ParserException(line, message);
    }
    catch (const fly::ParserException &ex)
    {
        std::string what(ex.what());

        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_EQ(what.find(std::to_string(column)), std::string::npos);
        EXPECT_NE(what.find(message), std::string::npos);
        EXPECT_EQ(what.find(", column"), std::string::npos);
    }

    try
    {
        throw fly::ParserException(line, column, message);
    }
    catch (const fly::ParserException &ex)
    {
        std::string what(ex.what());

        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_NE(what.find(std::to_string(column)), std::string::npos);
        EXPECT_NE(what.find(message), std::string::npos);
        EXPECT_NE(what.find(", column"), std::string::npos);
    }
}

//==============================================================================
TEST(ParserExceptionTest, UnexpectedCharacterException)
{
    int line = 123;
    int column = 456;
    int c1 = '\0';
    int c2 = 'A';

    try
    {
        throw fly::UnexpectedCharacterException(line, column, c1);
    }
    catch (const fly::UnexpectedCharacterException &ex)
    {
        std::string what(ex.what());

        std::string hex = fly::String::Format("%x", c1);

        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_NE(what.find(std::to_string(column)), std::string::npos);
        EXPECT_NE(what.find(hex), std::string::npos);
        EXPECT_EQ(what.find("("), std::string::npos);
        EXPECT_EQ(what.find(")"), std::string::npos);
    }

    try
    {
        throw fly::UnexpectedCharacterException(line, column, c2);
    }
    catch (const fly::UnexpectedCharacterException &ex)
    {
        std::string what(ex.what());

        std::string str = fly::String::Format("'%c'", char(c2));
        std::string hex = fly::String::Format("%x", c2);

        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_NE(what.find(std::to_string(column)), std::string::npos);
        EXPECT_NE(what.find(str), std::string::npos);
        EXPECT_NE(what.find(hex), std::string::npos);
        EXPECT_NE(what.find('('), std::string::npos);
        EXPECT_NE(what.find(')'), std::string::npos);
    }
}

//==============================================================================
TEST(ParserExceptionTest, BadConversionException)
{
    int line = 123;
    int column = 456;
    std::string value("789");

    try
    {
        throw fly::BadConversionException(line, column, value);
    }
    catch (const fly::BadConversionException &ex)
    {
        std::string what(ex.what());

        EXPECT_NE(what.find(std::to_string(line)), std::string::npos);
        EXPECT_NE(what.find(std::to_string(column)), std::string::npos);
        EXPECT_NE(what.find(value), std::string::npos);
    }
}
