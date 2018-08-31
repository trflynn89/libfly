#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/logger/logger.h"
#include "fly/logger/logger_config.h"
#include "fly/path/path.h"
#include "fly/task/task_manager.h"
#include "fly/types/string.h"

#include "test/util/waitable_task_runner.h"

namespace
{
    /**
     * RAII helper class to redirect std::cout to a stringstream for inspection.
     */
    class CaptureCout
    {
    public:
        /**
         * Constructor. Redirect std::cout to a stringstream and store the
         * original streambuf target.
         */
        CaptureCout() : m_original(std::cout.rdbuf(m_target.rdbuf()))
        {
        }

        /**
         * Constructor. Restore std::cout to the original streambuf target.
         */
        ~CaptureCout()
        {
            std::cout.rdbuf(m_original);
        }

        /**
         * @return string The contents of std::cout.
         */
        std::string operator() () const
        {
            return m_target.str();
        }

    private:
        std::stringstream m_target;
        std::streambuf *m_original;
    };
}

//==============================================================================
class LoggerTest : public ::testing::Test
{
public:
    LoggerTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),

        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
        ),

        m_spLoggerConfig(std::make_shared<fly::LoggerConfig>()),

        m_spLogger(std::make_shared<fly::Logger>(
            m_spTaskRunner,
            m_spLoggerConfig,
            m_path
        ))
    {
        LOGC("Using path '%s'", m_path);
    }

    /**
     * Create the file directory.
     */
    void SetUp() override
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));

        ASSERT_TRUE(m_spLogger->Start());
        fly::Logger::SetInstance(m_spLogger);
    }

    /**
     * Delete the created directory.
     */
    void TearDown() override
    {
        fly::Logger::SetInstance(fly::LoggerPtr());

        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
    /**
     * Read the contents of the current log file.
     *
     * @return string Contents of the log file.
     */
    std::string FileContents() const
    {
        std::string path = m_spLogger->GetLogFilePath();

        std::ifstream stream(path, std::ios::in);
        std::stringstream sstream;

        if (stream.good())
        {
            sstream << stream.rdbuf();
        }

        return sstream.str();
    }

    /**
     * Measure the size, in bytes, of a file.
     *
     * @param string Path to the file.
     *
     * @return ssize_t Size of the file.
     */
    size_t FileSize(const std::string &path)
    {
        std::ifstream stream(path, std::ios::in);
        size_t size = 0;

        if (stream.good())
        {
            stream.ignore(std::numeric_limits<std::streamsize>::max());
            size = static_cast<size_t>(stream.gcount());
        }

        return size;
    }

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

    fly::TaskManagerPtr m_spTaskManager;
    fly::WaitableSequencedTaskRunnerPtr m_spTaskRunner;

    fly::LoggerConfigPtr m_spLoggerConfig;
    fly::LoggerPtr m_spLogger;
};

//==============================================================================
TEST_F(LoggerTest, FilePathTest)
{
    std::string path = m_spLogger->GetLogFilePath();
    EXPECT_TRUE(fly::String::StartsWith(path, m_path));

    std::ifstream stream(path, std::ios::in);
    EXPECT_TRUE(stream.good());
}

//==============================================================================
TEST_F(LoggerTest, ConsoleTest)
{
    CaptureCout capture;

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
    LOGD(-1, "Debug Log");
    LOGD(-1, "Debug Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents = FileContents();
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Debug Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, InfoTest)
{
    LOGW(-1, "Info Log");
    LOGW(-1, "Info Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents = FileContents();
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Info Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, WarningTest)
{
    LOGI(-1, "Warning Log");
    LOGI(-1, "Warning Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents = FileContents();
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Warning Log"), std::string::npos);
    EXPECT_EQ(std::count(contents.begin(), contents.end(), '\n'), 2);
}

//==============================================================================
TEST_F(LoggerTest, ErrorTest)
{
    LOGE(-1, "Error Log");
    LOGE(-1, "Error Log: %d", __LINE__);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();

    std::string contents = FileContents();
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
    while (++count < ((maxFileSize / expectedSize) + maxMessageSize))
    {
        LOGD(-1, "%s", random);
    }
    while (--count > 0)
    {
        m_spTaskRunner->WaitForTaskTypeToComplete<fly::LoggerTask>();
    }

    EXPECT_NE(path, m_spLogger->GetLogFilePath());

    size_t actualSize = FileSize(path) + FileSize(m_spLogger->GetLogFilePath());
    EXPECT_GE(actualSize, expectedSize * count);
}
