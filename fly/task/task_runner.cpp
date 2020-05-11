#include "fly/task/task_runner.hpp"

#include "fly/logger/logger.hpp"
#include "fly/task/task.hpp"
#include "fly/task/task_manager.hpp"

namespace fly {

//==============================================================================
TaskRunner::TaskRunner(std::weak_ptr<TaskManager> weak_task_manager) noexcept :
    m_weak_task_manager(weak_task_manager)
{
}

//==============================================================================
bool TaskRunner::post_task_with_delay(
    std::weak_ptr<Task> weak_task,
    std::chrono::milliseconds delay) noexcept
{
    std::shared_ptr<TaskManager> task_manager = m_weak_task_manager.lock();

    if (task_manager)
    {
        std::shared_ptr<TaskRunner> task_runner = shared_from_this();
        task_manager->post_task_with_delay(weak_task, task_runner, delay);

        return true;
    }

    return false;
}

//==============================================================================
bool TaskRunner::post_task_to_task_manager(
    std::weak_ptr<Task> weak_task) noexcept
{
    std::shared_ptr<TaskManager> task_manager = m_weak_task_manager.lock();

    if (task_manager)
    {
        std::shared_ptr<TaskRunner> task_runner = shared_from_this();
        task_manager->post_task(weak_task, task_runner);

        return true;
    }

    return false;
}

//==============================================================================
ParallelTaskRunner::ParallelTaskRunner(
    std::weak_ptr<TaskManager> weak_task_manager) noexcept :
    TaskRunner(weak_task_manager)
{
}

//==============================================================================
bool ParallelTaskRunner::post_task(std::weak_ptr<Task> weak_task) noexcept
{
    return post_task_to_task_manager(weak_task);
}

//==============================================================================
void ParallelTaskRunner::task_complete(const std::shared_ptr<Task> &) noexcept
{
}

//==============================================================================
SequencedTaskRunner::SequencedTaskRunner(
    std::weak_ptr<TaskManager> weak_task_manager) noexcept :
    TaskRunner(weak_task_manager),
    m_has_running_task(false)
{
}

//==============================================================================
bool SequencedTaskRunner::post_task(std::weak_ptr<Task> weak_task) noexcept
{
    m_pending_tasks.Push(std::move(weak_task));
    return maybe_post_task();
}

//==============================================================================
void SequencedTaskRunner::task_complete(const std::shared_ptr<Task> &) noexcept
{
    m_has_running_task.store(false);
    maybe_post_task();
}

//==============================================================================
bool SequencedTaskRunner::maybe_post_task() noexcept
{
    static std::chrono::seconds s_wait(0);
    bool expected = false;

    if (m_has_running_task.compare_exchange_strong(expected, true))
    {
        bool posted = false;
        std::weak_ptr<Task> weak_task;

        if (m_pending_tasks.Pop(weak_task, s_wait))
        {
            posted = post_task_to_task_manager(weak_task);
        }

        if (!posted)
        {
            m_has_running_task.store(false);
        }

        return posted;
    }

    return true;
}

} // namespace fly
