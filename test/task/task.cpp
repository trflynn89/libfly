#include <atomic>
#include <memory>

#include <gtest/gtest.h>

#include "fly/task/task.h"
#include "fly/task/task_manager.h"
#include "fly/task/task_runner.h"
#include "fly/types/concurrent_queue.h"

#include "test/util/waitable_task_runner.h"

namespace
{
    //==========================================================================
    class CountTask : public fly::Task
    {
    public:
        CountTask() : m_count(0)
        {
        }

        int GetCount() const
        {
            return m_count.load();
        }

    protected:
        void Run() override
        {
            ++m_count;
        }

    private:
        std::atomic_int m_count;
    };

    //==========================================================================
    class MarkerTask : public fly::Task
    {
    public:
        MarkerTask(
            fly::ConcurrentQueue<int> *pOrdering,
            int marker
        ) :
            m_pOrdering(pOrdering),
            m_marker(marker)
        {
        }

    protected:
        void Run() override
        {
            m_pOrdering->Push(m_marker);
        }

    private:
        fly::ConcurrentQueue<int> *m_pOrdering;
        int m_marker;
    };
}

//==============================================================================
class TaskTest : public ::testing::Test
{
public:
    TaskTest() : m_spTaskManager(std::make_shared<fly::TaskManager>(1))
    {
    }

    /**
     * Start the task manager.
     */
    void SetUp() override
    {
        ASSERT_TRUE(m_spTaskManager->Start());
    }

    /**
     * Stop the task manager.
     */
    void TearDown() override
    {
        ASSERT_TRUE(m_spTaskManager->Stop());
    }

protected:
    std::shared_ptr<fly::TaskManager> m_spTaskManager;
};

//==============================================================================
TEST_F(TaskTest, MultipleStartTest)
{
    ASSERT_FALSE(m_spTaskManager->Start());
}

//==============================================================================
TEST_F(TaskTest, MultipleStopTest)
{
    auto spTaskManager(std::make_shared<fly::TaskManager>(1));
    ASSERT_TRUE(spTaskManager->Start());

    ASSERT_TRUE(spTaskManager->Stop());
    ASSERT_FALSE(spTaskManager->Stop());
}

//==============================================================================
TEST_F(TaskTest, ParallelTaskRunnerTest)
{
    auto spTaskRunner(
        m_spTaskManager->CreateTaskRunner<fly::WaitableParallelTaskRunner>()
    );

    auto spTask(std::make_shared<CountTask>());

    EXPECT_TRUE(spTaskRunner->PostTask(spTask));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask));

    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();
    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();
    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();
}

//==============================================================================
TEST_F(TaskTest, SequencedTaskRunnerTest)
{
    auto spTaskRunner(
        m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
    );

    int marker = 0;
    fly::ConcurrentQueue<int> ordering;

    auto spTask1(std::make_shared<MarkerTask>(&ordering, 1));
    auto spTask2(std::make_shared<MarkerTask>(&ordering, 2));
    auto spTask3(std::make_shared<MarkerTask>(&ordering, 3));

    EXPECT_TRUE(spTaskRunner->PostTask(spTask1));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask2));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask3));

    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();
    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();
    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();

    ordering.Pop(marker);
    EXPECT_EQ(marker, 1);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 2);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 3);
}

//==============================================================================
TEST_F(TaskTest, DelayTaskTest)
{
    auto spTaskRunner(
        m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
    );

    auto spTask(std::make_shared<CountTask>());
    EXPECT_EQ(spTask->GetCount(), 0);

    EXPECT_TRUE(
        spTaskRunner->PostTaskWithDelay(spTask, std::chrono::milliseconds(10)
    ));

    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();
    EXPECT_EQ(spTask->GetCount(), 1);
}

//==============================================================================
TEST_F(TaskTest, ImmediateAndDelayTaskTest)
{
    auto spTaskRunner(
        m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
    );

    int marker = 0;
    fly::ConcurrentQueue<int> ordering;

    auto spTask1(std::make_shared<MarkerTask>(&ordering, 1));
    auto spTask2(std::make_shared<MarkerTask>(&ordering, 2));
    auto spTask3(std::make_shared<MarkerTask>(&ordering, 3));

    EXPECT_TRUE(
        spTaskRunner->PostTaskWithDelay(spTask1, std::chrono::milliseconds(10)
    ));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask2));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask3));

    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();
    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();
    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();

    ordering.Pop(marker);
    EXPECT_EQ(marker, 2);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 3);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 1);
}

//==============================================================================
TEST_F(TaskTest, CancelTaskTest)
{
    auto spTaskRunner(
        m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
    );

    {
        auto spTask(std::make_shared<CountTask>());
        EXPECT_EQ(spTask->GetCount(), 0);

        EXPECT_TRUE(spTaskRunner->PostTaskWithDelay(
            spTask, std::chrono::milliseconds(10)
        ));
    }

    EXPECT_FALSE(spTaskRunner->WaitForTaskTypeToComplete<CountTask>(
        std::chrono::milliseconds(20)
    ));
}

//==============================================================================
TEST_F(TaskTest, ImmediateAndDelayCancelTaskTest)
{
    auto spTaskRunner(
        m_spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
    );

    int marker = 0;
    fly::ConcurrentQueue<int> ordering;

    {
        auto spTask1(std::make_shared<CountTask>());
        EXPECT_EQ(spTask1->GetCount(), 0);

        EXPECT_TRUE(spTaskRunner->PostTaskWithDelay(
            spTask1, std::chrono::milliseconds(10)
        ));
    }

    auto spTask2(std::make_shared<MarkerTask>(&ordering, 2));
    auto spTask3(std::make_shared<MarkerTask>(&ordering, 3));

    EXPECT_TRUE(spTaskRunner->PostTask(spTask2));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask3));

    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();
    spTaskRunner->WaitForTaskTypeToComplete<MarkerTask>();

    EXPECT_FALSE(spTaskRunner->WaitForTaskTypeToComplete<CountTask>(
        std::chrono::milliseconds(20)
    ));

    ordering.Pop(marker);
    EXPECT_EQ(marker, 2);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 3);
}

//==============================================================================
TEST_F(TaskTest, RunnerBeforeManagerStartedTest)
{
    auto spTask(std::make_shared<CountTask>());
    EXPECT_EQ(spTask->GetCount(), 0);

    auto spTaskManager(std::make_shared<fly::TaskManager>(1));
    auto spTaskRunner(
        spTaskManager->CreateTaskRunner<fly::WaitableSequencedTaskRunner>()
    );

    EXPECT_TRUE(spTaskRunner->PostTask(spTask));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask));
    EXPECT_TRUE(spTaskRunner->PostTask(spTask));

    EXPECT_FALSE(spTaskRunner->WaitForTaskTypeToComplete<CountTask>(
        std::chrono::milliseconds(20)
    ));

    EXPECT_EQ(spTask->GetCount(), 0);

    ASSERT_TRUE(spTaskManager->Start());

    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();
    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();
    spTaskRunner->WaitForTaskTypeToComplete<CountTask>();

    EXPECT_EQ(spTask->GetCount(), 3);

    ASSERT_TRUE(spTaskManager->Stop());
}

//==============================================================================
TEST_F(TaskTest, ParallelRunnerAfterManagerDeletedTest)
{
    auto spTask(std::make_shared<CountTask>());
    EXPECT_EQ(spTask->GetCount(), 0);

    auto spTaskManager(std::make_shared<fly::TaskManager>(1));
    ASSERT_TRUE(spTaskManager->Start());

    auto spTaskRunner(
        spTaskManager->CreateTaskRunner<fly::ParallelTaskRunner>()
    );

    ASSERT_TRUE(spTaskManager->Stop());
    spTaskManager.reset();

    EXPECT_FALSE(spTaskRunner->PostTask(spTask));
    EXPECT_FALSE(
        spTaskRunner->PostTaskWithDelay(spTask, std::chrono::milliseconds(10)
    ));

    EXPECT_EQ(spTask->GetCount(), 0);
}

//==============================================================================
TEST_F(TaskTest, SequencedRunnerAfterManagerDeletedTest)
{
    auto spTask(std::make_shared<CountTask>());
    EXPECT_EQ(spTask->GetCount(), 0);

    auto spTaskManager(std::make_shared<fly::TaskManager>(1));
    ASSERT_TRUE(spTaskManager->Start());

    auto spTaskRunner(
        spTaskManager->CreateTaskRunner<fly::SequencedTaskRunner>()
    );

    ASSERT_TRUE(spTaskManager->Stop());
    spTaskManager.reset();

    EXPECT_FALSE(spTaskRunner->PostTask(spTask));
    EXPECT_FALSE(
        spTaskRunner->PostTaskWithDelay(spTask, std::chrono::milliseconds(10)
    ));

    EXPECT_EQ(spTask->GetCount(), 0);
}
