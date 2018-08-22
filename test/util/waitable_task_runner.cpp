#include "test/util/waitable_task_runner.h"

#include "fly/task/task.h"
#include "fly/task/task_manager.h"

namespace fly {

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

    if (spTask)
    {
        m_completedTasks.Push(typeid(*spTask).hash_code());
    }
}

}
