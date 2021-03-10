#include "fly/task/task_runner.hpp"

#include "fly/task/task_manager.hpp"

namespace fly {

//==================================================================================================
TaskRunner::TaskRunner(const std::shared_ptr<TaskManager> &task_manager) noexcept :
    m_weak_task_manager(task_manager)
{
}

//==================================================================================================
bool TaskRunner::post_task_to_task_manager(TaskLocation &&location, Task &&task)
{
    if (std::shared_ptr<TaskManager> task_manager = m_weak_task_manager.lock(); task_manager)
    {
        task_manager->post_task(std::move(location), std::move(task), shared_from_this());
        return true;
    }

    return false;
}

//==================================================================================================
bool TaskRunner::post_task_to_task_manager_with_delay(
    TaskLocation &&location,
    Task &&task,
    std::chrono::milliseconds delay)
{
    if (std::shared_ptr<TaskManager> task_manager = m_weak_task_manager.lock(); task_manager)
    {
        task_manager
            ->post_task_with_delay(std::move(location), std::move(task), shared_from_this(), delay);
        return true;
    }

    return false;
}

//==================================================================================================
void TaskRunner::execute(TaskLocation &&location, Task &&task)
{
    std::invoke(std::move(task), this, location);
    task_complete(std::move(location));
}

//==================================================================================================
std::shared_ptr<ParallelTaskRunner>
ParallelTaskRunner::create(const std::shared_ptr<TaskManager> &task_manager)
{
    // ParallelTaskRunner has a private constructor, thus cannot be used with std::make_shared. This
    // class is used to expose the private constructor locally.
    struct ParallelTaskRunnerImpl final : public ParallelTaskRunner
    {
        explicit ParallelTaskRunnerImpl(const std::shared_ptr<TaskManager> &task_manager) noexcept :
            ParallelTaskRunner(task_manager)
        {
        }
    };

    return std::make_shared<ParallelTaskRunnerImpl>(task_manager);
}

//==================================================================================================
ParallelTaskRunner::ParallelTaskRunner(const std::shared_ptr<TaskManager> &task_manager) noexcept :
    TaskRunner(task_manager)
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
std::shared_ptr<SequencedTaskRunner>
SequencedTaskRunner::create(const std::shared_ptr<TaskManager> &task_manager)
{
    // SequencedTaskRunner has a private constructor, thus cannot be used with std::make_shared.
    // This class is used to expose the private constructor locally.
    struct SequencedTaskRunnerImpl final : public SequencedTaskRunner
    {
        explicit SequencedTaskRunnerImpl(const std::shared_ptr<TaskManager> &task_manager) noexcept
            :
            SequencedTaskRunner(task_manager)
        {
        }
    };

    return std::make_shared<SequencedTaskRunnerImpl>(task_manager);
}

//==================================================================================================
SequencedTaskRunner::SequencedTaskRunner(const std::shared_ptr<TaskManager> &task_manager) noexcept
    :
    TaskRunner(task_manager)
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
