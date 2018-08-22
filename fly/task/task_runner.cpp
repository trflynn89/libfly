#include "fly/task/task_runner.h"

#include "fly/logger/logger.h"
#include "fly/task/task.h"
#include "fly/task/task_manager.h"

namespace fly {

//==============================================================================
TaskRunner::TaskRunner(const TaskManagerWPtr &wpTaskManager) :
    m_wpTaskManager(wpTaskManager)
{
}

//==============================================================================
void TaskRunner::PostTaskWithDelay(
    const TaskWPtr &wpTask,
    std::chrono::milliseconds delay)
{
    TaskManagerPtr spTaskManager = m_wpTaskManager.lock();
    TaskRunnerPtr spTaskRunner = shared_from_this();

    if (spTaskManager)
    {
        spTaskManager->postTaskWithDelay(wpTask, spTaskRunner, delay);
    }
}

//==============================================================================
void TaskRunner::PostTaskToTaskManager(const TaskWPtr &wpTask)
{
    TaskManagerPtr spTaskManager = m_wpTaskManager.lock();
    TaskRunnerPtr spTaskRunner = shared_from_this();

    if (spTaskManager)
    {
        spTaskManager->postTask(wpTask, spTaskRunner);
    }
}

//==============================================================================
ParallelTaskRunner::ParallelTaskRunner(const TaskManagerWPtr &wpTaskManager) :
    TaskRunner(wpTaskManager)
{
}

//==============================================================================
void ParallelTaskRunner::PostTask(const TaskWPtr &wpTask)
{
    PostTaskToTaskManager(wpTask);
}

//==============================================================================
void ParallelTaskRunner::TaskComplete(const TaskPtr &)
{
}

//==============================================================================
SequencedTaskRunner::SequencedTaskRunner(const TaskManagerWPtr &wpTaskManager) :
    TaskRunner(wpTaskManager),
    m_aHasRunningTask(false)
{
}

//==============================================================================
void SequencedTaskRunner::PostTask(const TaskWPtr &wpTask)
{
    TaskPtr spTask = wpTask.lock();

    if (spTask)
    {
        m_pendingTasks.Push(wpTask);
        maybePostTask();
    }
}

//==============================================================================
void SequencedTaskRunner::TaskComplete(const TaskPtr &)
{
    m_aHasRunningTask.store(false);
    maybePostTask();
}

//==============================================================================
void SequencedTaskRunner::maybePostTask()
{
    static std::chrono::seconds wait(0);
    bool expected = false;

    if (m_aHasRunningTask.compare_exchange_strong(expected, true))
    {
        TaskWPtr wpTask;

        if (m_pendingTasks.Pop(wpTask, wait))
        {
            PostTaskToTaskManager(wpTask);
        }
        else
        {
            m_aHasRunningTask.store(false);
        }
    }
}

}
