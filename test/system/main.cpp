#include <csignal>
#include <functional>
#include <future>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/config/config_manager.h"
#include "fly/system/system.h"
#include "fly/system/system_monitor.h"

#ifdef FLY_LINUX
    #include "test/mock/mock_system.h"
#endif

namespace
{
    static int s_lastSignal = 0;

    void handleSignal(int signal)
    {
        s_lastSignal = signal;
    }
}

//==============================================================================
TEST(SystemTest, PrintBacktraceTest)
{
    fly::System::PrintBacktrace();
}

//==============================================================================
TEST(SystemTest, LocalTimeTest)
{
    std::string time = fly::System::LocalTime();
    EXPECT_FALSE(time.empty());
}

//==============================================================================
TEST(SystemTest, ErrorCodeTest)
{
    int code = fly::System::GetErrorCode();

    std::string error1 = fly::System::GetErrorString();
    std::string error2 = fly::System::GetErrorString(code);

    EXPECT_FALSE(error1.empty());
    EXPECT_FALSE(error2.empty());
    EXPECT_EQ(error1, error2);
}

//==============================================================================
TEST(SystemTest, SignalTest)
{
    fly::System::SignalHandler handler(&handleSignal);
    fly::System::SetSignalHandler(handler);

    std::raise(SIGINT);
    EXPECT_EQ(s_lastSignal, SIGINT);

    std::raise(SIGSEGV);
    EXPECT_EQ(s_lastSignal, SIGSEGV);

    fly::System::SetSignalHandler(nullptr);
}

//==============================================================================
class SystemMonitorTest : public ::testing::Test
{
public:
    SystemMonitorTest() :
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::CONFIG_TYPE_INI, std::string(), std::string()
        )),

        m_spMonitor(std::make_shared<fly::SystemMonitorImpl>(m_spConfigManager)),
        m_aKeepRunning(true)
    {
    }

    /**
     * Create and start the system monitor.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(m_spMonitor && m_spMonitor->Start());

        // Give the monitor a bit of time to acquire initial system values
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    /**
     * Stop the system monitor.
     */
    virtual void TearDown()
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
        fly::MockSystem mock(fly::MockCall::READ);
        m_spMonitor->Stop();

        m_spMonitor = std::make_shared<fly::SystemMonitorImpl>(m_spConfigManager);

        ASSERT_FALSE(m_spMonitor->Start());
        ASSERT_EQ(m_spMonitor->GetSystemCpuCount(), 0);
    }

    {
        TearDown();
        SetUp();

        fly::MockSystem mock1(fly::MockCall::READ);
        fly::MockSystem mock2(fly::MockCall::TIMES);
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

    std::string consumed((totalBefore - systemBefore) / 10, '\0');
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
    fly::MockSystem mock1(fly::MockCall::SYSINFO);
    fly::MockSystem mock2(fly::MockCall::READ);

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
