#include "fly/logger/styler.hpp"

#include "fly/fly.hpp"
#include "fly/types/string/string.hpp"
#include "test/util/capture_stream.hpp"

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include <catch2/catch.hpp>

#include <iostream>
#include <sstream>
#include <string>

#if defined(FLY_LINUX)

template <typename... Modifiers>
void test_styler(std::string &&expected_escape, Modifiers &&... modifiers)
{
    // These tests redirect the standard output and error streams to a file, making ::isatty() no
    // longer return true. Force it to succeed so the StylerProxy will manipulate the stream.
    fly::MockSystem mock(fly::MockCall::IsATTY, false);
    {
        fly::CaptureStream capture(fly::CaptureStream::Stream::Stdout);
        std::cout << fly::Styler(modifiers...) << "stylized text";

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK(fly::String::starts_with(contents, expected_escape));
        CHECK(fly::String::ends_with(contents, "\x1b[0m"));
    }
    {
        fly::CaptureStream capture(fly::CaptureStream::Stream::Stderr);
        std::cerr << fly::Styler(modifiers...) << "stylized text";

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK(fly::String::starts_with(contents, expected_escape));
        CHECK(fly::String::ends_with(contents, "\x1b[0m"));
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

    SECTION("Non-terminal standard output stream")
    {
        fly::CaptureStream capture(fly::CaptureStream::Stream::Stdout);
        std::cout << fly::Styler(fly::Color::Red) << "non-stylized text";

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK_FALSE(fly::String::starts_with(contents, "\x1b[38;5;1m"));
        CHECK_FALSE(fly::String::ends_with(contents, "\x1b[0m"));
        CHECK(contents == "non-stylized text");
    }

    SECTION("Non-terminal standard error stream")
    {
        fly::CaptureStream capture(fly::CaptureStream::Stream::Stderr);
        std::cerr << fly::Styler(fly::Color::Red) << "non-stylized text";

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK_FALSE(fly::String::starts_with(contents, "\x1b[38;5;1m"));
        CHECK_FALSE(fly::String::ends_with(contents, "\x1b[0m"));
        CHECK(contents == "non-stylized text");
    }

    SECTION("Manipulated with single style")
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

    SECTION("Manipulate with single standard foreground color")
    {
        test_styler("\x1b[38;5;0m", fly::Color::Black);
        test_styler("\x1b[38;5;1m", fly::Color::Red);
        test_styler("\x1b[38;5;2m", fly::Color::Green);
        test_styler("\x1b[38;5;3m", fly::Color::Yellow);
        test_styler("\x1b[38;5;4m", fly::Color::Blue);
        test_styler("\x1b[38;5;5m", fly::Color::Magenta);
        test_styler("\x1b[38;5;6m", fly::Color::Cyan);
        test_styler("\x1b[38;5;7m", fly::Color::White);
    }

    SECTION("Manipulate with single standard background color")
    {
        test_styler("\x1b[48;5;0m", fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;1m", fly::Color(fly::Color::Red, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;2m", fly::Color(fly::Color::Green, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;3m", fly::Color(fly::Color::Yellow, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;4m", fly::Color(fly::Color::Blue, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;5m", fly::Color(fly::Color::Magenta, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;6m", fly::Color(fly::Color::Cyan, fly::Color::Plane::Background));
        test_styler("\x1b[48;5;7m", fly::Color(fly::Color::White, fly::Color::Plane::Background));
    }

    SECTION("Manipulate with single 256-color foreground color")
    {
        for (std::uint32_t color = 0; color <= 255; ++color)
        {
            test_styler("\x1b[38;5;" + std::to_string(color) + "m", color);
        }
    }

    SECTION("Manipulate with single 256-color background color")
    {
        for (std::uint32_t color = 0; color <= 255; ++color)
        {
            test_styler(
                "\x1b[48;5;" + std::to_string(color) + "m",
                fly::Color(color, fly::Color::Plane::Background));
        }
    }

    SECTION("Manipulate with multiple colors")
    {
        test_styler("\x1b[38;5;0;38;5;1m", fly::Color::Black, fly::Color::Red);
        test_styler("\x1b[38;5;1;38;5;0m", fly::Color::Red, fly::Color::Black);

        test_styler(
            "\x1b[38;5;0;48;5;0m",
            fly::Color::Black,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));

        test_styler(
            "\x1b[48;5;1;38;5;0m",
            fly::Color(fly::Color::Red, fly::Color::Plane::Background),
            fly::Color::Black);
    }

    SECTION("Manipulate with styles and colors")
    {
        test_styler("\x1b[1;38;5;1m", fly::Style::Bold, fly::Color::Red);
        test_styler("\x1b[1;38;5;1m", fly::Color::Red, fly::Style::Bold);

        test_styler("\x1b[1;9;38;5;1m", fly::Style::Bold, fly::Style::Strike, fly::Color::Red);
        test_styler("\x1b[1;9;38;5;1m", fly::Style::Bold, fly::Color::Red, fly::Style::Strike);
        test_styler("\x1b[1;9;38;5;1m", fly::Color::Red, fly::Style::Bold, fly::Style::Strike);

        test_styler(
            "\x1b[1;38;5;1;48;5;0m",
            fly::Style::Bold,
            fly::Color::Red,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler(
            "\x1b[1;38;5;1;48;5;0m",
            fly::Color::Red,
            fly::Style::Bold,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background));
        test_styler(
            "\x1b[1;38;5;1;48;5;0m",
            fly::Color::Red,
            fly::Color(fly::Color::Black, fly::Color::Plane::Background),
            fly::Style::Bold);
    }
}

#endif
