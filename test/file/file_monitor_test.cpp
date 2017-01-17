#include <cstdio>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>

#include <gtest/gtest.h>

#include <fly/file/file_monitor_impl.h>
#include <fly/logging/logger.h>
#include <fly/string/string.h>
#include <fly/system/system.h>

//==============================================================================
class FileMonitorTest : public ::testing::Test
{
public:
    FileMonitorTest() :
        m_spMonitor(std::make_shared<fly::FileMonitorImpl>()),

        m_path1(fly::System::Join(
            fly::System::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_path2(fly::System::Join(
            fly::System::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),

        m_file1(fly::String::GenerateRandomString(10) + ".txt"),
        m_file2(fly::String::GenerateRandomString(10) + ".txt"),
        m_file3(fly::String::GenerateRandomString(10) + ".txt"),

        m_fullPath1(fly::System::Join(m_path1, m_file1)),
        m_fullPath2(fly::System::Join(m_path1, m_file2)),
        m_fullPath3(fly::System::Join(m_path2, m_file3))
    {
    }

    /**
     * Create and start the file monitor.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::System::MakeDirectory(m_path1));
        ASSERT_TRUE(fly::System::MakeDirectory(m_path2));

        auto callback = std::bind(&FileMonitorTest::HandleEvent, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        ASSERT_TRUE(m_spMonitor && m_spMonitor->Start());
        ASSERT_TRUE(m_spMonitor->AddFile(m_path1, m_file1, callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_path1, m_file2, callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_path2, m_file3, callback));
    }

    /**
     * Stop the file monitor and delete the created directory.
     */
    virtual void TearDown()
    {
        m_spMonitor->Stop();
        ASSERT_TRUE(fly::System::RemoveDirectory(m_path1));
        ASSERT_TRUE(fly::System::RemoveDirectory(m_path2));
    }

protected:
    /**
     * Handle a file change notification.
     *
     * @param path Path to the changed file.
     * @param file Name of the changed file.
     * @param FileEvent The type of event that occurred.
     */
    void HandleEvent(
        const std::string &path,
        const std::string &file,
        fly::FileMonitor::FileEvent eventType)
    {
        const std::string full = fly::System::Join(path, file);

        switch (eventType)
        {
        case fly::FileMonitor::FILE_CREATED:
            ++m_numCreatedFiles[full];
            break;

        case fly::FileMonitor::FILE_DELETED:
            ++m_numDeletedFiles[full];
            break;

        case fly::FileMonitor::FILE_CHANGED:
            ++m_numChangedFiles[full];
            break;

        default:
            ++m_numOtherEvents[full];
            break;
        }
    }

    /**
     * Create a file with the given contents.
     *
     * @param path Full path to the file to create.
     * @param file Name of the file to create.
     * @param string Contents of the file to create.
     */
    void CreateFile(const std::string &path, const std::string &contents)
    {
        {
            std::ofstream stream(path, std::ios::out);
            ASSERT_TRUE(stream.good());
            stream << contents;
        }
        {
            std::ifstream stream(path, std::ios::in);
            ASSERT_TRUE(stream.good());

            std::stringstream sstream;
            sstream << stream.rdbuf();

            ASSERT_EQ(contents, sstream.str());
        }
    }

    fly::FileMonitorPtr m_spMonitor;

    std::string m_path1;
    std::string m_path2;

    std::string m_file1;
    std::string m_file2;
    std::string m_file3;

    std::string m_fullPath1;
    std::string m_fullPath2;
    std::string m_fullPath3;

    std::map<std::string, unsigned int> m_numCreatedFiles;
    std::map<std::string, unsigned int> m_numDeletedFiles;
    std::map<std::string, unsigned int> m_numChangedFiles;
    std::map<std::string, unsigned int> m_numOtherEvents;
};

//==============================================================================
TEST_F(FileMonitorTest, NonExistingPathTest)
{
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1 + "foo", m_file1, [](...) { }));
}

//==============================================================================
TEST_F(FileMonitorTest, NullCallbackTest)
{
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1, m_file1, nullptr));
}

//==============================================================================
TEST_F(FileMonitorTest, NoChangeTest)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(FileMonitorTest, CreateTest)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    CreateFile(m_fullPath1, std::string());
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(FileMonitorTest, DeleteTest)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    CreateFile(m_fullPath1, std::string());
    std::remove(m_fullPath1.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(FileMonitorTest, ChangeTest)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    CreateFile(m_fullPath1, "abcdefghi");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(FileMonitorTest, OtherFileTest)
{

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    std::string path = fly::System::Join(m_path1, m_file1 + ".diff");
    CreateFile(path, "abcdefghi");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    path = path.substr(0, path.length() - 8);
    CreateFile(path, "abcdefghi");

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(FileMonitorTest, MultipleFileTest)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    EXPECT_EQ(m_numCreatedFiles[m_fullPath2], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath2], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath2], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath2], 0);

    EXPECT_EQ(m_numCreatedFiles[m_fullPath3], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath3], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath3], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath3], 0);

    CreateFile(m_fullPath1, std::string());

    CreateFile(m_fullPath2, std::string());
    std::remove(m_fullPath2.c_str());

    CreateFile(m_fullPath3, "abcdefghi");
    std::remove(m_fullPath3.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    EXPECT_EQ(m_numCreatedFiles[m_fullPath2], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath2], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath2], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath2], 0);

    EXPECT_EQ(m_numCreatedFiles[m_fullPath3], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath3], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath3], 1);
    EXPECT_EQ(m_numOtherEvents[m_fullPath3], 0);
}
