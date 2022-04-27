#include "test/util/capture_stream.hpp"

#include "fly/assert/detail/assert_enabled.hpp"
#include "fly/types/string/format.hpp"

#include "catch2/catch_test_macros.hpp"

#include <initializer_list>
#include <optional>
#include <string>

namespace {

struct Trace
{
    std::string_view m_file;
    std::string_view m_function;
    std::uint32_t m_line {0};
};

template <typename... CaptureTypes>
void validate_assertion(
    Trace trace,
    std::string_view output,
    std::string_view expression,
    std::optional<std::string_view> message = {},
    std::initializer_list<std::string_view> capture_names = {},
    CaptureTypes &&...captures)
{
    CATCH_CHECK(output.find("Assertion failed:") != std::string::npos);

    if (message.has_value())
    {
        CATCH_CHECK(output.find(message.value()) != std::string::npos);
    }

    auto formatted_expression = fly::string::format("FLY_ASSERT({})", expression);
    CATCH_CHECK(output.find(formatted_expression) != std::string::npos);

    auto formatted_location = fly::string::format("at {}:{}", trace.m_file, trace.m_line);
    CATCH_CHECK(output.find(formatted_location) != std::string::npos);

    auto formatted_function = fly::string::format("in {}", trace.m_function);
    CATCH_CHECK(output.find(formatted_function) != std::string::npos);

    if constexpr (sizeof...(CaptureTypes) > 0)
    {
        CATCH_CHECK(output.find("Captures:") != std::string::npos);

        auto names = std::data(capture_names);
        std::size_t index = 0;

        auto validate_capture = [&output, &index, &names](auto const &capture) {
            auto formatted_capture = fly::string::format("{} => {}", names[index++], capture);
            CATCH_CHECK(output.find(formatted_capture) != std::string::npos);
        };

        (validate_capture(captures), ...);
    }
    else
    {
        CATCH_CHECK(output.find("Captures:") == std::string::npos);
    }

    CATCH_CHECK(output.find("Call stack:") != std::string::npos);
}

#define TEST_ASSERT(expression, ...)                                                               \
    do                                                                                             \
    {                                                                                              \
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);                \
        FLY_ASSERT_IMPL(void, expression __VA_OPT__(, ) __VA_ARGS__);                              \
                                                                                                   \
        trace = {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)};                    \
        output = capture();                                                                        \
    } while (0)

} // namespace

CATCH_TEST_CASE("Assert", "[assert]")
{
    Trace trace;
    std::string output;

    CATCH_SECTION("Successful assertion does not log anything")
    {
        int foo = 123;
        char bar = 'x';

        TEST_ASSERT(true);
        CATCH_CHECK(output.empty());

        TEST_ASSERT(foo == 123, "Message");
        CATCH_CHECK(output.empty());

        TEST_ASSERT(bar == 'x', foo, bar);
        CATCH_CHECK(output.empty());

        TEST_ASSERT(!false, "Message", foo, bar);
        CATCH_CHECK(output.empty());
    }

    CATCH_SECTION("Failed assertion logs to stderr (without message or captures)")
    {
        TEST_ASSERT(false);
        validate_assertion(trace, output, "false");

        int foo = 123;
        TEST_ASSERT(foo > 124);
        validate_assertion(trace, output, "foo > 124");
    }

    CATCH_SECTION("Failed assertion logs to stderr (with message)")
    {
        int foo = 123;
        TEST_ASSERT(foo > 124, "Message to be logged");
        validate_assertion(trace, output, "foo > 124", "Message to be logged");
    }

    CATCH_SECTION("Failed assertion logs to stderr (with captures)")
    {
        int foo = 123;
        TEST_ASSERT(foo > 124, foo);
        validate_assertion(trace, output, "foo > 124", {}, {"foo"}, foo);
    }

    CATCH_SECTION("Failed assertion logs to stderr (with message and captures)")
    {
        int foo = 123;
        TEST_ASSERT(foo > 124, "Message to be logged", foo);
        validate_assertion(trace, output, "foo > 124", "Message to be logged", {"foo"}, foo);
    }
}
