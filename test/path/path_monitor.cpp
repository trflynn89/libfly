#include "fly/path/path_monitor.hpp"

#include "test/util/task_manager.hpp"

#include "fly/path/path_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_test_macros.hpp"

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

using namespace fly::literals::numeric_literals;

namespace {

constexpr const char *s_path_monitor_file = "path_monitor.cpp";
const std::chrono::seconds s_wait_time(5);

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

CATCH_TEST_CASE("PathMonitor", "[path]")
{
    auto task_runner = fly::test::WaitableSequencedTaskRunner::create(fly::test::task_manager());

    auto monitor =
        std::make_shared<fly::PathMonitorImpl>(task_runner, std::make_shared<TestPathConfig>());
    CATCH_REQUIRE(monitor->start());

    fly::test::PathUtil::ScopedTempDirectory path0;
    fly::test::PathUtil::ScopedTempDirectory path1;
    fly::test::PathUtil::ScopedTempDirectory path2;

    std::filesystem::path file0 = path0.file();
    std::filesystem::path file1 = path1.file();
    std::filesystem::path file2 = path1.file();
    std::filesystem::path file3 = path2.file();

    fly::ConcurrentQueue<fly::PathMonitor::PathEvent> event_queue;

    std::map<std::filesystem::path, unsigned int> created_files;
    std::map<std::filesystem::path, unsigned int> deleted_files;
    std::map<std::filesystem::path, unsigned int> changed_files;

    auto handle_event = [&](const std::filesystem::path &path, fly::PathMonitor::PathEvent event)
    {
        switch (event)
        {
            case fly::PathMonitor::PathEvent::Created:
                ++created_files[path];
                break;

            case fly::PathMonitor::PathEvent::Deleted:
                ++deleted_files[path];
                break;

            case fly::PathMonitor::PathEvent::Changed:
                ++changed_files[path];
                break;

            default:
                CATCH_FAIL("Unrecognized PathEvent: " << event);
                break;
        }

        event_queue.push(std::move(event));
    };

    CATCH_REQUIRE(monitor->add_path(path0(), handle_event));
    CATCH_REQUIRE(monitor->add_path(path1(), handle_event));

    CATCH_REQUIRE(monitor->add_file(file0, handle_event));
    CATCH_REQUIRE(monitor->add_file(file1, handle_event));
    CATCH_REQUIRE(monitor->add_file(file2, handle_event));
    CATCH_REQUIRE(monitor->add_file(file3, handle_event));

    CATCH_SECTION("Verify streaming of path events")
    {
        {
            std::stringstream stream;
            stream << static_cast<fly::PathMonitor::PathEvent>(-1);
            CATCH_CHECK(stream.str().empty());
        }
        {
            std::stringstream stream;
            stream << fly::PathMonitor::PathEvent::None;
            CATCH_CHECK(stream.str() == "None");
        }
        {
            std::stringstream stream;
            stream << fly::PathMonitor::PathEvent::Created;
            CATCH_CHECK(stream.str() == "Created");
        }
        {
            std::stringstream stream;
            stream << fly::PathMonitor::PathEvent::Deleted;
            CATCH_CHECK(stream.str() == "Deleted");
        }
        {
            std::stringstream stream;
            stream << fly::PathMonitor::PathEvent::Changed;
            CATCH_CHECK(stream.str() == "Changed");
        }
    }

    CATCH_SECTION("Cannot monitor paths that do not exist")
    {
        CATCH_CHECK_FALSE(monitor->add_path(path0() / "_", handle_event));
        CATCH_CHECK_FALSE(monitor->add_file(path0() / "_" / "foo.txt", handle_event));
    }

    CATCH_SECTION("Cannot monitor without a valid callback")
    {
        CATCH_CHECK_FALSE(monitor->add_path(path0(), nullptr));
        CATCH_CHECK_FALSE(monitor->add_file(file1, nullptr));
    }

    CATCH_SECTION("Cannot monitor directories as files, nor files as directories")
    {
        CATCH_CHECK_FALSE(monitor->add_file(path0(), handle_event));
        CATCH_CHECK_FALSE(monitor->add_path(file1, handle_event));
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Cannot start monitor when ::inotify_init1() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::InotifyInit1);

        monitor = std::make_shared<fly::PathMonitorImpl>(
            task_runner,
            std::make_shared<fly::PathConfig>());

        CATCH_CHECK_FALSE(monitor->start());

        CATCH_CHECK_FALSE(monitor->add_path(path0(), handle_event));
        CATCH_CHECK_FALSE(monitor->add_file(file1, handle_event));
    }

    CATCH_SECTION("Cannot monitor paths when ::inotify_add_watch() fails")
    {
        monitor->remove_all_paths();

        fly::test::MockSystem mock(fly::test::MockCall::InotifyAddWatch);

        CATCH_CHECK_FALSE(monitor->add_path(path0(), handle_event));
        CATCH_CHECK_FALSE(monitor->add_file(file1, handle_event));
    }

#endif

    CATCH_SECTION("No events triggered without path changes")
    {
        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files.empty());
        CATCH_CHECK(deleted_files.empty());
        CATCH_CHECK(changed_files.empty());
    }

    CATCH_SECTION("Creating a file issues a PathEvent::Created event")
    {
        fly::PathMonitor::PathEvent event;

        CATCH_CHECK(created_files[file0] == 0);
        CATCH_CHECK(deleted_files[file0] == 0);
        CATCH_CHECK(changed_files[file0] == 0);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file0, std::string()));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_CHECK(created_files[file0] == 1);
        CATCH_CHECK(deleted_files[file0] == 0);
        CATCH_CHECK(changed_files[file0] == 0);
    }

    CATCH_SECTION("Deleting a file issues a PathEvent::Deleted event")
    {
        fly::PathMonitor::PathEvent event;

        CATCH_CHECK(created_files[file0] == 0);
        CATCH_CHECK(deleted_files[file0] == 0);
        CATCH_CHECK(changed_files[file0] == 0);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file0, std::string()));
        std::filesystem::remove(file0);
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_CHECK(created_files[file0] == 1);
        CATCH_CHECK(deleted_files[file0] == 1);
        CATCH_CHECK(changed_files[file0] == 0);
    }

    CATCH_SECTION("Changing a file issues a PathEvent::Changed event")
    {
        fly::PathMonitor::PathEvent event;

        CATCH_CHECK(created_files[file0] == 0);
        CATCH_CHECK(deleted_files[file0] == 0);
        CATCH_CHECK(changed_files[file0] == 0);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file0, "abcdefghi"));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_CHECK(created_files[file0] == 1);
        CATCH_CHECK(deleted_files[file0] == 0);
        CATCH_CHECK(changed_files[file0] == 1);
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Cannot poll monitor when ::poll() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Poll);
        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file1, "abcdefghi"));
        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);
    }

    CATCH_SECTION("Cannot poll monitor when ::read() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Read);
        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file1, "abcdefghi"));
        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);
    }

#endif

    CATCH_SECTION("Unmonitored do not issue any event")
    {
        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);

        auto path = std::filesystem::path(file1).concat(".diff");
        CATCH_REQUIRE(fly::test::PathUtil::write_file(path.string(), "abcdefghi"));

        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);

        path = std::filesystem::path(path.string().substr(0, path.string().length() - 8));
        CATCH_REQUIRE(fly::test::PathUtil::write_file(path, "abcdefghi"));

        task_runner->wait_for_task_to_complete(s_path_monitor_file);

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);
    }

    CATCH_SECTION("Monitor can handle many events")
    {
        fly::PathMonitor::PathEvent event;

        CATCH_CHECK(created_files[file1] == 0);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);

        CATCH_CHECK(created_files[file2] == 0);
        CATCH_CHECK(deleted_files[file2] == 0);
        CATCH_CHECK(changed_files[file2] == 0);

        CATCH_CHECK(created_files[file3] == 0);
        CATCH_CHECK(deleted_files[file3] == 0);
        CATCH_CHECK(changed_files[file3] == 0);

        CATCH_CHECK(created_files[file0] == 0);
        CATCH_CHECK(deleted_files[file0] == 0);
        CATCH_CHECK(changed_files[file0] == 0);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file1, std::string()));

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file2, std::string()));
        std::filesystem::remove(file2);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file3, "abcdefghi"));
        std::filesystem::remove(file3);

        CATCH_REQUIRE(fly::test::PathUtil::write_file(file0, "abcdefghi"));
        std::filesystem::remove(file0);

        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));
        CATCH_REQUIRE(event_queue.pop(event, s_wait_time));

        CATCH_CHECK(created_files[file1] == 1);
        CATCH_CHECK(deleted_files[file1] == 0);
        CATCH_CHECK(changed_files[file1] == 0);

        CATCH_CHECK(created_files[file2] == 1);
        CATCH_CHECK(deleted_files[file2] == 1);
        CATCH_CHECK(changed_files[file2] == 0);

        CATCH_CHECK(created_files[file3] == 1);
        CATCH_CHECK(deleted_files[file3] == 1);
        CATCH_CHECK(changed_files[file3] == 1);

        CATCH_CHECK(created_files[file0] == 1);
        CATCH_CHECK(deleted_files[file0] == 1);
        CATCH_CHECK(changed_files[file0] == 1);
    }

    CATCH_SECTION("Edge case removal of paths")
    {
        // Test removing files and paths that were not being monitored.
        CATCH_CHECK_FALSE(monitor->remove_file(path1() / "was not"));
        CATCH_CHECK_FALSE(monitor->remove_path(path1() / "monitoring"));
        CATCH_CHECK_FALSE(monitor->remove_path("any of this"));

        // For the monitor with two monitored files and a monitored path:
        // 1. Remove one of the files - should succeed.
        // 2. Remove the whole path - should succeed.
        // 3. Remove the second file - should fail, wasn't being monitored any more.
        // 4. Remove the whole path - should fail.
        CATCH_CHECK(monitor->remove_file(file1));
        CATCH_CHECK(monitor->remove_path(path1()));
        CATCH_CHECK_FALSE(monitor->remove_file(file2));
        CATCH_CHECK_FALSE(monitor->remove_path(path1()));

        // For the monitor with one monitored file and a monitored path:
        // 1. Remove the monitored file - should succeed.
        // 2. Remove the whole path - should succeed.
        CATCH_CHECK(monitor->remove_file(file0));
        CATCH_CHECK(monitor->remove_path(path0()));

        // For the monitor with one monitored file and no monitored paths:
        // 1. Remove the monitored file - should succeed.
        // 2. Remove the whole path - should fail, path will gets removed when the last monitored
        //    file is removed.
        CATCH_CHECK(monitor->remove_file(file3));
        CATCH_CHECK_FALSE(monitor->remove_path(path2()));
    }

    monitor->remove_all_paths();
}
