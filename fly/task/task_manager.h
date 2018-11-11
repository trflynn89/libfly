#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <vector>

#include "fly/types/concurrent_queue.h"

namespace fly {

class Task;
class TaskRunner;

/**
 * Class to manage a pool of threads for executing tasks posted by any task
 * runner. Also manages a timer thread to hold delayed tasks until their
 * scheduled time.
 *
 * The task manager makes no guarantee on the order of task execution; when a
 * task is given to the task manager, it will be executed as soon as a worker
 * thread is available. Instead, ordering is controlled by the task runners. A
 * task runner may hold on to a task in accordance with its defined behavior
 * until it is ready for the task manager to execute the task.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class TaskManager : public std::enable_shared_from_this<TaskManager>
{
    friend class TaskRunner;

public:
    /**
     * Constructor.
     *
     * @param int Number of worker threads to create.
     */
    TaskManager(int);

    /**
     * Create the worker threads and timer thread.
     *
     * @return bool True if the threads were created in this invocation.
     */
    bool Start();

    /**
     * Destroy the worker threads and timer thread, blocking until the threads
     * exit.
     *
     * @return bool True if the threads were destroyed in this invocation.
     */
    bool Stop();

    /**
     * Create a task runner, holding a weak reference to this task manager.
     *
     * @tparam TaskRunnerType The type of task runner to create.
     *
     * @return SequencedTaskRunner The created task runner.
     */
    template <typename TaskRunnerType>
    std::shared_ptr<TaskRunnerType> CreateTaskRunner();

private:
    /**
     * Wrapper structure to associate a task with its task runner and the point
     * in time that the task should be executed.
     */
    struct TaskHolder
    {
        std::weak_ptr<Task> m_wpTask;
        std::weak_ptr<TaskRunner> m_wpTaskRunner;
        std::chrono::steady_clock::time_point m_schedule;
    };

    /**
     * Post a task to be executed as soon as a worker thread is available.
     *
     * @param Task Weak reference to the task the be executed.
     * @param TaskRunner Weak reference to the task runner posting the task.
     */
    void postTask(
        const std::weak_ptr<Task> &,
        const std::weak_ptr<TaskRunner> &
    );

    /**
     * Schedule a task to be posted for execution after some delay.
     *
     * @param Task Weak reference to the task the be executed.
     * @param TaskRunner Weak reference to the task runner posting the task.
     * @param milliseconds Delay before posting the task.
     */
    void postTaskWithDelay(
        const std::weak_ptr<Task> &,
        const std::weak_ptr<TaskRunner> &,
        std::chrono::milliseconds
    );

    /**
     * Worker thread for executing tasks.
     */
    void workerThread();

    /**
     * Timer thread for holding delayed tasks until their scheduled time.
     */
    void timerThread();

    ConcurrentQueue<TaskHolder> m_tasks;

    std::mutex m_delayedTasksMutex;
    std::vector<TaskHolder> m_delayedTasks;

    std::atomic_bool m_aKeepRunning;

    std::vector<std::future<void>> m_futures;

    int m_numWorkers;
};

//==============================================================================
template <typename TaskRunnerType>
std::shared_ptr<TaskRunnerType> TaskManager::CreateTaskRunner()
{
    static_assert(std::is_base_of<TaskRunner, TaskRunnerType>::value,
        "Given type is not a task runner");

    const std::shared_ptr<TaskManager> spTaskManager = shared_from_this();
    return std::shared_ptr<TaskRunnerType>(new TaskRunnerType(spTaskManager));
}

}
