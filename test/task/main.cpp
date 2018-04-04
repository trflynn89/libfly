#include <atomic>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include "fly/fly.h"
#include "fly/task/runner.h"

FLY_CLASS_PTRS(CountTask);

//==============================================================================
class CountTask : public fly::Runner
{
public:
    CountTask(bool run) :
        Runner("CountTask", std::thread::hardware_concurrency()),
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
        m_spTask1(std::make_shared<CountTask>(true)),
        m_spTask2(std::make_shared<CountTask>(false))
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
