#pragma once

#include <memory>
#include <typeinfo>

#include "fly/fly.h"
#include "fly/task/task_runner.h"
#include "fly/types/concurrent_queue.h"

namespace fly {

FLY_CLASS_PTRS(WaitableSequencedTaskRunner);

FLY_CLASS_PTRS(Task);
FLY_CLASS_PTRS(TaskManager);

/**
 * Subclass of the sequenced task runner to provide the same sequenced behavior,
 * but also to allow waiting for a specific task to be complete. Only meant to
 * be used by unit tests.
 *
 * @author Timothy Flynn (trflynn89@gmail.com)
 * @version August 12, 2018
 */
class WaitableSequencedTaskRunner : public SequencedTaskRunner
{
    friend class TaskManager;

public:
    /**
     * Wait indefinitely for a specific task type to complete execution.
     *
     * @tparam TaskType The task subclass to wait on.
     */
    template <typename TaskType>
    void WaitForTaskTypeToComplete();

protected:
    WaitableSequencedTaskRunner(const TaskManagerWPtr &);

    /**
     * When a task is complete, perform the same operations as the parent task
     * runner, but also track the type of completed task.
     */
    void TaskComplete(const TaskPtr &) override;

private:
    ConcurrentQueue<size_t> m_completedTasks;
};

//==============================================================================
template <typename TaskType>
void WaitableSequencedTaskRunner::WaitForTaskTypeToComplete()
{
    static_assert(std::is_base_of<Task, TaskType>::value,
        "Given type is not a task");

    static size_t expected_hash = typeid(TaskType).hash_code();
    size_t completed_hash = 0;

    while (expected_hash != completed_hash)
    {
        m_completedTasks.Pop(completed_hash);
    }
}

}
