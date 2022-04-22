#include "fly/types/string/string.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <cctype>
#include <string>

CATCH_TEMPLATE_TEST_CASE("BasicClassifier", "[string]", char, wchar_t, char8_t, char16_t, char32_t)
{
    using BasicString = fly::BasicString<TestType>;

    using string_type = typename BasicString::string_type;
    using char_type = typename BasicString::char_type;
    using view_type = typename BasicString::view_type;

    CATCH_SECTION("Get the size of a string-like type")
    {
        char_type const *cstr = FLY_STR(char_type, "ten chars!");
        string_type str = cstr;
        view_type view = str;
        char_type const arr[] = {'t', 'e', 'n', ' ', 'c', 'h', 'a', 'r', 's', '!'};

        CATCH_CHECK(BasicString::size(cstr) == 10);
        CATCH_CHECK(BasicString::size(str) == 10);
        CATCH_CHECK(BasicString::size(view) == 10);
        CATCH_CHECK(BasicString::size(arr) == 10);
    }

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

    CATCH_SECTION("Convert a character to an upper-case alphabetic character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::to_upper(ch) ==
                static_cast<char_type>(std::toupper(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::toupper returning an
            // upper-case character when cast to unsigned char (which is how the spec suggests to
            // avoid undefined behavior).
            for (char_type ch = 0xaa41; ch <= 0xaa5a; ++ch)
            {
                CATCH_CHECK(
                    ch != static_cast<char_type>(std::toupper(static_cast<unsigned char>(ch))));
                CATCH_CHECK(ch == BasicString::to_upper(ch));
            }
        }
    }

    CATCH_SECTION("Convert a character to a lower-case alphabetic character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::to_lower(ch) ==
                static_cast<char_type>(std::tolower(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::toupper returning a lower-case
            // character when cast to unsigned char (which is how the spec suggests to avoid
            // undefined behavior).
            for (char_type ch = 0xaa61; ch <= 0xaa7a; ++ch)
            {
                CATCH_CHECK(
                    ch != static_cast<char_type>(std::tolower(static_cast<unsigned char>(ch))));
                CATCH_CHECK(ch == BasicString::to_lower(ch));
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

    CATCH_SECTION("Check if a character is a whitespace character")
    {
        for (char_type ch = 0; (ch >= 0) && (ch < 0x80); ++ch)
        {
            CATCH_CHECK(
                BasicString::is_space(ch) ==
                static_cast<bool>(std::isspace(static_cast<unsigned char>(ch))));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isspace returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            CATCH_CHECK(std::isspace(static_cast<unsigned char>(0xaa20)));
            CATCH_CHECK_FALSE(BasicString::is_space(0xaa20));

            CATCH_CHECK(std::isspace(static_cast<unsigned char>(0xaa0a)));
            CATCH_CHECK_FALSE(BasicString::is_space(0xaa0a));

            CATCH_CHECK(std::isspace(static_cast<unsigned char>(0xaa09)));
            CATCH_CHECK_FALSE(BasicString::is_space(0xaa09));
        }
    }
}
