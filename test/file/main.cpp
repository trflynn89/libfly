#include <cstdio>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/file/file_monitor_impl.h"
#include "fly/file/path.h"
#include "fly/logger/logger.h"
#include "fly/string/string.h"

//==============================================================================
class FileMonitorTest : public ::testing::Test
{
public:
    FileMonitorTest() :
        m_spMonitor(std::make_shared<fly::FileMonitorImpl>()),

        m_path1(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_path2(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),

        m_file1(fly::String::GenerateRandomString(10) + ".txt"),
        m_file2(fly::String::GenerateRandomString(10) + ".txt"),
        m_file3(fly::String::GenerateRandomString(10) + ".txt"),

        m_fullPath1(fly::Path::Join(m_path1, m_file1)),
        m_fullPath2(fly::Path::Join(m_path1, m_file2)),
        m_fullPath3(fly::Path::Join(m_path2, m_file3))
    {
    }

    /**
     * Create and start the file monitor.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path1));
        ASSERT_TRUE(fly::Path::MakePath(m_path2));

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

        ASSERT_TRUE(fly::Path::RemovePath(m_path1));
        ASSERT_TRUE(fly::Path::RemovePath(m_path2));
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
        const std::string full = fly::Path::Join(path, file);

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

    std::string path = fly::Path::Join(m_path1, m_file1 + ".diff");
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

//==============================================================================
TEST_F(FileMonitorTest, RemoveTest)
{
    // Test removing files and paths that were not being monitored
    EXPECT_FALSE(m_spMonitor->RemoveFile("was not", m_file1));
    EXPECT_FALSE(m_spMonitor->RemoveFile(m_path1, "monitoring"));
    EXPECT_FALSE(m_spMonitor->RemovePath("any of this"));

    // For the path with two monitored files:
    // 1. Remove one of the files - should succeed
    // 2. Remove the whole path - should succeed
    // 3. Remove the second file - should fail, wasn't being monitored any more
    // 4. Remove the whole path - should fail
    EXPECT_TRUE(m_spMonitor->RemoveFile(m_path1, m_file1));
    EXPECT_TRUE(m_spMonitor->RemovePath(m_path1));
    EXPECT_FALSE(m_spMonitor->RemoveFile(m_path1, m_file2));
    EXPECT_FALSE(m_spMonitor->RemovePath(m_path1));

    // For the path with one monitored file:
    // 1. Remove the monitored file - should succeed
    // 2. Remove the whole path - should fail, path will gets removed when the
    //    last monitored file is removed
    EXPECT_TRUE(m_spMonitor->RemoveFile(m_path2, m_file3));
    EXPECT_FALSE(m_spMonitor->RemovePath(m_path2));
}

//==============================================================================
TEST(PathTest, MakeAndRemovePathTest)
{
    std::string path(fly::Path::Join(
        fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
    ));

    // Should not be able to remove a non-existing path
    EXPECT_FALSE(fly::Path::RemovePath(path));

    // Should be able to make path and receive no errors trying to make it again
    EXPECT_TRUE(fly::Path::MakePath(path));
    EXPECT_TRUE(fly::Path::MakePath(path));
    EXPECT_TRUE(fly::Path::MakePath(path));

    // Should be able to remove path once
    EXPECT_TRUE(fly::Path::RemovePath(path));
    EXPECT_FALSE(fly::Path::RemovePath(path));
}

//==============================================================================
TEST(PathTest, SeparatorTest)
{
    const char sep = fly::Path::GetSeparator();

#if defined(FLY_WINDOWS)
    EXPECT_EQ(sep, '\\');
#elif defined(FLY_LINUX)
    EXPECT_EQ(sep, '/');
#else
    EXPECT_TRUE(false);
#endif
}

//==============================================================================
TEST(PathTest, TempDirectoryTest)
{
    std::string temp(fly::Path::GetTempDirectory());
    EXPECT_FALSE(temp.empty());
}

//==============================================================================
TEST(PathTest, JoinTest)
{
    std::string path1(fly::Path::GetTempDirectory());
    std::string path2(fly::String::GenerateRandomString(10));

    std::string path(fly::Path::Join(path1, path2));
    EXPECT_TRUE(fly::String::StartsWith(path, path1));
    EXPECT_TRUE(fly::String::EndsWith(path, path2));
}
