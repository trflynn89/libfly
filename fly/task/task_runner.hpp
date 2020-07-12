#pragma once

#include "fly/types/concurrency/concurrent_queue.hpp"

#include <atomic>
#include <chrono>
#include <memory>

namespace fly {

class ParallelTaskRunner;
class SequencedTaskRunner;
class Task;
class TaskManager;

/**
 * Base class for controlling the execution of tasks.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class TaskRunner : public std::enable_shared_from_this<TaskRunner>
{
    friend class TaskManager;

public:
    /**
     * Destructor.
     */
    virtual ~TaskRunner() = default;

    /**
     * Post a task for execution.
     *
     * Once a task is posted, it may be attempted to be cancelled by deleting the task object or the
     * task runner itself. This will only cancel the task if the task manager has not yet begun
     * executing the task.
     *
     * @param weak_task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    virtual bool post_task(std::weak_ptr<Task> weak_task) = 0;

    /**
     * Schedule a task to be posted after a delay. The task is given to the task manager immediately
     * to be stored by its timer thread. Once the given delay has expired, the task will be handed
     * back to the task runner to govern when the task will be posted from there.
     *
     * Once a task is posted, it may be attempted to be cancelled by deleting the task object or the
     * task runner itself. This will only cancel the task if the task manager has not yet begun
     * executing the task.
     *
     * @param weak_task The task to be executed.
     * @param delay Delay before posting the task.
     *
     * @return True if the task was posted for delayed execution.
     */
    bool post_task_with_delay(std::weak_ptr<Task> weak_task, std::chrono::milliseconds delay);

protected:
    /**
     * Private constructor. Task runners may only be created by the task manager.
     *
     * @param weak_task_manager The task manager.
     */
    TaskRunner(std::weak_ptr<TaskManager> weak_task_manager) noexcept;

    /**
     * Completion notification triggered by the task manager that a task has finished execution (or
     * was skipped).
     *
     * @param task The (possibly null) task that was executed or skipped.
     */
    virtual void task_complete(const std::shared_ptr<Task> &task) = 0;

    /**
     * Forward a task to the task manager to be executed as soon as a worker thread is available.
     *
     * @param task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    bool post_task_to_task_manager(std::weak_ptr<Task> task);

private:
    std::weak_ptr<TaskManager> m_weak_task_manager;
};

/**
 * Task runner implementation for executing tasks in parallel. Tasks posted to this task runner may
 * be executed in any order.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class ParallelTaskRunner : public TaskRunner
{
    friend class TaskManager;

public:
    bool post_task(std::weak_ptr<Task>) override;

protected:
    explicit ParallelTaskRunner(std::weak_ptr<TaskManager>) noexcept;

    /**
     * This implementation does nothing.
     *
     * @param task The (possibly null) task that was executed or skipped.
     */
    void task_complete(const std::shared_ptr<Task> &task) override;
};

/**
 * Task runner implementation for executing tasks in sequence. Only one task posted to this task
 * runner will execute at a time. Tasks are executed in a FIFO manner; once one task completes, the
 * next task in line will be posted for execution.
 *
 * The caveat is with delayed tasks. If task A is posted with some delay, then task B is posted with
 * no delay, task B will be posted for execution first. Task A will only be posted for execution
 * once its delay has expired.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class SequencedTaskRunner : public TaskRunner
{
    friend class TaskManager;

public:
    bool post_task(std::weak_ptr<Task>) override;

protected:
    explicit SequencedTaskRunner(std::weak_ptr<TaskManager>) noexcept;

    /**
     * When a task is complete, post the next task in the pending queue.
     *
     * @param task The (possibly null) task that was executed or skipped.
     */
    void task_complete(const std::shared_ptr<Task> &task) override;

private:
    /**
     * If no task has been posted for execution, post the first task in the pending queue.
     *
     * @return True if the task was posted for execution or added to the pending queue.
     */
    bool maybe_post_task();

    ConcurrentQueue<std::weak_ptr<Task>> m_pending_tasks;
    std::atomic_bool m_has_running_task {false};
};

} // namespace fly
