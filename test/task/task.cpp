#include "fly/task/task.hpp"

#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "test/util/waitable_task_runner.hpp"

#include <catch2/catch.hpp>

#include <atomic>
#include <chrono>
#include <memory>

namespace {

/**
 * A simple task to count the number of times it is run.
 */
class CountTask : public fly::Task
{
public:
    CountTask() noexcept : m_count(0)
    {
    }

    int get_count() const
    {
        return m_count.load();
    }

protected:
    void run() override
    {
        ++m_count;
    }

private:
    std::atomic_int m_count;
};

/**
 * A simple task to track its execution order.
 */
class MarkerTask : public fly::Task
{
public:
    MarkerTask(fly::ConcurrentQueue<int> *ordering, int marker) noexcept :
        m_ordering(ordering),
        m_marker(marker)
    {
    }

protected:
    void run() override
    {
        m_ordering->push(std::move(m_marker));
    }

private:
    fly::ConcurrentQueue<int> *m_ordering;
    int m_marker;
};

/**
 * A simple task to track its start and end time.
 */
class TimerTask : public fly::Task
{
public:
    TimerTask() noexcept : m_start_time(std::chrono::high_resolution_clock::now())
    {
    }

    std::chrono::milliseconds time() const
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(m_stop_time - m_start_time);
    }

protected:
    void run() override
    {
        m_stop_time = std::chrono::high_resolution_clock::now();
    }

private:
    std::chrono::high_resolution_clock::time_point m_start_time;
    std::chrono::high_resolution_clock::time_point m_stop_time;
};

} // namespace

TEST_CASE("Task", "[task]")
{
    auto task_manager = std::make_shared<fly::TaskManager>(1);
    CHECK(task_manager->start());

    SECTION("Cannot start the task manager multiple times")
    {
        CHECK_FALSE(task_manager->start());
    }

    SECTION("Cannot stop the task manager multiple times")
    {
        REQUIRE(task_manager->stop());
        CHECK_FALSE(task_manager->stop());

        // Delete the task manager so the CHECK at the bottom doesn't fail.
        task_manager.reset();
    }

    SECTION("Parallel task runner does not enforce execution order")
    {
        auto task_runner = task_manager->create_task_runner<fly::WaitableParallelTaskRunner>();
        auto task = std::make_shared<CountTask>();

        CHECK(task_runner->post_task(task));
        CHECK(task_runner->post_task(task));
        CHECK(task_runner->post_task(task));

        task_runner->wait_for_task_to_complete<CountTask>();
        task_runner->wait_for_task_to_complete<CountTask>();
        task_runner->wait_for_task_to_complete<CountTask>();
    }

    SECTION("Sequenced task runner enforces execution order")
    {
        auto task_runner = task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

        int marker = 0;
        fly::ConcurrentQueue<int> ordering;

        auto task1 = std::make_shared<MarkerTask>(&ordering, 1);
        auto task2 = std::make_shared<MarkerTask>(&ordering, 2);
        auto task3 = std::make_shared<MarkerTask>(&ordering, 3);

        CHECK(task_runner->post_task(task1));
        CHECK(task_runner->post_task(task2));
        CHECK(task_runner->post_task(task3));

        task_runner->wait_for_task_to_complete<MarkerTask>();
        task_runner->wait_for_task_to_complete<MarkerTask>();
        task_runner->wait_for_task_to_complete<MarkerTask>();

        ordering.pop(marker);
        CHECK(marker == 1);

        ordering.pop(marker);
        CHECK(marker == 2);

        ordering.pop(marker);
        CHECK(marker == 3);
    }

    SECTION("Delayed tasks execute no sooner than their specified delay")
    {
        auto task_runner = task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();
        auto task = std::make_shared<TimerTask>();

        const std::chrono::milliseconds delay(10);
        CHECK(task_runner->post_task_with_delay(task, delay));

        task_runner->wait_for_task_to_complete<TimerTask>();
        CHECK(task->time() >= delay);
    }

    SECTION("Delayed tasks execute after immediate tasks posted at the same time")
    {
        auto task_runner = task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

        int marker = 0;
        fly::ConcurrentQueue<int> ordering;

        auto task1 = std::make_shared<MarkerTask>(&ordering, 1);
        auto task2 = std::make_shared<MarkerTask>(&ordering, 2);
        auto task3 = std::make_shared<MarkerTask>(&ordering, 3);

        CHECK(task_runner->post_task_with_delay(task1, std::chrono::milliseconds(10)));
        CHECK(task_runner->post_task(task2));
        CHECK(task_runner->post_task(task3));

        task_runner->wait_for_task_to_complete<MarkerTask>();
        task_runner->wait_for_task_to_complete<MarkerTask>();
        task_runner->wait_for_task_to_complete<MarkerTask>();

        ordering.pop(marker);
        CHECK(marker == 2);

        ordering.pop(marker);
        CHECK(marker == 3);

        ordering.pop(marker);
        CHECK(marker == 1);
    }

    SECTION("Cancelled tasks do not execute")
    {
        auto task_runner = task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

        {
            auto task = std::make_shared<CountTask>();
            CHECK(task->get_count() == 0);

            CHECK(task_runner->post_task_with_delay(task, std::chrono::milliseconds(10)));
        }

        CHECK_FALSE(
            task_runner->wait_for_task_to_complete<CountTask>(std::chrono::milliseconds(20)));
    }

    SECTION("Cancelled tasks do not execute while immediate tasks do execute")
    {
        auto task_runner = task_manager->create_task_runner<fly::WaitableSequencedTaskRunner>();

        int marker = 0;
        fly::ConcurrentQueue<int> ordering;

        {
            auto task1 = std::make_shared<CountTask>();
            CHECK(task1->get_count() == 0);

            CHECK(task_runner->post_task_with_delay(task1, std::chrono::milliseconds(10)));
        }

        auto task2 = std::make_shared<MarkerTask>(&ordering, 2);
        auto task3 = std::make_shared<MarkerTask>(&ordering, 3);

        CHECK(task_runner->post_task(task2));
        CHECK(task_runner->post_task(task3));

        task_runner->wait_for_task_to_complete<MarkerTask>();
        task_runner->wait_for_task_to_complete<MarkerTask>();

        CHECK_FALSE(
            task_runner->wait_for_task_to_complete<CountTask>(std::chrono::milliseconds(20)));

        ordering.pop(marker);
        CHECK(marker == 2);

        ordering.pop(marker);
        CHECK(marker == 3);
    }

    SECTION("Parallel tasks cannot be posted after the task manager is deleted")
    {
        auto task = std::make_shared<CountTask>();
        CHECK(task->get_count() == 0);

        auto task_runner = task_manager->create_task_runner<fly::ParallelTaskRunner>();

        CHECK(task_manager->stop());
        task_manager.reset();

        CHECK_FALSE(task_runner->post_task(task));
        CHECK_FALSE(task_runner->post_task_with_delay(task, std::chrono::milliseconds(10)));

        CHECK(task->get_count() == 0);
    }

    SECTION("Sequenced tasks cannot be posted after the task manager is deleted")
    {
        auto task = std::make_shared<CountTask>();
        CHECK(task->get_count() == 0);

        auto task_runner = task_manager->create_task_runner<fly::SequencedTaskRunner>();

        CHECK(task_manager->stop());
        task_manager.reset();

        CHECK_FALSE(task_runner->post_task(task));
        CHECK_FALSE(task_runner->post_task_with_delay(task, std::chrono::milliseconds(10)));

        CHECK(task->get_count() == 0);
    }

    if (task_manager)
    {
        CHECK(task_manager->stop());
    }
}
