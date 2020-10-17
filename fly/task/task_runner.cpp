#include "fly/task/task_runner.hpp"

#include "fly/task/task_manager.hpp"

namespace fly {

//==================================================================================================
TaskRunner::TaskRunner(std::weak_ptr<TaskManager> weak_task_manager) noexcept :
    m_weak_task_manager(std::move(weak_task_manager))
{
}

//==================================================================================================
bool TaskRunner::post_task_to_task_manager(TaskLocation &&location, Task &&task)
{
    std::shared_ptr<TaskManager> task_manager = m_weak_task_manager.lock();
    if (!task_manager)
    {
        return false;
    }

    std::weak_ptr<TaskRunner> task_runner = shared_from_this();
    task_manager->post_task(std::move(location), std::move(task), std::move(task_runner));

    return true;
}

//==================================================================================================
bool TaskRunner::post_task_to_task_manager_with_delay(
    TaskLocation &&location,
    Task &&task,
    std::chrono::milliseconds delay)
{
    std::shared_ptr<TaskManager> task_manager = m_weak_task_manager.lock();
    if (!task_manager)
    {
        return false;
    }

    std::weak_ptr<TaskRunner> task_runner = shared_from_this();
    task_manager
        ->post_task_with_delay(std::move(location), std::move(task), std::move(task_runner), delay);

    return true;
}

//==================================================================================================
void TaskRunner::execute(TaskLocation &&location, Task &&task)
{
    std::move(task)(this, location);
    task_complete(std::move(location));
}

//==================================================================================================
ParallelTaskRunner::ParallelTaskRunner(std::weak_ptr<TaskManager> weak_task_manager) noexcept :
    TaskRunner(std::move(weak_task_manager))
{
}

//==================================================================================================
bool ParallelTaskRunner::post_task_internal(TaskLocation &&location, Task &&task)
{
    return post_task_to_task_manager(std::move(location), std::move(task));
}

//==================================================================================================
void ParallelTaskRunner::task_complete(TaskLocation &&)
{
}

//==================================================================================================
SequencedTaskRunner::SequencedTaskRunner(std::weak_ptr<TaskManager> weak_task_manager) noexcept :
    TaskRunner(std::move(weak_task_manager))
{
}

//==================================================================================================
bool SequencedTaskRunner::post_task_internal(TaskLocation &&location, Task &&task)
{
    m_pending_tasks.push({std::move(location), std::move(task)});
    return maybe_post_task();
}

//==================================================================================================
void SequencedTaskRunner::task_complete(TaskLocation &&)
{
    m_has_running_task.store(false);
    maybe_post_task();
}

//==================================================================================================
bool SequencedTaskRunner::maybe_post_task()
{
    static std::chrono::seconds s_wait(0);
    bool expected = false;

    if (m_has_running_task.compare_exchange_strong(expected, true))
    {
        bool posted = false;
        PendingTask task;

        if (m_pending_tasks.pop(task, s_wait))
        {
            posted = post_task_to_task_manager(std::move(task.m_location), std::move(task.m_task));
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
