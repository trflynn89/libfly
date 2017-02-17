#include <atomic>
#include <chrono>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/config/config_manager.h"
#include "fly/task/runner.h"

DEFINE_CLASS_PTRS(CountTask);

//==============================================================================
class CountTask : public fly::Runner
{
public:
    CountTask(fly::ConfigManagerPtr &spConfigManager, bool run) :
        Runner(spConfigManager, "CountTask"),
        m_callCount(0),
        m_run(run)
    {
    }

    unsigned int GetCallCount() const
    {
        return m_callCount.load();
    }

protected:
    virtual bool StartRunner()
    {
        return m_run;
    }

    virtual void StopRunner()
    {
    }

    virtual bool DoWork()
    {
        ++m_callCount;

        std::this_thread::sleep_for(std::chrono::seconds(1));
        return true;
    }

private:
    std::atomic_uint m_callCount;
    bool m_run;
};

//==============================================================================
class RunnerTest : public ::testing::Test
{
public:
    RunnerTest() :
        m_spConfigManager(std::make_shared<fly::ConfigManager>(
            fly::ConfigManager::CONFIG_TYPE_INI, "", ""
        )),
        m_spTask1(std::make_shared<CountTask>(m_spConfigManager, true)),
        m_spTask2(std::make_shared<CountTask>(m_spConfigManager, false))
    {
    }

    /**
     * Create the file directory.
     */
    virtual void SetUp()
    {
        ASSERT_TRUE(m_spTask1->Start());
    }

    /**
     * Delete the created directory.
     */
    virtual void TearDown()
    {
        m_spTask1->Stop();
    }

protected:
    fly::ConfigManagerPtr m_spConfigManager;
    CountTaskPtr m_spTask1;
    CountTaskPtr m_spTask2;
};

//==============================================================================
TEST_F(RunnerTest, DoWorkTest)
{
    unsigned int count1 = m_spTask1->GetCallCount();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    unsigned int count2 = m_spTask1->GetCallCount();

    EXPECT_LT(count1, count2);
}

//==============================================================================
TEST_F(RunnerTest, FailedStartTest)
{
    EXPECT_FALSE(m_spTask2->Start());
}

//==============================================================================
TEST_F(RunnerTest, NeverStartedTest)
{
    unsigned int count1 = m_spTask2->GetCallCount();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    unsigned int count2 = m_spTask2->GetCallCount();

    EXPECT_EQ(count1, 0);
    EXPECT_EQ(count2, 0);
}
