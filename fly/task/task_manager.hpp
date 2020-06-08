#pragma once

#include "fly/types/concurrency/concurrent_queue.hpp"

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

namespace fly {

class Task;
class TaskRunner;

/**
 * Class to manage a pool of threads for executing tasks posted by any task runner. Also manages a
 * timer thread to hold delayed tasks until their scheduled time.
 *
 * The task manager makes no guarantee on the order of task execution; when a task is given to the
 * task manager, it will be executed as soon as a worker thread is available. Instead, ordering is
 * controlled by the task runners. A task runner may hold on to a task in accordance with its
 * defined behavior until it is ready for the task manager to execute the task.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class TaskManager : public std::enable_shared_from_this<TaskManager>
{
    friend class TaskRunner;

public:
    /**
     * Constructor.
     *
     * @param num_workers Number of worker threads to create.
     */
    explicit TaskManager(std::uint32_t num_workers) noexcept;

    /**
     * Create the worker threads and timer thread.
     *
     * @return True if the threads were created in this invocation.
     */
    bool start() noexcept;

    /**
     * Destroy the worker threads and timer thread, blocking until the threads exit.
     *
     * @return True if the threads were destroyed in this invocation.
     */
    bool stop() noexcept;

    /**
     * Create a task runner, holding a weak reference to this task manager.
     *
     * @tparam TaskRunnerType The type of task runner to create.
     *
     * @return The created task runner.
     */
    template <typename TaskRunnerType>
    std::shared_ptr<TaskRunnerType> create_task_runner() noexcept;

private:
    /**
     * Wrapper structure to associate a task with its task runner and the point in time that the
     * task should be executed.
     */
    struct TaskHolder
    {
        std::weak_ptr<Task> m_weak_task;
        std::weak_ptr<TaskRunner> m_weak_task_runner;
        std::chrono::steady_clock::time_point m_schedule;
    };

    /**
     * Post a task to be executed as soon as a worker thread is available.
     *
     * @param weak_task The task to be executed.
     * @param weak_task_runner The task runner posting the task.
     */
    void
    post_task(std::weak_ptr<Task> weak_task, std::weak_ptr<TaskRunner> weak_task_runner) noexcept;

    /**
     * Schedule a task to be posted for execution after some delay.
     *
     * @param weak_task The task to be executed.
     * @param weak_task_runner The task runner posting the task.
     * @param delay Delay before posting the task.
     */
    void post_task_with_delay(
        std::weak_ptr<Task> weak_task,
        std::weak_ptr<TaskRunner> weak_task_runner,
        std::chrono::milliseconds delay) noexcept;

    /**
     * Worker thread for executing tasks.
     */
    void worker_thread() noexcept;

    /**
     * Timer thread for holding delayed tasks until their scheduled time.
     */
    void timer_thread() noexcept;

    ConcurrentQueue<TaskHolder> m_tasks;

    std::mutex m_delayed_tasks_mutex;
    std::vector<TaskHolder> m_delayed_tasks;

    std::atomic_bool m_keep_running;

    std::vector<std::future<void>> m_futures;

    std::uint32_t m_num_workers;
};

//==================================================================================================
template <typename TaskRunnerType>
std::shared_ptr<TaskRunnerType> TaskManager::create_task_runner() noexcept
{
    static_assert(std::is_base_of_v<TaskRunner, TaskRunnerType>);

    const std::shared_ptr<TaskManager> task_manager = shared_from_this();
    return std::shared_ptr<TaskRunnerType>(new TaskRunnerType(task_manager));
}

} // namespace fly
