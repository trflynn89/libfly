#include <cstring>
#include <fstream>
#include <limits>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/logger/logger_config.h"
#include "fly/path/path.h"
#include "fly/types/string.h"

//==============================================================================
class LoggerTest : public ::testing::Test
{
public:
    LoggerTest() :
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::Ini, std::string(), std::string()
        )),

        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),

        m_spLogger(std::make_shared<fly::Logger>(m_spConfigManager, m_path))
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
        m_spLogger->Stop();

        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
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

    fly::ConfigManagerPtr m_spConfigManager;

    std::string m_path;

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
TEST_F(LoggerTest, MacroTest)
{
    LOGC("Console Log");
    LOGC("Console Log: %d", __LINE__);

    LOGC_NO_LOCK("Lockless console Log");
    LOGC_NO_LOCK("Lockless console Log: %d", __LINE__);

    LOGD(-1, "Debug Log");
    LOGD(-1, "Debug Log: %d", __LINE__);

    LOGI(-1, "Warning Log");
    LOGI(-1, "Warning Log: %d", __LINE__);

    LOGW(-1, "Info Log");
    LOGW(-1, "Info Log: %d", __LINE__);

    LOGE(-1, "Error Log");
    LOGE(-1, "Error Log: %d", __LINE__);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::string path = m_spLogger->GetLogFilePath();
    std::ifstream stream(path, std::ios::in);
    EXPECT_TRUE(stream.good());

    std::stringstream sstream;
    sstream << stream.rdbuf();

    std::string contents = sstream.str();
    EXPECT_FALSE(contents.empty());

    EXPECT_NE(contents.find(__FILE__), std::string::npos);
    EXPECT_NE(contents.find(__FUNCTION__), std::string::npos);
    EXPECT_NE(contents.find("Debug Log"), std::string::npos);
    EXPECT_NE(contents.find("Warning Log"), std::string::npos);
    EXPECT_NE(contents.find("Info Log"), std::string::npos);
    EXPECT_NE(contents.find("Error Log"), std::string::npos);
}

//==============================================================================
TEST_F(LoggerTest, RolloverTest)
{
    fly::LoggerConfigPtr spConfig = m_spLogger->GetLogConfig();
    std::string path = m_spLogger->GetLogFilePath();

    size_t maxMessageSize = spConfig->MaxMessageSize();
    size_t maxFileSize = spConfig->MaxLogFileSize();

    std::string random = fly::String::GenerateRandomString(maxMessageSize);

    ssize_t expectedSize = LogSize(random);
    size_t count = 0;

    // Create enough log points to fill the log file, plus some extra to start
    // filling a second log file
    while (++count < ((maxFileSize / expectedSize) + maxMessageSize))
    {
        LOGD(-1, "%s", random);
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
    EXPECT_NE(path, m_spLogger->GetLogFilePath());

    size_t actualSize = FileSize(path) + FileSize(m_spLogger->GetLogFilePath());
    EXPECT_GE(actualSize, expectedSize * count);
}
