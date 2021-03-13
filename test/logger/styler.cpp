#include "fly/logger/styler.hpp"

#include "test/util/capture_stream.hpp"

#include "fly/fly.hpp"
#include "fly/traits/traits.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_test_macros.hpp"

#include <iostream>
#include <sstream>
#include <string>

using namespace fly::literals::styler_literals;

#if defined(FLY_LINUX) || defined(FLY_MACOS)

template <typename... Modifiers>
void test_styler(std::string &&expected_escape, Modifiers &&...modifiers)
{
    constexpr bool style_or_color = fly::any_same_v<fly::logger::Style, Modifiers...> ||
        fly::any_same_v<fly::logger::Color, Modifiers...> ||
        fly::any_same_v<fly::logger::Color::StandardColor, Modifiers...>;

    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stdout);
        std::cout << fly::logger::Styler(modifiers...) << "stylized text";

        const std::string contents = capture();
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.starts_with(expected_escape));

        if constexpr (style_or_color)
        {
            CATCH_CHECK(contents.ends_with("\x1b[0m"));
        }
    }
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        std::cerr << fly::logger::Styler(modifiers...) << "stylized text";

        const std::string contents = capture();
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.starts_with(expected_escape));

        if constexpr (style_or_color)
        {
            CATCH_CHECK(contents.ends_with("\x1b[0m"));
        }
    }
}

CATCH_TEST_CASE("Styler", "[logger]")
{
    CATCH_SECTION("Non-standard output or error stream")
    {
        std::stringstream stream;

        stream << fly::logger::Styler(fly::logger::Color::Red) << "non-stylized text";
        const std::string contents = stream.str();

        CATCH_CHECK_FALSE(contents.starts_with("\x1b[38;5;1m"));
        CATCH_CHECK_FALSE(contents.ends_with("\x1b[0m"));
        CATCH_CHECK(contents == "non-stylized text");
    }

    CATCH_SECTION("Manipulated with a single style")
    {
        test_styler("\x1b[1m", fly::logger::Style::Bold);
        test_styler("\x1b[2m", fly::logger::Style::Dim);
        test_styler("\x1b[3m", fly::logger::Style::Italic);
        test_styler("\x1b[4m", fly::logger::Style::Underline);
        test_styler("\x1b[5m", fly::logger::Style::Blink);
        test_styler("\x1b[9m", fly::logger::Style::Strike);
    }

    CATCH_SECTION("Manipulate with multiple styles")
    {
        test_styler("\x1b[1;2m", fly::logger::Style::Bold, fly::logger::Style::Dim);
        test_styler(
            "\x1b[1;2;3m",
            fly::logger::Style::Bold,
            fly::logger::Style::Dim,
            fly::logger::Style::Italic);
        test_styler(
            "\x1b[1;2;3;4m",
            fly::logger::Style::Bold,
            fly::logger::Style::Dim,
            fly::logger::Style::Italic,
            fly::logger::Style::Underline);
        test_styler(
            "\x1b[1;2;3;4;5m",
            fly::logger::Style::Bold,
            fly::logger::Style::Dim,
            fly::logger::Style::Italic,
            fly::logger::Style::Underline,
            fly::logger::Style::Blink);
        test_styler(
            "\x1b[1;2;3;4;5;9m",
            fly::logger::Style::Bold,
            fly::logger::Style::Dim,
            fly::logger::Style::Italic,
            fly::logger::Style::Underline,
            fly::logger::Style::Blink,
            fly::logger::Style::Strike);
        test_styler(
            "\x1b[9;5;4;3;2;1m",
            fly::logger::Style::Strike,
            fly::logger::Style::Blink,
            fly::logger::Style::Underline,
            fly::logger::Style::Italic,
            fly::logger::Style::Dim,
            fly::logger::Style::Bold);
    }

    CATCH_SECTION("Manipulate with a single standard foreground color")
    {
        test_styler("\x1b[30m", fly::logger::Color::Black);
        test_styler("\x1b[31m", fly::logger::Color::Red);
        test_styler("\x1b[32m", fly::logger::Color::Green);
        test_styler("\x1b[33m", fly::logger::Color::Yellow);
        test_styler("\x1b[34m", fly::logger::Color::Blue);
        test_styler("\x1b[35m", fly::logger::Color::Magenta);
        test_styler("\x1b[36m", fly::logger::Color::Cyan);
        test_styler("\x1b[37m", fly::logger::Color::White);
    }

    CATCH_SECTION("Manipulate with a single standard background color")
    {
        test_styler(
            "\x1b[40m",
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background));
        test_styler(
            "\x1b[41m",
            fly::logger::Color(fly::logger::Color::Red, fly::logger::Color::Background));
        test_styler(
            "\x1b[42m",
            fly::logger::Color(fly::logger::Color::Green, fly::logger::Color::Background));
        test_styler(
            "\x1b[43m",
            fly::logger::Color(fly::logger::Color::Yellow, fly::logger::Color::Background));
        test_styler(
            "\x1b[44m",
            fly::logger::Color(fly::logger::Color::Blue, fly::logger::Color::Background));
        test_styler(
            "\x1b[45m",
            fly::logger::Color(fly::logger::Color::Magenta, fly::logger::Color::Background));
        test_styler(
            "\x1b[46m",
            fly::logger::Color(fly::logger::Color::Cyan, fly::logger::Color::Background));
        test_styler(
            "\x1b[47m",
            fly::logger::Color(fly::logger::Color::White, fly::logger::Color::Background));
    }

    CATCH_SECTION("Manipulate with a single 256-color foreground color")
    {
        for (std::uint32_t color = fly::logger::Color::White + 1; color <= 255; ++color)
        {
            test_styler(
                "\x1b[38;5;" + std::to_string(color) + "m",
                fly::logger::Color(static_cast<std::uint8_t>(color)));
        }
    }

    CATCH_SECTION("Manipulate with a single 256-color foreground color literal")
    {
        test_styler("\x1b[30m", 0_c);
        test_styler("\x1b[31m", 1_c);
        test_styler("\x1b[32m", 2_c);
        test_styler("\x1b[33m", 3_c);
        test_styler("\x1b[34m", 4_c);
        test_styler("\x1b[35m", 5_c);
        test_styler("\x1b[36m", 6_c);
        test_styler("\x1b[37m", 7_c);
        test_styler("\x1b[38;5;8m", 8_c);
        test_styler("\x1b[38;5;16m", 16_c);
        test_styler("\x1b[38;5;32m", 32_c);
        test_styler("\x1b[38;5;64m", 64_c);
        test_styler("\x1b[38;5;128m", 128_c);
        test_styler("\x1b[38;5;255m", 255_c);
    }

    CATCH_SECTION("Manipulate with a single 256-color background color")
    {
        for (std::uint32_t color = fly::logger::Color::White + 1; color <= 255; ++color)
        {
            test_styler(
                "\x1b[48;5;" + std::to_string(color) + "m",
                fly::logger::Color(
                    static_cast<std::uint8_t>(color),
                    fly::logger::Color::Background));
        }
    }

    CATCH_SECTION("Manipulate with multiple colors")
    {
        test_styler("\x1b[30;31m", fly::logger::Color::Black, fly::logger::Color::Red);
        test_styler("\x1b[31;30m", fly::logger::Color::Red, fly::logger::Color::Black);

        test_styler(
            "\x1b[30;40m",
            fly::logger::Color::Black,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background));

        test_styler(
            "\x1b[41;30m",
            fly::logger::Color(fly::logger::Color::Red, fly::logger::Color::Background),
            fly::logger::Color::Black);
    }

    CATCH_SECTION("Manipulate with a single cursor position")
    {
        test_styler("\x1b[1A", fly::logger::Cursor::Up);
        test_styler("\x1b[1B", fly::logger::Cursor::Down);
        test_styler("\x1b[1C", fly::logger::Cursor::Forward);
        test_styler("\x1b[1D", fly::logger::Cursor::Backward);
    }

    CATCH_SECTION("Manipulate with a single cursor position with explicit distance")
    {
        test_styler("\x1b[2A", fly::logger::Cursor(fly::logger::Cursor::Up, 2));
        test_styler("\x1b[3B", fly::logger::Cursor(fly::logger::Cursor::Down, 3));
        test_styler("\x1b[4C", fly::logger::Cursor(fly::logger::Cursor::Forward, 4));
        test_styler("\x1b[5D", fly::logger::Cursor(fly::logger::Cursor::Backward, 5));
    }

    CATCH_SECTION("Manipulate with a single cursor position with distance 0 becomes distance 1")
    {
        test_styler("\x1b[1A", fly::logger::Cursor(fly::logger::Cursor::Up, 0));
        test_styler("\x1b[1B", fly::logger::Cursor(fly::logger::Cursor::Down, 0));
        test_styler("\x1b[1C", fly::logger::Cursor(fly::logger::Cursor::Forward, 0));
        test_styler("\x1b[1D", fly::logger::Cursor(fly::logger::Cursor::Backward, 0));
    }

    CATCH_SECTION("Manipulate with multiple cursor positions")
    {
        test_styler("\x1b[1A\x1b[1B", fly::logger::Cursor::Up, fly::logger::Cursor::Down);
        test_styler(
            "\x1b[1A\x1b[1B\x1b[1C",
            fly::logger::Cursor::Up,
            fly::logger::Cursor::Down,
            fly::logger::Cursor::Forward);
        test_styler(
            "\x1b[1A\x1b[1B\x1b[1C\x1b[1D",
            fly::logger::Cursor::Up,
            fly::logger::Cursor::Down,
            fly::logger::Cursor::Forward,
            fly::logger::Cursor::Backward);
        test_styler(
            "\x1b[1D\x1b[1C\x1b[1B\x1b[1A",
            fly::logger::Cursor::Backward,
            fly::logger::Cursor::Forward,
            fly::logger::Cursor::Down,
            fly::logger::Cursor::Up);
    }

    CATCH_SECTION("Manipulate with styles and colors")
    {
        test_styler("\x1b[1;31m", fly::logger::Style::Bold, fly::logger::Color::Red);
        test_styler("\x1b[1;31m", fly::logger::Color::Red, fly::logger::Style::Bold);

        test_styler(
            "\x1b[1;9;31m",
            fly::logger::Style::Bold,
            fly::logger::Style::Strike,
            fly::logger::Color::Red);
        test_styler(
            "\x1b[1;9;31m",
            fly::logger::Style::Bold,
            fly::logger::Color::Red,
            fly::logger::Style::Strike);
        test_styler(
            "\x1b[1;9;31m",
            fly::logger::Color::Red,
            fly::logger::Style::Bold,
            fly::logger::Style::Strike);

        test_styler(
            "\x1b[1;31;40m",
            fly::logger::Style::Bold,
            fly::logger::Color::Red,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::logger::Color::Red,
            fly::logger::Style::Bold,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::logger::Color::Red,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background),
            fly::logger::Style::Bold);
    }

    CATCH_SECTION("Manipulate with styles, colors, and cursor positions")
    {
        test_styler("\x1b[1;31m", fly::logger::Style::Bold, fly::logger::Color::Red);
        test_styler("\x1b[1;31m", fly::logger::Color::Red, fly::logger::Style::Bold);

        test_styler(
            "\x1b[1;9;31m",
            fly::logger::Style::Bold,
            fly::logger::Style::Strike,
            fly::logger::Color::Red);
        test_styler(
            "\x1b[1;9;31m",
            fly::logger::Style::Bold,
            fly::logger::Color::Red,
            fly::logger::Style::Strike);
        test_styler(
            "\x1b[1;9;31m",
            fly::logger::Color::Red,
            fly::logger::Style::Bold,
            fly::logger::Style::Strike);

        test_styler(
            "\x1b[1;31m\x1b[1A",
            fly::logger::Style::Bold,
            fly::logger::Color::Red,
            fly::logger::Cursor::Up);
        test_styler(
            "\x1b[1;31m\x1b[1A",
            fly::logger::Cursor::Up,
            fly::logger::Color::Red,
            fly::logger::Style::Bold);

        test_styler(
            "\x1b[1;31;40m",
            fly::logger::Style::Bold,
            fly::logger::Color::Red,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::logger::Color::Red,
            fly::logger::Style::Bold,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background));
        test_styler(
            "\x1b[1;31;40m",
            fly::logger::Color::Red,
            fly::logger::Color(fly::logger::Color::Black, fly::logger::Color::Background),
            fly::logger::Style::Bold);
    }
}

#endif
