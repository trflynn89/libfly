#include "fly/path/path_monitor.hpp"

#include "fly/path/path_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/numeric/literals.hpp"

#include <catch2/catch.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
#include <string>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include "test/util/path_util.hpp"
#include "test/util/waitable_task_runner.hpp"

namespace {

std::chrono::seconds s_wait_time(5);

/**
 * Subclass of the path config to decrease the poll interval for faster testing.
 */
class TestPathConfig : public fly::PathConfig
{
public:
    TestPathConfig() noexcept : fly::PathConfig()
    {
        m_default_poll_interval = 10_i64;
    }
};

} // namespace

//==================================================================================================
class PathMonitorTest : public ::testing::Test
{
public:
    PathMonitorTest() noexcept :
        m_task_manager(std::make_shared<fly::TaskManager>(1)),
        m_task_runner(m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>()),

        m_monitor(std::make_shared<fly::PathMonitorImpl>(
            m_task_runner,
            std::make_shared<TestPathConfig>())),

        m_file0(m_path0.file()),
        m_file1(m_path1.file()),
        m_file2(m_path1.file()),
        m_file3(m_path2.file())
    {
        m_callback = std::bind(
            &PathMonitorTest::handle_event,
            this,
            std::placeholders::_1,
            std::placeholders::_2);
    }

    /**
     * Start the task and config managers.
     */
    void SetUp() override
    {
        ASSERT_TRUE(m_task_manager->start());
        ASSERT_TRUE(m_monitor->start());

        ASSERT_TRUE(m_monitor->add_path(m_path0(), m_callback));
        ASSERT_TRUE(m_monitor->add_path(m_path1(), m_callback));

        ASSERT_TRUE(m_monitor->add_file(m_file0, m_callback));
        ASSERT_TRUE(m_monitor->add_file(m_file1, m_callback));
        ASSERT_TRUE(m_monitor->add_file(m_file2, m_callback));
        ASSERT_TRUE(m_monitor->add_file(m_file3, m_callback));
    }

    /**
     * Stop the task manager and stop monitoring all paths.
     */
    void TearDown() override
    {
        ASSERT_TRUE(m_task_manager->stop());
        m_monitor->remove_all_paths();
    }

protected:
    /**
     * Handle a file change notification.
     *
     * @param path Path to the changed file.
     * @param event The type of event that occurred.
     */
    void handle_event(const std::filesystem::path &path, fly::PathMonitor::PathEvent event)
    {
        switch (event)
        {
            case fly::PathMonitor::PathEvent::Created:
                ++m_created_files[path];
                break;

            case fly::PathMonitor::PathEvent::Deleted:
                ++m_deleted_files[path];
                break;

            case fly::PathMonitor::PathEvent::Changed:
                ++m_changed_files[path];
                break;

            default:
                ++m_other_files[path];
                break;
        }

        m_event_queue.push(std::move(event));
    }

    std::shared_ptr<fly::TaskManager> m_task_manager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_task_runner;

    std::shared_ptr<fly::PathMonitor> m_monitor;
    fly::PathMonitor::PathEventCallback m_callback;

    fly::PathUtil::ScopedTempDirectory m_path0;
    fly::PathUtil::ScopedTempDirectory m_path1;
    fly::PathUtil::ScopedTempDirectory m_path2;

    std::filesystem::path m_file0;
    std::filesystem::path m_file1;
    std::filesystem::path m_file2;
    std::filesystem::path m_file3;

    fly::ConcurrentQueue<fly::PathMonitor::PathEvent> m_event_queue;

    std::map<std::filesystem::path, unsigned int> m_created_files;
    std::map<std::filesystem::path, unsigned int> m_deleted_files;
    std::map<std::filesystem::path, unsigned int> m_changed_files;
    std::map<std::filesystem::path, unsigned int> m_other_files;
};

//==================================================================================================
TEST_F(PathMonitorTest, PathEventStream)
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

//==================================================================================================
TEST_F(PathMonitorTest, NonExistingPath)
{
    ASSERT_FALSE(m_monitor->add_path(m_path0() / "_", m_callback));
    ASSERT_FALSE(m_monitor->add_file(m_path0() / "_" / "foo.txt", m_callback));
}

//==================================================================================================
TEST_F(PathMonitorTest, NullCallback)
{
    ASSERT_FALSE(m_monitor->add_path(m_path0(), nullptr));
    ASSERT_FALSE(m_monitor->add_file(m_file1, nullptr));
}

//==================================================================================================
TEST_F(PathMonitorTest, WrongType)
{
    ASSERT_FALSE(m_monitor->add_file(m_path0(), m_callback));
    ASSERT_FALSE(m_monitor->add_path(m_file1, m_callback));
}

#if defined(FLY_LINUX)

//==================================================================================================
TEST_F(PathMonitorTest, MockFailedStartMonitor)
{
    fly::MockSystem mock(fly::MockCall::InotifyInit1);

    m_monitor =
        std::make_shared<fly::PathMonitorImpl>(m_task_runner, std::make_shared<fly::PathConfig>());

    ASSERT_FALSE(m_monitor->start());

    ASSERT_FALSE(m_monitor->add_path(m_path0(), m_callback));
    ASSERT_FALSE(m_monitor->add_file(m_file1, m_callback));
}

//==================================================================================================
TEST_F(PathMonitorTest, MockFailedadd_path)
{
    m_monitor->remove_all_paths();

    fly::MockSystem mock(fly::MockCall::InotifyAddWatch);

    ASSERT_FALSE(m_monitor->add_path(m_path0(), m_callback));
    ASSERT_FALSE(m_monitor->add_file(m_file1, m_callback));
}

#endif

//==================================================================================================
TEST_F(PathMonitorTest, NoChangeTest_PathLevel)
{
    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file0], 0);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, NoChangeTest_FileLevel)
{
    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, CreateTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file0], 0);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file0, std::string()));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file0], 1);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, CreateTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file1, std::string()));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file1], 1);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, DeleteTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file0], 0);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file0, std::string()));
    std::filesystem::remove(m_file0);
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file0], 1);
    EXPECT_EQ(m_deleted_files[m_file0], 1);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, DeleteTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file1, std::string()));
    std::filesystem::remove(m_file1);
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file1], 1);
    EXPECT_EQ(m_deleted_files[m_file1], 1);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, ChangeTest_PathLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file0], 0);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file0, "abcdefghi"));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file0], 1);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 1);
    EXPECT_EQ(m_other_files[m_file0], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, ChangeTest_FileLevel)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file1, "abcdefghi"));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file1], 1);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 1);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

#if defined(FLY_LINUX)

//==================================================================================================
TEST_F(PathMonitorTest, MockFailedPoll)
{
    fly::MockSystem mock(fly::MockCall::Poll);
    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file1, "abcdefghi"));
    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, MockFailedRead)
{
    fly::MockSystem mock(fly::MockCall::Read);
    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file1, "abcdefghi"));
    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

#endif

//==================================================================================================
TEST_F(PathMonitorTest, OtherFile)
{
    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    auto path = std::filesystem::path(m_file1).concat(".diff");
    ASSERT_TRUE(fly::PathUtil::write_file(path.string(), "abcdefghi"));

    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    path = std::filesystem::path(path.string().substr(0, path.string().length() - 8));
    ASSERT_TRUE(fly::PathUtil::write_file(path, "abcdefghi"));

    m_task_runner->wait_for_task_to_complete<fly::PathMonitorTask>();

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, MultipleFile)
{
    fly::PathMonitor::PathEvent event;

    EXPECT_EQ(m_created_files[m_file1], 0);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    EXPECT_EQ(m_created_files[m_file2], 0);
    EXPECT_EQ(m_deleted_files[m_file2], 0);
    EXPECT_EQ(m_changed_files[m_file2], 0);
    EXPECT_EQ(m_other_files[m_file2], 0);

    EXPECT_EQ(m_created_files[m_file3], 0);
    EXPECT_EQ(m_deleted_files[m_file3], 0);
    EXPECT_EQ(m_changed_files[m_file3], 0);
    EXPECT_EQ(m_other_files[m_file3], 0);

    EXPECT_EQ(m_created_files[m_file0], 0);
    EXPECT_EQ(m_deleted_files[m_file0], 0);
    EXPECT_EQ(m_changed_files[m_file0], 0);
    EXPECT_EQ(m_other_files[m_file0], 0);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file1, std::string()));

    ASSERT_TRUE(fly::PathUtil::write_file(m_file2, std::string()));
    std::filesystem::remove(m_file2);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file3, "abcdefghi"));
    std::filesystem::remove(m_file3);

    ASSERT_TRUE(fly::PathUtil::write_file(m_file0, "abcdefghi"));
    std::filesystem::remove(m_file0);

    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));
    ASSERT_TRUE(m_event_queue.pop(event, s_wait_time));

    EXPECT_EQ(m_created_files[m_file1], 1);
    EXPECT_EQ(m_deleted_files[m_file1], 0);
    EXPECT_EQ(m_changed_files[m_file1], 0);
    EXPECT_EQ(m_other_files[m_file1], 0);

    EXPECT_EQ(m_created_files[m_file2], 1);
    EXPECT_EQ(m_deleted_files[m_file2], 1);
    EXPECT_EQ(m_changed_files[m_file2], 0);
    EXPECT_EQ(m_other_files[m_file2], 0);

    EXPECT_EQ(m_created_files[m_file3], 1);
    EXPECT_EQ(m_deleted_files[m_file3], 1);
    EXPECT_EQ(m_changed_files[m_file3], 1);
    EXPECT_EQ(m_other_files[m_file3], 0);

    EXPECT_EQ(m_created_files[m_file0], 1);
    EXPECT_EQ(m_deleted_files[m_file0], 1);
    EXPECT_EQ(m_changed_files[m_file0], 1);
    EXPECT_EQ(m_other_files[m_file0], 0);
}

//==================================================================================================
TEST_F(PathMonitorTest, Remove)
{
    // Test removing files and paths that were not being monitored.
    EXPECT_FALSE(m_monitor->remove_file(m_path1() / "was not"));
    EXPECT_FALSE(m_monitor->remove_path(m_path1() / "monitoring"));
    EXPECT_FALSE(m_monitor->remove_path("any of this"));

    // For the monitor with two monitored files and a monitored path:
    // 1. Remove one of the files - should succeed.
    // 2. Remove the whole path - should succeed.
    // 3. Remove the second file - should fail, wasn't being monitored any more.
    // 4. Remove the whole path - should fail.
    EXPECT_TRUE(m_monitor->remove_file(m_file1));
    EXPECT_TRUE(m_monitor->remove_path(m_path1()));
    EXPECT_FALSE(m_monitor->remove_file(m_file2));
    EXPECT_FALSE(m_monitor->remove_path(m_path1()));

    // For the monitor with one monitored file and a monitored path:
    // 1. Remove the monitored file - should succeed.
    // 2. Remove the whole path - should succeed.
    EXPECT_TRUE(m_monitor->remove_file(m_file0));
    EXPECT_TRUE(m_monitor->remove_path(m_path0()));

    // For the monitor with one monitored file and no monitored paths:
    // 1. Remove the monitored file - should succeed.
    // 2. Remove the whole path - should fail, path will gets removed when the
    //    last monitored file is removed.
    EXPECT_TRUE(m_monitor->remove_file(m_file3));
    EXPECT_FALSE(m_monitor->remove_path(m_path2()));
}
