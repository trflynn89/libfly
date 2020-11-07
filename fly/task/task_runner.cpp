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
    return maybe_post_task(std::move(location), std::move(task));
}

//==================================================================================================
void SequencedTaskRunner::task_complete(TaskLocation &&)
{
    maybe_post_task({}, {});
}

//==================================================================================================
bool SequencedTaskRunner::maybe_post_task(TaskLocation &&location, Task &&task)
{
    bool posted_or_queued = false;

    std::lock_guard<std::mutex> lock(m_pending_tasks_mutex);

    if ((task == nullptr) || !m_has_running_task)
    {
        if (!m_pending_tasks.empty())
        {
            PendingTask pending_task = std::move(m_pending_tasks.front());
            m_pending_tasks.pop();

            posted_or_queued = post_task_to_task_manager(
                std::move(pending_task.m_location),
                std::move(pending_task.m_task));
        }
        else if (task != nullptr)
        {
            posted_or_queued = post_task_to_task_manager(std::move(location), std::move(task));
            task = nullptr;
        }

        m_has_running_task = posted_or_queued;
    }

    if (task != nullptr)
    {
        m_pending_tasks.push({std::move(location), std::move(task)});
        posted_or_queued = true;
    }

    return posted_or_queued;
}

} // namespace fly
