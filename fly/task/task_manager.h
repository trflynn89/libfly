#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <mutex>
#include <typeinfo>
#include <vector>

#include "fly/fly.h"
#include "fly/types/concurrent_queue.h"

namespace fly {

FLY_CLASS_PTRS(TaskManager);

FLY_CLASS_PTRS(Task);
FLY_CLASS_PTRS(TaskRunner);

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
     * Constructor. Create the worker threads and timer thread.
     */
    TaskManager(int);

    /**
     * Destructor. Destroy the worker threads and timer thread, blocking until
     * the threads exit.
     */
    ~TaskManager();

    /**
     * Create a task runner, holding a weak reference to this task manager.
     *
     * @tparam TaskRunnerType The type of task runner to create.
     *
     * @return SequencedTaskRunnerPtr The created task runner.
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
        TaskWPtr m_wpTask;
        TaskRunnerWPtr m_wpTaskRunner;
        std::chrono::steady_clock::time_point m_schedule;
    };

    /**
     * Post a task to be executed as soon as a worker thread is available.
     *
     * @param TaskWPtr Weak reference to the task the be executed.
     * @param TaskRunnerWPtr Weak reference to the task runner posting the task.
     */
    void postTask(const TaskWPtr &, const TaskRunnerWPtr &);

    /**
     * Schedule a task to be posted for execution after some delay.
     *
     * @param TaskWPtr Weak reference to the task the be executed.
     * @param TaskRunnerWPtr Weak reference to the task runner posting the task.
     * @param milliseconds Delay before posting the task.
     */
    void postTaskWithDelay(
        const TaskWPtr &,
        const TaskRunnerWPtr &,
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

    const TaskManagerPtr spTaskManager = shared_from_this();
    return std::shared_ptr<TaskRunnerType>(new TaskRunnerType(spTaskManager));
}

}
