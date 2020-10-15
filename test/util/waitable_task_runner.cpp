#include "test/util/waitable_task_runner.hpp"

#include "fly/task/task_manager.hpp"

namespace fly::test {

//==================================================================================================
void WaitableTaskRunner::task_complete(fly::TaskLocation &&location)
{
    m_completed_tasks.push(location.m_file);
}

//==================================================================================================
void WaitableTaskRunner::wait_for_task_to_complete(const std::string &location)
{
    std::string completed_location;

    do
    {
        m_completed_tasks.pop(completed_location);
    } while (completed_location.find(location) == std::string::npos);
}

//==================================================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    std::weak_ptr<TaskManager> task_manager) noexcept :
    fly::ParallelTaskRunner(task_manager)
{
}

//==================================================================================================
void WaitableParallelTaskRunner::task_complete(fly::TaskLocation &&location)
{
    fly::TaskLocation copy = location;
    ParallelTaskRunner::task_complete(std::move(location));
    WaitableTaskRunner::task_complete(std::move(copy));
}

//==================================================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    std::weak_ptr<TaskManager> task_manager) noexcept :
    fly::SequencedTaskRunner(task_manager)
{
}

//==================================================================================================
void WaitableSequencedTaskRunner::task_complete(fly::TaskLocation &&location)
{
    fly::TaskLocation copy = location;
    SequencedTaskRunner::task_complete(std::move(location));
    WaitableTaskRunner::task_complete(std::move(copy));
}

} // namespace fly::test
