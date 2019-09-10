#include "fly/path/path_monitor.h"

#include "fly/literals.h"
#include "fly/path/path_config.h"
#include "fly/task/task_manager.h"
#include "fly/types/concurrent_queue.h"
#include "fly/types/string.h"

#include <gtest/gtest.h>

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.h"
#endif

#include "test/util/path_util.h"
#include "test/util/waitable_task_runner.h"

namespace {

std::chrono::seconds s_waitTime(5);

/**
 * Subclass of the path config to decrease the poll interval for faster
 * testing.
 */
class TestPathConfig : public fly::PathConfig
{
public:
    TestPathConfig() noexcept : fly::PathConfig()
    {
        m_defaultPollInterval = 10_i64;
    }
};

} // namespace

//==============================================================================
class PathMonitorTest : public ::testing::Test
{
public:
    PathMonitorTest() noexcept :
        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager
                ->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()),

        m_spMonitor(std::make_shared<fly::PathMonitorImpl>(
            m_spTaskRunner,
            std::make_shared<TestPathConfig>())),

        m_path0(fly::PathUtil::GenerateTempDirectory()),
        m_path1(fly::PathUtil::GenerateTempDirectory()),
        m_path2(fly::PathUtil::GenerateTempDirectory()),

        m_file0(m_path0 / (fly::String::GenerateRandomString(10) + ".txt")),
        m_file1(m_path1 / (fly::String::GenerateRandomString(10) + ".txt")),
        m_file2(m_path1 / (fly::String::GenerateRandomString(10) + ".txt")),
        m_file3(m_path2 / (fly::String::GenerateRandomString(10) + ".txt"))
    {
        m_callback = std::bind(
            &PathMonitorTest::HandleEvent,
            this,
            std::placeholders::_1,
            std::placeholders::_2);
    }

    /**
     * Create and start the task manager and path monitor.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(std::filesystem::create_directories(m_path0));
        ASSERT_TRUE(std::filesystem::create_directories(m_path1));
        ASSERT_TRUE(std::filesystem::create_directories(m_path2));

        ASSERT_TRUE(m_spTaskManager->Start());
        ASSERT_TRUE(m_spMonitor->Start());

        ASSERT_TRUE(m_spMonitor->AddPath(m_path0, m_callback));
        ASSERT_TRUE(m_spMonitor->AddPath(m_path1, m_callback));

        ASSERT_TRUE(m_spMonitor->AddFile(m_file0, m_callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_file1, m_callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_file2, m_callback));
        ASSERT_TRUE(m_spMonitor->AddFile(m_file3, m_callback));
    }

    /**
     * Stop the task manager and delete the created directory.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_spTaskManager->Stop());

        m_spMonitor->RemoveAllPaths();
        std::filesystem::remove_all(m_path0);
        std::filesystem::remove_all(m_path1);
        std::filesystem::remove_all(m_path2);
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
        const std::filesystem::path &path,
        fly::PathMonitor::PathEvent event) noexcept
    {
        switch (event)
        {
            case fly::PathMonitor::PathEvent::Created:
                ++m_numCreatedFiles[path];
                break;

            case fly::PathMonitor::PathEvent::Deleted:
                ++m_numDeletedFiles[path];
                break;

            case fly::PathMonitor::PathEvent::Changed:
                ++m_numChangedFiles[path];
                break;

            default:
                ++m_numOtherEvents[path];
                break;
        }

        m_eventQueue.Push(std::move(event));
    }

    std::shared_ptr<fly::TaskManager> m_spTaskManager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_spTaskRunner;

    std::shared_ptr<fly::PathMonitor> m_spMonitor;
    fly::PathMonitor::PathEventCallback m_callback;

    std::filesystem::path m_path0;
    std::filesystem::path m_path1;
    std::filesystem::path m_path2;

    std::filesystem::path m_file0;
    std::filesystem::path m_file1;
    std::filesystem::path m_file2;
    std::filesystem::path m_file3;

    fly::ConcurrentQueue<fly::PathMonitor::PathEvent> m_eventQueue;

    std::map<std::filesystem::path, unsigned int> m_numCreatedFiles;
    std::map<std::filesystem::path, unsigned int> m_numDeletedFiles;
    std::map<std::filesystem::path, unsigned int> m_numChangedFiles;
    std::map<std::filesystem::path, unsigned int> m_numOtherEvents;
};

//==============================================================================
TEST_F(PathMonitorTest, PathEventStreamTest)
{
    {
        std::stringstream stream;
        stream << static_cast<fly::PathMonitor::PathEvent>(-1);
        EXPECT_TRUE(stream.str().empty());
    }
    {
        std::stringstream stream;
        stream << fly::PathMonitor::PathEvent::None;
        EXPECT_EQ(stream.str(), "None");
    }
    {
        std::stringstream stream;
        stream << fly::PathMonitor::PathEvent::Created;
        EXPECT_EQ(stream.str(), "Created");
    }
    {
        std::stringstream stream;
        stream << fly::PathMonitor::PathEvent::Deleted;
        EXPECT_EQ(stream.str(), "Deleted");
    }
    {
        std::stringstream stream;
        stream << fly::PathMonitor::PathEvent::Changed;
        EXPECT_EQ(stream.str(), "Changed");
    }
}

//==============================================================================
TEST_F(PathMonitorTest, NonExistingPathTest)
{
    ASSERT_FALSE(m_spMonitor->AddPath(m_path0 / "_", m_callback));
    ASSERT_FALSE(m_spMonitor->AddFile(m_path0 / "_" / "foo.txt", m_callback));
}

//==============================================================================
TEST_F(PathMonitorTest, NullCallbackTest)
{
    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, nullptr));
    ASSERT_FALSE(m_spMonitor->AddFile(m_file1, nullptr));
}

//==============================================================================
TEST_F(PathMonitorTest, WrongTypeTest)
{
    ASSERT_FALSE(m_spMonitor->AddFile(m_path0, m_callback));
    ASSERT_FALSE(m_spMonitor->AddPath(m_file1, m_callback));
}

#if defined(FLY_LINUX)

//==============================================================================
TEST_F(PathMonitorTest, MockFailedStartMonitorTest)
{
    fly::MockSystem mock(fly::MockCall::InotifyInit1);

    m_spMonitor = std::make_shared<fly::PathMonitorImpl>(
        m_spTaskRunner, std::make_shared<fly::PathConfig>());

    ASSERT_FALSE(m_spMonitor->Start());

    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, m_callback));
    ASSERT_FALSE(m_spMonitor->AddFile(m_file1, m_callback));
}

//==============================================================================
TEST_F(PathMonitorTest, MockFailedAddPathTest)
{
    m_spMonitor->RemoveAllPaths();

    fly::MockSystem mock(fly::MockCall::InotifyAddWatch);

    ASSERT_FALSE(m_spMonitor->AddPath(m_path0, m_callback));
    ASSERT_FALSE(m_spMonitor->AddFile(m_file1, m_callback));
}

#endif

//==============================================================================
TEST_F(PathMonitorTest, NoChangeTest_PathLevel)
{
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, NoChangeTest_FileLevel)
{
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, CreateTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file0, std::string()));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, CreateTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file1, std::string()));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, DeleteTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file0, std::string()));
    std::filesystem::remove(m_file0);
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 1);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, DeleteTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file1, std::string()));
    std::filesystem::remove(m_file1);
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 1);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, ChangeTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file0, "abcdefghi"));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 1);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, ChangeTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file1, "abcdefghi"));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 1);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

#if defined(FLY_LINUX)

//==============================================================================
TEST_F(PathMonitorTest, MockFailedPollTest)
{
    fly::MockSystem mock(fly::MockCall::Poll);
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file1, "abcdefghi"));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, MockFailedReadTest)
{
    fly::MockSystem mock(fly::MockCall::Read);
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file1, "abcdefghi"));
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

#endif

//==============================================================================
TEST_F(PathMonitorTest, OtherFileTest)
{
    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    auto path = std::filesystem::path(m_file1).concat(".diff");
    ASSERT_TRUE(fly::PathUtil::WriteFile(path.string(), "abcdefghi"));

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    path = std::filesystem::path(
        path.string().substr(0, path.string().length() - 8));
    ASSERT_TRUE(fly::PathUtil::WriteFile(path, "abcdefghi"));

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::PathMonitorTask>();

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, MultipleFileTest)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_numCreatedFiles[m_file1], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    EXPECT_EQ(m_numCreatedFiles[m_file2], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file2], 0);
    EXPECT_EQ(m_numChangedFiles[m_file2], 0);
    EXPECT_EQ(m_numOtherEvents[m_file2], 0);

    EXPECT_EQ(m_numCreatedFiles[m_file3], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file3], 0);
    EXPECT_EQ(m_numChangedFiles[m_file3], 0);
    EXPECT_EQ(m_numOtherEvents[m_file3], 0);

    EXPECT_EQ(m_numCreatedFiles[m_file0], 0);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 0);
    EXPECT_EQ(m_numChangedFiles[m_file0], 0);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file1, std::string()));

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file2, std::string()));
    std::filesystem::remove(m_file2);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file3, "abcdefghi"));
    std::filesystem::remove(m_file3);

    ASSERT_TRUE(fly::PathUtil::WriteFile(m_file0, "abcdefghi"));
    std::filesystem::remove(m_file0);

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));
    ASSERT_TRUE(m_eventQueue.Pop(event, s_waitTime));

    EXPECT_EQ(m_numCreatedFiles[m_file1], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file1], 0);
    EXPECT_EQ(m_numChangedFiles[m_file1], 0);
    EXPECT_EQ(m_numOtherEvents[m_file1], 0);

    EXPECT_EQ(m_numCreatedFiles[m_file2], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file2], 1);
    EXPECT_EQ(m_numChangedFiles[m_file2], 0);
    EXPECT_EQ(m_numOtherEvents[m_file2], 0);

    EXPECT_EQ(m_numCreatedFiles[m_file3], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file3], 1);
    EXPECT_EQ(m_numChangedFiles[m_file3], 1);
    EXPECT_EQ(m_numOtherEvents[m_file3], 0);

    EXPECT_EQ(m_numCreatedFiles[m_file0], 1);
    EXPECT_EQ(m_numDeletedFiles[m_file0], 1);
    EXPECT_EQ(m_numChangedFiles[m_file0], 1);
    EXPECT_EQ(m_numOtherEvents[m_file0], 0);
}

//==============================================================================
TEST_F(PathMonitorTest, RemoveTest)
{
    // Test removing files and paths that were not being monitored
    EXPECT_FALSE(m_spMonitor->RemoveFile(m_path1 / "was not"));
    EXPECT_FALSE(m_spMonitor->RemovePath(m_path1 / "monitoring"));
    EXPECT_FALSE(m_spMonitor->RemovePath("any of this"));

    // For the monitor with two monitored files and a monitored path:
    // 1. Remove one of the files - should succeed
    // 2. Remove the whole path - should succeed
    // 3. Remove the second file - should fail, wasn't being monitored any more
    // 4. Remove the whole path - should fail
    EXPECT_TRUE(m_spMonitor->RemoveFile(m_file1));
    EXPECT_TRUE(m_spMonitor->RemovePath(m_path1));
    EXPECT_FALSE(m_spMonitor->RemoveFile(m_file2));
    EXPECT_FALSE(m_spMonitor->RemovePath(m_path1));

    // For the monitor with one monitored file and a monitored path:
    // 1. Remove the monitored file - should succeed
    // 2. Remove the whole path - should succeed
    EXPECT_TRUE(m_spMonitor->RemoveFile(m_file0));
    EXPECT_TRUE(m_spMonitor->RemovePath(m_path0));

    // For the monitor with one monitored file and no monitored paths:
    // 1. Remove the monitored file - should succeed
    // 2. Remove the whole path - should fail, path will gets removed when the
    //    last monitored file is removed
    EXPECT_TRUE(m_spMonitor->RemoveFile(m_file3));
    EXPECT_FALSE(m_spMonitor->RemovePath(m_path2));
}
