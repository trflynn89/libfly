#include "test/util/capture_stream.hpp"

#include "fly/fly.hpp"
#include "fly/logger/logger.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_test_macros.hpp"

#include <memory>

CATCH_TEST_CASE("ConsoleLogger", "[logger]")
{
    auto logger = fly::Logger::create_console_logger("test", std::make_shared<fly::LoggerConfig>());

    CATCH_SECTION("Debug log points")
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stdout);
        logger->debug("Debug Log");

        const std::string contents = capture();
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Debug Log") != std::string::npos);
    }

    CATCH_SECTION("Informational log points")
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stdout);
        logger->info("Info Log");

        const std::string contents = capture();
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Info Log") != std::string::npos);
    }

    CATCH_SECTION("Warning log points")
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        logger->warn("Warning Log");

        const std::string contents = capture();
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Warning Log") != std::string::npos);
    }

    CATCH_SECTION("Error log points")
    {
        fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
        logger->error("Error Log");

        const std::string contents = capture();
        CATCH_REQUIRE_FALSE(contents.empty());

        CATCH_CHECK(contents.find("Error Log") != std::string::npos);
    }

#if defined(FLY_LINUX) || defined(FLY_MACOS)
    CATCH_SECTION("Validate style of console logs")
    {
        // Get the substring of a log point that should be styled.
        auto styled_contents = [](const std::string &contents, std::string &&log)
        {
            auto pos = contents.find(": " + log);
            CATCH_REQUIRE(pos != std::string::npos);

            return contents.substr(0, pos);
        };

        CATCH_SECTION("Validate style of debug console logs")
        {
            fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stdout);
            logger->debug("Debug Log");

            const std::string contents = capture();
            CATCH_REQUIRE_FALSE(contents.empty());

            const std::string styled = styled_contents(contents, "Debug Log");
            CATCH_CHECK(styled.starts_with("\x1b[0m"));
            CATCH_CHECK(styled.ends_with("\x1b[0m"));
        }

        CATCH_SECTION("Validate style of informational console logs")
        {
            fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stdout);
            logger->info("Info Log");

            const std::string contents = capture();
            CATCH_REQUIRE_FALSE(contents.empty());

            const std::string styled = styled_contents(contents, "Info Log");
            CATCH_CHECK(styled.starts_with("\x1b[0;32m"));
            CATCH_CHECK(styled.ends_with("\x1b[0m"));
        }

        CATCH_SECTION("Validate style of warning console logs")
        {
            fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
            logger->warn("Warning Log");

            const std::string contents = capture();
            CATCH_REQUIRE_FALSE(contents.empty());

            const std::string styled = styled_contents(contents, "Warning Log");
            CATCH_CHECK(styled.starts_with("\x1b[0;33m"));
            CATCH_CHECK(styled.ends_with("\x1b[0m"));
        }

        CATCH_SECTION("Validate style of error console logs")
        {
            fly::test::CaptureStream capture(fly::test::CaptureStream::Stream::Stderr);
            logger->error("Error Log");

            const std::string contents = capture();
            CATCH_REQUIRE_FALSE(contents.empty());

            const std::string styled = styled_contents(contents, "Error Log");
            CATCH_CHECK(styled.starts_with("\x1b[1;31m"));
            CATCH_CHECK(styled.ends_with("\x1b[0m"));
        }
    }
#endif
}
