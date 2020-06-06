#include "fly/system/system_monitor.hpp"

#include "fly/system/system_config.hpp"
#include "fly/task/task_manager.hpp"
#include "fly/types/numeric/literals.hpp"

#include <gtest/gtest.h>

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.hpp"
#endif

#include "test/util/waitable_task_runner.hpp"

namespace {

/**
 * Subclass of the system config to decrease the poll interval for faster
 * testing.
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

//==================================================================================================
class SystemMonitorTest : public ::testing::Test
{
public:
    SystemMonitorTest() noexcept :
        m_task_manager(std::make_shared<fly::TaskManager>(1)),

        m_task_runner(m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>()),

        m_monitor(std::make_shared<fly::SystemMonitorImpl>(
            m_task_runner,
            std::make_shared<TestSystemConfig>())),

        m_keep_running(true)
    {
    }

    /**
     * Start the task manager and system monitor.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(m_task_manager->start());
        ASSERT_TRUE(m_monitor->start());
        m_task_runner->wait_for_task_to_complete<fly::SystemMonitorTask>();
    }

    /**
     * Stop the task manager.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_task_manager->stop());
    }

    /**
     * Thread to spin infinitely until signaled to stop.
     */
    void spin_thread() noexcept
    {
        while (m_keep_running.load())
        {
        }
    }

protected:
    std::shared_ptr<fly::TaskManager> m_task_manager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_task_runner;

    std::shared_ptr<fly::SystemMonitor> m_monitor;
    std::atomic_bool m_keep_running;
};

//==================================================================================================
TEST_F(SystemMonitorTest, CpuUsage)
{
    std::uint32_t count_before = m_monitor->get_system_cpu_count();
    double process_before = m_monitor->get_process_cpu_usage();

    std::future<void> result =
        std::async(std::launch::async, &SystemMonitorTest::spin_thread, this);

    m_task_runner->wait_for_task_to_complete<fly::SystemMonitorTask>();

    std::uint32_t count_after = m_monitor->get_system_cpu_count();
    double system_after = m_monitor->get_system_cpu_usage();
    double process_after = m_monitor->get_process_cpu_usage();

    m_keep_running.store(false);
    ASSERT_TRUE(result.valid());
    result.get();

    ASSERT_EQ(count_before, count_after);
    ASSERT_GT(system_after, 0_u64);
    ASSERT_LT(process_before, process_after);
}

#if defined(FLY_LINUX)

//==================================================================================================
TEST_F(SystemMonitorTest, MockCpuUsage)
{
    {
        fly::MockSystem mock(fly::MockCall::Read);

        m_monitor = std::make_shared<fly::SystemMonitorImpl>(
            m_task_runner,
            std::make_shared<fly::SystemConfig>());

        ASSERT_FALSE(m_monitor->start());
        ASSERT_EQ(m_monitor->get_system_cpu_count(), 0);
    }

    {
        m_monitor = std::make_shared<fly::SystemMonitorImpl>(
            m_task_runner,
            std::make_shared<fly::SystemConfig>());

        ASSERT_TRUE(m_monitor->start());
        m_task_runner->wait_for_task_to_complete<fly::SystemMonitorTask>();

        fly::MockSystem mock1(fly::MockCall::Read);
        fly::MockSystem mock2(fly::MockCall::Times);

        double system_before = m_monitor->get_system_cpu_usage();
        double process_before = m_monitor->get_process_cpu_usage();

        std::future<void> result =
            std::async(std::launch::async, &SystemMonitorTest::spin_thread, this);

        m_task_runner->wait_for_task_to_complete<fly::SystemMonitorTask>();

        double system_after = m_monitor->get_system_cpu_usage();
        double process_after = m_monitor->get_process_cpu_usage();

        m_keep_running.store(false);
        ASSERT_TRUE(result.valid());
        result.get();

        ASSERT_EQ(system_before, system_after);
        ASSERT_EQ(process_before, process_after);
    }
}

#endif

//==================================================================================================
TEST_F(SystemMonitorTest, MemoryUsage)
{
    std::uint64_t total_before = m_monitor->get_total_system_memory();
    std::uint64_t system_before = m_monitor->get_system_memory_usage();
    std::uint64_t process_before = m_monitor->get_process_memory_usage();

    auto size = static_cast<std::string::size_type>((total_before - system_before) / 10);

    std::string consumed(size, '\0');
    m_task_runner->wait_for_task_to_complete<fly::SystemMonitorTask>();

    std::uint64_t total_after = m_monitor->get_total_system_memory();
    std::uint64_t system_after = m_monitor->get_system_memory_usage();
    std::uint64_t process_after = m_monitor->get_process_memory_usage();

    ASSERT_EQ(total_before, total_after);
    ASSERT_GT(system_before, 0_u64);
    ASSERT_GT(system_after, 0_u64);
    ASSERT_LT(process_before, process_after);
}

#if defined(FLY_LINUX)

//==================================================================================================
TEST_F(SystemMonitorTest, MockMemoryUsage)
{
    fly::MockSystem mock1(fly::MockCall::Sysinfo);
    fly::MockSystem mock2(fly::MockCall::Read);

    std::uint64_t total_before = m_monitor->get_total_system_memory();
    std::uint64_t system_before = m_monitor->get_system_memory_usage();
    std::uint64_t process_before = m_monitor->get_process_memory_usage();

    std::string consumed((total_before - system_before) / 10, '\0');
    m_task_runner->wait_for_task_to_complete<fly::SystemMonitorTask>();

    std::uint64_t total_after = m_monitor->get_total_system_memory();
    std::uint64_t system_after = m_monitor->get_system_memory_usage();
    std::uint64_t process_after = m_monitor->get_process_memory_usage();

    ASSERT_EQ(total_before, total_after);
    ASSERT_EQ(system_before, system_after);
    ASSERT_EQ(process_before, process_after);
}

#endif
