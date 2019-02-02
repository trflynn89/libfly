#include "fly/task/task_runner.h"

#include "fly/logger/logger.h"
#include "fly/task/task.h"
#include "fly/task/task_manager.h"

namespace fly {

//==============================================================================
TaskRunner::TaskRunner(const std::weak_ptr<TaskManager> &wpTaskManager) :
    m_wpTaskManager(wpTaskManager)
{
}

//==============================================================================
bool TaskRunner::PostTaskWithDelay(
    const std::weak_ptr<Task> &wpTask,
    std::chrono::milliseconds delay)
{
    std::shared_ptr<TaskManager> spTaskManager = m_wpTaskManager.lock();

    if (spTaskManager)
    {
        std::shared_ptr<TaskRunner> spTaskRunner = shared_from_this();
        spTaskManager->postTaskWithDelay(wpTask, spTaskRunner, delay);

        return true;
    }

    return false;
}

//==============================================================================
bool TaskRunner::PostTaskToTaskManager(const std::weak_ptr<Task> &wpTask)
{
    std::shared_ptr<TaskManager> spTaskManager = m_wpTaskManager.lock();

    if (spTaskManager)
    {
        std::shared_ptr<TaskRunner> spTaskRunner = shared_from_this();
        spTaskManager->postTask(wpTask, spTaskRunner);

        return true;
    }

    return false;
}

//==============================================================================
ParallelTaskRunner::ParallelTaskRunner(
    const std::weak_ptr<TaskManager> &wpTaskManager) :
    TaskRunner(wpTaskManager)
{
}

//==============================================================================
bool ParallelTaskRunner::PostTask(const std::weak_ptr<Task> &wpTask)
{
    return PostTaskToTaskManager(wpTask);
}

//==============================================================================
void ParallelTaskRunner::TaskComplete(const std::shared_ptr<Task> &)
{
}

//==============================================================================
SequencedTaskRunner::SequencedTaskRunner(
    const std::weak_ptr<TaskManager> &wpTaskManager) :
    TaskRunner(wpTaskManager),
    m_aHasRunningTask(false)
{
}

//==============================================================================
bool SequencedTaskRunner::PostTask(const std::weak_ptr<Task> &wpTask)
{
    m_pendingTasks.Push(wpTask);
    return maybePostTask();
}

//==============================================================================
void SequencedTaskRunner::TaskComplete(const std::shared_ptr<Task> &)
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
        std::weak_ptr<Task> wpTask;

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

} // namespace fly
