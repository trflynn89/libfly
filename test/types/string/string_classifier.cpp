#include "fly/types/string/string.hpp"

#include "catch2/catch.hpp"

#include <cctype>
#include <string>

CATCH_TEMPLATE_TEST_CASE(
    "BasicStringClassifier",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using BasicString = fly::BasicString<StringType>;
    using char_type = typename BasicString::char_type;

    CATCH_SECTION("Check if a character is an alphabetic character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::is_alpha(ch) ==
                static_cast<bool>(std::isalpha(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isalpha returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (char_type ch = 0xaa41; ch <= 0xaa5a; ++ch)
            {
                CATCH_CHECK(std::isalpha(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_alpha(ch));
            }
            for (char_type ch = 0xaa61; ch <= 0xaa7a; ++ch)
            {
                CATCH_CHECK(std::isalpha(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_alpha(ch));
            }
        }
    }

    CATCH_SECTION("Check if a character is an upper-case alphabetic character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::is_upper(ch) ==
                static_cast<bool>(std::isupper(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isupper returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (char_type ch = 0xaa41; ch <= 0xaa5a; ++ch)
            {
                CATCH_CHECK(std::isupper(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_upper(ch));
            }
        }
    }

    CATCH_SECTION("Check if a character is a lower-case alphabetic character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::is_lower(ch) ==
                static_cast<bool>(std::islower(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::islower returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (char_type ch = 0xaa61; ch <= 0xaa7a; ++ch)
            {
                CATCH_CHECK(std::islower(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_lower(ch));
            }
        }
    }

    CATCH_SECTION("Check if a character is a decimal digit character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::is_digit(ch) ==
                static_cast<bool>(std::isdigit(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isdigit returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (char_type ch = 0xaa30; ch <= 0xaa39; ++ch)
            {
                CATCH_CHECK(std::isdigit(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_digit(ch));
            }
        }
    }

    CATCH_SECTION("Check if a character is a hexadecimal digit character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::is_x_digit(ch) ==
                static_cast<bool>(std::isxdigit(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isxdigit returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (char_type ch = 0xaa30; ch <= 0xaa39; ++ch)
            {
                CATCH_CHECK(std::isxdigit(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_x_digit(ch));
            }
            for (char_type ch = 0xaa41; ch <= 0xaa46; ++ch)
            {
                CATCH_CHECK(std::isxdigit(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_x_digit(ch));
            }
            for (char_type ch = 0xaa61; ch <= 0xaa66; ++ch)
            {
                CATCH_CHECK(std::isxdigit(static_cast<unsigned char>(ch)));
                CATCH_CHECK_FALSE(BasicString::is_x_digit(ch));
            }
        }
    }
}