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
        for (int i = 0; i < 0x80; ++i)
        {
            const auto ch = static_cast<char_type>(i);
            CATCH_CHECK(BasicString::is_alpha(ch) == static_cast<bool>(std::isalpha(i)));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isalpha returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (int i = 0xaa41; i <= 0xaa5a; ++i)
            {
                CATCH_CHECK(std::isalpha(static_cast<unsigned char>(i)));
                CATCH_CHECK_FALSE(BasicString::is_alpha(static_cast<char_type>(i)));
            }

            for (int i = 0xaa61; i <= 0xaa7a; ++i)
            {
                CATCH_CHECK(std::isalpha(static_cast<unsigned char>(i)));
                CATCH_CHECK_FALSE(BasicString::is_alpha(static_cast<char_type>(i)));
            }
        }
    }

    CATCH_SECTION("Check if a character is a decimal digit character")
    {
        for (int i = 0; i < 0x80; ++i)
        {
            const auto ch = static_cast<char_type>(i);
            CATCH_CHECK(BasicString::is_digit(ch) == static_cast<bool>(std::isdigit(i)));
        }

        if constexpr (sizeof(char_type) > 1)
        {
            // Spot check some values that incorrectly result in std::isdigit returning true when
            // cast to unsigned char (which is how the spec suggests to avoid undefined behavior).
            for (int i = 0xaa30; i <= 0xaa39; ++i)
            {
                CATCH_CHECK(std::isdigit(static_cast<unsigned char>(i)));
                CATCH_CHECK_FALSE(BasicString::is_digit(static_cast<char_type>(i)));
            }
        }
    }
}
