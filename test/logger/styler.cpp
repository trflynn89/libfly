#include "fly/logger/styler.hpp"

#include "fly/fly.hpp"
#include "fly/traits/traits.hpp"
#include "fly/types/string/string.hpp"
#include "test/util/capture_stream.hpp"

#include <catch2/catch.hpp>

#include <iostream>
#include <sstream>
#include <string>

#if defined(FLY_LINUX)

template <typename... Modifiers>
void test_styler(std::string &&expected_escape, Modifiers &&... modifiers)
{
    constexpr bool style_or_color =
        fly::any_same_v<fly::Style, Modifiers...> || fly::any_same_v<fly::Color, Modifiers...>;

    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stdout);
        std::cout << fly::Styler(modifiers...) << "stylized text";

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK(fly::String::starts_with(contents, expected_escape));

        if constexpr (style_or_color)
        {
            CHECK(fly::String::ends_with(contents, "\x1b[0m"));
        }
    }
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        std::cerr << fly::Styler(modifiers...) << "stylized text";

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK(fly::String::starts_with(contents, expected_escape));

        if constexpr (style_or_color)
        {
            CHECK(fly::String::ends_with(contents, "\x1b[0m"));
        }
    }
}

TEST_CASE("Styler", "[logger]")
{
    SECTION("Non-standard output or error stream")
    {
        std::stringstream stream;

        stream << fly::Styler(fly::Color::Red) << "non-stylized text";
        const std::string contents = stream.str();

        CHECK_FALSE(fly::String::starts_with(contents, "\x1b[38;5;1m"));
        CHECK_FALSE(fly::String::ends_with(contents, "\x1b[0m"));
        CHECK(contents == "non-stylized text");
    }

    SECTION("Manipulated with a single style")
    {
        test_styler("\x1b[1m", fly::Style::Bold);
        test_styler("\x1b[2m", fly::Style::Dim);
        test_styler("\x1b[3m", fly::Style::Italic);
        test_styler("\x1b[4m", fly::Style::Underline);
        test_styler("\x1b[5m", fly::Style::Blink);
        test_styler("\x1b[9m", fly::Style::Strike);
    }

    SECTION("Manipulate with multiple styles")
    {
        test_styler("\x1b[1;2m", fly::Style::Bold, fly::Style::Dim);
        test_styler("\x1b[1;2;3m", fly::Style::Bold, fly::Style::Dim, fly::Style::Italic);
        test_styler(
            "\x1b[1;2;3;4m",
            fly::Style::Bold,
            fly::Style::Dim,
            fly::Style::Italic,
            fly::Style::Underline);
        test_styler(
            "\x1b[1;2;3;4;5m",
            fly::Style::Bold,
            fly::Style::Dim,
            fly::Style::Italic,
            fly::Style::Underline,
            fly::Style::Blink);
        test_styler(
            "\x1b[1;2;3;4;5;9m",
            fly::Style::Bold,
            fly::Style::Dim,
            fly::Style::Italic,
            fly::Style::Underline,
            fly::Style::Blink,
            fly::Style::Strike);
        test_styler(
            "\x1b[9;5;4;3;2;1m",
            fly::Style::Strike,
            fly::Style::Blink,
            fly::Style::Underline,
            fly::Style::Italic,
            fly::Style::Dim,
            fly::Style::Bold);
    }

    SECTION("Manipulate with a single standard foreground color")
    {
        test_styler("\x1b[30m", fly::Color::Black);
        test_styler("\x1b[31m", fly::Color::Red);
        test_styler("\x1b[32m", fly::Color::Green);
        test_styler("\x1b[33m", fly::Color::Yellow);
        test_styler("\x1b[34m", fly::Color::Blue);
        test_styler("\x1b[35m", fly::Color::Magenta);
        test_styler("\x1b[36m", fly::Color::Cyan);
        test_styler("\x1b[37m", fly::Color::White);
    }

    SECTION("Manipulate with a single standard background color")
    {
        test_styler("\x1b[40m", fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler("\x1b[41m", fly::Color(fly::Color::Red, fly::Color::Plane::Background));
        test_styler("\x1b[42m", fly::Color(fly::Color::Green, fly::Color::Plane::Background));
        test_styler("\x1b[43m", fly::Color(fly::Color::Yellow, fly::Color::Plane::Background));
        test_styler("\x1b[44m", fly::Color(fly::Color::Blue, fly::Color::Plane::Background));
        test_styler("\x1b[45m", fly::Color(fly::Color::Magenta, fly::Color::Plane::Background));
        test_styler("\x1b[46m", fly::Color(fly::Color::Cyan, fly::Color::Plane::Background));
        test_styler("\x1b[47m", fly::Color(fly::Color::White, fly::Color::Plane::Background));
    }

    SECTION("Manipulate with a single 256-color foreground color")
    {
        for (std::uint32_t color = fly::Color::White + 1; color <= 255; ++color)
        {
            test_styler("\x1b[38;5;" + std::to_string(color) + "m", color);
        }
    }

    SECTION("Manipulate with a single 256-color background color")
    {
        for (std::uint32_t color = fly::Color::White + 1; color <= 255; ++color)
        {
            test_styler(
                "\x1b[48;5;" + std::to_string(color) + "m",
                fly::Color(color, fly::Color::Plane::Background));
        }
    }

    SECTION("Manipulate with multiple colors")
    {
        test_styler("\x1b[30;31m", fly::Color::Black, fly::Color::Red);
        test_styler("\x1b[31;30m", fly::Color::Red, fly::Color::Black);

        test_styler(
            "\x1b[30;40m",
            fly::Color::Black,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));

        test_styler(
            "\x1b[41;30m",
            fly::Color(fly::Color::Red, fly::Color::Plane::Background),
            fly::Color::Black);
    }

    SECTION("Manipulate with a single cursor position")
    {
        test_styler("\x1b[1A", fly::Cursor::Direction::Up);
        test_styler("\x1b[1B", fly::Cursor::Direction::Down);
        test_styler("\x1b[1C", fly::Cursor::Direction::Forward);
        test_styler("\x1b[1D", fly::Cursor::Direction::Backward);
    }

    SECTION("Manipulate with a single cursor position with explicit distance")
    {
        test_styler("\x1b[2A", fly::Cursor(fly::Cursor::Direction::Up, 2));
        test_styler("\x1b[3B", fly::Cursor(fly::Cursor::Direction::Down, 3));
        test_styler("\x1b[4C", fly::Cursor(fly::Cursor::Direction::Forward, 4));
        test_styler("\x1b[5D", fly::Cursor(fly::Cursor::Direction::Backward, 5));
    }

    SECTION("Manipulate with a single cursor position with distance 0 becomes distance 1")
    {
        test_styler("\x1b[1A", fly::Cursor(fly::Cursor::Direction::Up, 0));
        test_styler("\x1b[1B", fly::Cursor(fly::Cursor::Direction::Down, 0));
        test_styler("\x1b[1C", fly::Cursor(fly::Cursor::Direction::Forward, 0));
        test_styler("\x1b[1D", fly::Cursor(fly::Cursor::Direction::Backward, 0));
    }

    SECTION("Manipulate with multiple cursor positions")
    {
        test_styler("\x1b[1A\x1b[1B", fly::Cursor::Direction::Up, fly::Cursor::Direction::Down);
        test_styler(
            "\x1b[1A\x1b[1B\x1b[1C",
            fly::Cursor::Direction::Up,
            fly::Cursor::Direction::Down,
            fly::Cursor::Direction::Forward);
        test_styler(
            "\x1b[1A\x1b[1B\x1b[1C\x1b[1D",
            fly::Cursor::Direction::Up,
            fly::Cursor::Direction::Down,
            fly::Cursor::Direction::Forward,
            fly::Cursor::Direction::Backward);
        test_styler(
            "\x1b[1D\x1b[1C\x1b[1B\x1b[1A",
            fly::Cursor::Direction::Backward,
            fly::Cursor::Direction::Forward,
            fly::Cursor::Direction::Down,
            fly::Cursor::Direction::Up);
    }

    SECTION("Manipulate with styles and colors")
    {
        test_styler("\x1b[1;31m", fly::Style::Bold, fly::Color::Red);
        test_styler("\x1b[1;31m", fly::Color::Red, fly::Style::Bold);

        test_styler("\x1b[1;9;31m", fly::Style::Bold, fly::Style::Strike, fly::Color::Red);
        test_styler("\x1b[1;9;31m", fly::Style::Bold, fly::Color::Red, fly::Style::Strike);
        test_styler("\x1b[1;9;31m", fly::Color::Red, fly::Style::Bold, fly::Style::Strike);

        test_styler(
            "\x1b[1;31;40m",
            fly::Style::Bold,
            fly::Color::Red,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::Color::Red,
            fly::Style::Bold,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::Color::Red,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background),
            fly::Style::Bold);
    }

    SECTION("Manipulate with styles, colors, and cursor positions")
    {
        test_styler("\x1b[1;31m", fly::Style::Bold, fly::Color::Red);
        test_styler("\x1b[1;31m", fly::Color::Red, fly::Style::Bold);

        test_styler("\x1b[1;9;31m", fly::Style::Bold, fly::Style::Strike, fly::Color::Red);
        test_styler("\x1b[1;9;31m", fly::Style::Bold, fly::Color::Red, fly::Style::Strike);
        test_styler("\x1b[1;9;31m", fly::Color::Red, fly::Style::Bold, fly::Style::Strike);

        test_styler(
            "\x1b[1;31m\x1b[1A",
            fly::Style::Bold,
            fly::Color::Red,
            fly::Cursor::Direction::Up);
        test_styler(
            "\x1b[1;31m\x1b[1A",
            fly::Cursor::Direction::Up,
            fly::Color::Red,
            fly::Style::Bold);

        test_styler(
            "\x1b[1;31;40m",
            fly::Style::Bold,
            fly::Color::Red,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::Color::Red,
            fly::Style::Bold,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::Color::Red,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background),
            fly::Style::Bold);
    }
}

#endif
