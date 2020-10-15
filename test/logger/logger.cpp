#include "fly/logger/logger.hpp"

#include "fly/logger/log_sink.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "test/util/task_manager.hpp"

#include <catch2/catch.hpp>

#include <limits>
#include <memory>
#include <string>
#include <vector>

using namespace fly::literals::numeric_literals;

namespace {

/**
 * Test log sink to store received logs in a queue for verification.
 */
class QueueSink : public fly::LogSink
{
public:
    QueueSink(fly::ConcurrentQueue<fly::Log> &logs) : m_logs(logs)
    {
    }

    bool initialize() override
    {
        return true;
    }

    bool stream(fly::Log &&log) override
    {
        m_logs.push(std::move(log));
        return true;
    }

private:
    fly::ConcurrentQueue<fly::Log> &m_logs;
};

/**
 * Test log sink to drop all received logs.
 */
class DropSink : public fly::LogSink
{
public:
    bool initialize() override
    {
        return true;
    }

    bool stream(fly::Log &&) override
    {
        return true;
    }
};

/**
 * Test log sink to purposefully fail initialiation.
 */
class FailInitSink : public fly::LogSink
{
public:
    bool initialize() override
    {
        return false;
    }

    bool stream(fly::Log &&) override
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
    FailStreamSink(fly::ConcurrentQueue<fly::Log> &logs) : QueueSink(logs)
    {
    }

    bool stream(fly::Log &&log) override
    {
        return !QueueSink::stream(std::move(log));
    }
};

} // namespace

TEST_CASE("Logger", "[logger]")
{
    auto logger_config = std::make_shared<fly::LoggerConfig>();
    fly::ConcurrentQueue<fly::Log> received_logs;

    auto validate_log_points = [&](fly::Log::Level expected_level,
                                   const char *expected_function,
                                   std::vector<std::string> &&expected_messages) {
        double last_time = 0.0;

        for (std::size_t i = 0; i < expected_messages.size(); ++i)
        {
            fly::Log log;
            received_logs.pop(log);
            CAPTURE(log.m_message);

            CHECK(log.m_index == i);
            CHECK(log.m_level == expected_level);
            CHECK(log.m_time >= last_time);
            CHECK(fly::String::starts_with(log.m_message, expected_messages[i]));

            if (expected_function != nullptr)
            {
                CHECK(std::string(log.m_trace.m_file) == std::string(__FILE__));
                CHECK(std::string(log.m_trace.m_function) == std::string(expected_function));
                CHECK(log.m_trace.m_line > 0_u32);
            }

            last_time = log.m_time;
        }

        CHECK(received_logs.empty());
    };

    SECTION("Cannot create logger with null sink")
    {
        auto logger = fly::Logger::create_logger("test", logger_config, nullptr);
        CHECK_FALSE(logger);
    }

    SECTION("Cannot create logger with sink that fails initialization")
    {
        auto logger =
            fly::Logger::create_logger("test", logger_config, std::make_unique<FailInitSink>());
        CHECK_FALSE(logger);
    }

    SECTION("Cannot create logger with duplicate name")
    {
        auto logger1 =
            fly::Logger::create_logger("test", logger_config, std::make_unique<DropSink>());
        CHECK(logger1 != nullptr);

        auto logger2 =
            fly::Logger::create_logger("test", logger_config, std::make_unique<DropSink>());
        CHECK_FALSE(logger2);
    }

    SECTION("Cannot fetch logger that doesn't exist")
    {
        CHECK_FALSE(fly::Logger::get("test"));
    }

    SECTION("Logger automatically deregisters itself on destruction")
    {
        {
            auto logger =
                fly::Logger::create_logger("test", logger_config, std::make_unique<DropSink>());
            CHECK(fly::Logger::get("test"));
        }

        CHECK_FALSE(fly::Logger::get("test"));
    }

    SECTION("Resetting default logger causes initial default logger to be set as default")
    {
        fly::Logger *default_logger = fly::Logger::get_default_logger();

        auto logger =
            fly::Logger::create_logger("test", logger_config, std::make_unique<DropSink>());
        REQUIRE(logger);

        fly::Logger::set_default_logger(logger);
        CHECK(fly::Logger::get_default_logger() == logger.get());

        fly::Logger::set_default_logger(nullptr);
        CHECK(fly::Logger::get_default_logger() == default_logger);
    }

    SECTION("Log points")
    {
        // Run all of the log point tests with both synchronous and asynchronous loggers.
        const bool synchronous_logger = GENERATE(true, false);

        auto task_manager = std::make_shared<fly::TaskManager>(1);
        REQUIRE(task_manager->start());

        auto task_runner =
            fly::test::task_manager()->create_task_runner<fly::SequencedTaskRunner>();
        auto sink = std::make_unique<QueueSink>(received_logs);

        auto logger = synchronous_logger ?
            fly::Logger::create_logger("test", logger_config, std::move(sink)) :
            fly::Logger::create_logger("test", task_runner, logger_config, std::move(sink));
        REQUIRE(logger);

        fly::Logger::set_default_logger(logger);
        REQUIRE(fly::Logger::get_default_logger() == logger.get());

        SECTION("Logger that fails streaming stops accepting logs")
        {
            auto fsink = std::make_unique<FailStreamSink>(received_logs);

            logger = synchronous_logger ?
                fly::Logger::create_logger("fail", logger_config, std::move(fsink)) :
                fly::Logger::create_logger("fail", task_runner, logger_config, std::move(fsink));

            logger->debug("This log will be received");
            logger->debug("This log will be rejected");

            validate_log_points(fly::Log::Level::Debug, nullptr, {"This log will be received"});
        }

        SECTION("Debug log points")
        {
            std::vector<std::string> expectations = {
                "Debug Log",
                "Debug Log: 123",
            };

            SECTION("Without trace information")
            {
                logger->debug("Debug Log");
                logger->debug("Debug Log: %d", 123);

                validate_log_points(fly::Log::Level::Debug, nullptr, std::move(expectations));
            }

            SECTION("With trace information")
            {
                logger->debug({__FILE__, __FUNCTION__, 123_u32}, "Debug Log");
                logger->debug({__FILE__, __FUNCTION__, 123_u32}, "Debug Log: %d", 123);

                validate_log_points(fly::Log::Level::Debug, __FUNCTION__, std::move(expectations));
            }

            SECTION("With macro invocation")
            {
                LOGD("Debug Log");
                LOGD("Debug Log: %d", 123);

                validate_log_points(fly::Log::Level::Debug, __FUNCTION__, std::move(expectations));
            }
        }

        SECTION("Informational log points")
        {
            std::vector<std::string> expectations = {
                "Info Log",
                "Info Log: 123",
            };

            SECTION("Without trace information")
            {
                logger->info("Info Log");
                logger->info("Info Log: %d", 123);

                validate_log_points(fly::Log::Level::Info, nullptr, std::move(expectations));
            }

            SECTION("With trace information")
            {
                logger->info({__FILE__, __FUNCTION__, 123_u32}, "Info Log");
                logger->info({__FILE__, __FUNCTION__, 123_u32}, "Info Log: %d", 123);

                validate_log_points(fly::Log::Level::Info, __FUNCTION__, std::move(expectations));
            }

            SECTION("With macro invocation")
            {
                LOGI("Info Log");
                LOGI("Info Log: %d", 123);

                validate_log_points(fly::Log::Level::Info, __FUNCTION__, std::move(expectations));
            }
        }

        SECTION("Warning log points")
        {
            std::vector<std::string> expectations = {
                "Warning Log",
                "Warning Log: 123",
            };

            SECTION("Without trace information")
            {
                logger->warn("Warning Log");
                logger->warn("Warning Log: %d", 123);

                validate_log_points(fly::Log::Level::Warn, nullptr, std::move(expectations));
            }

            SECTION("With trace information")
            {
                logger->warn({__FILE__, __FUNCTION__, 123_u32}, "Warning Log");
                logger->warn({__FILE__, __FUNCTION__, 123_u32}, "Warning Log: %d", 123);

                validate_log_points(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
            }

            SECTION("With macro invocation")
            {
                LOGW("Warning Log");
                LOGW("Warning Log: %d", 123);

                validate_log_points(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
            }

            SECTION("With system macro invocation")
            {
                LOGS("Warning Log");
                LOGS("Warning Log: %d", 123);

                validate_log_points(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
            }
        }

        SECTION("Error log points")
        {
            std::vector<std::string> expectations = {
                "Error Log",
                "Error Log: 123",
            };

            SECTION("Without trace information")
            {
                logger->error("Error Log");
                logger->error("Error Log: %d", 123);

                validate_log_points(fly::Log::Level::Error, nullptr, std::move(expectations));
            }

            SECTION("With trace information")
            {
                logger->error({__FILE__, __FUNCTION__, 123_u32}, "Error Log");
                logger->error({__FILE__, __FUNCTION__, 123_u32}, "Error Log: %d", 123);

                validate_log_points(fly::Log::Level::Error, __FUNCTION__, std::move(expectations));
            }

            SECTION("With macro invocation")
            {
                LOGE("Error Log");
                LOGE("Error Log: %d", 123);

                validate_log_points(fly::Log::Level::Error, __FUNCTION__, std::move(expectations));
            }
        }

        fly::Logger::set_default_logger(nullptr);
    }

    // Keep this test last so the logger registry will go out-of-scope without the default logger
    // having been reset to the initial default logger.
    SECTION("Not resetting default logger is safe")
    {
        auto logger =
            fly::Logger::create_logger("def", logger_config, std::make_unique<DropSink>());
        REQUIRE(logger);

        fly::Logger::set_default_logger(logger);
        CHECK(fly::Logger::get_default_logger() == logger.get());
    }
}
