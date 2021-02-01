#pragma once

#include "fly/task/task_types.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace fly {

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
    bool start();

    /**
     * Destroy the worker threads and timer thread, blocking until the threads exit. This must be
     * eplicitly called from the same thread that started the task manager.
     *
     * TODO: TaskManager should be created as a unique_ptr so it can be started and stopped in an
     * RAII fashion. It cannot be stopped from the destructor because a task thread may be the last
     * owner of a shared_ptr to this task manager. In that case, when that shared_ptr is destroyed,
     * the destructor would call stop, resulting in the task thread trying to join itself.
     *
     * @return True if the threads were destroyed in this invocation.
     */
    bool stop();

    /**
     * Create a task runner, holding a weak reference to this task manager.
     *
     * @tparam TaskRunnerType The type of task runner to create.
     *
     * @return The created task runner.
     */
    template <typename TaskRunnerType>
    std::shared_ptr<TaskRunnerType> create_task_runner();

private:
    /**
     * Wrapper structure to associate a task with its task runner and the point in time that the
     * task should be executed.
     */
    struct TaskHolder
    {
        TaskLocation m_location;
        Task m_task;
        std::weak_ptr<TaskRunner> m_weak_task_runner;
        std::chrono::steady_clock::time_point m_schedule;
    };

    /**
     * Post a task to be executed as soon as a worker thread is available.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     * @param weak_task_runner The task runner posting the task.
     */
    void
    post_task(TaskLocation &&location, Task &&task, std::weak_ptr<TaskRunner> weak_task_runner);

    /**
     * Schedule a task to be posted for execution after some delay.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     * @param weak_task_runner The task runner posting the task.
     * @param delay Delay before posting the task.
     */
    void post_task_with_delay(
        TaskLocation &&location,
        Task &&task,
        std::weak_ptr<TaskRunner> weak_task_runner,
        std::chrono::milliseconds delay);

    /**
     * Worker thread for executing tasks.
     */
    void worker_thread();

    /**
     * Timer thread for holding delayed tasks until their scheduled time.
     */
    void timer_thread();

    ConcurrentQueue<TaskHolder> m_tasks;

    std::mutex m_delayed_tasks_mutex;
    std::vector<TaskHolder> m_delayed_tasks;

    std::atomic_bool m_keep_running;

    std::vector<std::jthread> m_threads;
    std::uint32_t m_thread_count;
};

//==================================================================================================
template <typename TaskRunnerType>
std::shared_ptr<TaskRunnerType> TaskManager::create_task_runner()
{
    static_assert(std::is_base_of_v<TaskRunner, TaskRunnerType>);

    const std::shared_ptr<TaskManager> task_manager = shared_from_this();
    return std::shared_ptr<TaskRunnerType>(new TaskRunnerType(task_manager));
}

} // namespace fly
