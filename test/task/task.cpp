#include "fly/task/task.hpp"

#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "test/util/waitable_task_runner.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <memory>

namespace {

//==========================================================================
class CountTask : public fly::Task
{
public:
    CountTask() noexcept : m_count(0)
    {
    }

    int get_count() const noexcept
    {
        return m_count.load();
    }

protected:
    void run() noexcept override
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
    MarkerTask(fly::ConcurrentQueue<int> *ordering, int marker) noexcept :
        m_ordering(ordering),
        m_marker(marker)
    {
    }

protected:
    void run() noexcept override
    {
        m_ordering->Push(std::move(m_marker));
    }

private:
    fly::ConcurrentQueue<int> *m_ordering;
    int m_marker;
};

} // namespace

//==============================================================================
class TaskTest : public ::testing::Test
{
public:
    TaskTest() noexcept : m_task_manager(std::make_shared<fly::TaskManager>(1))
    {
    }

    /**
     * Start the task manager.
     */
    void SetUp() noexcept override
    {
        ASSERT_TRUE(m_task_manager->start());
    }

    /**
     * Stop the task manager.
     */
    void TearDown() noexcept override
    {
        ASSERT_TRUE(m_task_manager->stop());
    }

protected:
    std::shared_ptr<fly::TaskManager> m_task_manager;
};

//==============================================================================
TEST_F(TaskTest, MultipleStart)
{
    ASSERT_FALSE(m_task_manager->start());
}

//==============================================================================
TEST_F(TaskTest, MultipleStop)
{
    auto task_manager = std::make_shared<fly::TaskManager>(1);
    ASSERT_TRUE(task_manager->start());

    ASSERT_TRUE(task_manager->stop());
    ASSERT_FALSE(task_manager->stop());
}

//==============================================================================
TEST_F(TaskTest, ParallelTaskRunner)
{
    auto task_runner =
        m_task_manager->create_task_runner<fly::WaitableParallelTaskRunner>();

    auto task = std::make_shared<CountTask>();

    EXPECT_TRUE(task_runner->post_task(task));
    EXPECT_TRUE(task_runner->post_task(task));
    EXPECT_TRUE(task_runner->post_task(task));

    task_runner->wait_for_task_to_complete<CountTask>();
    task_runner->wait_for_task_to_complete<CountTask>();
    task_runner->wait_for_task_to_complete<CountTask>();
}

//==============================================================================
TEST_F(TaskTest, SequencedTaskRunner)
{
    auto task_runner =
        m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    int marker = 0;
    fly::ConcurrentQueue<int> ordering;

    auto task1 = std::make_shared<MarkerTask>(&ordering, 1);
    auto task2 = std::make_shared<MarkerTask>(&ordering, 2);
    auto task3 = std::make_shared<MarkerTask>(&ordering, 3);

    EXPECT_TRUE(task_runner->post_task(task1));
    EXPECT_TRUE(task_runner->post_task(task2));
    EXPECT_TRUE(task_runner->post_task(task3));

    task_runner->wait_for_task_to_complete<MarkerTask>();
    task_runner->wait_for_task_to_complete<MarkerTask>();
    task_runner->wait_for_task_to_complete<MarkerTask>();

    ordering.Pop(marker);
    EXPECT_EQ(marker, 1);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 2);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 3);
}

//==============================================================================
TEST_F(TaskTest, DelayTask)
{
    auto task_runner =
        m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    auto task = std::make_shared<CountTask>();
    EXPECT_EQ(task->get_count(), 0);

    EXPECT_TRUE(
        task_runner->post_task_with_delay(task, std::chrono::milliseconds(10)));

    task_runner->wait_for_task_to_complete<CountTask>();
    EXPECT_EQ(task->get_count(), 1);
}

//==============================================================================
TEST_F(TaskTest, ImmediateAndDelayTask)
{
    auto task_runner =
        m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    int marker = 0;
    fly::ConcurrentQueue<int> ordering;

    auto task1 = std::make_shared<MarkerTask>(&ordering, 1);
    auto task2 = std::make_shared<MarkerTask>(&ordering, 2);
    auto task3 = std::make_shared<MarkerTask>(&ordering, 3);

    EXPECT_TRUE(task_runner->post_task_with_delay(
        task1,
        std::chrono::milliseconds(10)));
    EXPECT_TRUE(task_runner->post_task(task2));
    EXPECT_TRUE(task_runner->post_task(task3));

    task_runner->wait_for_task_to_complete<MarkerTask>();
    task_runner->wait_for_task_to_complete<MarkerTask>();
    task_runner->wait_for_task_to_complete<MarkerTask>();

    ordering.Pop(marker);
    EXPECT_EQ(marker, 2);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 3);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 1);
}

//==============================================================================
TEST_F(TaskTest, CancelTask)
{
    auto task_runner =
        m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    {
        auto task = std::make_shared<CountTask>();
        EXPECT_EQ(task->get_count(), 0);

        EXPECT_TRUE(task_runner->post_task_with_delay(
            task,
            std::chrono::milliseconds(10)));
    }

    EXPECT_FALSE(task_runner->wait_for_task_to_complete<CountTask>(
        std::chrono::milliseconds(20)));
}

//==============================================================================
TEST_F(TaskTest, ImmediateAndDelayCancelTask)
{
    auto task_runner =
        m_task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    int marker = 0;
    fly::ConcurrentQueue<int> ordering;

    {
        auto task1 = std::make_shared<CountTask>();
        EXPECT_EQ(task1->get_count(), 0);

        EXPECT_TRUE(task_runner->post_task_with_delay(
            task1,
            std::chrono::milliseconds(10)));
    }

    auto task2 = std::make_shared<MarkerTask>(&ordering, 2);
    auto task3 = std::make_shared<MarkerTask>(&ordering, 3);

    EXPECT_TRUE(task_runner->post_task(task2));
    EXPECT_TRUE(task_runner->post_task(task3));

    task_runner->wait_for_task_to_complete<MarkerTask>();
    task_runner->wait_for_task_to_complete<MarkerTask>();

    EXPECT_FALSE(task_runner->wait_for_task_to_complete<CountTask>(
        std::chrono::milliseconds(20)));

    ordering.Pop(marker);
    EXPECT_EQ(marker, 2);

    ordering.Pop(marker);
    EXPECT_EQ(marker, 3);
}

//==============================================================================
TEST_F(TaskTest, RunnerBeforeManagerStarted)
{
    auto task = std::make_shared<CountTask>();
    EXPECT_EQ(task->get_count(), 0);

    auto task_manager = std::make_shared<fly::TaskManager>(1);
    auto task_runner =
        task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

    EXPECT_TRUE(task_runner->post_task(task));
    EXPECT_TRUE(task_runner->post_task(task));
    EXPECT_TRUE(task_runner->post_task(task));

    EXPECT_FALSE(task_runner->wait_for_task_to_complete<CountTask>(
        std::chrono::milliseconds(20)));

    EXPECT_EQ(task->get_count(), 0);

    ASSERT_TRUE(task_manager->start());

    task_runner->wait_for_task_to_complete<CountTask>();
    task_runner->wait_for_task_to_complete<CountTask>();
    task_runner->wait_for_task_to_complete<CountTask>();

    EXPECT_EQ(task->get_count(), 3);

    ASSERT_TRUE(task_manager->stop());
}

//==============================================================================
TEST_F(TaskTest, ParallelRunnerAfterManagerDeleted)
{
    auto task = std::make_shared<CountTask>();
    EXPECT_EQ(task->get_count(), 0);

    auto task_manager = std::make_shared<fly::TaskManager>(1);
    ASSERT_TRUE(task_manager->start());

    auto task_runner =
        task_manager->create_task_runner<fly::ParallelTaskRunner>();

    ASSERT_TRUE(task_manager->stop());
    task_manager.reset();

    EXPECT_FALSE(task_runner->post_task(task));
    EXPECT_FALSE(
        task_runner->post_task_with_delay(task, std::chrono::milliseconds(10)));

    EXPECT_EQ(task->get_count(), 0);
}

//==============================================================================
TEST_F(TaskTest, SequencedRunnerAfterManagerDeleted)
{
    auto task = std::make_shared<CountTask>();
    EXPECT_EQ(task->get_count(), 0);

    auto task_manager = std::make_shared<fly::TaskManager>(1);
    ASSERT_TRUE(task_manager->start());

    auto task_runner =
        task_manager->create_task_runner<fly::SequencedTaskRunner>();

    ASSERT_TRUE(task_manager->stop());
    task_manager.reset();

    EXPECT_FALSE(task_runner->post_task(task));
    EXPECT_FALSE(
        task_runner->post_task_with_delay(task, std::chrono::milliseconds(10)));

    EXPECT_EQ(task->get_count(), 0);
}
