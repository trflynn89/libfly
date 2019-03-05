#include "test/util/waitable_task_runner.h"

#include "fly/task/task.h"
#include "fly/task/task_manager.h"

namespace fly {

//==============================================================================
void WaitableTaskRunner::TaskComplete(const std::shared_ptr<Task> &spTask)
{
    if (spTask)
    {
        const auto &task = *(spTask.get());
        m_completedTasks.Push(typeid(task).hash_code());
    }
}

//==============================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    const std::weak_ptr<TaskManager> &wpTaskManager) :
    ParallelTaskRunner(wpTaskManager)
{
}

//==============================================================================
void WaitableParallelTaskRunner::TaskComplete(
    const std::shared_ptr<Task> &spTask)
{
    ParallelTaskRunner::TaskComplete(spTask);
    WaitableTaskRunner::TaskComplete(spTask);
}

//==============================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    const std::weak_ptr<TaskManager> &wpTaskManager) :
    SequencedTaskRunner(wpTaskManager)
{
}

//==============================================================================
void WaitableSequencedTaskRunner::TaskComplete(
    const std::shared_ptr<Task> &spTask)
{
    SequencedTaskRunner::TaskComplete(spTask);
    WaitableTaskRunner::TaskComplete(spTask);
}

} // namespace fly
