#include "fly/system/system_monitor.hpp"

#include "fly/system/system_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/numeric/literals.hpp"
#include "test/util/task_manager.hpp"

#include <catch2/catch.hpp>

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

constexpr const char *s_system_monitor_file = "system_monitor.cpp";

/**
 * Subclass of the system config to decrease the poll interval for faster testing.
 */
class TestSystemConfig : public fly::SystemConfig
{
public:
    TestSystemConfig() noexcept : fly::SystemConfig()
    {
        m_default_poll_interval = 100_i64;
    }
};

} // namespace

TEST_CASE("SystemMonitor", "[system]")
{
    auto task_runner =
        fly::test::task_manager()->create_task_runner<fly::test::WaitableSequencedTaskRunner>();

    auto monitor =
        std::make_shared<fly::SystemMonitorImpl>(task_runner, std::make_shared<TestSystemConfig>());
    REQUIRE(monitor->start());

    // Wait for one poll to complete before proceeding.
    task_runner->wait_for_task_to_complete(s_system_monitor_file);

    // Thread to spin indefinitely until signaled to stop.
    std::atomic_bool keep_running(true);

    auto spin_thread = [&keep_running]() {
        while (keep_running.load())
        {
        }
    };

    SECTION("Validate CPU usage increased while running a spin thread")
    {
        std::uint32_t count_before = monitor->get_system_cpu_count();
        double process_before = monitor->get_process_cpu_usage();

        std::future<void> result = std::async(std::launch::async, spin_thread);
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint32_t count_after = monitor->get_system_cpu_count();
        double system_after = monitor->get_system_cpu_usage();
        double process_after = monitor->get_process_cpu_usage();

        keep_running.store(false);
        REQUIRE(result.valid());
        result.get();

        CHECK(count_before == count_after);
        CHECK(system_after > 0_u64);
        CHECK(process_before < process_after);
    }

#if defined(FLY_LINUX)

    SECTION("Cannot start system manager when ::read() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Read);

        monitor = std::make_shared<fly::SystemMonitorImpl>(
            task_runner,
            std::make_shared<fly::SystemConfig>());

        CHECK_FALSE(monitor->start());
        CHECK(monitor->get_system_cpu_count() == 0);
    }

    SECTION("Cannot update system CPU when ::read() fails")
    {
        monitor = std::make_shared<fly::SystemMonitorImpl>(
            task_runner,
            std::make_shared<fly::SystemConfig>());

        CHECK(monitor->start());
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        fly::test::MockSystem mock(fly::test::MockCall::Read);

        double system_before = monitor->get_system_cpu_usage();

        std::future<void> result = std::async(std::launch::async, spin_thread);
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        double system_after = monitor->get_system_cpu_usage();

        keep_running.store(false);
        REQUIRE(result.valid());
        result.get();

        CHECK(system_before == Approx(system_after));
    }

    SECTION("Cannot update process CPU when ::times() fails")
    {
        monitor = std::make_shared<fly::SystemMonitorImpl>(
            task_runner,
            std::make_shared<fly::SystemConfig>());

        CHECK(monitor->start());
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        fly::test::MockSystem mock(fly::test::MockCall::Times);

        double process_before = monitor->get_process_cpu_usage();

        std::future<void> result = std::async(std::launch::async, spin_thread);
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        double process_after = monitor->get_process_cpu_usage();

        keep_running.store(false);
        REQUIRE(result.valid());
        result.get();

        CHECK(process_before == Approx(process_after));
    }

#endif

    SECTION("Validate memory usage increased after allocating a large block")
    {
        std::uint64_t total_before = monitor->get_total_system_memory();
        std::uint64_t system_before = monitor->get_system_memory_usage();
        std::uint64_t process_before = monitor->get_process_memory_usage();

        auto size = static_cast<std::string::size_type>((total_before - system_before) / 10);

        std::string consumed(size, '\0');
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint64_t total_after = monitor->get_total_system_memory();
        std::uint64_t system_after = monitor->get_system_memory_usage();
        std::uint64_t process_after = monitor->get_process_memory_usage();

        CHECK(total_before == total_after);
        CHECK(system_before > 0_u64);
        CHECK(system_after > 0_u64);
        CHECK(process_before < process_after);
    }

#if defined(FLY_LINUX)

    SECTION("Cannot update system memory when ::sysinfo() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Sysinfo);

        std::uint64_t total_before = monitor->get_total_system_memory();
        std::uint64_t system_before = monitor->get_system_memory_usage();

        std::string consumed(1 << 10, '\0');
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint64_t total_after = monitor->get_total_system_memory();
        std::uint64_t system_after = monitor->get_system_memory_usage();

        CHECK(total_before == total_after);
        CHECK(system_before == system_after);
    }

    SECTION("Cannot update process memory when ::read() fails")
    {
        fly::test::MockSystem mock(fly::test::MockCall::Read);

        std::uint64_t process_before = monitor->get_process_memory_usage();

        std::string consumed(1 << 10, '\0');
        task_runner->wait_for_task_to_complete(s_system_monitor_file);

        std::uint64_t process_after = monitor->get_process_memory_usage();

        CHECK(process_before == process_after);
    }

#endif
}
