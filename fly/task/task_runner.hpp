#pragma once

#include "fly/fly.hpp"
#include "fly/task/task_types.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>

/**
 * Helper macro to create a TaskLocation from the current location.
 */
// clang-format off
#define FROM_HERE fly::TaskLocation {__FILE__, __FUNCTION__, static_cast<std::uint32_t>(__LINE__)}
// clang-format on

namespace fly {

class TaskManager;

/**
 * Base class for controlling the execution of tasks.
 *
 * Once a task is posted, it may be attempted to be cancelled in a number of ways:
 *
 * 1. Wrap the body of the task in a lambda and implement that lambda to check whether its owning
 *    class has been deleted through the use of smart pointers. For example:
 *
 *        std::weak_ptr<MyClass> weak_self = shared_from_this();
 *
 *        auto task = [weak_self]()
 *        {
 *            if (auto self = weak_self.lock(); self)
 *            {
 *                // Task body here.
 *            }
 *        };
 *
 *        task_runner->post_task(std::move(task));
 *
 * 2. Deleting the task runner onto which the task was posted. This will only cancel the task if the
 *    task manager has not yet instructed the task runner to execute the task.
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
     * Post a task for execution. The task may be any callable type (lambda, std::function, etc).
     *
     * @tparam TaskType Callable type of the task.
     *
     * @param location The location from which the task was posted (use FROM_HERE).
     * @param task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    template <class TaskType>
    bool post_task(TaskLocation &&location, TaskType &&task)
    {
        return post_task_internal(std::move(location), wrap_task(std::move(task)));
    }

    /**
     * Schedule a task to be posted after a delay. The task may be any callable type (lambda,
     * std::function, etc).
     *
     * @tparam TaskType Callable type of the task.
     *
     * @param location The location from which the task was posted (use FROM_HERE).
     * @param task The task to be executed.
     * @param delay Delay before posting the task.
     *
     * @return True if the task was posted for delayed execution.
     */
    template <class TaskType>
    bool
    post_task_with_delay(TaskLocation &&location, TaskType &&task, std::chrono::milliseconds delay)
    {
        return post_task_to_task_manager_with_delay(
            std::move(location),
            wrap_task(std::move(task)),
            delay);
    }

protected:
    /**
     * Private constructor. Task runners may only be created by the task manager.
     *
     * @param weak_task_manager The task manager.
     */
    TaskRunner(std::weak_ptr<TaskManager> weak_task_manager) noexcept;

    /**
     * Post a task for execution in accordance with the concrete task runner's policy.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    virtual bool post_task_internal(TaskLocation &&location, Task &&task) = 0;

    /**
     * Completion notification triggered by the task manager that a task has finished execution.
     *
     * @param location The location from which the task was posted.
     */
    virtual void task_complete(TaskLocation &&location) = 0;

    /**
     * Forward a task to the task manager to be executed as soon as a worker thread is available.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    bool post_task_to_task_manager(TaskLocation &&location, Task &&task);

    /**
     * Forward a task to the task manager to be scheduled for excution after a delay. The task will
     * be stored on the task manager's timer thread. Once the given delay has expired, the task will
     * be handed back to the task runner to govern when the task will be posted from there.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     * @param delay Delay before posting the task.
     *
     * @return True if the task was posted for delayed execution.
     */
    bool post_task_to_task_manager_with_delay(
        TaskLocation &&location,
        Task &&task,
        std::chrono::milliseconds delay);

private:
    /**
     * Wrap a task in a generic lambda to be agnostic to the return type of the task.
     *
     * @tparam TaskType Callable type of the task.
     *
     * @param task The task to be executed.
     *
     * @return The wrapped task.
     */
    template <class TaskType>
    Task wrap_task(TaskType &&task)
    {
        static_assert(std::is_invocable_v<TaskType>, "Tasks must be invocable");

        auto wrapped_task = [task = std::move(task)]() mutable {
            // TODO support supplying the result of the task to the owner of the task runner.
            FLY_UNUSED(std::move(task)());
        };

        return wrapped_task;
    }

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

protected:
    explicit ParallelTaskRunner(std::weak_ptr<TaskManager>) noexcept;

    /**
     * Post a task for execution immediately.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    bool post_task_internal(TaskLocation &&location, Task &&task) override;

    /**
     * This implementation does nothing.
     *
     * @param location The location from which the task was posted.
     */
    void task_complete(TaskLocation &&location) override;
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

protected:
    explicit SequencedTaskRunner(std::weak_ptr<TaskManager>) noexcept;

    /**
     * Post a task for execution within this sequence. If a task is not already running, the task is
     * posted for execution immediately. Otherwise, the task is queued until the currently running
     * task (and all tasks queued before it) have completed.
     *
     * @param location The location from which the task was posted.
     * @param task The task to be executed.
     *
     * @return True if the task was posted for execution.
     */
    bool post_task_internal(TaskLocation &&location, Task &&task) override;

    /**
     * When a task is complete, post the next task in the pending queue.
     *
     * @param location The location from which the task was posted.
     */
    void task_complete(TaskLocation &&location) override;

private:
    /**
     * Structure to hold a task until it is ready to be executed within its sequence.
     */
    struct PendingTask
    {
        TaskLocation m_location;
        Task m_task;
    };

    /**
     * If no task has been posted for execution, post the first task in the pending queue.
     *
     * @return True if the task was posted for execution or added to the pending queue.
     */
    bool maybe_post_task();

    ConcurrentQueue<PendingTask> m_pending_tasks;
    std::atomic_bool m_has_running_task {false};
};

} // namespace fly
