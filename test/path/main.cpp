#include <algorithm>
#include <cstdio>
#include <fstream>
#include <functional>
#include <map>
#include <sstream>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/config/config_manager.h"
#include "fly/logger/logger.h"
#include "fly/path/path.h"
#include "fly/path/path_monitor.h"
#include "fly/string/string.h"

#ifdef FLY_LINUX
    #include "test/mock/mock_system.h"
#endif

//==============================================================================
class PathMonitorTest : public ::testing::Test
{
public:
    PathMonitorTest() :
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::INI, std::string(), std::string()
        )),

        m_spMonitor(std::make_shared<fly::PathMonitorImpl>(m_spConfigManager)),

        m_path0(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_path1(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),
        m_path2(fly::Path::Join(
            fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
        )),

        m_file0(fly::String::GenerateRandomString(10) + ".txt"),
        m_file1(fly::String::GenerateRandomString(10) + ".txt"),
        m_file2(fly::String::GenerateRandomString(10) + ".txt"),
        m_file3(fly::String::GenerateRandomString(10) + ".txt"),

        m_fullPath0(fly::Path::Join(m_path0, m_file0)),
        m_fullPath1(fly::Path::Join(m_path1, m_file1)),
        m_fullPath2(fly::Path::Join(m_path1, m_file2)),
        m_fullPath3(fly::Path::Join(m_path2, m_file3))
    {
    }

    /**
     * Create and start the path monitor.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(fly::Path::MakePath(m_path0));
        ASSERT_TRUE(fly::Path::MakePath(m_path1));
        ASSERT_TRUE(fly::Path::MakePath(m_path2));

        auto callback = std::bind(&PathMonitorTest::HandleEvent, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        ASSERT_TRUE(m_spMonitor && m_spMonitor->Start());
        ASSERT_TRUE(m_spMonitor->AddPath(m_path0, callback));
        ASSERT_TRUE(m_spMonitor->AddPath(m_path1, callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_path1, m_file1, callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_path1, m_file2, callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_path2, m_file3, callback));
    }

    /**
     * Stop the path monitor and delete the created directory.
     */
    virtual void TearDown()
    {
        m_spMonitor->Stop();

        ASSERT_TRUE(fly::Path::RemovePath(m_path0));
        ASSERT_TRUE(fly::Path::RemovePath(m_path1));
        ASSERT_TRUE(fly::Path::RemovePath(m_path2));
    }

protected:
    /**
     * Handle a file change notification.
     *
     * @param path Path to the changed file.
     * @param file Name of the changed file.
     * @param PathEvent The type of event that occurred.
     */
    void HandleEvent(
        const std::string &path,
        const std::string &file,
        fly::PathMonitor::PathEvent eventType)
    {
        const std::string full = fly::Path::Join(path, file);

        switch (eventType)
        {
        case fly::PathMonitor::FILE_CREATED:
            ++m_numCreatedFiles[full];
            break;

        case fly::PathMonitor::FILE_DELETED:
            ++m_numDeletedFiles[full];
            break;

        case fly::PathMonitor::FILE_CHANGED:
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

    fly::ConfigManagerPtr m_spConfigManager;

    fly::PathMonitorPtr m_spMonitor;

    std::string m_path0;
    std::string m_path1;
    std::string m_path2;

    std::string m_file0;
    std::string m_file1;
    std::string m_file2;
    std::string m_file3;

    std::string m_fullPath0;
    std::string m_fullPath1;
    std::string m_fullPath2;
    std::string m_fullPath3;

    std::map<std::string, unsigned int> m_numCreatedFiles;
    std::map<std::string, unsigned int> m_numDeletedFiles;
    std::map<std::string, unsigned int> m_numChangedFiles;
    std::map<std::string, unsigned int> m_numOtherEvents;
};

//==============================================================================
TEST_F(PathMonitorTest, NonExistingPathTest)
{
    ASSERT_FALSE(m_spMonitor->AddPath(m_path0 + "foo", [](...) { }));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1 + "foo", m_file1, [](...) { }));
}

//==============================================================================
TEST_F(PathMonitorTest, NullCallbackTest)
{
    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, nullptr));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1, m_file1, nullptr));
}

#ifdef FLY_LINUX

//==============================================================================
TEST_F(PathMonitorTest, MockFailedStartMonitorTest)
{
    m_spMonitor->RemoveAllPaths();
    m_spMonitor->Stop();

    fly::MockSystem mock(fly::MockCall::INOTIFY_INIT1);

    ASSERT_FALSE(m_spMonitor->Start());

    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, [](...) { }));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1, m_file1, [](...) { }));
}

//==============================================================================
TEST_F(PathMonitorTest, MockFailedAddPathTest)
{
    m_spMonitor->RemoveAllPaths();

    fly::MockSystem mock(fly::MockCall::INOTIFY_ADD_WATCH);

    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, [](...) { }));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1, m_file1, [](...) { }));
}

#endif

//==============================================================================
TEST_F(PathMonitorTest, NoChangeTest_PathLevel)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, NoChangeTest_FileLevel)
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
TEST_F(PathMonitorTest, CreateTest_PathLevel)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    CreateFile(m_fullPath0, std::string());
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, CreateTest_FileLevel)
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
TEST_F(PathMonitorTest, DeleteTest_PathLevel)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    CreateFile(m_fullPath0, std::string());
    std::remove(m_fullPath0.c_str());

    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, DeleteTest_FileLevel)
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
TEST_F(PathMonitorTest, ChangeTest_PathLevel)
{
    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    CreateFile(m_fullPath0, "abcdefghi");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, ChangeTest_FileLevel)
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

#ifdef FLY_LINUX

//==============================================================================
TEST_F(PathMonitorTest, MockFailedPollTest)
{
    fly::MockSystem mock(fly::MockCall::POLL);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    CreateFile(m_fullPath1, "abcdefghi");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, MockFailedReadTest)
{
    fly::MockSystem mock(fly::MockCall::READ);
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    CreateFile(m_fullPath1, "abcdefghi");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

#endif

//==============================================================================
TEST_F(PathMonitorTest, OtherFileTest)
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
TEST_F(PathMonitorTest, MultipleFileTest)
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

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    CreateFile(m_fullPath1, std::string());

    CreateFile(m_fullPath2, std::string());
    std::remove(m_fullPath2.c_str());

    CreateFile(m_fullPath3, "abcdefghi");
    std::remove(m_fullPath3.c_str());

    CreateFile(m_fullPath0, "abcdefghi");
    std::remove(m_fullPath0.c_str());

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

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, RemoveTest)
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

    std::string path2(fly::Path::Join(
        path, fly::String::GenerateRandomString(10)
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

    // Should not be able to make a path if it already exists as a file
    std::ofstream(path, std::ios::out);

    EXPECT_FALSE(fly::Path::MakePath(path));
    EXPECT_FALSE(fly::Path::MakePath(path2));

    // Should not be able to remove a file
    EXPECT_FALSE(fly::Path::RemovePath(path));
    EXPECT_EQ(::remove(path.c_str()), 0);

    // Should be able to recursively make and remove a directory
    EXPECT_TRUE(fly::Path::MakePath(path2));
    EXPECT_TRUE(fly::Path::RemovePath(path));
}

#ifdef FLY_LINUX

//==============================================================================
TEST(PathTest, MockRemovePathTest)
{
    std::string path(fly::Path::Join(
        fly::Path::GetTempDirectory(), fly::String::GenerateRandomString(10)
    ));

    EXPECT_TRUE(fly::Path::MakePath(path));

    {
        fly::MockSystem mock(fly::MockCall::FTS_READ);
        EXPECT_FALSE(fly::Path::RemovePath(path));
    }

    {
        fly::MockSystem mock(fly::MockCall::REMOVE);
        EXPECT_FALSE(fly::Path::RemovePath(path));
    }

    EXPECT_TRUE(fly::Path::RemovePath(path));
}

#endif

//==============================================================================
TEST(PathTest, ListPathTest)
{
    std::vector<std::string> directories;
    std::vector<std::string> files;

    std::string path1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path1Full(fly::Path::Join(fly::Path::GetTempDirectory(), path1));
    EXPECT_TRUE(fly::Path::MakePath(path1Full));

    std::string path2(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path2Full(fly::Path::Join(path1Full, path2));
    EXPECT_TRUE(fly::Path::MakePath(path2Full));

    std::string path3(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path3Full(fly::Path::Join(path1Full, path3));
    EXPECT_TRUE(fly::Path::MakePath(path3Full));

    std::string path4(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path4Full(fly::Path::Join(path2Full, path4));
    EXPECT_TRUE(fly::Path::MakePath(path4Full));

    std::string file1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file1Full(fly::Path::Join(path1Full, file1));
    std::ofstream(file1Full, std::ios::out);

    std::string file2(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file2Full(fly::Path::Join(path2Full, file2));
    std::ofstream(file2Full, std::ios::out);

    std::string file3(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file3Full(fly::Path::Join(path3Full, file3));
    std::ofstream(file3Full, std::ios::out);

    EXPECT_TRUE(fly::Path::ListPath(path1Full, directories, files));
    {
        std::vector<std::string> expectedDirectories = { path2, path3 };
        std::vector<std::string> expectedFiles = { file1 };

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

        std::sort(expectedDirectories.begin(), expectedDirectories.end());
        std::sort(expectedFiles.begin(), expectedFiles.end());

        EXPECT_EQ(directories, expectedDirectories);
        EXPECT_EQ(files, expectedFiles);
    }

    EXPECT_TRUE(fly::Path::ListPath(path2Full, directories, files));
    {
        std::vector<std::string> expectedDirectories = { path4 };
        std::vector<std::string> expectedFiles = { file2 };

        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());

        std::sort(expectedDirectories.begin(), expectedDirectories.end());
        std::sort(expectedFiles.begin(), expectedFiles.end());

        EXPECT_EQ(directories, expectedDirectories);
        EXPECT_EQ(files, expectedFiles);
    }

    EXPECT_TRUE(fly::Path::ListPath(path3Full, directories, files));
    {
        std::vector<std::string> expectedFiles = { file3 };

        std::sort(files.begin(), files.end());

        std::sort(expectedFiles.begin(), expectedFiles.end());

        EXPECT_TRUE(directories.empty());
        EXPECT_EQ(files, expectedFiles);
    }

    EXPECT_TRUE(fly::Path::ListPath(path4Full, directories, files));
    {
        EXPECT_TRUE(directories.empty());
        EXPECT_TRUE(files.empty());
    }

    EXPECT_FALSE(fly::Path::ListPath(file1Full, directories, files));
    EXPECT_FALSE(fly::Path::ListPath(file2Full, directories, files));
    EXPECT_FALSE(fly::Path::ListPath(file3Full, directories, files));
    EXPECT_FALSE(fly::Path::ListPath(fly::String::GenerateRandomString(10), directories, files));

    EXPECT_TRUE(fly::Path::RemovePath(path1Full));
}

#ifdef FLY_LINUX

//==============================================================================
TEST(PathTest, MockListPathTest)
{
    fly::MockSystem mock(fly::MockCall::READDIR);

    std::vector<std::string> directories;
    std::vector<std::string> files;

    std::string path1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path1Full(fly::Path::Join(fly::Path::GetTempDirectory(), path1));
    EXPECT_TRUE(fly::Path::MakePath(path1Full));

    std::string path2(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string path2Full(fly::Path::Join(path1Full, path2));
    EXPECT_TRUE(fly::Path::MakePath(path2Full));

    std::string file1(fly::Path::Join(fly::String::GenerateRandomString(10)));
    std::string file1Full(fly::Path::Join(path1Full, file1));
    std::ofstream(file1Full, std::ios::out);

    EXPECT_TRUE(fly::Path::ListPath(path1Full, directories, files));
    EXPECT_TRUE(directories.empty());
    EXPECT_TRUE(files.empty());

    EXPECT_TRUE(fly::Path::RemovePath(path1Full));
}

#endif

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

#ifdef FLY_LINUX

//==============================================================================
TEST(PathTest, MockTempDirectoryTest)
{
    fly::MockSystem mock(fly::MockCall::GETENV);

    std::string temp(fly::Path::GetTempDirectory());
    EXPECT_FALSE(temp.empty());
}

#endif

//==============================================================================
TEST(PathTest, JoinTest)
{
    std::string path1(fly::Path::GetTempDirectory());
    std::string path2(fly::String::GenerateRandomString(10));
    std::string path;

    std::string separator2x(2, fly::Path::GetSeparator());
    std::string separator3x(3, fly::Path::GetSeparator());

    path = fly::Path::Join(path1, path2);
    EXPECT_TRUE(fly::String::StartsWith(path, path1));
    EXPECT_TRUE(fly::String::EndsWith(path, path2));

    path = fly::Path::Join(path1, separator3x + path2);
    EXPECT_TRUE(fly::String::StartsWith(path, path1));
    EXPECT_TRUE(fly::String::EndsWith(path, path2));
    EXPECT_EQ(path.find(separator2x), std::string::npos);
}

//==============================================================================
TEST(PathTest, SplitTest)
{
    std::string path0(fly::Path::Join(
        fly::String::GenerateRandomString(10)
    ));

    std::string path1(fly::Path::Join(
        fly::Path::GetTempDirectory()
    ));

    std::string path2(fly::Path::Join(
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10)
    ));

    std::string path3(fly::Path::Join(
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10)
    ));

    std::vector<std::string> segments0 = fly::Path::Split(path0);
    std::vector<std::string> segments1 = fly::Path::Split(path1);
    std::vector<std::string> segments2 = fly::Path::Split(path2);
    std::vector<std::string> segments3 = fly::Path::Split(path3);

    EXPECT_EQ(segments0.size(), 2);
    EXPECT_NE(path0.find(segments0[0]), std::string::npos);
    EXPECT_NE(path0.find(segments0[1]), std::string::npos);
    EXPECT_EQ(path0.find(segments0[0]), path0.find(segments0[1]));

    EXPECT_EQ(segments1.size(), 2);
    EXPECT_NE(path1.find(segments1[0]), std::string::npos);
    EXPECT_NE(path1.find(segments1[1]), std::string::npos);
    EXPECT_LT(path1.find(segments1[0]), path1.find(segments1[1]));

    EXPECT_EQ(segments2.size(), 2);
    EXPECT_NE(path2.find(segments2[0]), std::string::npos);
    EXPECT_NE(path2.find(segments2[1]), std::string::npos);
    EXPECT_LT(path2.find(segments2[0]), path2.find(segments2[1]));

    EXPECT_EQ(segments3.size(), 2);
    EXPECT_NE(path3.find(segments3[0]), std::string::npos);
    EXPECT_NE(path3.find(segments3[1]), std::string::npos);
    EXPECT_LT(path3.find(segments3[0]), path3.find(segments3[1]));
}

//==============================================================================
TEST(PathTest, SplitAndJoinTest)
{
    std::string path(fly::Path::Join(
        fly::Path::GetTempDirectory(),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10),
        fly::String::GenerateRandomString(10)
    ));

    std::vector<std::string> segments = fly::Path::Split(path);
    std::string newPath = segments.front();

    for (size_t i = 1; i < segments.size(); ++i)
    {
        newPath = fly::Path::Join(newPath, segments[i]);
    }

    EXPECT_EQ(path, newPath);
}
