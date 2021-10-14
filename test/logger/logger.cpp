#include "fly/logger/logger.hpp"

#include "test/util/task_manager.hpp"

#include "fly/logger/logger_config.hpp"
#include "fly/logger/sink.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"

#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"

#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace fly::literals::numeric_literals;

namespace {

/**
 * Test log sink to store received logs in a queue for verification.
 */
class QueueSink : public fly::logger::Sink
{
public:
    QueueSink(fly::ConcurrentQueue<fly::logger::Log> &logs) : m_logs(logs)
    {
    }

    bool initialize() override
    {
        return true;
    }

    bool stream(fly::logger::Log &&log) override
    {
        m_logs.push(std::move(log));
        return true;
    }

private:
    fly::ConcurrentQueue<fly::logger::Log> &m_logs;
};

/**
 * Test log sink to drop all received logs.
 */
class DropSink : public fly::logger::Sink
{
public:
    bool initialize() override
    {
        return true;
    }

    bool stream(fly::logger::Log &&) override
    {
        return true;
    }
};

/**
 * Test log sink to purposefully fail initialiation.
 */
class FailInitSink : public fly::logger::Sink
{
public:
    bool initialize() override
    {
        return false;
    }

    bool stream(fly::logger::Log &&) override
    {
        return true;
    }
};

/**
 * Test log sink to purposefully fail streaming.
 */
class FailStreamSink : public QueueSink
{
public:
    FailStreamSink(fly::ConcurrentQueue<fly::logger::Log> &logs) : QueueSink(logs)
    {
    }

    bool stream(fly::logger::Log &&log) override
    {
        return !QueueSink::stream(std::move(log));
    }
};

} // namespace

CATCH_TEST_CASE("Logger", "[logger]")
{
    auto logger_config = std::make_shared<fly::logger::LoggerConfig>();
    fly::ConcurrentQueue<fly::logger::Log> received_logs;

    auto validate_log_points = [&](fly::logger::Level expected_level,
                                   const char *expected_function,
                                   std::vector<std::string> &&expected_messages) {
        double last_time = 0.0;

        for (std::size_t i = 0; i < expected_messages.size(); ++i)
        {
            fly::logger::Log log;
            received_logs.pop(log);
            CATCH_CAPTURE(log.m_message);

            CATCH_CHECK(log.m_index == i);
            CATCH_CHECK(log.m_level == expected_level);
            CATCH_CHECK(log.m_time >= last_time);
            CATCH_CHECK(log.m_message.starts_with(expected_messages[i]));

            if (expected_function != nullptr)
            {
                CATCH_CHECK(std::string(log.m_trace.m_file) == std::string(__FILE__));
                CATCH_CHECK(std::string(log.m_trace.m_function) == std::string(expected_function));
                CATCH_CHECK(log.m_trace.m_line > 0_u32);
            }

            last_time = log.m_time;
        }

        CATCH_CHECK(received_logs.empty());
    };

    CATCH_SECTION("Cannot create logger with null sink")
    {
        auto logger = fly::logger::Logger::create("test", logger_config, nullptr);
        CATCH_CHECK_FALSE(logger);
    }

    CATCH_SECTION("Cannot create logger with sink that fails initialization")
    {
        auto logger =
            fly::logger::Logger::create("test", logger_config, std::make_unique<FailInitSink>());
        CATCH_CHECK_FALSE(logger);
    }

    CATCH_SECTION("Cannot create logger with duplicate name")
    {
        auto logger1 =
            fly::logger::Logger::create("test", logger_config, std::make_unique<DropSink>());
        CATCH_CHECK(logger1 != nullptr);

        auto logger2 =
            fly::logger::Logger::create("test", logger_config, std::make_unique<DropSink>());
        CATCH_CHECK_FALSE(logger2);
    }

    CATCH_SECTION("Cannot fetch logger that doesn't exist")
    {
        CATCH_CHECK_FALSE(fly::logger::Logger::get("test"));
    }

    CATCH_SECTION("Logger automatically deregisters itself on destruction")
    {
        {
            auto logger =
                fly::logger::Logger::create("test", logger_config, std::make_unique<DropSink>());
            CATCH_CHECK(fly::logger::Logger::get("test"));
        }

        CATCH_CHECK_FALSE(fly::logger::Logger::get("test"));
    }

    CATCH_SECTION("Resetting default logger causes initial default logger to be set as default")
    {
        fly::logger::Logger *default_logger = fly::logger::Logger::get_default_logger();

        auto logger =
            fly::logger::Logger::create("test", logger_config, std::make_unique<DropSink>());
        CATCH_REQUIRE(logger);

        fly::logger::Logger::set_default_logger(logger);
        CATCH_CHECK(fly::logger::Logger::get_default_logger() == logger.get());

        fly::logger::Logger::set_default_logger(nullptr);
        CATCH_CHECK(fly::logger::Logger::get_default_logger() == default_logger);
    }

    CATCH_SECTION("Log points")
    {
        // Run all of the log point tests with both synchronous and asynchronous loggers.
        const bool synchronous_logger = GENERATE(true, false);

        auto task_runner = fly::task::SequencedTaskRunner::create(fly::test::task_manager());
        auto sink = std::make_unique<QueueSink>(received_logs);

        auto logger = synchronous_logger ?
            fly::logger::Logger::create("test", logger_config, std::move(sink)) :
            fly::logger::Logger::create("test", task_runner, logger_config, std::move(sink));
        CATCH_REQUIRE(logger);

        fly::logger::Logger::set_default_logger(logger);
        CATCH_REQUIRE(fly::logger::Logger::get_default_logger() == logger.get());

        CATCH_SECTION("Logger that fails streaming stops accepting logs")
        {
            auto fsink = std::make_unique<FailStreamSink>(received_logs);

            logger = synchronous_logger ?
                fly::logger::Logger::create("fail", logger_config, std::move(fsink)) :
                fly::logger::Logger::create("fail", task_runner, logger_config, std::move(fsink));

            logger->debug("This log will be received");
            logger->debug("This log will be rejected");

            validate_log_points(fly::logger::Level::Debug, nullptr, {"This log will be received"});
        }

        CATCH_SECTION("Debug log points")
        {
            std::vector<std::string> expectations = {
                "Debug Log",
                "Debug Log: 123",
            };

            CATCH_SECTION("Without trace information")
            {
                logger->debug("Debug Log");
                logger->debug("Debug Log: {}", 123);

                validate_log_points(fly::logger::Level::Debug, nullptr, std::move(expectations));
            }

            CATCH_SECTION("With trace information")
            {
                logger->debug({__FILE__, __FUNCTION__, 123_u32}, "Debug Log");
                logger->debug({__FILE__, __FUNCTION__, 123_u32}, "Debug Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Debug,
                    __FUNCTION__,
                    std::move(expectations));
            }

            CATCH_SECTION("With macro invocation")
            {
                LOGD("Debug Log");
                LOGD("Debug Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Debug,
                    __FUNCTION__,
                    std::move(expectations));
            }
        }

        CATCH_SECTION("Informational log points")
        {
            std::vector<std::string> expectations = {
                "Info Log",
                "Info Log: 123",
            };

            CATCH_SECTION("Without trace information")
            {
                logger->info("Info Log");
                logger->info("Info Log: {}", 123);

                validate_log_points(fly::logger::Level::Info, nullptr, std::move(expectations));
            }

            CATCH_SECTION("With trace information")
            {
                logger->info({__FILE__, __FUNCTION__, 123_u32}, "Info Log");
                logger->info({__FILE__, __FUNCTION__, 123_u32}, "Info Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Info,
                    __FUNCTION__,
                    std::move(expectations));
            }

            CATCH_SECTION("With macro invocation")
            {
                LOGI("Info Log");
                LOGI("Info Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Info,
                    __FUNCTION__,
                    std::move(expectations));
            }
        }

        CATCH_SECTION("Warning log points")
        {
            std::vector<std::string> expectations = {
                "Warning Log",
                "Warning Log: 123",
            };

            CATCH_SECTION("Without trace information")
            {
                logger->warn("Warning Log");
                logger->warn("Warning Log: {}", 123);

                validate_log_points(fly::logger::Level::Warn, nullptr, std::move(expectations));
            }

            CATCH_SECTION("With trace information")
            {
                logger->warn({__FILE__, __FUNCTION__, 123_u32}, "Warning Log");
                logger->warn({__FILE__, __FUNCTION__, 123_u32}, "Warning Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Warn,
                    __FUNCTION__,
                    std::move(expectations));
            }

            CATCH_SECTION("With macro invocation")
            {
                LOGW("Warning Log");
                LOGW("Warning Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Warn,
                    __FUNCTION__,
                    std::move(expectations));
            }

            CATCH_SECTION("With system macro invocation")
            {
                LOGS("Warning Log");
                LOGS("Warning Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Warn,
                    __FUNCTION__,
                    std::move(expectations));
            }
        }

        CATCH_SECTION("Error log points")
        {
            std::vector<std::string> expectations = {
                "Error Log",
                "Error Log: 123",
            };

            CATCH_SECTION("Without trace information")
            {
                logger->error("Error Log");
                logger->error("Error Log: {}", 123);

                validate_log_points(fly::logger::Level::Error, nullptr, std::move(expectations));
            }

            CATCH_SECTION("With trace information")
            {
                logger->error({__FILE__, __FUNCTION__, 123_u32}, "Error Log");
                logger->error({__FILE__, __FUNCTION__, 123_u32}, "Error Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Error,
                    __FUNCTION__,
                    std::move(expectations));
            }

            CATCH_SECTION("With macro invocation")
            {
                LOGE("Error Log");
                LOGE("Error Log: {}", 123);

                validate_log_points(
                    fly::logger::Level::Error,
                    __FUNCTION__,
                    std::move(expectations));
            }
        }

        fly::logger::Logger::set_default_logger(nullptr);
    }

    // Keep this test last so the logger registry will go out-of-scope without the default logger
    // having been reset to the initial default logger.
    CATCH_SECTION("Not resetting default logger is safe")
    {
        auto logger =
            fly::logger::Logger::create("def", logger_config, std::make_unique<DropSink>());
        CATCH_REQUIRE(logger);

        fly::logger::Logger::set_default_logger(logger);
        CATCH_CHECK(fly::logger::Logger::get_default_logger() == logger.get());
    }
}
