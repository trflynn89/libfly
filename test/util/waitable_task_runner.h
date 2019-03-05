#pragma once

#include "fly/task/task_runner.h"
#include "fly/types/concurrent_queue.h"

#include <chrono>
#include <memory>
#include <typeinfo>

namespace fly {

class Task;
class TaskManager;

/**
 * A pseudo task runner to allow waiting for a specific task to be complete. It
 * is not a valid test runner in itself, in that it doesn't allow actually
 * running tasks. But the implementations below extend this class for common
 * functionality. Only meant to be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class WaitableTaskRunner
{
public:
    /**
     * Destructor.
     */
    virtual ~WaitableTaskRunner() = default;

    /**
     * Wait indefinitely for a specific task type to complete execution.
     *
     * @tparam TaskType The task subclass to wait on.
     */
    template <typename TaskType>
    void WaitForTaskTypeToComplete();

    /**
     * Wait for a specific task type to complete execution.
     *
     * @tparam TaskType The task subclass to wait on.
     *
     * @param duration Time to wait for a completion.
     *
     * @return bool True if a completed task was found in the given duration.
     */
    template <typename TaskType, typename R, typename P>
    bool WaitForTaskTypeToComplete(std::chrono::duration<R, P>);

protected:
    /**
     * When a task is complete, track it if the task is still valid.
     *
     * @param Task The (possibly NULL) task that was executed or skipped.
     */
    virtual void TaskComplete(const std::shared_ptr<Task> &) = 0;

private:
    ConcurrentQueue<size_t> m_completedTasks;
};

/**
 * Subclass of the parallel task runner to provide the same parllel behavior,
 * but also to allow waiting for a specific task to be complete. Only meant to
 * be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class WaitableParallelTaskRunner :
    public ParallelTaskRunner,
    public WaitableTaskRunner
{
    friend class TaskManager;

protected:
    WaitableParallelTaskRunner(const std::weak_ptr<TaskManager> &);

    /**
     * When a task is complete, perform the same operations as this runner's
     * parents.
     *
     * @param Task The (possibly NULL) task that was executed or skipped.
     */
    void TaskComplete(const std::shared_ptr<Task> &) override;
};

/**
 * Subclass of the sequenced task runner to provide the same sequenced behavior,
 * but also to allow waiting for a specific task to be complete. Only meant to
 * be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class WaitableSequencedTaskRunner :
    public SequencedTaskRunner,
    public WaitableTaskRunner
{
    friend class TaskManager;

protected:
    WaitableSequencedTaskRunner(const std::weak_ptr<TaskManager> &);

    /**
     * When a task is complete, perform the same operations as this runner's
     * parents.
     *
     * @param Task The (possibly NULL) task that was executed or skipped.
     */
    void TaskComplete(const std::shared_ptr<Task> &) override;
};

//==============================================================================
template <typename TaskType>
void WaitableTaskRunner::WaitForTaskTypeToComplete()
{
    static_assert(
        std::is_base_of<Task, TaskType>::value, "Given type is not a task");

    static size_t expected_hash = typeid(TaskType).hash_code();
    size_t completed_hash = 0;

    while (expected_hash != completed_hash)
    {
        m_completedTasks.Pop(completed_hash);
    }
}

//==============================================================================
template <typename TaskType, typename R, typename P>
bool WaitableTaskRunner::WaitForTaskTypeToComplete(
    std::chrono::duration<R, P> duration)
{
    static_assert(
        std::is_base_of<Task, TaskType>::value, "Given type is not a task");

    auto deadline = std::chrono::high_resolution_clock::now() + duration;

    static size_t expected_hash = typeid(TaskType).hash_code();
    size_t completed_hash = 0;

    while (expected_hash != completed_hash)
    {
        auto before = std::chrono::high_resolution_clock::now();
        m_completedTasks.Pop(completed_hash, duration);
        auto after = std::chrono::high_resolution_clock::now();

        if (after > deadline)
        {
            break;
        }

        duration -=
            std::chrono::duration_cast<decltype(duration)>(after - before);
    }

    return (expected_hash == completed_hash);
}

} // namespace fly
