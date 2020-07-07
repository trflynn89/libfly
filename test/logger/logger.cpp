#include "fly/logger/logger.hpp"

#include "fly/coders/coder_config.hpp"
#include "fly/coders/huffman/huffman_decoder.hpp"
#include "fly/logger/logger_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/numeric/literals.hpp"
#include "fly/types/string/string.hpp"
#include "test/util/capture_stream.hpp"
#include "test/util/path_util.hpp"
#include "test/util/waitable_task_runner.hpp"

#include <catch2/catch.hpp>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace {

/**
 * Subclass of the logger config to decrease the default log file size for faster testing.
 */
class MutableLoggerConfig : public fly::LoggerConfig
{
public:
    MutableLoggerConfig() noexcept : fly::LoggerConfig()
    {
        m_default_max_log_file_size = 1 << 10;
    }

    void disable_compression()
    {
        m_default_compress_log_files = false;
    }
};

/**
 * Subclass of the coder config to contain invalid values.
 */
class MutableCoderConfig : public fly::CoderConfig
{
public:
    void invalidate_max_code_length()
    {
        m_default_huffman_encoder_max_code_length = std::numeric_limits<fly::code_type>::digits;
    }
};

/**
 * Measure the size, in bytes, of a log point.
 *
 * @param string Message to store in the log.
 *
 * @return uintmax_t Size of the log point.
 */
std::uintmax_t log_size(const std::string &message)
{
    fly::Log log;

    log.m_message = message;
    log.m_level = fly::Log::Level::Debug;
    log.m_line = __LINE__;

    ::snprintf(log.m_file, sizeof(log.m_file), "%s", __FILE__);
    ::snprintf(log.m_function, sizeof(log.m_function), "%s", __FUNCTION__);

    return fly::String::format("%d\t%s", 1, log).length();
}

} // namespace

TEST_CASE("Logger", "[logger]")
{
    auto task_manager = std::make_shared<fly::TaskManager>(1);
    REQUIRE(task_manager->start());

    auto task_runner = task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    auto logger_config = std::make_shared<MutableLoggerConfig>();
    auto coder_config = std::make_shared<MutableCoderConfig>();

    fly::PathUtil::ScopedTempDirectory path;

    auto logger = std::make_shared<fly::Logger>(task_runner, logger_config, coder_config, path());
    fly::Logger::set_instance(logger);

    REQUIRE(logger->start());

    // Verify log points after calling one of the logging macros.
    auto validate_log_points = [&](fly::Log::Level expected_level,
                                   std::string &&expected_function,
                                   std::vector<std::string> &&expected_messages) {
        for (std::size_t i = 0; i < expected_messages.size(); ++i)
        {
            task_runner->wait_for_task_to_complete<fly::LoggerTask>();
        }

        const std::string contents = fly::PathUtil::read_file(logger->get_log_file_path());
        REQUIRE_FALSE(contents.empty());

        std::size_t count = 0;
        double last_time = 0.0;

        for (const std::string &log : fly::String::split(contents, '\x1e'))
        {
            const std::vector<std::string> sections = fly::String::split(log, '\x1f');
            REQUIRE(sections.size() == 7_zu);

            const auto index = fly::String::convert<std::size_t>(sections[0]).value();
            const auto level = static_cast<fly::Log::Level>(
                fly::String::convert<std::uint8_t>(sections[1]).value());
            const auto time = fly::String::convert<double>(sections[2]).value();
            const auto file = sections[3];
            const auto function = sections[4];
            const auto line = fly::String::convert<std::uint32_t>(sections[5]).value();
            const auto message = sections[6];

            CHECK(index == count);
            CHECK(level == expected_level);
            CHECK(time >= last_time);
            CHECK(file == __FILE__);
            CHECK(function == expected_function);
            CHECK(line > 0_u32);
            CHECK(fly::String::starts_with(message, expected_messages[count]));

            ++count;
            last_time = time;
        }

        CHECK(count == expected_messages.size());
    };

    SECTION("Valid logger file paths should be created after starting logger")
    {
        std::filesystem::path log_file = logger->get_log_file_path();
        CHECK(fly::String::starts_with(log_file.string(), path().string()));

        REQUIRE(std::filesystem::exists(log_file));
    }

    SECTION("Cannot start logger with a bad file path")
    {
        logger = std::make_shared<fly::Logger>(task_runner, logger_config, coder_config, __FILE__);
        CHECK_FALSE(logger->start());
    }

    SECTION("Validate macros which log to stdout")
    {
        fly::CaptureStream capture(fly::CaptureStream::Stream::Stdout);

        LOGC("Console Log");
        LOGC("Console Log: %d", 123);

        LOGC_NO_LOCK("Lockless console Log");
        LOGC_NO_LOCK("Lockless console Log: %d", 456);

        const std::string contents = capture();
        REQUIRE_FALSE(contents.empty());

        CHECK(contents.find("Console Log") != std::string::npos);
        CHECK(contents.find("123") != std::string::npos);
        CHECK(contents.find("Lockless console Log") != std::string::npos);
        CHECK(contents.find("456") != std::string::npos);
        CHECK(std::count(contents.begin(), contents.end(), '\n') == 4);
    }

    SECTION("Validate debug logging macro")
    {
        LOGD("Debug Log");
        LOGD("Debug Log: %d", 123);

        std::vector<std::string> expectations = {
            "Debug Log",
            "Debug Log: 123",
        };

        validate_log_points(fly::Log::Level::Debug, __FUNCTION__, std::move(expectations));
    }

    SECTION("Validate informational logging macro")
    {
        LOGI("Info Log");
        LOGI("Info Log: %d", 123);

        std::vector<std::string> expectations = {
            "Info Log",
            "Info Log: 123",
        };

        validate_log_points(fly::Log::Level::Info, __FUNCTION__, std::move(expectations));
    }

    SECTION("Validate warning logging macro")
    {
        LOGW("Warning Log");
        LOGW("Warning Log: %d", 123);

        std::vector<std::string> expectations = {
            "Warning Log",
            "Warning Log: 123",
        };

        validate_log_points(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
    }

    SECTION("Validate system logging macro")
    {
        LOGS("System Log");
        LOGS("System Log: %d", 123);

        std::vector<std::string> expectations = {
            "System Log",
            "System Log: 123",
        };

        validate_log_points(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
    }

    SECTION("Validate error logging macro")
    {
        LOGE("Error Log");
        LOGE("Error Log: %d", 123);

        std::vector<std::string> expectations = {
            "Error Log",
            "Error Log: 123",
        };

        validate_log_points(fly::Log::Level::Error, __FUNCTION__, std::move(expectations));
    }

    SECTION("Logger should compress log files by default")
    {
        std::filesystem::path log_file = logger->get_log_file_path();

        std::uintmax_t max_log_file_size = logger_config->max_log_file_size();
        std::uint32_t max_message_size = logger_config->max_message_size();

        std::string random = fly::String::generate_random_string(max_message_size);

        std::uintmax_t expected_size = log_size(random);
        std::uintmax_t count = 0;

        // Create enough log points to fill the log file, plus some extra to start a second log.
        while (++count < ((max_log_file_size / expected_size) + 10))
        {
            LOGD("%s", random);
            task_runner->wait_for_task_to_complete<fly::LoggerTask>();
        }

        CHECK(log_file != logger->get_log_file_path());

        std::filesystem::path compressed_path = log_file;
        compressed_path.replace_extension(".log.enc");

        REQUIRE_FALSE(std::filesystem::exists(log_file));
        REQUIRE(std::filesystem::exists(compressed_path));

        fly::HuffmanDecoder decoder;
        REQUIRE(decoder.decode_file(compressed_path, log_file));

        std::uintmax_t actual_size = std::filesystem::file_size(log_file);
        CHECK(actual_size >= max_message_size);
    }

    SECTION("When compression is disabled, logger should produce uncompressed logs")
    {
        logger_config->disable_compression();

        std::filesystem::path log_file = logger->get_log_file_path();

        std::uintmax_t max_log_file_size = logger_config->max_log_file_size();
        std::uint32_t max_message_size = logger_config->max_message_size();

        std::string random = fly::String::generate_random_string(max_message_size);

        std::uintmax_t expected_size = log_size(random);
        std::uintmax_t count = 0;

        // Create enough log points to fill the log file, plus some extra to start a second log.
        while (++count < ((max_log_file_size / expected_size) + 10))
        {
            LOGD("%s", random);
            task_runner->wait_for_task_to_complete<fly::LoggerTask>();
        }

        CHECK(log_file != logger->get_log_file_path());
        REQUIRE(std::filesystem::exists(log_file));

        fly::HuffmanDecoder decoder;
        CHECK_FALSE(decoder.decode_file(log_file, path.file()));

        std::uintmax_t actual_size = std::filesystem::file_size(log_file);
        CHECK(actual_size >= max_message_size);
    }

    SECTION("When compression fails, logger should produce uncompressed logs")
    {
        coder_config->invalidate_max_code_length();

        std::filesystem::path log_file = logger->get_log_file_path();

        std::uintmax_t max_log_file_size = logger_config->max_log_file_size();
        std::uint32_t max_message_size = logger_config->max_message_size();

        std::string random = fly::String::generate_random_string(max_message_size);

        std::uintmax_t expected_size = log_size(random);
        std::uintmax_t count = 0;

        // Create enough log points to fill the log file, plus some extra to start a second log.
        while (++count < ((max_log_file_size / expected_size) + 10))
        {
            LOGD("%s", random);
            task_runner->wait_for_task_to_complete<fly::LoggerTask>();
        }

        CHECK(log_file != logger->get_log_file_path());
        REQUIRE(std::filesystem::exists(log_file));

        fly::HuffmanDecoder decoder;
        CHECK_FALSE(decoder.decode_file(log_file, path.file()));

        std::uintmax_t actual_size = std::filesystem::file_size(log_file);
        CHECK(actual_size >= max_message_size);
    }

    REQUIRE(task_manager->stop());
    fly::Logger::set_instance(nullptr);
}
