#include "test/util/waitable_task_runner.h"

#include "fly/task/task.h"
#include "fly/task/task_manager.h"

namespace fly {

//==============================================================================
void WaitableTaskRunner::TaskComplete(const TaskPtr &spTask)
{
    if (spTask)
    {
        const auto &task = *(spTask.get());
        m_completedTasks.Push(typeid(task).hash_code());
    }
}

//==============================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    const TaskManagerWPtr &wpTaskManager
) :
    ParallelTaskRunner(wpTaskManager)
{
}

//==============================================================================
void WaitableParallelTaskRunner::TaskComplete(const TaskPtr &spTask)
{
    ParallelTaskRunner::TaskComplete(spTask);
    WaitableTaskRunner::TaskComplete(spTask);
}

//==============================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    const TaskManagerWPtr &wpTaskManager
) :
    SequencedTaskRunner(wpTaskManager)
{
}

//==============================================================================
void WaitableSequencedTaskRunner::TaskComplete(const TaskPtr &spTask)
{
    SequencedTaskRunner::TaskComplete(spTask);
    WaitableTaskRunner::TaskComplete(spTask);
}

}
