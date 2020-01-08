#include "fly/logger/logger.h"

#include "fly/logger/logger_config.h"
#include "fly/task/task_manager.h"
#include "fly/types/numeric/literals.h"
#include "fly/types/string/string.h"
#include "test/util/capture_stream.h"
#include "test/util/path_util.h"
#include "test/util/waitable_task_runner.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

namespace {

/**
 * Subclass of the logger config to decrease the default log file size for
 * faster testing.
 */
class TestLoggerConfig : public fly::LoggerConfig
{
public:
    TestLoggerConfig() noexcept : fly::LoggerConfig()
    {
        m_defaultMaxLogFileSize = 1 << 10;
    }
};

} // namespace

//==============================================================================
class LoggerTest : public ::testing::Test
{
public:
    LoggerTest() noexcept :
        m_path(fly::PathUtil::GenerateTempDirectory()),

        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager
                ->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()),

        m_spLoggerConfig(std::make_shared<TestLoggerConfig>()),

        m_spLogger(std::make_shared<fly::Logger>(
            m_spTaskRunner,
            m_spLoggerConfig,
            m_path))
    {
    }

    /**
     * Create the file directory and start the task manager and logger.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path));

        ASSERT_TRUE(m_spTaskManager->Start());

        ASSERT_TRUE(m_spLogger->Start());
        fly::Logger::SetInstance(m_spLogger);
    }

    /**
     * Delete the created directory and stop the task manager.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_spTaskManager->Stop());

        fly::Logger::SetInstance(nullptr);
        m_spLogger.reset();

        std::filesystem::remove_all(m_path);
    }

protected:
    /**
     * Verify log points after calling one of the logging macros.
     *
     * @param Level The level (debug, info, etc.) of the logs.
     * @param string Name of the function that generated the logs.
     * @param vector List of expected formatted messages to verify.
     */
    void RunLogTest(
        fly::Log::Level expectedLevel,
        const std::string &expectedFunction,
        std::vector<std::string> expectedMessages)
    {
        for (std::size_t i = 0; i < expectedMessages.size(); ++i)
        {
            m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
        }

        const std::string contents =
            fly::PathUtil::ReadFile(m_spLogger->GetLogFilePath());
        ASSERT_FALSE(contents.empty());

        std::size_t count = 0;
        double lastTime = 0.0;

        for (const std::string &log : fly::String::Split(contents, '\n'))
        {
            const std::vector<std::string> sections =
                fly::String::Split(log, '\t');
            ASSERT_EQ(sections.size(), 7_zu);

            const auto index =
                fly::String::Convert<std::size_t>(sections[0]);
            const auto level = static_cast<fly::Log::Level>(
                fly::String::Convert<std::uint8_t>(sections[1]));
            const auto time = fly::String::Convert<double>(sections[2]);
            const auto file = sections[3];
            const auto function = sections[4];
            const auto line = fly::String::Convert<std::uint32_t>(sections[5]);
            const auto message = sections[6];

            EXPECT_EQ(index, count);
            EXPECT_EQ(level, expectedLevel);
            EXPECT_GE(time, lastTime);
            EXPECT_EQ(file, __FILE__);
            EXPECT_EQ(function, expectedFunction);
            EXPECT_GT(line, 0_u32);
            EXPECT_TRUE(
                fly::String::StartsWith(message, expectedMessages[count]));

            ++count;
            lastTime = time;
        }

        EXPECT_EQ(count, expectedMessages.size());
    }

    /**
     * Measure the size, in bytes, of a log point.
     *
     * @param string Message to store in the log.
     *
     * @return uintmax_t Size of the log point.
     */
    std::uintmax_t LogSize(const std::string &message) noexcept
    {
        fly::Log log;

        log.m_message = message;
        log.m_level = fly::Log::Level::Debug;
        log.m_line = __LINE__;

        ::snprintf(log.m_file, sizeof(log.m_file), "%s", __FILE__);
        ::snprintf(log.m_function, sizeof(log.m_function), "%s", __FUNCTION__);

        return fly::String::Format("%d\t%s", 1, log).length();
    }

    std::filesystem::path m_path;

    std::shared_ptr<fly::TaskManager> m_spTaskManager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_spTaskRunner;

    std::shared_ptr<fly::LoggerConfig> m_spLoggerConfig;
    std::shared_ptr<fly::Logger> m_spLogger;
};

//==============================================================================
TEST_F(LoggerTest, FilePathTest)
{
    std::filesystem::path path = m_spLogger->GetLogFilePath();
    EXPECT_TRUE(fly::String::StartsWith(path.string(), m_path.string()));

    std::ifstream stream(path, std::ios::in);
    EXPECT_TRUE(stream.good());
}

//==============================================================================
TEST_F(LoggerTest, BadFilePathTest)
{
    fly::Logger::SetInstance(nullptr);

    m_spLogger = std::make_shared<fly::Logger>(
        m_spTaskRunner,
        m_spLoggerConfig,
        fly::PathUtil::GenerateTempDirectory());

    EXPECT_FALSE(m_spLogger->Start());
}

//==============================================================================
TEST_F(LoggerTest, ConsoleTest)
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

//==============================================================================
TEST_F(LoggerTest, DebugTest)
{
    LOGD("Debug Log");
    LOGD("Debug Log: %d", 123);

    std::vector<std::string> expectations = {
        "Debug Log",
        "Debug Log: 123",
    };

    RunLogTest(fly::Log::Level::Debug, __FUNCTION__, std::move(expectations));
}

//==============================================================================
TEST_F(LoggerTest, InfoTest)
{
    LOGI("Info Log");
    LOGI("Info Log: %d", 123);

    std::vector<std::string> expectations = {
        "Info Log",
        "Info Log: 123",
    };

    RunLogTest(fly::Log::Level::Info, __FUNCTION__, std::move(expectations));
}

//==============================================================================
TEST_F(LoggerTest, WarningTest)
{
    LOGW("Warning Log");
    LOGW("Warning Log: %d", 123);

    std::vector<std::string> expectations = {
        "Warning Log",
        "Warning Log: 123",
    };

    RunLogTest(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
}

//==============================================================================
TEST_F(LoggerTest, SystemTest)
{
    LOGS("System Log");
    LOGS("System Log: %d", 123);

    std::vector<std::string> expectations = {
        "System Log",
        "System Log: 123",
    };

    RunLogTest(fly::Log::Level::Warn, __FUNCTION__, std::move(expectations));
}

//==============================================================================
TEST_F(LoggerTest, ErrorTest)
{
    LOGE("Error Log");
    LOGE("Error Log: %d", 123);

    std::vector<std::string> expectations = {
        "Error Log",
        "Error Log: 123",
    };

    RunLogTest(fly::Log::Level::Error, __FUNCTION__, std::move(expectations));
}

//==============================================================================
TEST_F(LoggerTest, RolloverTest)
{
    std::filesystem::path path = m_spLogger->GetLogFilePath();

    std::uintmax_t maxLogFileSize = m_spLoggerConfig->MaxLogFileSize();
    std::uint32_t maxMessageSize = m_spLoggerConfig->MaxMessageSize();

    std::string random = fly::String::GenerateRandomString(maxMessageSize);

    std::uintmax_t expectedSize = LogSize(random);
    std::uintmax_t count = 0;

    // Create enough log points to fill the log file, plus some extra to start
    // filling a second log file
    while (++count < ((maxLogFileSize / expectedSize) + 10))
    {
        LOGD("%s", random);
        m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    }

    EXPECT_NE(path, m_spLogger->GetLogFilePath());

    std::uintmax_t actualSize = std::filesystem::file_size(path);
    EXPECT_GE(actualSize, maxMessageSize);
}
