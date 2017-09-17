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
    std::future<void> result = std::async(
        std::launch::async, &SystemMonitorTest::SpinThread, this
    );

    std::this_thread::sleep_for(std::chrono::seconds(3));
    m_aKeepRunning.store(false);

    ASSERT_GT(m_spMonitor->GetCpuUsage(), 0.0);

    ASSERT_TRUE(result.valid());
    result.get();
}

//==============================================================================
TEST_F(SystemMonitorTest, MemoryUsageTest)
{
    std::this_thread::sleep_for(std::chrono::seconds(3));

    uint64_t totalBefore = m_spMonitor->GetTotalMemory();
    uint64_t freeBefore = m_spMonitor->GetFreeMemory();
    uint64_t processBefore = m_spMonitor->GetProcessMemory();

    std::string consumed(freeBefore / 10, '\0');
    std::this_thread::sleep_for(std::chrono::seconds(3));

    uint64_t totalAfter = m_spMonitor->GetTotalMemory();
    uint64_t freeAfter = m_spMonitor->GetFreeMemory();
    uint64_t processAfter = m_spMonitor->GetProcessMemory();

    ASSERT_EQ(totalBefore, totalAfter);
    ASSERT_GT(freeBefore, freeAfter);
    ASSERT_LT(processBefore, processAfter);
}
