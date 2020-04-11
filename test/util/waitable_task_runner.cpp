#include "test/util/waitable_task_runner.hpp"

#include "fly/task/task.hpp"
#include "fly/task/task_manager.hpp"

namespace fly {

//==============================================================================
void WaitableTaskRunner::TaskComplete(
    const std::shared_ptr<Task> &spTask) noexcept
{
    if (spTask)
    {
        const auto &task = *(spTask.get());
        m_completedTasks.Push(typeid(task).hash_code());
    }
}

//==============================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    std::weak_ptr<TaskManager> wpTaskManager) noexcept :
    ParallelTaskRunner(wpTaskManager)
{
}

//==============================================================================
void WaitableParallelTaskRunner::TaskComplete(
    const std::shared_ptr<Task> &spTask) noexcept
{
    ParallelTaskRunner::TaskComplete(spTask);
    WaitableTaskRunner::TaskComplete(spTask);
}

//==============================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    std::weak_ptr<TaskManager> wpTaskManager) noexcept :
    SequencedTaskRunner(wpTaskManager)
{
}

//==============================================================================
void WaitableSequencedTaskRunner::TaskComplete(
    const std::shared_ptr<Task> &spTask) noexcept
{
    SequencedTaskRunner::TaskComplete(spTask);
    WaitableTaskRunner::TaskComplete(spTask);
}

} // namespace fly
