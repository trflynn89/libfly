#include <csignal>
#include <functional>
#include <future>
#include <string>
#include <thread>

#include <gtest/gtest.h>

#include "fly/system/system.h"
#include "fly/system/system_monitor_impl.h"

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
        m_spMonitor(std::make_shared<fly::SystemMonitorImpl>()),
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
    fly::SystemMonitorPtr m_spMonitor;
    std::atomic_bool m_aKeepRunning;
};

//==============================================================================
TEST_F(SystemMonitorTest, CpuUsageTest)
{
    ASSERT_GT(m_spMonitor->GetSystemCpuCount(), 0);

    double systemBefore = m_spMonitor->GetSystemCpuUsage();
    double processBefore = m_spMonitor->GetProcessCpuUsage();

    std::future<void> result = std::async(
        std::launch::async, &SystemMonitorTest::SpinThread, this
    );

    std::this_thread::sleep_for(std::chrono::seconds(3));

    double systemAfter = m_spMonitor->GetSystemCpuUsage();
    double processAfter = m_spMonitor->GetProcessCpuUsage();

    m_aKeepRunning.store(false);
    ASSERT_TRUE(result.valid());
    result.get();

    ASSERT_LT(systemBefore, systemAfter);
    ASSERT_LT(processBefore, processAfter);
}

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
    ASSERT_LT(systemBefore, systemAfter);
    ASSERT_LT(processBefore, processAfter);
}
