#include <cstdio>
#include <fstream>
#include <functional>

#include <gtest/gtest.h>

#include <fly/file/file_monitor_impl.h>
#include <fly/string/string.h>
#include <fly/system/system.h>

//==============================================================================
class FileMonitorTest : public ::testing::Test
{
public:
    FileMonitorTest() :
        m_spMonitor(),
        m_path("."),
        m_file(fly::String::GenerateRandomString(10) + ".txt"),
        m_numCreatedFiles(0),
        m_numDeletedFiles(0),
        m_numChangedFiles(0),
        m_numOtherEvents(0)
    {
    }

    /**
     * Create and start the file monitor.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::System::MakeDirectory(m_path));
        std::remove(GetFullPath().c_str());

        auto callback = std::bind(&FileMonitorTest::HandleEvent, this, std::placeholders::_1);
        m_spMonitor = std::make_shared<fly::FileMonitorImpl>(callback, m_path, m_file);

        ASSERT_TRUE(m_spMonitor && m_spMonitor->Start());
    }

    /**
     * Stop the file monitor and delete the created file.
     */
    virtual void TearDown()
    {
        m_spMonitor->Stop();
        std::remove(GetFullPath().c_str());
    }

protected:
    /**
     * Handle a file change notification.
     *
     * @param FileEvent The type of event that occurred.
     */
    void HandleEvent(fly::FileMonitor::FileEvent eventType)
    {
        switch (eventType)
        {
        case fly::FileMonitor::FILE_CREATED:
            ++m_numCreatedFiles;
            break;

        case fly::FileMonitor::FILE_DELETED:
            ++m_numDeletedFiles;
            break;

        case fly::FileMonitor::FILE_CHANGED:
            ++m_numChangedFiles;
            break;

        default:
            ++m_numOtherEvents;
            break;
        }
    }

    /**
     * @return The full path to the file being monitored.
     */
    std::string GetFullPath() const
    {
        static const char sep = fly::System::GetSeparator();
        return fly::String::Join(sep, m_path, m_file);
    }

    fly::FileMonitorPtr m_spMonitor;

    std::string m_path;
    std::string m_file;

    unsigned int m_numCreatedFiles;
    unsigned int m_numDeletedFiles;
    unsigned int m_numChangedFiles;
    unsigned int m_numOtherEvents;
};

//==============================================================================
TEST_F(FileMonitorTest, NonExistingPathTest)
{
    m_spMonitor->Stop();
    m_spMonitor = std::make_shared<fly::FileMonitorImpl>(nullptr, m_path + "foo", m_file);
    ASSERT_FALSE(m_spMonitor->Start());
}

//==============================================================================
TEST_F(FileMonitorTest, NoChangeTest)
{
    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);
}

//==============================================================================
TEST_F(FileMonitorTest, NullCallbackTest)
{
    m_spMonitor->Stop();
    m_spMonitor = std::make_shared<fly::FileMonitorImpl>(nullptr, m_path, m_file);
    ASSERT_TRUE(m_spMonitor->Start());

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    std::ofstream stream(GetFullPath(), std::ios::out);
    stream.close();

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);
}

//==============================================================================
TEST_F(FileMonitorTest, CreateTest)
{
    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    std::ofstream stream(GetFullPath(), std::ios::out);
    stream.close();

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 1);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);
}

//==============================================================================
TEST_F(FileMonitorTest, DeleteTest)
{
    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    std::ofstream stream(GetFullPath(), std::ios::out);
    stream.close();
    std::remove(GetFullPath().c_str());

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 1);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);
}

//==============================================================================
TEST_F(FileMonitorTest, ChangeTest)
{
    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    std::ofstream stream(GetFullPath(), std::ios::out);
    stream << "abcdefghi";
    stream.close();

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 1);
    EXPECT_EQ(m_numOtherEvents, 0);
}

//==============================================================================
TEST_F(FileMonitorTest, OtherFileTest)
{
    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    std::string file = GetFullPath() + ".diff";
    std::ofstream stream(file, std::ios::out);
    stream << "abcdefghi";
    stream.close();
    std::remove(file.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);

    file = file.substr(0, file.length() - 6);
    stream.open(file, std::ios::out);
    stream << "abcdefghi";
    stream.close();
    std::remove(file.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(8));

    EXPECT_EQ(m_numCreatedFiles, 0);
    EXPECT_EQ(m_numDeletedFiles, 0);
    EXPECT_EQ(m_numChangedFiles, 0);
    EXPECT_EQ(m_numOtherEvents, 0);
}
