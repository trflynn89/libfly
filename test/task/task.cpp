#include "fly/task/task_manager.hpp"
#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"
#include "test/util/waitable_task_runner.hpp"

#include <catch2/catch.hpp>

#include <atomic>
#include <chrono>
#include <memory>

using namespace std::chrono_literals;

namespace {

/**
 * A simple task to track whether it was exected.
 */
void standalone_task(bool &task_was_called)
{
    task_was_called = true;
}

/**
 * A simple task to track whether it was exected.
 */
class TaskClass
{
public:
    explicit TaskClass(bool &task_was_called) : m_task_was_called(task_was_called)
    {
    }

    static void static_task(bool &task_was_called)
    {
        task_was_called = true;
    }

    void member_task()
    {
        m_task_was_called = true;
    }

private:
    bool &m_task_was_called;
};

/**
 * A simple task to count the number of times it is run.
 */
class CountTask
{
public:
    CountTask() noexcept : m_count(0)
    {
    }

    int get_count() const
    {
        return m_count.load();
    }

    void run()
    {
        ++m_count;
    }

private:
    std::atomic_int m_count;
};

/**
 * A simple task to track its execution order.
 */
class MarkerTask
{
public:
    MarkerTask(fly::ConcurrentQueue<int> *ordering) noexcept : m_ordering(ordering)
    {
    }

    void run(int marker)
    {
        m_ordering->push(std::move(marker));
    }

private:
    fly::ConcurrentQueue<int> *m_ordering;
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

    void run()
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
    REQUIRE(task_manager->start());

    SECTION("Cannot start the task manager multiple times")
    {
        CHECK_FALSE(task_manager->start());
    }

    SECTION("Cannot stop the task manager multiple times")
    {
        REQUIRE(task_manager->stop());
        CHECK_FALSE(task_manager->stop());

        // Delete the task manager so the REQUIRE at the bottom doesn't fail.
        task_manager.reset();
    }

    SECTION("Tasks may be posted as lambdas")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableParallelTaskRunner>();

        bool task_was_called = false;
        auto task = [&task_was_called]() { task_was_called = true; };

        REQUIRE(task_runner->post_task(FROM_HERE, std::move(task)));
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK(task_was_called);
    }

    SECTION("Tasks may be posted as standalone functions")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableParallelTaskRunner>();

        bool task_was_called = false;
        auto task = std::bind(standalone_task, std::ref(task_was_called));

        REQUIRE(task_runner->post_task(FROM_HERE, std::move(task)));
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK(task_was_called);
    }

    SECTION("Tasks may be posted as static class functions")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableParallelTaskRunner>();

        bool task_was_called = false;
        auto task = std::bind(TaskClass::static_task, std::ref(task_was_called));

        REQUIRE(task_runner->post_task(FROM_HERE, std::move(task)));
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK(task_was_called);
    }

    SECTION("Tasks may be posted as member class functions")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableParallelTaskRunner>();

        bool task_was_called = false;
        TaskClass task_class(task_was_called);

        auto task = std::bind(&TaskClass::member_task, &task_class);

        REQUIRE(task_runner->post_task(FROM_HERE, std::move(task)));
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK(task_was_called);
    }

    SECTION("Tasks may be cancelled with weak pointers")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableParallelTaskRunner>();

        bool task_was_called = false;
        auto task_class = std::make_shared<TaskClass>(task_was_called);
        std::weak_ptr<TaskClass> weak_task_class = task_class;

        auto task = [weak_task_class]() {
            if (auto strong_task_class = weak_task_class.lock(); strong_task_class)
            {
                strong_task_class->member_task();
            }
        };

        task_class.reset();

        REQUIRE(task_runner->post_task(FROM_HERE, std::move(task)));
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK_FALSE(task_was_called);
    }

    SECTION("Cancelled tasks do not execute while other tasks do execute")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableSequencedTaskRunner>();

        fly::ConcurrentQueue<int> ordering;
        MarkerTask marker_task(&ordering);

        bool task_was_called = false;
        auto task_class = std::make_shared<TaskClass>(task_was_called);
        std::weak_ptr<TaskClass> weak_task_class = task_class;

        auto task = [weak_task_class]() {
            if (auto strong_task_class = weak_task_class.lock(); strong_task_class)
            {
                strong_task_class->member_task();
            }
        };

        task_class.reset();

        REQUIRE(task_runner->post_task(FROM_HERE, std::move(task)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &marker_task, 2)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &marker_task, 3)));

        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK_FALSE(task_was_called);

        int marker = 0;
        ordering.pop(marker);
        CHECK(marker == 2);

        ordering.pop(marker);
        CHECK(marker == 3);
    }

    SECTION("Parallel task runner does not enforce execution order")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableParallelTaskRunner>();

        CountTask task;
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&CountTask::run, &task)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&CountTask::run, &task)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&CountTask::run, &task)));

        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);
    }

    SECTION("Sequenced task runner enforces execution order")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableSequencedTaskRunner>();

        fly::ConcurrentQueue<int> ordering;
        MarkerTask task(&ordering);

        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &task, 1)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &task, 2)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &task, 3)));

        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);

        int marker = 0;
        ordering.pop(marker);
        CHECK(marker == 1);

        ordering.pop(marker);
        CHECK(marker == 2);

        ordering.pop(marker);
        CHECK(marker == 3);
    }

    SECTION("Delayed tasks execute no sooner than their specified delay")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableSequencedTaskRunner>();

        TimerTask task;
        const std::chrono::milliseconds delay(10);

        REQUIRE(
            task_runner->post_task_with_delay(FROM_HERE, std::bind(&TimerTask::run, &task), delay));
        task_runner->wait_for_task_to_complete(__FILE__);

        CHECK(task.time() >= delay);
    }

    SECTION("Delayed tasks execute after immediate tasks posted at the same time")
    {
        auto task_runner =
            task_manager->create_task_runner<fly::test::WaitableSequencedTaskRunner>();

        fly::ConcurrentQueue<int> ordering;
        MarkerTask task(&ordering);

        REQUIRE(task_runner
                    ->post_task_with_delay(FROM_HERE, std::bind(&MarkerTask::run, &task, 1), 10ms));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &task, 2)));
        REQUIRE(task_runner->post_task(FROM_HERE, std::bind(&MarkerTask::run, &task, 3)));

        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);
        task_runner->wait_for_task_to_complete(__FILE__);

        int marker = 0;
        ordering.pop(marker);
        CHECK(marker == 2);

        ordering.pop(marker);
        CHECK(marker == 3);

        ordering.pop(marker);
        CHECK(marker == 1);
    }

    SECTION("Parallel tasks cannot be posted after the task manager is deleted")
    {
        auto task_runner = task_manager->create_task_runner<fly::ParallelTaskRunner>();

        REQUIRE(task_manager->stop());
        task_manager.reset();

        CHECK_FALSE(task_runner->post_task(FROM_HERE, []() {}));
        CHECK_FALSE(task_runner->post_task_with_delay(
            FROM_HERE,
            []() {},
            0ms));
    }

    SECTION("Sequenced tasks cannot be posted after the task manager is deleted")
    {
        auto task_runner = task_manager->create_task_runner<fly::SequencedTaskRunner>();

        REQUIRE(task_manager->stop());
        task_manager.reset();

        CHECK_FALSE(task_runner->post_task(FROM_HERE, []() {}));
        CHECK_FALSE(task_runner->post_task_with_delay(
            FROM_HERE,
            []() {},
            0ms));
    }

    if (task_manager)
    {
        REQUIRE(task_manager->stop());
    }
}
