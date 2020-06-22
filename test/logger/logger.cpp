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

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace {

/**
 * Subclass of the logger config to decrease the default log file size for faster testing.
 */
class TestLoggerConfig : public fly::LoggerConfig
{
public:
    TestLoggerConfig(bool compress_log_files) noexcept : fly::LoggerConfig()
    {
        m_default_compress_log_files = compress_log_files;
        m_default_max_log_file_size = 1 << 10;
    }
};

/**
 * Subclass of the coder config to contain invalid values.
 */
class BadCoderConfig : public fly::CoderConfig
{
public:
    BadCoderConfig() noexcept : fly::CoderConfig()
    {
        m_default_huffman_encoder_max_code_length =
            std::numeric_limits<decltype(m_default_huffman_encoder_max_code_length)>::max();
    }
};

} // namespace

//==================================================================================================
class LoggerTest : public ::testing::Test
{
public:
    LoggerTest() noexcept :
        m_path(fly::PathUtil::generate_temp_directory()),

        m_task_manager(std::make_shared<fly::TaskManager>(1)),
        m_task_runner(m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>()),

        m_logger_config(std::make_shared<TestLoggerConfig>(false)),
        m_coder_config(std::make_shared<fly::CoderConfig>()),

        m_logger(
            std::make_shared<fly::Logger>(m_task_runner, m_logger_config, m_coder_config, m_path))
    {
    }

    /**
     * Create the file directory and start the task manager and logger.
     */
    void SetUp() override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path));

        ASSERT_TRUE(m_task_manager->start());

        ASSERT_TRUE(m_logger->start());
        fly::Logger::set_instance(m_logger);
    }

    /**
     * Delete the created directory and stop the task manager.
     */
    void TearDown() override
    {
        ASSERT_TRUE(m_task_manager->stop());

        fly::Logger::set_instance(nullptr);
        m_logger.reset();

        std::filesystem::remove_all(m_path);
    }

protected:
    /**
     * Verify log points after calling one of the logging macros.
     *
     * @param expected_level The level (debug, info, etc.) of the logs.
     * @param expected_function Name of the function that generated the logs.
     * @param expected_messages List of expected formatted messages to verify.
     */
    void run_log_test(
        fly::Log::Level expected_level,
        const std::string &expected_function,
        std::vector<std::string> expected_messages)
    {
        for (std::size_t i = 0; i < expected_messages.size(); ++i)
        {
            m_task_runner->wait_for_task_to_complete<fly::LoggerTask>();
        }

        const std::string contents = fly::PathUtil::read_file(m_logger->get_log_file_path());
        ASSERT_FALSE(contents.empty());

        std::size_t count = 0;
        double last_time = 0.0;

        for (const std::string &log : fly::String::split(contents, '\x1e'))
        {
            const std::vector<std::string> sections = fly::String::split(log, '\x1f');
            ASSERT_EQ(sections.size(), 7_zu);

            const auto index = fly::String::convert<std::size_t>(sections[0]).value();
            const auto level = static_cast<fly::Log::Level>(
                fly::String::convert<std::uint8_t>(sections[1]).value());
            const auto time = fly::String::convert<double>(sections[2]).value();
            const auto file = sections[3];
            const auto function = sections[4];
            const auto line = fly::String::convert<std::uint32_t>(sections[5]).value();
            const auto message = sections[6];

            EXPECT_EQ(index, count);
            EXPECT_EQ(level, expected_level);
            EXPECT_GE(time, last_time);
            EXPECT_EQ(file, __FILE__);
            EXPECT_EQ(function, expected_function);
            EXPECT_GT(line, 0_u32);
            EXPECT_TRUE(fly::String::starts_with(message, expected_messages[count]));

            ++count;
            last_time = time;
        }

        EXPECT_EQ(count, expected_messages.size());
    }

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

    std::filesystem::path m_path;

    std::shared_ptr<fly::TaskManager> m_task_manager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_task_runner;

    std::shared_ptr<fly::LoggerConfig> m_logger_config;
    std::shared_ptr<fly::CoderConfig> m_coder_config;

    std::shared_ptr<fly::Logger> m_logger;
};

//==================================================================================================
TEST_F(LoggerTest, GoodFilePath)
{
    std::filesystem::path path = m_logger->get_log_file_path();
    EXPECT_TRUE(fly::String::starts_with(path.string(), m_path.string()));

    std::ifstream stream(path, std::ios::in);
    EXPECT_TRUE(stream.good());
}

//==================================================================================================
TEST_F(LoggerTest, BadFilePath)
{
    fly::Logger::set_instance(nullptr);

    m_logger = std::make_shared<fly::Logger>(
        m_task_runner,
        m_logger_config,
        m_coder_config,
        fly::PathUtil::generate_temp_directory());

    EXPECT_FALSE(m_logger->start());
}

//==================================================================================================
TEST_F(LoggerTest, ConsoleLog)
{
    fly::CaptureStream capture(fly::CaptureStream::Stream::Stdout);

    LOGC("Console Log");
    LOGC("Console Log: %d", 123);

    LOGC_NO_LOCK("Lockless console Log");
    LOGC_NO_LOCK("Lockless console Log: %d", 456);

    std::string contents = capture();
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find("Console Log"), std::string::npos);
    EXPECT_NE(contents.find("123"), std::string::npos);
    EXPECT_NE(contents.find("Lockless console Log"), std::string::npos);
    EXPECT_NE(contents.find("456"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 4);
}

//==================================================================================================
TEST_F(LoggerTest, DebugLog)
{
    LOGD("Debug Log");
    LOGD("Debug Log: %d", 123);

    std::vector<std::string> expectations = {
        "Debug Log",
        "Debug Log: 123",
    };

    run_log_test(fly::Log::Level::Debug, __FUNCTION__, std::move(expectations));
}

//==================================================================================================
TEST_F(LoggerTest, InfoLog)
{
    LOGI("Info Log");
    LOGI("Info Log: %d", 123);

    std::vector<std::string> expectations = {
        "Info Log",
        "Info Log: 123",
    };

    run_log_test(fly::Log::Level::Info, __FUNCTION__, std::move(expectations));
}

//==================================================================================================
TEST_F(LoggerTest, WarningLog)
{
    LOGW("Warning Log");
    LOGW("Warning Log: %d", 123);

    std::vector<std::string> expectations = {
        "Warning Log",
        "Warning Log: 123",
    };

    run_log_test(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
}

//==================================================================================================
TEST_F(LoggerTest, SystemLog)
{
    LOGS("System Log");
    LOGS("System Log: %d", 123);

    std::vector<std::string> expectations = {
        "System Log",
        "System Log: 123",
    };

    run_log_test(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
}

//==================================================================================================
TEST_F(LoggerTest, ErrorLog)
{
    LOGE("Error Log");
    LOGE("Error Log: %d", 123);

    std::vector<std::string> expectations = {
        "Error Log",
        "Error Log: 123",
    };

    run_log_test(fly::Log::Level::Error, __FUNCTION__, std::move(expectations));
}

//==================================================================================================
TEST_F(LoggerTest, RolloverUncompressed)
{
    std::filesystem::path path = m_logger->get_log_file_path();

    std::uintmax_t max_log_file_size = m_logger_config->max_log_file_size();
    std::uint32_t max_message_size = m_logger_config->max_message_size();

    std::string random = fly::String::generate_random_string(max_message_size);

    std::uintmax_t expected_size = log_size(random);
    std::uintmax_t count = 0;

    // Create enough log points to fill the log file, plus some extra to start filling a second log.
    while (++count < ((max_log_file_size / expected_size) + 10))
    {
        LOGD("%s", random);
        m_task_runner->wait_for_task_to_complete<fly::LoggerTask>();
    }

    EXPECT_NE(path, m_logger->get_log_file_path());
    ASSERT_TRUE(std::filesystem::exists(path));

    std::uintmax_t actual_size = std::filesystem::file_size(path);
    EXPECT_GE(actual_size, max_message_size);
}

//==================================================================================================
TEST_F(LoggerTest, RolloverCompressed)
{
    m_logger_config = std::make_shared<TestLoggerConfig>(true);

    m_logger =
        std::make_shared<fly::Logger>(m_task_runner, m_logger_config, m_coder_config, m_path);
    ASSERT_TRUE(m_logger->start());
    fly::Logger::set_instance(m_logger);

    std::filesystem::path path = m_logger->get_log_file_path();

    std::uintmax_t max_log_file_size = m_logger_config->max_log_file_size();
    std::uint32_t max_message_size = m_logger_config->max_message_size();

    std::string random = fly::String::generate_random_string(max_message_size);

    std::uintmax_t expected_size = log_size(random);
    std::uintmax_t count = 0;

    // Create enough log points to fill the log file, plus some extra to start filling a second log.
    while (++count < ((max_log_file_size / expected_size) + 10))
    {
        LOGD("%s", random);
        m_task_runner->wait_for_task_to_complete<fly::LoggerTask>();
    }

    EXPECT_NE(path, m_logger->get_log_file_path());

    std::filesystem::path compressed_path = path;
    compressed_path.replace_extension(".log.enc");

    ASSERT_FALSE(std::filesystem::exists(path));
    ASSERT_TRUE(std::filesystem::exists(compressed_path));

    fly::HuffmanDecoder decoder;
    ASSERT_TRUE(decoder.decode_file(compressed_path, path));

    std::uintmax_t actual_size = std::filesystem::file_size(path);
    EXPECT_GE(actual_size, max_message_size);
}

//==================================================================================================
TEST_F(LoggerTest, RolloverCompressedFailed)
{
    m_logger_config = std::make_shared<TestLoggerConfig>(true);
    m_coder_config = std::make_shared<BadCoderConfig>();

    m_logger =
        std::make_shared<fly::Logger>(m_task_runner, m_logger_config, m_coder_config, m_path);
    ASSERT_TRUE(m_logger->start());
    fly::Logger::set_instance(m_logger);

    std::filesystem::path path = m_logger->get_log_file_path();

    std::uintmax_t max_log_file_size = m_logger_config->max_log_file_size();
    std::uint32_t max_message_size = m_logger_config->max_message_size();

    std::string random = fly::String::generate_random_string(max_message_size);

    std::uintmax_t expected_size = log_size(random);
    std::uintmax_t count = 0;

    // Create enough log points to fill the log file, plus some extra to start filling a second log.
    while (++count < ((max_log_file_size / expected_size) + 10))
    {
        LOGD("%s", random);
        m_task_runner->wait_for_task_to_complete<fly::LoggerTask>();
    }

    EXPECT_NE(path, m_logger->get_log_file_path());
    ASSERT_TRUE(std::filesystem::exists(path));

    std::uintmax_t actual_size = std::filesystem::file_size(path);
    EXPECT_GE(actual_size, max_message_size);
}
