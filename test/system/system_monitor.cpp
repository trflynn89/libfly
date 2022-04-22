#include "fly/system/system_monitor.hpp"

#include "test/util/task_manager.hpp"

#include "fly/system/system_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/numeric/literals.hpp"

#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include "test/util/waitable_task_runner.hpp"

using namespace fly::literals::numeric_literals;

namespace {

constexpr char const *s_system_monitor_file = "system_monitor.cpp";

/**
 * Subclass of the system config to decrease the poll interval for faster testing.
 */
class TestSystemConfig : public fly::system::SystemConfig
{
public:
    TestSystemConfig() noexcept :
        fly::system::SystemConfig()
    {
        m_default_poll_interval = 100_i64;
    }
};

} // namespace

CATCH_TEST_CASE("SystemMonitor", "[system]")
{
    auto config = std::make_shared<TestSystemConfig>();
    auto task_runner = fly::test::WaitableSequencedTaskRunner::create(fly::test::task_manager());
    auto monitor = fly::system::SystemMonitor::create(task_runner, config);
    CATCH_REQUIRE(monitor);

    // Wait for one poll to complete before proceeding.
    task_runner->wait_for_task_to_complete(s_system_monitor_file);

    // Thread to spin indefinitely until signaled to stop.
    std::atomic_bool keep_running(true);

    auto spin_thread = [&keep_running]() {
        while (keep_running.load())
        {
        }
    };

    CATCH_SECTION("Validate CPU usage increased while running a spin thread")
    {
        std::uint32_t count_before = monitor->get_system_cpu_count();
        double process_before = monitor->get_process_cpu_usage();

        std::future<void> result = std::async(std::launch::async, spin_thread);
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint32_t count_after = monitor->get_system_cpu_count();
        double system_after = monitor->get_system_cpu_usage();
        double process_after = monitor->get_process_cpu_usage();

        keep_running.store(false);
        CATCH_REQUIRE(result.valid());
        result.get();

        CATCH_CHECK(count_before == count_after);
        CATCH_CHECK(system_after > 0_u64);
        CATCH_CHECK(process_before < process_after);
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Cannot start system manager when ::read() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Read);

        monitor = fly::system::SystemMonitor::create(task_runner, config);
        CATCH_CHECK_FALSE(monitor);
    }

    CATCH_SECTION("Cannot update system CPU when ::read() fails")
    {
        monitor = fly::system::SystemMonitor::create(task_runner, config);
        CATCH_REQUIRE(monitor);

        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        fly::test::MockSystem mock(fly::test::MockCall::Read);

        double system_before = monitor->get_system_cpu_usage();

        std::future<void> result = std::async(std::launch::async, spin_thread);
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        double system_after = monitor->get_system_cpu_usage();

        keep_running.store(false);
        CATCH_REQUIRE(result.valid());
        result.get();

        CATCH_CHECK(system_before == Catch::Approx(system_after));
    }

    CATCH_SECTION("Cannot update process CPU when ::times() fails")
    {
        monitor = fly::system::SystemMonitor::create(task_runner, config);
        CATCH_REQUIRE(monitor);

        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        fly::test::MockSystem mock(fly::test::MockCall::Times);

        double process_before = monitor->get_process_cpu_usage();

        std::future<void> result = std::async(std::launch::async, spin_thread);
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        double process_after = monitor->get_process_cpu_usage();

        keep_running.store(false);
        CATCH_REQUIRE(result.valid());
        result.get();

        CATCH_CHECK(process_before == Catch::Approx(process_after));
    }

#endif

    CATCH_SECTION("Validate memory usage increased after allocating a large block")
    {
        std::uint64_t total_before = monitor->get_total_system_memory();
        std::uint64_t system_before = monitor->get_system_memory_usage();
        std::uint64_t process_before = monitor->get_process_memory_usage();

        std::string consumed(4 << 20, '\0');
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint64_t total_after = monitor->get_total_system_memory();
        std::uint64_t system_after = monitor->get_system_memory_usage();
        std::uint64_t process_after = monitor->get_process_memory_usage();

        CATCH_CHECK(total_before == total_after);
        CATCH_CHECK(system_before > 0_u64);
        CATCH_CHECK(system_after > 0_u64);
        CATCH_CHECK(process_before < process_after);
    }

#if defined(FLY_LINUX)

    CATCH_SECTION("Cannot update system memory when ::sysinfo() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sysinfo);

        std::uint64_t total_before = monitor->get_total_system_memory();
        std::uint64_t system_before = monitor->get_system_memory_usage();

        std::string consumed(1 << 10, '\0');
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint64_t total_after = monitor->get_total_system_memory();
        std::uint64_t system_after = monitor->get_system_memory_usage();

        CATCH_CHECK(total_before == total_after);
        CATCH_CHECK(system_before == system_after);
    }

    CATCH_SECTION("Cannot update process memory when ::read() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Read);

        std::uint64_t process_before = monitor->get_process_memory_usage();

        std::string consumed(1 << 10, '\0');
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint64_t process_after = monitor->get_process_memory_usage();

        CATCH_CHECK(process_before == process_after);
    }

#endif
}
