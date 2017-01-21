#include <fstream>
#include <limits>
#include <sstream>

#include <gtest/gtest.h>

#include <fly/file/path.h>
#include <fly/logging/logger.h>
#include <fly/string/string.h>

//==============================================================================
class LoggerTest : public ::testing::Test
{
public:
    LoggerTest() :
        m_path(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_spLogger(std::make_shared<fly::Logger>(m_path))
    {
        LOGC("Using path '%s'", m_path);
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path));

        ASSERT_TRUE(m_spLogger->Start());
        fly::Logger::SetInstance(m_spLogger);
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        fly::Logger::SetInstance(fly::LoggerPtr());
        m_spLogger->Stop();

        ASSERT_TRUE(fly::Path::RemovePath(m_path));
    }

protected:
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
    std::string random = fly::String::GenerateRandomString(1 << 10);
    std::string path = m_spLogger->GetLogFilePath();
    std::streamsize expectedSize = 0;

    while (path == m_spLogger->GetLogFilePath())
    {
        expectedSize += random.length();
        LOGD(-1, "%s", random);
    }

    EXPECT_NE(path, m_spLogger->GetLogFilePath());

    std::ifstream stream(path, std::ios::in);
    EXPECT_TRUE(stream.good());

    stream.ignore(std::numeric_limits<std::streamsize>::max());
    std::streamsize actualSize = stream.gcount();
    EXPECT_GE(actualSize, expectedSize  / 8);
}
