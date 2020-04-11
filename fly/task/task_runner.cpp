#include "fly/task/task_runner.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task.hpp"
#include "fly/task/task_manager.hpp"

namespace fly {

//==============================================================================
TaskRunner::TaskRunner(std::weak_ptr<TaskManager> wpTaskManager) noexcept :
    m_wpTaskManager(wpTaskManager)
{
}

//==============================================================================
bool TaskRunner::PostTaskWithDelay(
    std::weak_ptr<Task> wpTask,
    std::chrono::milliseconds delay) noexcept
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
bool TaskRunner::PostTaskToTaskManager(std::weak_ptr<Task> wpTask) noexcept
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
    std::weak_ptr<TaskManager> wpTaskManager) noexcept :
    TaskRunner(wpTaskManager)
{
}

//==============================================================================
bool ParallelTaskRunner::PostTask(std::weak_ptr<Task> wpTask) noexcept
{
    return PostTaskToTaskManager(wpTask);
}

//==============================================================================
void ParallelTaskRunner::TaskComplete(const std::shared_ptr<Task> &) noexcept
{
}

//==============================================================================
SequencedTaskRunner::SequencedTaskRunner(
    std::weak_ptr<TaskManager> wpTaskManager) noexcept :
    TaskRunner(wpTaskManager),
    m_aHasRunningTask(false)
{
}

//==============================================================================
bool SequencedTaskRunner::PostTask(std::weak_ptr<Task> wpTask) noexcept
{
    m_pendingTasks.Push(std::move(wpTask));
    return maybePostTask();
}

//==============================================================================
void SequencedTaskRunner::TaskComplete(const std::shared_ptr<Task> &) noexcept
{
    m_aHasRunningTask.store(false);
    maybePostTask();
}

//==============================================================================
bool SequencedTaskRunner::maybePostTask() noexcept
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
