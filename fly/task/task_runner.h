#pragma once

#include <atomic>
#include <chrono>
#include <memory>

#include "fly/fly.h"
#include "fly/types/concurrent_queue.h"

namespace fly {

FLY_CLASS_PTRS(TaskRunner);
FLY_CLASS_PTRS(ParallelTaskRunner);
FLY_CLASS_PTRS(SequencedTaskRunner);

FLY_CLASS_PTRS(Task);
FLY_CLASS_PTRS(TaskManager);

/**
 * Base class for controlling the execution of tasks.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
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
     * Once a task is posted, it may be attempted to be cancelled by deleting
     * the task object or the task runner itself. This will only cancel the task
     * if the task manager has not yet begun executing the task.
     *
     * @param TaskWPtr Weak reference to the task the be executed.
     *
     * @return bool True if the task was posted for execution.
     */
    virtual bool PostTask(const TaskWPtr &) = 0;

    /**
     * Schedule a task to be posted after a delay. The task is given to the
     * task manager immediately to be stored by its timer thread. Once the given
     * delay has expired, the task will be handed back to the task runner to
     * govern when the task will be posted from there.
     *
     * Once a task is posted, it may be attempted to be cancelled by deleting
     * the task object or the task runner itself. This will only cancel the task
     * if the task manager has not yet begun executing the task.
     *
     * @param TaskWPtr Weak reference to the task the be executed.
     * @param milliseconds Delay before posting the task.
     *
     * @return bool True if the task was posted for delayed execution.
     */
    bool PostTaskWithDelay(const TaskWPtr &, std::chrono::milliseconds);

protected:
    /**
     * Private constructor. Task runners may only be created by the task
     * manager.
     *
     * @param TaskManagerWPtr Weak reference to the task manager.
     */
    TaskRunner(const TaskManagerWPtr &);

    /**
     * Completion notification triggered by the task manager that a task has
     * finished execution (or was skipped).
     *
     * @param TaskPtr The (possibly NULL) task that was executed or skipped.
     */
    virtual void TaskComplete(const TaskPtr &) = 0;

    /**
     * Forward a task to the task manager to be executed as soon as a worker
     * thread is available.
     *
     * @param TaskWPtr Weak reference to the task the be executed.
     *
     * @return bool True if the task was posted for execution.
     */
    bool PostTaskToTaskManager(const TaskWPtr &);

private:
    TaskManagerWPtr m_wpTaskManager;
};

/**
 * Task runner implementation for executing a task in parallel. Tasks posted to
 * this task runner may be executed in any order.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class ParallelTaskRunner : public TaskRunner
{
    friend class TaskManager;

public:
    bool PostTask(const TaskWPtr &) override;

protected:
    ParallelTaskRunner(const TaskManagerWPtr &);

    /**
     * This implementation does nothing.
     *
     * @param TaskPtr The (possibly NULL) task that was executed or skipped.
     */
    void TaskComplete(const TaskPtr &) override;
};

/**
 * Task runner implementation for executing a task in sequence. Only one task
 * posted to this task runner will execute at a time. Tasks are executed in a
 * FIFO manner; once one task completes, the next task in line will be posted
 * for execution.
 *
 * The caveat is with delayed tasks. If task A is posted with some delay, then
 * task B is posted with no delay, task B will be posted for execution first.
 * Task A will only be posted for execution once its delay has expired.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class SequencedTaskRunner : public TaskRunner
{
    friend class TaskManager;

public:
    bool PostTask(const TaskWPtr &) override;

protected:
    SequencedTaskRunner(const TaskManagerWPtr &);

    /**
     * When a task is complete, post the next task in the pending queue.
     *
     * @param TaskPtr The (possibly NULL) task that was executed or skipped.
     */
    void TaskComplete(const TaskPtr &) override;

private:
    /**
     * If no task has been posted for execution, post the first task in the
     * pending queue.
     *
     * @return bool True if the task was posted for execution or added to the
     *              pending queue.
     */
    bool maybePostTask();

    ConcurrentQueue<TaskWPtr> m_pendingTasks;
    std::atomic_bool m_aHasRunningTask;
};

}
