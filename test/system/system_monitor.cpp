#include "fly/system/system_monitor.h"

#include "fly/literals.h"
#include "fly/system/system_config.h"
#include "fly/task/task_manager.h"

#include <gtest/gtest.h>

#include <functional>
#include <future>
#include <memory>
#include <string>
#include <thread>

#if defined(FLY_LINUX)
#    include "test/mock/mock_system.h"
#endif

#include "test/util/waitable_task_runner.h"

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
        m_defaultPollInterval = 100_i64;
    }
};

} // namespace

//==============================================================================
class SystemMonitorTest : public ::testing::Test
{
public:
    SystemMonitorTest() noexcept :
        m_spTaskManager(std::make_shared<fly::TaskManager>(1)),

        m_spTaskRunner(
            m_spTaskManager
                ->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()),

        m_spMonitor(std::make_shared<fly::SystemMonitorImpl>(
            m_spTaskRunner,
            std::make_shared<TestSystemConfig>())),

        m_aKeepRunning(true)
    {
    }

    /**
     * Start the task manager and system monitor.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(m_spTaskManager->Start());
        ASSERT_TRUE(m_spMonitor->Start());
        m_spTaskRunner->WaitForTaskTypeToComplete<fly::SystemMonitorTask>();
    }

    /**
     * Stop the task manager.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_spTaskManager->Stop());
    }

    /**
     * Thread to spin infinitely until signaled to stop.
     */
    void SpinThread() noexcept
    {
        while (m_aKeepRunning.load())
        {
        }
    }

protected:
    std::shared_ptr<fly::TaskManager> m_spTaskManager;
    std::shared_ptr<fly::WaitableSequencedTaskRunner> m_spTaskRunner;

    std::shared_ptr<fly::SystemMonitor> m_spMonitor;
    std::atomic_bool m_aKeepRunning;
};

//==============================================================================
TEST_F(SystemMonitorTest, CpuUsageTest)
{
    std::uint32_t countBefore = m_spMonitor->GetSystemCpuCount();
    double processBefore = m_spMonitor->GetProcessCpuUsage();

    std::future<void> result =
        std::async(std::launch::async, &SystemMonitorTest::SpinThread, this);

    m_spTaskRunner->WaitForTaskTypeToComplete<fly::SystemMonitorTask>();

    std::uint32_t countAfter = m_spMonitor->GetSystemCpuCount();
    double systemAfter = m_spMonitor->GetSystemCpuUsage();
    double processAfter = m_spMonitor->GetProcessCpuUsage();

    m_aKeepRunning.store(false);
    ASSERT_TRUE(result.valid());
    result.get();

    ASSERT_EQ(countBefore, countAfter);
    ASSERT_GT(systemAfter, 0_u64);
    ASSERT_LT(processBefore, processAfter);
}

#if defined(FLY_LINUX)

//==============================================================================
TEST_F(SystemMonitorTest, MockCpuUsageTest)
{
    {
        fly::MockSystem mock(fly::MockCall::Read);

        m_spMonitor = std::make_shared<fly::SystemMonitorImpl>(
            m_spTaskRunner, std::make_shared<fly::SystemConfig>());

        ASSERT_FALSE(m_spMonitor->Start());
        ASSERT_EQ(m_spMonitor->GetSystemCpuCount(), 0);
    }

    {
        m_spMonitor = std::make_shared<fly::SystemMonitorImpl>(
            m_spTaskRunner, std::make_shared<fly::SystemConfig>());

        ASSERT_TRUE(m_spMonitor->Start());
        m_spTaskRunner->WaitForTaskTypeToComplete<fly::SystemMonitorTask>();

        fly::MockSystem mock1(fly::MockCall::Read);
        fly::MockSystem mock2(fly::MockCall::Times);

        double systemBefore = m_spMonitor->GetSystemCpuUsage();
        double processBefore = m_spMonitor->GetProcessCpuUsage();

        std::future<void> result = std::async(
            std::launch::async, &SystemMonitorTest::SpinThread, this);

        m_spTaskRunner->WaitForTaskTypeToComplete<fly::SystemMonitorTask>();

        double systemAfter = m_spMonitor->GetSystemCpuUsage();
        double processAfter = m_spMonitor->GetProcessCpuUsage();

        m_aKeepRunning.store(false);
        ASSERT_TRUE(result.valid());
        result.get();

        ASSERT_EQ(systemBefore, systemAfter);
        ASSERT_EQ(processBefore, processAfter);
    }
}

#endif

//==============================================================================
TEST_F(SystemMonitorTest, MemoryUsageTest)
{
    std::uint64_t totalBefore = m_spMonitor->GetTotalSystemMemory();
    std::uint64_t systemBefore = m_spMonitor->GetSystemMemoryUsage();
    std::uint64_t processBefore = m_spMonitor->GetProcessMemoryUsage();

    auto size =
        static_cast<std::string::size_type>((totalBefore - systemBefore) / 10);

    std::string consumed(size, '\0');
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::SystemMonitorTask>();

    std::uint64_t totalAfter = m_spMonitor->GetTotalSystemMemory();
    std::uint64_t systemAfter = m_spMonitor->GetSystemMemoryUsage();
    std::uint64_t processAfter = m_spMonitor->GetProcessMemoryUsage();

    ASSERT_EQ(totalBefore, totalAfter);
    ASSERT_GT(systemBefore, 0_u64);
    ASSERT_GT(systemAfter, 0_u64);
    ASSERT_LT(processBefore, processAfter);
}

#if defined(FLY_LINUX)

//==============================================================================
TEST_F(SystemMonitorTest, MockMemoryUsageTest)
{
    fly::MockSystem mock1(fly::MockCall::Sysinfo);
    fly::MockSystem mock2(fly::MockCall::Read);

    std::uint64_t totalBefore = m_spMonitor->GetTotalSystemMemory();
    std::uint64_t systemBefore = m_spMonitor->GetSystemMemoryUsage();
    std::uint64_t processBefore = m_spMonitor->GetProcessMemoryUsage();

    std::string consumed((totalBefore - systemBefore) / 10, '\0');
    m_spTaskRunner->WaitForTaskTypeToComplete<fly::SystemMonitorTask>();

    std::uint64_t totalAfter = m_spMonitor->GetTotalSystemMemory();
    std::uint64_t systemAfter = m_spMonitor->GetSystemMemoryUsage();
    std::uint64_t processAfter = m_spMonitor->GetProcessMemoryUsage();

    ASSERT_EQ(totalBefore, totalAfter);
    ASSERT_EQ(systemBefore, systemAfter);
    ASSERT_EQ(processBefore, processAfter);
}

#endif
