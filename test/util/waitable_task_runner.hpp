#pragma once

#include "fly/task/task_runner.hpp"
#include "fly/types/concurrency/concurrent_queue.hpp"

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
 * @author Timothy Flynn (trflynn89@pm.me)
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
    void wait_for_task_to_complete() noexcept;

    /**
     * Wait for a specific task type to complete execution.
     *
     * @tparam TaskType The task subclass to wait on.
     *
     * @param duration Time to wait for a completion.
     *
     * @return True if a completed task was found in the given duration.
     */
    template <typename TaskType, typename R, typename P>
    bool
    wait_for_task_to_complete(std::chrono::duration<R, P> duration) noexcept;

protected:
    /**
     * When a task is complete, track it if the task is still valid.
     *
     * @param task The (possibly NULL) task that was executed or skipped.
     */
    virtual void task_complete(const std::shared_ptr<Task> &task) noexcept = 0;

private:
    ConcurrentQueue<std::size_t> m_completed_tasks;
};

/**
 * Subclass of the parallel task runner to provide the same parllel behavior,
 * but also to allow waiting for a specific task to be complete. Only meant to
 * be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class WaitableParallelTaskRunner :
    public ParallelTaskRunner,
    public WaitableTaskRunner
{
    friend class TaskManager;

protected:
    WaitableParallelTaskRunner(
        std::weak_ptr<TaskManager> task_manager) noexcept;

    /**
     * When a task is complete, perform the same operations as this runner's
     * parents.
     *
     * @param task The (possibly NULL) task that was executed or skipped.
     */
    void task_complete(const std::shared_ptr<Task> &task) noexcept override;
};

/**
 * Subclass of the sequenced task runner to provide the same sequenced behavior,
 * but also to allow waiting for a specific task to be complete. Only meant to
 * be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@pm.me)
 * @version August 12, 2018
 */
class WaitableSequencedTaskRunner :
    public SequencedTaskRunner,
    public WaitableTaskRunner
{
    friend class TaskManager;

protected:
    WaitableSequencedTaskRunner(
        std::weak_ptr<TaskManager> task_manager) noexcept;

    /**
     * When a task is complete, perform the same operations as this runner's
     * parents.
     *
     * @param task The (possibly NULL) task that was executed or skipped.
     */
    void task_complete(const std::shared_ptr<Task> &task) noexcept override;
};

//==============================================================================
template <typename TaskType>
void WaitableTaskRunner::wait_for_task_to_complete() noexcept
{
    static_assert(
        std::is_base_of<Task, TaskType>::value,
        "Given type is not a task");

    static std::size_t s_expected_hash = typeid(TaskType).hash_code();
    std::size_t completed_hash = 0;

    while (s_expected_hash != completed_hash)
    {
        m_completed_tasks.pop(completed_hash);
    }
}

//==============================================================================
template <typename TaskType, typename R, typename P>
bool WaitableTaskRunner::wait_for_task_to_complete(
    std::chrono::duration<R, P> duration) noexcept
{
    static_assert(
        std::is_base_of<Task, TaskType>::value,
        "Given type is not a task");

    const auto deadline = std::chrono::high_resolution_clock::now() + duration;

    static std::size_t s_expected_hash = typeid(TaskType).hash_code();
    std::size_t completed_hash = 0;

    while (s_expected_hash != completed_hash)
    {
        auto before = std::chrono::high_resolution_clock::now();
        m_completed_tasks.pop(completed_hash, duration);
        auto after = std::chrono::high_resolution_clock::now();

        if (after > deadline)
        {
            break;
        }

        duration -=
            std::chrono::duration_cast<decltype(duration)>(after - before);
    }

    return (s_expected_hash == completed_hash);
}

} // namespace fly
