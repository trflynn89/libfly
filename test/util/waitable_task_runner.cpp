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
std::shared_ptr<WaitableParallelTaskRunner>
WaitableParallelTaskRunner::create(const std::shared_ptr<TaskManager> &task_manager)
{
    // WaitableParallelTaskRunner has a private constructor, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructor locally.
    struct WaitableParallelTaskRunnerImpl final : public WaitableParallelTaskRunner
    {
        explicit WaitableParallelTaskRunnerImpl(
            const std::shared_ptr<TaskManager> &task_manager) noexcept :
            WaitableParallelTaskRunner(task_manager)
        {
        }
    };

    return std::make_shared<WaitableParallelTaskRunnerImpl>(task_manager);
}

//==================================================================================================
WaitableParallelTaskRunner::WaitableParallelTaskRunner(
    const std::shared_ptr<TaskManager> &task_manager) noexcept :
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
std::shared_ptr<WaitableSequencedTaskRunner>
WaitableSequencedTaskRunner::create(const std::shared_ptr<TaskManager> &task_manager)
{
    // WaitableSequencedTaskRunner has a private constructor, thus cannot be used with
    // std::make_shared. This class is used to expose the private constructor locally.
    struct WaitableSequencedTaskRunnerImpl final : public WaitableSequencedTaskRunner
    {
        explicit WaitableSequencedTaskRunnerImpl(
            const std::shared_ptr<TaskManager> &task_manager) noexcept :
            WaitableSequencedTaskRunner(task_manager)
        {
        }
    };

    return std::make_shared<WaitableSequencedTaskRunnerImpl>(task_manager);
}

//==================================================================================================
WaitableSequencedTaskRunner::WaitableSequencedTaskRunner(
    const std::shared_ptr<TaskManager> &task_manager) noexcept :
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
