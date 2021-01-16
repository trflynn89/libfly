#include "fly/types/string/detail/string_lexer.hpp"

#include "fly/types/string/string_literal.hpp"

#include "catch2/catch.hpp"

#include <string>

CATCH_TEMPLATE_TEST_CASE(
    "BasicStringLexer",
    "[string]",
    std::string,
    std::wstring,
    std::u8string,
    std::u16string,
    std::u32string)
{
    using StringType = TestType;
    using char_type = typename StringType::value_type;
    using Lexer = fly::detail::BasicStringLexer<StringType>;

    CATCH_SECTION("Cannot consume from empty lexer")
    {
        Lexer lexer(FLY_ARR(char_type, ""));

        CATCH_CHECK_FALSE(lexer.peek());
        CATCH_CHECK_FALSE(lexer.consume());
        CATCH_CHECK_FALSE(lexer.consume_if(FLY_CHR(char_type, '\0')));
        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Lexer accepts null-terminated string")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.view() == FLY_ARR(char_type, "ab"));

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'b')));
        CATCH_CHECK_FALSE(lexer.consume());
    }

    CATCH_SECTION("Lexer accepts non-null-terminated string")
    {
        constexpr const char_type str[] {
            static_cast<char_type>(0x61), // a
            static_cast<char_type>(0x62), // b
        };

        Lexer lexer(str);
        CATCH_CHECK(lexer.view() == FLY_ARR(char_type, "ab"));

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'b')));
        CATCH_CHECK_FALSE(lexer.consume());
    }

    CATCH_SECTION("Peeking does not advance internal pointer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        auto p1 = lexer.peek();
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 0);

        auto p2 = lexer.peek();
        CATCH_REQUIRE(p2.has_value());
        CATCH_CHECK(p2.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Cannot peek past end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));

        auto p1 = lexer.peek(0);
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'a'));

        auto p2 = lexer.peek(1);
        CATCH_REQUIRE(p2.has_value());
        CATCH_CHECK(p2.value() == FLY_CHR(char_type, 'b'));

        CATCH_CHECK_FALSE(lexer.peek(2));
    }

    CATCH_SECTION("Consuming character advances internal pointer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        auto p1 = lexer.peek();
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 0);

        auto c1 = lexer.consume();
        CATCH_REQUIRE(c1.has_value());
        CATCH_CHECK(c1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 1);

        auto p2 = lexer.peek();
        CATCH_REQUIRE(p2.has_value());
        CATCH_CHECK(p2.value() == FLY_CHR(char_type, 'b'));
        CATCH_CHECK(lexer.position() == 1);
    }

    CATCH_SECTION("Cannot consume past end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        auto c1 = lexer.consume();
        CATCH_REQUIRE(c1.has_value());
        CATCH_CHECK(c1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 1);

        auto c2 = lexer.consume();
        CATCH_REQUIRE(c2.has_value());
        CATCH_CHECK(c2.value() == FLY_CHR(char_type, 'b'));
        CATCH_CHECK(lexer.position() == 2);

        CATCH_CHECK_FALSE(lexer.consume());
        CATCH_CHECK(lexer.position() == 2);
    }

    CATCH_SECTION("Conditional consumption fails if character does not match")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_if(FLY_CHR(char_type, 'b')));
        CATCH_CHECK(lexer.position() == 0);

        auto c1 = lexer.consume();
        CATCH_REQUIRE(c1.has_value());
        CATCH_CHECK(c1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 1);

        CATCH_CHECK_FALSE(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.position() == 1);

        auto c2 = lexer.consume();
        CATCH_REQUIRE(c2.has_value());
        CATCH_CHECK(c2.value() == FLY_CHR(char_type, 'b'));
        CATCH_CHECK(lexer.position() == 2);
    }

    CATCH_SECTION("Conditional consumption advances internal pointer if character matches")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        auto p1 = lexer.peek();
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.position() == 1);

        auto p2 = lexer.peek();
        CATCH_REQUIRE(p2.has_value());
        CATCH_CHECK(p2.value() == FLY_CHR(char_type, 'b'));
        CATCH_CHECK(lexer.position() == 1);
    }

    CATCH_SECTION("Cannot conditionally consume past end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.position() == 1);

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'b')));
        CATCH_CHECK(lexer.position() == 2);

        CATCH_CHECK_FALSE(lexer.consume_if(FLY_CHR(char_type, '\0')));
        CATCH_CHECK(lexer.position() == 2);
    }

    CATCH_SECTION("Cannot consume number if no number exists")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Cannot consume number past end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "1"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 1);
        CATCH_CHECK(lexer.position() == 1);

        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK(lexer.position() == 1);
    }

    CATCH_SECTION("Cannot consume number if number exists past internal pointer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab1"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Number consumption stops at first non-digit character")
    {
        Lexer lexer(FLY_ARR(char_type, "1ab"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 1);
        CATCH_CHECK(lexer.position() == 1);

        auto p1 = lexer.peek();
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'a'));
        CATCH_CHECK(lexer.position() == 1);
    }

    CATCH_SECTION("Number consumption stops at end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "1"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 1);
        CATCH_CHECK(lexer.position() == 1);

        CATCH_CHECK_FALSE(lexer.peek());
    }

    CATCH_SECTION("Number consumption consumes all digits in a row")
    {
        Lexer lexer(FLY_ARR(char_type, "123"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 123);
        CATCH_CHECK(lexer.position() == 3);

        CATCH_CHECK_FALSE(lexer.peek());
    }

    CATCH_SECTION("Number consumption can succeed multiple times per lexer if separated")
    {
        Lexer lexer(FLY_ARR(char_type, "123a456"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 123);
        CATCH_CHECK(lexer.position() == 3);

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.position() == 4);

        auto n2 = lexer.consume_number();
        CATCH_REQUIRE(n2.has_value());
        CATCH_CHECK(n2.value() == 456);
        CATCH_CHECK(lexer.position() == 7);
    }
}
