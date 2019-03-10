#include "fly/logger/logger.h"

#include "fly/logger/logger_config.h"
#include "fly/path/path.h"
#include "fly/task/task_manager.h"
#include "fly/types/string.h"
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

namespace {

/**
 * Subclass of the logger config to decrease the default log file size for
 * faster testing.
 */
class TestLoggerConfig : public fly::LoggerConfig
{
public:
    TestLoggerConfig() : fly::LoggerConfig()
    {
        m_defaultMaxLogFileSize = 1 << 10;
    }
};

} // namespace

//==============================================================================
class LoggerTest : public ::testing::Test
{
public:
    LoggerTest() :
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
    void SetUp() override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path));

        ASSERT_TRUE(m_spTaskManager->Start());

        ASSERT_TRUE(m_spLogger->Start());
        fly::Logger::SetInstance(m_spLogger);
    }

    /**
     * Delete the created directory and stop the task manager.
     */
    void TearDown() override
    {
        ASSERT_TRUE(m_spTaskManager->Stop());

        fly::Logger::SetInstance(nullptr);
        m_spLogger.reset();

        std::filesystem::remove_all(m_path);
    }

protected:
    /**
     * Measure the size, in bytes, of a log point.
     *
     * @param string Message to store in the log.
     *
     * @return size_t Size of the log point.
     */
    size_t LogSize(const std::string &message)
    {
        fly::Log log;

        log.m_message = message;
        log.m_level = fly::Log::Level::Debug;
        log.m_line = __LINE__;

        ::snprintf(log.m_file, sizeof(log.m_file), "%s", __FILE__);
        ::snprintf(log.m_function, sizeof(log.m_function), "%s", __FUNCTION__);

        return fly::String::Format("%d\t%s", 1, log).length();
    }

    std::string m_path;

    std::shared_ptr<fly::TaskManager> m_spTaskManager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_spTaskRunner;

    std::shared_ptr<fly::LoggerConfig> m_spLoggerConfig;
    std::shared_ptr<fly::Logger> m_spLogger;
};

//==============================================================================
TEST_F(LoggerTest, LoggerConfigTest)
{
    EXPECT_EQ(fly::LoggerConfig::GetName(), "logger");
}

//==============================================================================
TEST_F(LoggerTest, FilePathTest)
{
    std::string path = m_spLogger->GetLogFilePath();
    EXPECT_TRUE(fly::String::StartsWith(path, m_path));

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
    LOGC("Console Log: %d", __LINE__);

    LOGC_NO_LOCK("Lockless console Log");
    LOGC_NO_LOCK("Lockless console Log: %d", __LINE__);

    std::string contents = capture();
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find("Console Log"), std::string::npos);
    EXPECT_NE(contents.find("Lockless console Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 4);
}

//==============================================================================
TEST_F(LoggerTest, DebugTest)
{
    LOGD("Debug Log");
    LOGD("Debug Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents =
        fly::PathUtil::ReadFile(m_spLogger->GetLogFilePath());
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Debug Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, InfoTest)
{
    LOGW("Info Log");
    LOGW("Info Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents =
        fly::PathUtil::ReadFile(m_spLogger->GetLogFilePath());
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Info Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, WarningTest)
{
    LOGI("Warning Log");
    LOGI("Warning Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents =
        fly::PathUtil::ReadFile(m_spLogger->GetLogFilePath());
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Warning Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, ErrorTest)
{
    LOGE("Error Log");
    LOGE("Error Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents =
        fly::PathUtil::ReadFile(m_spLogger->GetLogFilePath());
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Error Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, RolloverTest)
{
    std::string path = m_spLogger->GetLogFilePath();

    size_t maxMessageSize = m_spLoggerConfig->MaxMessageSize();
    size_t maxFileSize = m_spLoggerConfig->MaxLogFileSize();

    std::string random = fly::String::GenerateRandomString(maxMessageSize);

    ssize_t expectedSize = LogSize(random);
    size_t count = 0;

    // Create enough log points to fill the log file, plus some extra to start
    // filling a second log file
    while (++count < ((maxFileSize / expectedSize) + 10))
    {
        LOGD("%s", random);
        m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    }

    EXPECT_NE(path, m_spLogger->GetLogFilePath());

    size_t actualSize = std::filesystem::file_size(path);
    EXPECT_GE(actualSize, maxMessageSize);
}
