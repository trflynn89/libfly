#include <functional>
#include <future>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/config/config_manager.h"
#include "fly/system/system_monitor.h"

#ifdef FLY_LINUX
    #include "test/mock/mock_system.h"
#endif

//==============================================================================
class SystemMonitorTest : public ::testing::Test
{
public:
    SystemMonitorTest() :
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::ConfigFileType::Ini, std::string(), std::string()
        )),

        m_spMonitor(std::make_shared<fly::SystemMonitorImpl>(m_spConfigManager)),
        m_aKeepRunning(true)
    {
    }

    /**
     * Create and start the system monitor.
     */
    void SetUp() override
    {
        ASSERT_TRUE(m_spMonitor && m_spMonitor->Start());

        // Give the monitor a bit of time to acquire initial system values
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    /**
     * Stop the system monitor.
     */
    void TearDown() override
    {
        m_spMonitor->Stop();
    }

    /**
     * Thread to spin infinitely until signaled to stop.
     */
    void SpinThread()
    {
        while (m_aKeepRunning.load())
        {
        }
    }

protected:
    fly::ConfigManagerPtr m_spConfigManager;

    fly::SystemMonitorPtr m_spMonitor;
    std::atomic_bool m_aKeepRunning;
};

//==============================================================================
TEST_F(SystemMonitorTest, CpuUsageTest)
{
    uint32_t countBefore = m_spMonitor->GetSystemCpuCount();
    double processBefore = m_spMonitor->GetProcessCpuUsage();

    std::future<void> result = std::async(
        std::launch::async, &SystemMonitorTest::SpinThread, this
    );

    std::this_thread::sleep_for(std::chrono::seconds(5));

    uint32_t countAfter = m_spMonitor->GetSystemCpuCount();
    double systemAfter = m_spMonitor->GetSystemCpuUsage();
    double processAfter = m_spMonitor->GetProcessCpuUsage();

    m_aKeepRunning.store(false);
    ASSERT_TRUE(result.valid());
    result.get();

    ASSERT_EQ(countBefore, countAfter);
    ASSERT_GT(systemAfter, U64(0));
    ASSERT_LT(processBefore, processAfter);
}

#ifdef FLY_LINUX

//==============================================================================
TEST_F(SystemMonitorTest, MockCpuUsageTest)
{
    {
        fly::MockSystem mock(fly::MockCall::Read);
        m_spMonitor->Stop();

        m_spMonitor = std::make_shared<fly::SystemMonitorImpl>(m_spConfigManager);

        ASSERT_FALSE(m_spMonitor->Start());
        ASSERT_EQ(m_spMonitor->GetSystemCpuCount(), 0);
    }

    {
        TearDown();
        SetUp();

        fly::MockSystem mock1(fly::MockCall::Read);
        fly::MockSystem mock2(fly::MockCall::Times);
        std::this_thread::sleep_for(std::chrono::seconds(3));

        double systemBefore = m_spMonitor->GetSystemCpuUsage();
        double processBefore = m_spMonitor->GetProcessCpuUsage();

        std::future<void> result = std::async(
            std::launch::async, &SystemMonitorTest::SpinThread, this
        );

        std::this_thread::sleep_for(std::chrono::seconds(5));

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
    uint64_t totalBefore = m_spMonitor->GetTotalSystemMemory();
    uint64_t systemBefore = m_spMonitor->GetSystemMemoryUsage();
    uint64_t processBefore = m_spMonitor->GetProcessMemoryUsage();

    auto size = static_cast<std::string::size_type>(
        (totalBefore - systemBefore) / 10
    );

    std::string consumed(size, '\0');
    std::this_thread::sleep_for(std::chrono::seconds(3));

    uint64_t totalAfter = m_spMonitor->GetTotalSystemMemory();
    uint64_t systemAfter = m_spMonitor->GetSystemMemoryUsage();
    uint64_t processAfter = m_spMonitor->GetProcessMemoryUsage();

    ASSERT_EQ(totalBefore, totalAfter);
    ASSERT_GT(systemBefore, U64(0));
    ASSERT_GT(systemAfter, U64(0));
    ASSERT_LT(processBefore, processAfter);
}

#ifdef FLY_LINUX

//==============================================================================
TEST_F(SystemMonitorTest, MockMemoryUsageTest)
{
    fly::MockSystem mock1(fly::MockCall::Sysinfo);
    fly::MockSystem mock2(fly::MockCall::Read);

    uint64_t totalBefore = m_spMonitor->GetTotalSystemMemory();
    uint64_t systemBefore = m_spMonitor->GetSystemMemoryUsage();
    uint64_t processBefore = m_spMonitor->GetProcessMemoryUsage();

    std::string consumed((totalBefore - systemBefore) / 10, '\0');
    std::this_thread::sleep_for(std::chrono::seconds(3));

    uint64_t totalAfter = m_spMonitor->GetTotalSystemMemory();
    uint64_t systemAfter = m_spMonitor->GetSystemMemoryUsage();
    uint64_t processAfter = m_spMonitor->GetProcessMemoryUsage();

    ASSERT_EQ(totalBefore, totalAfter);
    ASSERT_EQ(systemBefore, systemAfter);
    ASSERT_EQ(processBefore, processAfter);
}

#endif
