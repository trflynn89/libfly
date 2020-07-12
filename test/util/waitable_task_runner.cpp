#include "test/util/waitable_task_runner.hpp"

#include "fly/task/task.hpp"
#include "fly/task/task_manager.hpp"

namespace fly::test {

//==================================================================================================
void WaitableTaskRunner::task_complete(const std::shared_ptr<Task> &task)
{
    if (task)
    {
        const auto &task_ref = *(task.get());
        m_completed_tasks.push(typeid(task_ref).hash_code());
    }
}

//==================================================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    std::weak_ptr<TaskManager> task_manager) noexcept :
    fly::ParallelTaskRunner(task_manager)
{
}

//==================================================================================================
void WaitableParallelTaskRunner::task_complete(const std::shared_ptr<Task> &task)
{
    ParallelTaskRunner::task_complete(task);
    WaitableTaskRunner::task_complete(task);
}

//==================================================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    std::weak_ptr<TaskManager> task_manager) noexcept :
    fly::SequencedTaskRunner(task_manager)
{
}

//==================================================================================================
void WaitableSequencedTaskRunner::task_complete(const std::shared_ptr<Task> &task)
{
    SequencedTaskRunner::task_complete(task);
    WaitableTaskRunner::task_complete(task);
}

} // namespace fly::test
