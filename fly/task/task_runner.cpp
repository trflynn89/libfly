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
bool TaskRunner::PostTaskWithDelay(
    const TaskWPtr &wpTask,
    std::chrono::milliseconds delay)
{
    TaskManagerPtr spTaskManager = m_wpTaskManager.lock();

    if (spTaskManager)
    {
        TaskRunnerPtr spTaskRunner = shared_from_this();
        spTaskManager->postTaskWithDelay(wpTask, spTaskRunner, delay);

        return true;
    }

    return false;
}

//==============================================================================
bool TaskRunner::PostTaskToTaskManager(const TaskWPtr &wpTask)
{
    TaskManagerPtr spTaskManager = m_wpTaskManager.lock();

    if (spTaskManager)
    {
        TaskRunnerPtr spTaskRunner = shared_from_this();
        spTaskManager->postTask(wpTask, spTaskRunner);

        return true;
    }

    return false;
}

//==============================================================================
ParallelTaskRunner::ParallelTaskRunner(const TaskManagerWPtr &wpTaskManager) :
    TaskRunner(wpTaskManager)
{
}

//==============================================================================
bool ParallelTaskRunner::PostTask(const TaskWPtr &wpTask)
{
    return PostTaskToTaskManager(wpTask);
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
bool SequencedTaskRunner::PostTask(const TaskWPtr &wpTask)
{
    m_pendingTasks.Push(wpTask);
    return maybePostTask();
}

//==============================================================================
void SequencedTaskRunner::TaskComplete(const TaskPtr &)
{
    m_aHasRunningTask.store(false);
    maybePostTask();
}

//==============================================================================
bool SequencedTaskRunner::maybePostTask()
{
    static std::chrono::seconds wait(0);
    bool expected = false;

    if (m_aHasRunningTask.compare_exchange_strong(expected, true))
    {
        bool posted = false;
        TaskWPtr wpTask;

        if (m_pendingTasks.Pop(wpTask, wait))
        {
            posted = PostTaskToTaskManager(wpTask);
        }

        if (!posted)
        {
            m_aHasRunningTask.store(false);
        }

        return posted;
    }

    return true;
}

}
