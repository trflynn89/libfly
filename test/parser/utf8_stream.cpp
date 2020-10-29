#include "fly/parser/utf8_stream.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"
#include "fly/types/string/string_literal.hpp"

#include <catch2/catch.hpp>

#include <sstream>
#include <vector>

CATCH_TEST_CASE("UTF8StreamSupport", "[parser]")
{
    CATCH_CHECK(fly::UTF8Stream::supports_utf8_stream<char>());
    CATCH_CHECK_FALSE(fly::UTF8Stream::supports_utf8_stream<wchar_t>());

    if constexpr (fly::is_macos())
    {
        CATCH_CHECK_FALSE(fly::UTF8Stream::supports_utf8_stream<char8_t>());
    }
    else
    {
        CATCH_CHECK(fly::UTF8Stream::supports_utf8_stream<char8_t>());
    }

    CATCH_CHECK_FALSE(fly::UTF8Stream::supports_utf8_stream<char16_t>());
    CATCH_CHECK_FALSE(fly::UTF8Stream::supports_utf8_stream<char32_t>());
}

CATCH_TEMPLATE_TEST_CASE("UTF8Stream", "[parser]", char, char8_t)
{
    using char_type = TestType;

    if constexpr (fly::UTF8Stream::supports_utf8_stream<char_type>())
    {
        using string_type = std::basic_string<char_type>;
        using int_type = typename std::char_traits<char_type>::int_type;
        constexpr const int_type eof = std::char_traits<char_type>::eof();

        std::basic_stringstream<char_type> stream;

        auto utf8_stream = fly::UTF8Stream::create(stream);
        CATCH_REQUIRE(utf8_stream);

        CATCH_SECTION("Operations on an empty stream")
        {
            CATCH_CHECK(utf8_stream->template peek<int_type>() == eof);
            CATCH_CHECK(utf8_stream->template get<int_type>() == eof);

            string_type value;
            CATCH_CHECK_FALSE(utf8_stream->getline(value));
            CATCH_CHECK(value.empty());
        }

        CATCH_SECTION("Operations on a single-line stream")
        {
            static string_type s_test = FLY_STR(char_type, "test");
            stream << s_test;

            CATCH_SECTION("Peek and get")
            {
                for (std::size_t i = 0; i < s_test.size(); ++i)
                {
                    CATCH_CHECK(utf8_stream->template peek<char_type>() == s_test[i]);
                    CATCH_CHECK(utf8_stream->template get<char_type>() == s_test[i]);
                }

                CATCH_CHECK(utf8_stream->template peek<int_type>() == eof);
                CATCH_CHECK(utf8_stream->template get<int_type>() == eof);
            }

            CATCH_SECTION("Get line")
            {
                string_type value;
                CATCH_CHECK(utf8_stream->getline(value));
                CATCH_CHECK(value == s_test);

                CATCH_CHECK_FALSE(utf8_stream->getline(value));
                CATCH_CHECK(value.empty());
            }
        }

        CATCH_SECTION("Operations on a multi-line stream")
        {
            static string_type s_test = FLY_STR(char_type, "test\nmultiple\nlines");
            stream << s_test;

            CATCH_SECTION("Peek and get")
            {
                for (std::size_t i = 0; i < s_test.size(); ++i)
                {
                    CATCH_CHECK(utf8_stream->template peek<char_type>() == s_test[i]);
                    CATCH_CHECK(utf8_stream->template get<char_type>() == s_test[i]);
                }

                CATCH_CHECK(utf8_stream->template peek<int_type>() == eof);
                CATCH_CHECK(utf8_stream->template get<int_type>() == eof);
            }

            CATCH_SECTION("Get line")
            {
                auto lines = fly::BasicString<string_type>::split(s_test, FLY_CHR(char_type, '\n'));
                string_type value;

                for (const auto &line : lines)
                {
                    CATCH_CHECK(utf8_stream->getline(value));
                    CATCH_CHECK(value == line);
                }

                CATCH_CHECK_FALSE(utf8_stream->getline(value));
                CATCH_CHECK(value.empty());
            }
        }

        CATCH_SECTION("Operations on a multi-line stream with beginning newline")
        {
            static string_type s_test = FLY_STR(char_type, "\ntest\nmultiple\nlines");
            stream << s_test;

            auto lines = fly::BasicString<string_type>::split(s_test, FLY_CHR(char_type, '\n'));
            string_type value;

            CATCH_CHECK(utf8_stream->getline(value));
            CATCH_CHECK(value.empty());

            for (const auto &line : lines)
            {
                CATCH_CHECK(utf8_stream->getline(value));
                CATCH_CHECK(value == line);
            }

            CATCH_CHECK_FALSE(utf8_stream->getline(value));
            CATCH_CHECK(value.empty());
        }

        CATCH_SECTION("Operations on a multi-line stream with ending newline")
        {
            static string_type s_test = FLY_STR(char_type, "test\nmultiple\nlines\n");
            stream << s_test;

            auto lines = fly::BasicString<string_type>::split(s_test, FLY_CHR(char_type, '\n'));
            string_type value;

            for (const auto &line : lines)
            {
                CATCH_CHECK(utf8_stream->getline(value));
                CATCH_CHECK(value == line);
            }

            CATCH_CHECK_FALSE(utf8_stream->getline(value));
            CATCH_CHECK(value.empty());
        }
    }
}
