#include "fly/types/string/lexer.hpp"

#include "fly/types/string/literals.hpp"

#include "catch2/catch_template_test_macros.hpp"
#include "catch2/catch_test_macros.hpp"

#include <string>

CATCH_TEMPLATE_TEST_CASE("BasicLexer", "[string]", char, wchar_t, char8_t, char16_t, char32_t)
{
    using char_type = TestType;
    using Lexer = fly::BasicLexer<char_type>;

    CATCH_SECTION("Cannot consume from empty lexer")
    {
        Lexer lexer(FLY_ARR(char_type, ""));

        CATCH_CHECK_FALSE(lexer.peek());
        CATCH_CHECK_FALSE(lexer.consume());
        CATCH_CHECK_FALSE(lexer.consume_if(FLY_CHR(char_type, '\0')));
        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK_FALSE(lexer.consume_hex_number());
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

    CATCH_SECTION("Lexer accepts an already-existing string view")
    {
        const char_type *str = FLY_STR(char_type, "ab");
        auto view = std::basic_string_view<char_type>(str);

        Lexer lexer(view);
        CATCH_CHECK(lexer.view() == FLY_ARR(char_type, "ab"));

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'a')));
        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'b')));
        CATCH_CHECK_FALSE(lexer.consume());
    }

    CATCH_SECTION("Setting the lexer's position mutates internal pointer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        lexer.set_position(1);
        CATCH_CHECK(lexer.position() == 1);

        auto p1 = lexer.peek();
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'b'));

        lexer.set_position(0);
        CATCH_CHECK(lexer.position() == 0);

        auto p2 = lexer.peek();
        CATCH_REQUIRE(p2.has_value());
        CATCH_CHECK(p2.value() == FLY_CHR(char_type, 'a'));

        lexer.set_position(2);
        CATCH_CHECK(lexer.position() == 2);
        CATCH_CHECK_FALSE(lexer.peek());
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

    CATCH_SECTION("Cannot consume decimal number if no number exists")
    {
        Lexer lexer(FLY_ARR(char_type, "ab"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Cannot consume decimal number past end of lexer")
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

    CATCH_SECTION("Cannot consume decimal number if number exists past internal pointer")
    {
        Lexer lexer(FLY_ARR(char_type, "ab1"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Decimal number consumption stops at first non-digit character")
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

    CATCH_SECTION("Decimal number consumption stops at end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "1"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 1);
        CATCH_CHECK(lexer.position() == 1);

        CATCH_CHECK_FALSE(lexer.peek());
    }

    CATCH_SECTION("Decimal number consumption consumes all digits in a row")
    {
        Lexer lexer(FLY_ARR(char_type, "123"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 123);
        CATCH_CHECK(lexer.position() == 3);

        CATCH_CHECK_FALSE(lexer.peek());
    }

    CATCH_SECTION("Decimal number consumption can succeed multiple times per lexer if separated")
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

    CATCH_SECTION("Cannot consume hex number if no number exists")
    {
        Lexer lexer(FLY_ARR(char_type, "xy"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_hex_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Cannot consume hex number past end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "1"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_hex_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 1);
        CATCH_CHECK(lexer.position() == 1);

        CATCH_CHECK_FALSE(lexer.consume_hex_number());
        CATCH_CHECK(lexer.position() == 1);
    }

    CATCH_SECTION("Cannot consume hex number if number exists past internal pointer")
    {
        Lexer lexer(FLY_ARR(char_type, "xy1"));
        CATCH_CHECK(lexer.position() == 0);

        CATCH_CHECK_FALSE(lexer.consume_hex_number());
        CATCH_CHECK(lexer.position() == 0);
    }

    CATCH_SECTION("Hex number consumption stops at first non-digit character")
    {
        Lexer lexer(FLY_ARR(char_type, "1ax"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_hex_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 0x1a);
        CATCH_CHECK(lexer.position() == 2);

        auto p1 = lexer.peek();
        CATCH_REQUIRE(p1.has_value());
        CATCH_CHECK(p1.value() == FLY_CHR(char_type, 'x'));
        CATCH_CHECK(lexer.position() == 2);
    }

    CATCH_SECTION("Hex number consumption stops at end of lexer")
    {
        Lexer lexer(FLY_ARR(char_type, "1a"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_hex_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 0x1a);
        CATCH_CHECK(lexer.position() == 2);

        CATCH_CHECK_FALSE(lexer.peek());
    }

    CATCH_SECTION("Hex number consumption consumes all digits in a row")
    {
        Lexer lexer(FLY_ARR(char_type, "123a"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_hex_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 0x123a);
        CATCH_CHECK(lexer.position() == 4);

        CATCH_CHECK_FALSE(lexer.peek());
    }

    CATCH_SECTION("Hex number consumption can succeed multiple times per lexer if separated")
    {
        Lexer lexer(FLY_ARR(char_type, "123ax456B"));
        CATCH_CHECK(lexer.position() == 0);

        auto n1 = lexer.consume_hex_number();
        CATCH_REQUIRE(n1.has_value());
        CATCH_CHECK(n1.value() == 0x123a);
        CATCH_CHECK(lexer.position() == 4);

        CATCH_CHECK(lexer.consume_if(FLY_CHR(char_type, 'x')));
        CATCH_CHECK(lexer.position() == 5);

        auto n2 = lexer.consume_hex_number();
        CATCH_REQUIRE(n2.has_value());
        CATCH_CHECK(n2.value() == 0x456b);
        CATCH_CHECK(lexer.position() == 9);
    }
}
