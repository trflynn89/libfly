#include <functional>
#include <map>
#include <string>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/path/path.h"
#include "fly/path/path_config.h"
#include "fly/path/path_monitor.h"
#include "fly/task/task_manager.h"
#include "fly/types/concurrent_queue.h"
#include "fly/types/string.h"

#ifdef FLY_LINUX
    #include "test/mock/mock_system.h"
#endif

#include "test/util/path_util.h"
#include "test/util/waitable_task_runner.h"

namespace
{
    std::chrono::seconds s_waitTime(5);
}

//==============================================================================
class PathMonitorTest : public ::testing::Test
{
public:
    PathMonitorTest() :
        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
        ),

        m_spMonitor(std::make_shared<fly::PathMonitorImpl>(
            m_spTaskRunner,
            std::make_shared<fly::PathConfig>()
        )),

        m_path0(fly::PathUtil::GenerateTempDirectory()),
        m_path1(fly::PathUtil::GenerateTempDirectory()),
        m_path2(fly::PathUtil::GenerateTempDirectory()),

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
    void SetUp() override
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
    void TearDown() override
    {
        m_spMonitor->RemoveAllPaths();

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
        fly::PathMonitor::PathEvent event)
    {
        const std::string full = fly::Path::Join(path, file);

        switch (event)
        {
        case fly::PathMonitor::PathEvent::Created:
            ++m_numCreatedFiles[full];
            break;

        case fly::PathMonitor::PathEvent::Deleted:
            ++m_numDeletedFiles[full];
            break;

        case fly::PathMonitor::PathEvent::Changed:
            ++m_numChangedFiles[full];
            break;

        default:
            ++m_numOtherEvents[full];
            break;
        }

        m_eventQueue.Push(event);
    }

    fly::TaskManagerPtr m_spTaskManager;
    fly::WaitableSequencedTaskRunnerPtr m_spTaskRunner;

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

    fly::ConcurrentQueue<fly::PathMonitor::PathEvent> m_eventQueue;

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
    fly::MockSystem mock(fly::MockCall::InotifyInit1);

    m_spMonitor = std::make_shared<fly::PathMonitorImpl>(
        m_spTaskRunner,
        std::make_shared<fly::PathConfig>()
    );

    ASSERT_FALSE(m_spMonitor->Start());

    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, [](...) { }));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1, m_file1, [](...) { }));
}

//==============================================================================
TEST_F(PathMonitorTest, MockFailedAddPathTest)
{
    m_spMonitor->RemoveAllPaths();

    fly::MockSystem mock(fly::MockCall::InotifyAddWatch);

    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, [](...) { }));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path1, m_file1, [](...) { }));
}

#endif

//==============================================================================
TEST_F(PathMonitorTest, NoChangeTest_PathLevel)
{
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, NoChangeTest_FileLevel)
{
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, CreateTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath0, std::string()));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, CreateTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath1, std::string()));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, DeleteTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath0, std::string()));
    std::remove(m_fullPath0.c_str());
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, DeleteTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath1, std::string()));
    std::remove(m_fullPath1.c_str());
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, ChangeTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath0, "abcdefghi"));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath0], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath0], 1);
    EXPECT_EQ(m_numOtherEvents[m_fullPath0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, ChangeTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath1, "abcdefghi"));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 1);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

#ifdef FLY_LINUX

//==============================================================================
TEST_F(PathMonitorTest, MockFailedPollTest)
{
    fly::MockSystem mock(fly::MockCall::Poll);
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath1, "abcdefghi"));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, MockFailedReadTest)
{
    fly::MockSystem mock(fly::MockCall::Read);
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath1, "abcdefghi"));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

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
    ASSERT_TRUE(fly::PathUtil::WriteFile(path, "abcdefghi"));

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);

    path = path.substr(0, path.length() - 8);
    ASSERT_TRUE(fly::PathUtil::WriteFile(path, "abcdefghi"));

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numChangedFiles[m_fullPath1], 0);
    EXPECT_EQ(m_numOtherEvents[m_fullPath1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, MultipleFileTest)
{
    fly::PathMonitor::PathEvent event;

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

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath1, std::string()));

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath2, std::string()));
    std::remove(m_fullPath2.c_str());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath3, "abcdefghi"));
    std::remove(m_fullPath3.c_str());

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_fullPath0, "abcdefghi"));
    std::remove(m_fullPath0.c_str());

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

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
